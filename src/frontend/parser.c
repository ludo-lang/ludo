#include "session.h"

#include <stdint.h>
#include <string.h>

typedef struct parser {
    parsed_unit *unit;
    ludo_ast_node *nodes;
    size_t node_count;
    size_t node_capacity;
    ludo_diagnostic *diagnostics;
    size_t diagnostic_count;
    size_t diagnostic_capacity;
    size_t pos;
    size_t end;
    size_t depth;
    bool exhausted;
    ludo_frontend_status failure;
} parser;

/* Arena vectors move, ids do not. No parser function retains a node pointer
   across a recursive call or another allocation. */
static bool grow(parser *p, void **data, size_t *capacity, size_t count, size_t element) {
    if (p->exhausted) {
        return false;
    }
    if (count < *capacity) {
        return true;
    }
    size_t next = *capacity == 0 ? 64 : *capacity * 2;
    if (next < *capacity || next > SIZE_MAX / element) {
        p->exhausted = true;
        return false;
    }
    void *buffer = ludo_arena_alloc(&p->unit->arena, next * element);
    if (buffer == NULL) {
        p->exhausted = true;
        return false;
    }
    if (count != 0) {
        memcpy(buffer, *data, count * element);
    }
    *data = buffer;
    *capacity = next;
    return true;
}

static ludo_token token_at(const parser *p, size_t pos) { return p->unit->file.tokens[pos]; }

static ludo_token_kind peek(const parser *p) { return token_at(p, p->pos).kind; }

static size_t significant(const parser *p, size_t pos) {
    while (ludo_token_kind_is_trivia(token_at(p, pos).kind)) {
        pos++;
    }
    return pos;
}

static ludo_token_kind ahead(const parser *p) {
    if (peek(p) == LUDO_TOK_EOF) {
        return LUDO_TOK_EOF;
    }
    return token_at(p, significant(p, p->pos + 1)).kind;
}

static size_t take(parser *p) {
    size_t pos = p->pos;
    ludo_token token = token_at(p, pos);
    if (token.kind != LUDO_TOK_EOF) {
        p->end = token.offset + token.length;
        p->pos = significant(p, pos + 1);
    }
    return pos;
}

static bool eat(parser *p, ludo_token_kind kind) {
    if (peek(p) != kind) {
        return false;
    }
    (void)take(p);
    return true;
}

static ludo_span span_at(const parser *p, size_t pos) {
    ludo_token token = token_at(p, pos);
    return (ludo_span){p->unit->file.id, token.offset, token.length};
}

static ludo_node_id node(parser *p, ludo_ast_kind kind, size_t token) {
    void *data = p->nodes;
    if (!grow(p, &data, &p->node_capacity, p->node_count, sizeof *p->nodes)) {
        return 0;
    }
    p->nodes = data;
    ludo_node_id id = p->node_count++;
    p->nodes[id] = (ludo_ast_node){.kind = kind, .span = span_at(p, token), .token = token};
    return id;
}

static void add(parser *p, ludo_node_id parent, ludo_node_id child) {
    if (parent == 0 || child == 0) {
        return;
    }
    ludo_node_id last = p->nodes[parent].last_child;
    if (last == 0) {
        p->nodes[parent].first_child = child;
    } else {
        p->nodes[last].next_sibling = child;
    }
    while (p->nodes[child].next_sibling != 0) {
        child = p->nodes[child].next_sibling;
    }
    p->nodes[parent].last_child = child;
}

static ludo_node_id finish(parser *p, ludo_node_id id, size_t start) {
    if (id != 0) {
        size_t offset = token_at(p, start).offset;
        p->nodes[id].span.offset = offset;
        p->nodes[id].span.length = p->end >= offset ? p->end - offset : 0;
    }
    return id;
}

static void diagnostic(parser *p, size_t pos, const char *code, const char *message) {
    void *data = p->diagnostics;
    if (!grow(p, &data, &p->diagnostic_capacity, p->diagnostic_count, sizeof *p->diagnostics)) {
        return;
    }
    p->diagnostics = data;
    p->diagnostics[p->diagnostic_count++] = (ludo_diagnostic){code, message, span_at(p, pos)};
}

static ludo_node_id error(parser *p, const char *message) {
    diagnostic(p, p->pos, "bootstrap:parse-syntax", message);
    return node(p, LUDO_AST_ERROR, p->pos);
}

/* An insertion leaves the lookahead untouched. Owning loops either consume a
   separator/closing token or enforce progress. The error remains in the tree. */
static void expect(parser *p, ludo_token_kind kind, ludo_node_id owner) {
    if (!eat(p, kind)) {
        add(p, owner, error(p, ludo_token_kind_name(kind)));
    }
}

static bool literal(ludo_token_kind kind) {
    return kind == LUDO_TOK_INT || kind == LUDO_TOK_FLOAT || kind == LUDO_TOK_STRING ||
           kind == LUDO_TOK_KW_TRUE || kind == LUDO_TOK_KW_FALSE;
}

static bool expr_start(ludo_token_kind kind) {
    return literal(kind) || kind == LUDO_TOK_IDENT || kind == LUDO_TOK_DOLLAR ||
           kind == LUDO_TOK_UNDERSCORE || kind == LUDO_TOK_LPAREN || kind == LUDO_TOK_LBRACE ||
           kind == LUDO_TOK_KW_NOT || kind == LUDO_TOK_MINUS || kind == LUDO_TOK_TILDE ||
           kind == LUDO_TOK_AMP;
}

static bool type_start(ludo_token_kind kind) {
    return kind == LUDO_TOK_IDENT || kind == LUDO_TOK_DOLLAR || kind == LUDO_TOK_BANG ||
           kind == LUDO_TOK_QUESTION || kind == LUDO_TOK_CARET || kind == LUDO_TOK_LBRACKET ||
           kind == LUDO_TOK_KW_FN;
}

static bool assignment_operator(ludo_token_kind kind) {
    return kind == LUDO_TOK_EQ || kind == LUDO_TOK_PLUS_EQ || kind == LUDO_TOK_MINUS_EQ ||
           kind == LUDO_TOK_STAR_EQ || kind == LUDO_TOK_SLASH_EQ || kind == LUDO_TOK_PERCENT_EQ;
}

static bool block_end(ludo_token_kind kind) {
    return kind == LUDO_TOK_EOF || kind == LUDO_TOK_KW_END || kind == LUDO_TOK_KW_ELSE ||
           kind == LUDO_TOK_KW_ELSEIF;
}

static bool declaration_start(ludo_token_kind kind) {
    return kind == LUDO_TOK_KW_PUB || kind == LUDO_TOK_HASH || kind == LUDO_TOK_KW_TYPE ||
           kind == LUDO_TOK_KW_CONST || kind == LUDO_TOK_KW_FN || kind == LUDO_TOK_KW_EXTERN ||
           kind == LUDO_TOK_KW_PERSIST || kind == LUDO_TOK_KW_FRAME;
}

/* Bound the native parsing stack as a host resource. Exhaustion publishes no
   tree, just like a failed arena allocation. See the public status contract. */
enum { MAX_PARSE_DEPTH = 256 };

static bool enter(parser *p) {
    if (p->exhausted) {
        return false;
    }
    if (p->depth == MAX_PARSE_DEPTH) {
        p->exhausted = true;
        p->failure = LUDO_FRONTEND_RESOURCE_LIMIT;
        return false;
    }
    p->depth++;
    return true;
}

static int precedence(ludo_token_kind kind);
static ludo_node_id expression(parser *p);
static ludo_node_id binary(parser *p, int minimum);
/* What may follow a type, which is what decides where `T rescue` with no set
   named ends (ch1 §9.7.1, #152). A body follows only a function declaration's
   result; an interface member or extern signature ends at a keyword. */
typedef enum type_follow { FOLLOW_TYPE, FOLLOW_BODY } type_follow;

static ludo_node_id type_expr(parser *p, type_follow follow);
static ludo_node_id expression_argument(parser *p);
static ludo_node_id named_type(parser *p);
static ludo_node_id block(parser *p, bool tail);
static ludo_node_id statement(parser *p, bool tail);
static ludo_node_id pattern(parser *p);

static void progress(parser *p, size_t before, ludo_node_id owner) {
    if (!p->exhausted && p->pos == before && peek(p) != LUDO_TOK_EOF) {
        add(p, owner, error(p, "unexpected token"));
        (void)take(p);
    }
}

static ludo_node_id leaf(parser *p, ludo_ast_kind kind) {
    size_t pos = take(p);
    return node(p, kind, pos);
}

static ludo_node_id identifier(parser *p) {
    if (peek(p) == LUDO_TOK_IDENT) {
        return leaf(p, LUDO_AST_IDENT);
    }
    return error(p, "identifier expected");
}

static ludo_node_id constraints(parser *p) {
    size_t start = p->pos;
    ludo_node_id id = node(p, LUDO_AST_CONSTRAINTS, start);
    do {
        add(p, id, named_type(p));
    } while (!p->exhausted && eat(p, LUDO_TOK_PLUS));
    return finish(p, id, start);
}

static ludo_node_id generic_params(parser *p) {
    size_t start = take(p);
    ludo_node_id id = node(p, LUDO_AST_GENERIC_PARAMS, start);
    do {
        size_t before = p->pos;
        size_t name = p->pos;
        ludo_node_id param = node(p, LUDO_AST_GENERIC_PARAM, name);
        expect(p, LUDO_TOK_IDENT, param);
        if (eat(p, LUDO_TOK_COLON)) {
            add(p, param, constraints(p));
        }
        add(p, id, finish(p, param, name));
        progress(p, before, id);
    } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
    expect(p, LUDO_TOK_RBRACKET, id);
    return finish(p, id, start);
}

static ludo_node_id generic_args(parser *p) {
    size_t start = take(p);
    ludo_node_id id = node(p, LUDO_AST_GENERIC_ARGS, start);
    do {
        size_t before = p->pos;
        /* Bare names overlap. Prefer expression shape, leaving their role to
           checking; prefix-only type forms cannot be expression operands. */
        if (type_start(peek(p)) && !expr_start(peek(p))) {
            add(p, id, type_expr(p, FOLLOW_TYPE));
        } else {
            add(p, id, expression_argument(p));
        }
        progress(p, before, id);
    } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
    expect(p, LUDO_TOK_RBRACKET, id);
    return finish(p, id, start);
}

static ludo_node_id named_type(parser *p) {
    size_t start = p->pos;
    ludo_node_id id = node(p, LUDO_AST_NAMED_TYPE, start);
    if (eat(p, LUDO_TOK_DOLLAR)) {
        add(p, id, node(p, LUDO_AST_ROOT, start));
        expect(p, LUDO_TOK_DOT, id);
    }
    add(p, id, identifier(p));
    while (!p->exhausted && eat(p, LUDO_TOK_DOT)) {
        add(p, id, identifier(p));
    }
    if (peek(p) == LUDO_TOK_LBRACKET) {
        add(p, id, generic_args(p));
    }
    return finish(p, id, start);
}

static ludo_node_id prefix_type(parser *p, type_follow follow) {
    if (!enter(p)) {
        return 0;
    }
    size_t start = p->pos;
    ludo_node_id id;
    if (peek(p) == LUDO_TOK_QUESTION || peek(p) == LUDO_TOK_CARET) {
        ludo_ast_kind kind =
            peek(p) == LUDO_TOK_QUESTION ? LUDO_AST_OPTIONAL_TYPE : LUDO_AST_POINTER_TYPE;
        (void)take(p);
        id = node(p, kind, start);
        add(p, id, prefix_type(p, follow));
    } else if (eat(p, LUDO_TOK_LBRACKET)) {
        bool view = peek(p) == LUDO_TOK_RBRACKET;
        id = node(p, view ? LUDO_AST_VIEW_TYPE : LUDO_AST_ARRAY_TYPE, start);
        if (!view) {
            add(p, id, expression(p));
        }
        expect(p, LUDO_TOK_RBRACKET, id);
        add(p, id, prefix_type(p, follow));
    } else if (eat(p, LUDO_TOK_KW_FN)) {
        id = node(p, LUDO_AST_FN_TYPE, start);
        expect(p, LUDO_TOK_LPAREN, id);
        ludo_node_id params = node(p, LUDO_AST_PARAMS, p->pos);
        if (peek(p) != LUDO_TOK_RPAREN) {
            do {
                add(p, params, type_expr(p, FOLLOW_TYPE));
            } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
        }
        if (params != 0) {
            (void)finish(p, params, p->nodes[params].token);
        }
        add(p, id, params);
        expect(p, LUDO_TOK_RPAREN, id);
        if (eat(p, LUDO_TOK_ARROW)) {
            add(p, id, type_expr(p, follow));
        }
    } else if (peek(p) == LUDO_TOK_IDENT || peek(p) == LUDO_TOK_DOLLAR) {
        id = named_type(p);
    } else {
        id = error(p, "type expected");
    }
    p->depth--;
    return finish(p, id, start);
}

/* ch1 §9.7.1: the name after `rescue` is the error set unless the token after
   the complete name says the name began something else. Before `:` it is the
   next struct field, in any position. Before a body it is the body's first
   statement or its tail when that token continues a statement or ends the
   body. Everywhere else the name is the set: `x: T rescue E = g()`. */
static bool error_set_ahead(const parser *p, type_follow follow) {
    size_t pos = p->pos;
    if (token_at(p, pos).kind == LUDO_TOK_DOLLAR) {
        pos = significant(p, pos + 1);
        if (token_at(p, pos).kind != LUDO_TOK_DOT) {
            return true;
        }
        pos = significant(p, pos + 1);
    }
    if (token_at(p, pos).kind != LUDO_TOK_IDENT) {
        return true;
    }
    pos = significant(p, pos + 1);
    while (token_at(p, pos).kind == LUDO_TOK_DOT) {
        pos = significant(p, pos + 1);
        if (token_at(p, pos).kind != LUDO_TOK_IDENT) {
            return true;
        }
        pos = significant(p, pos + 1);
    }
    if (token_at(p, pos).kind == LUDO_TOK_LBRACKET) {
        size_t depth = 1;
        pos++;
        while (depth != 0 && token_at(p, pos).kind != LUDO_TOK_EOF) {
            if (token_at(p, pos).kind == LUDO_TOK_LBRACKET) {
                depth++;
            } else if (token_at(p, pos).kind == LUDO_TOK_RBRACKET) {
                depth--;
            }
            pos++;
        }
        pos = significant(p, pos);
    }
    ludo_token_kind next = token_at(p, pos).kind;
    if (next == LUDO_TOK_COLON) {
        return false;
    }
    if (follow == FOLLOW_TYPE) {
        return true;
    }
    return next != LUDO_TOK_COLON_EQ && !assignment_operator(next) && next != LUDO_TOK_LPAREN &&
           next != LUDO_TOK_BANG && next != LUDO_TOK_DOT && next != LUDO_TOK_QUESTION_DOT &&
           next != LUDO_TOK_CARET && next != LUDO_TOK_KW_END && precedence(next) == 0;
}

static ludo_node_id type_expr(parser *p, type_follow follow) {
    size_t start = p->pos;
    bool mutable = eat(p, LUDO_TOK_BANG);
    ludo_node_id id = prefix_type(p, follow);
    if (eat(p, LUDO_TOK_KW_RESCUE)) {
        ludo_node_id fallible = node(p, LUDO_AST_FALLIBLE_TYPE, start);
        add(p, fallible, id);
        if ((peek(p) == LUDO_TOK_IDENT || peek(p) == LUDO_TOK_DOLLAR) &&
            error_set_ahead(p, follow)) {
            add(p, fallible, named_type(p));
        }
        id = finish(p, fallible, start);
    }
    if (mutable) {
        ludo_node_id marked = node(p, LUDO_AST_MUTABLE_TYPE, start);
        add(p, marked, id);
        id = finish(p, marked, start);
    }
    return id;
}

static ludo_node_id parameters(parser *p) {
    size_t start = p->pos;
    ludo_node_id id = node(p, LUDO_AST_PARAMS, start);
    expect(p, LUDO_TOK_LPAREN, id);
    if (peek(p) != LUDO_TOK_RPAREN) {
        do {
            size_t before = p->pos;
            ludo_node_id param = node(p, LUDO_AST_PARAM, before);
            expect(p, LUDO_TOK_IDENT, param);
            expect(p, LUDO_TOK_COLON, param);
            add(p, param, type_expr(p, FOLLOW_TYPE));
            if (eat(p, LUDO_TOK_EQ)) {
                add(p, param, expression(p));
            }
            add(p, id, finish(p, param, before));
            progress(p, before, id);
        } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
    }
    expect(p, LUDO_TOK_RPAREN, id);
    return finish(p, id, start);
}

static void signature(parser *p, ludo_node_id id, bool generic, type_follow follow) {
    if (generic && peek(p) == LUDO_TOK_LBRACKET) {
        add(p, id, generic_params(p));
    }
    add(p, id, parameters(p));
    if (peek(p) == LUDO_TOK_ARROW) {
        size_t start = take(p);
        ludo_node_id returns = node(p, LUDO_AST_RETURNS, start);
        do {
            add(p, returns, type_expr(p, follow));
        } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
        add(p, id, finish(p, returns, start));
    }
}

static ludo_node_id type_body(parser *p) {
    size_t start = p->pos;
    ludo_node_id id;
    if (eat(p, LUDO_TOK_KW_STRUCT)) {
        id = node(p, LUDO_AST_STRUCT, start);
        while (!p->exhausted && !block_end(peek(p))) {
            size_t before = p->pos;
            ludo_node_id field = node(p, LUDO_AST_FIELD, before);
            expect(p, LUDO_TOK_IDENT, field);
            expect(p, LUDO_TOK_COLON, field);
            add(p, field, type_expr(p, FOLLOW_TYPE));
            if (eat(p, LUDO_TOK_EQ)) {
                add(p, field, expression(p));
            }
            add(p, id, finish(p, field, before));
            progress(p, before, id);
        }
        expect(p, LUDO_TOK_KW_END, id);
    } else if (eat(p, LUDO_TOK_KW_ENUM)) {
        id = node(p, LUDO_AST_ENUM, start);
        while (!p->exhausted && !block_end(peek(p))) {
            size_t before = p->pos;
            ludo_node_id variant = node(p, LUDO_AST_VARIANT, before);
            expect(p, LUDO_TOK_IDENT, variant);
            if (eat(p, LUDO_TOK_LPAREN)) {
                do {
                    add(p, variant, type_expr(p, FOLLOW_TYPE));
                } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
                expect(p, LUDO_TOK_RPAREN, variant);
            }
            add(p, id, finish(p, variant, before));
            progress(p, before, id);
        }
        expect(p, LUDO_TOK_KW_END, id);
    } else if (eat(p, LUDO_TOK_KW_DISTINCT)) {
        id = node(p, LUDO_AST_DISTINCT, start);
        if (eat(p, LUDO_TOK_KW_NUMERIC) && id != 0) {
            p->nodes[id].flags |= LUDO_AST_NUMERIC;
        }
        add(p, id, type_expr(p, FOLLOW_TYPE));
    } else if (eat(p, LUDO_TOK_KW_INTERFACE)) {
        id = node(p, LUDO_AST_INTERFACE, start);
        while (!p->exhausted && !block_end(peek(p))) {
            size_t before = p->pos;
            expect(p, LUDO_TOK_KW_FN, id);
            size_t name = p->pos;
            ludo_node_id member = node(p, LUDO_AST_FN, name);
            expect(p, LUDO_TOK_IDENT, member);
            signature(p, member, true, FOLLOW_TYPE);
            add(p, id, finish(p, member, before));
            progress(p, before, id);
        }
        expect(p, LUDO_TOK_KW_END, id);
    } else {
        return type_expr(p, FOLLOW_TYPE);
    }
    return finish(p, id, start);
}

static ludo_node_id aggregate(parser *p, ludo_node_id prefix, size_t start) {
    size_t brace = take(p);
    ludo_node_id id = node(p, LUDO_AST_AGGREGATE, brace);
    if (prefix != 0) {
        if (id != 0) {
            p->nodes[id].flags |= LUDO_AST_PREFIXED;
        }
        add(p, id, prefix);
    }
    while (!p->exhausted && peek(p) != LUDO_TOK_RBRACE && peek(p) != LUDO_TOK_EOF) {
        size_t before = p->pos;
        if (peek(p) == LUDO_TOK_IDENT && ahead(p) == LUDO_TOK_EQ) {
            ludo_node_id item = node(p, LUDO_AST_NAMED_ITEM, take(p));
            (void)take(p);
            add(p, item, expression(p));
            add(p, id, finish(p, item, before));
        } else if (eat(p, LUDO_TOK_LBRACKET)) {
            ludo_node_id item = node(p, LUDO_AST_KEYED_ITEM, before);
            add(p, item, expression(p));
            expect(p, LUDO_TOK_RBRACKET, item);
            expect(p, LUDO_TOK_EQ, item);
            add(p, item, expression(p));
            add(p, id, finish(p, item, before));
        } else {
            add(p, id, expression(p));
        }
        progress(p, before, id);
        if (!eat(p, LUDO_TOK_COMMA)) {
            break;
        }
    }
    expect(p, LUDO_TOK_RBRACE, id);
    return finish(p, id, start);
}

static ludo_node_id postfix(parser *p) {
    size_t start = p->pos;
    ludo_token_kind kind = peek(p);
    ludo_node_id id;
    bool can_name = false;
    bool root_only = false;
    if (literal(kind)) {
        id = leaf(p, LUDO_AST_LITERAL);
    } else if (kind == LUDO_TOK_IDENT || kind == LUDO_TOK_DOLLAR) {
        id = leaf(p, kind == LUDO_TOK_IDENT ? LUDO_AST_IDENT : LUDO_AST_ROOT);
        can_name = kind == LUDO_TOK_IDENT;
        root_only = kind == LUDO_TOK_DOLLAR;
    } else if (kind == LUDO_TOK_UNDERSCORE) {
        id = leaf(p, LUDO_AST_DISCARD);
    } else if (eat(p, LUDO_TOK_LPAREN)) {
        id = node(p, LUDO_AST_GROUP, start);
        add(p, id, expression(p));
        expect(p, LUDO_TOK_RPAREN, id);
        (void)finish(p, id, start);
    } else if (kind == LUDO_TOK_LBRACE) {
        id = aggregate(p, 0, start);
    } else {
        id = error(p, "expression expected");
        if (kind == LUDO_TOK_ERROR) {
            (void)take(p);
        }
        return id;
    }
    while (!p->exhausted) {
        kind = peek(p);
        if (kind == LUDO_TOK_DOT || kind == LUDO_TOK_QUESTION_DOT) {
            (void)take(p);
            size_t member = p->pos;
            ludo_node_id access =
                node(p, kind == LUDO_TOK_DOT ? LUDO_AST_MEMBER : LUDO_AST_OPTIONAL_MEMBER, member);
            add(p, access, id);
            expect(p, LUDO_TOK_IDENT, access);
            id = finish(p, access, start);
            can_name = (can_name || root_only) && kind == LUDO_TOK_DOT;
            root_only = false;
        } else if (eat(p, LUDO_TOK_LPAREN)) {
            ludo_node_id call = node(p, LUDO_AST_CALL, p->pos);
            add(p, call, id);
            if (peek(p) != LUDO_TOK_RPAREN) {
                do {
                    add(p, call, expression(p));
                } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
            }
            expect(p, LUDO_TOK_RPAREN, call);
            id = finish(p, call, start);
            can_name = false;
        } else if (kind == LUDO_TOK_LBRACKET) {
            size_t op = p->pos;
            ludo_node_id args = generic_args(p);
            ludo_node_id index = node(p, LUDO_AST_BRACKET, op);
            add(p, index, id);
            add(p, index, args);
            id = finish(p, index, start);
        } else if (kind == LUDO_TOK_BANG || kind == LUDO_TOK_CARET) {
            size_t op = take(p);
            ludo_node_id suffix =
                node(p, kind == LUDO_TOK_BANG ? LUDO_AST_MARK : LUDO_AST_DEREF, op);
            add(p, suffix, id);
            id = finish(p, suffix, start);
            can_name = false;
        } else if (kind == LUDO_TOK_LBRACE && can_name) {
            id = aggregate(p, id, start);
            can_name = false;
        } else {
            break;
        }
    }
    return id;
}

static ludo_node_id unary(parser *p) {
    if (!enter(p)) {
        return 0;
    }
    ludo_token_kind kind = peek(p);
    ludo_node_id id;
    if (kind == LUDO_TOK_KW_NOT || kind == LUDO_TOK_MINUS || kind == LUDO_TOK_TILDE ||
        kind == LUDO_TOK_AMP) {
        size_t start = take(p);
        id = node(p, LUDO_AST_UNARY, start);
        add(p, id, unary(p));
        (void)finish(p, id, start);
    } else {
        id = postfix(p);
    }
    p->depth--;
    return id;
}

/* Core grammar §1.6, loosest to tightest. Keeping the table in grammar order
   makes the two less usual choices visible: ranges bind above shifts and
   below addition, and cast binds more tightly than multiplication. */
typedef struct operator_precedence {
    ludo_token_kind kind;
    int precedence;
} operator_precedence;

static int precedence(ludo_token_kind kind) {
    static const operator_precedence operators[] = {
        {LUDO_TOK_KW_OR, 1},  {LUDO_TOK_KW_AND, 2}, {LUDO_TOK_EQ_EQ, 3},     {LUDO_TOK_BANG_EQ, 3},
        {LUDO_TOK_LT, 3},     {LUDO_TOK_LT_EQ, 3},  {LUDO_TOK_GT, 3},        {LUDO_TOK_GT_EQ, 3},
        {LUDO_TOK_PIPE, 4},   {LUDO_TOK_TILDE, 5},  {LUDO_TOK_AMP, 6},       {LUDO_TOK_SHL, 7},
        {LUDO_TOK_SHR, 7},    {LUDO_TOK_RANGE, 8},  {LUDO_TOK_REV_RANGE, 8}, {LUDO_TOK_PLUS, 9},
        {LUDO_TOK_MINUS, 9},  {LUDO_TOK_STAR, 10},  {LUDO_TOK_SLASH, 10},    {LUDO_TOK_PERCENT, 10},
        {LUDO_TOK_KW_AS, 11},
    };
    for (size_t i = 0; i < sizeof operators / sizeof operators[0]; i++) {
        if (operators[i].kind == kind) {
            return operators[i].precedence;
        }
    }
    return 0;
}

static ludo_node_id binary(parser *p, int minimum) {
    if (!enter(p)) {
        return 0;
    }
    size_t start = p->pos;
    ludo_node_id id = unary(p);
    bool range = false;
    while (!p->exhausted && precedence(peek(p)) >= minimum) {
        ludo_token_kind kind = peek(p);
        int power = precedence(kind);
        bool range_operator = kind == LUDO_TOK_RANGE || kind == LUDO_TOK_REV_RANGE;
        if (range_operator && range) {
            break; /* RangeExpr permits exactly one range at this level. */
        }
        size_t op = take(p);
        ludo_node_id parent = node(p, kind == LUDO_TOK_KW_AS ? LUDO_AST_CAST : LUDO_AST_BINARY, op);
        add(p, parent, id);
        add(p, parent, kind == LUDO_TOK_KW_AS ? prefix_type(p, FOLLOW_TYPE) : binary(p, power + 1));
        id = finish(p, parent, start);
        range = range_operator;
    }
    p->depth--;
    return id;
}

static bool named_path(const parser *p, ludo_node_id id) {
    bool member = false;
    while (id != 0 && p->nodes[id].kind == LUDO_AST_MEMBER) {
        member = true;
        id = p->nodes[id].first_child;
    }
    return id != 0 &&
           (p->nodes[id].kind == LUDO_AST_IDENT || (member && p->nodes[id].kind == LUDO_AST_ROOT));
}

static bool type_name_expression(const parser *p, ludo_node_id id) {
    if (id == 0) {
        return false;
    }
    if (p->nodes[id].kind == LUDO_AST_BRACKET) {
        return named_path(p, p->nodes[id].first_child);
    }
    return p->nodes[id].kind != LUDO_AST_ROOT && named_path(p, id);
}

static ludo_node_id expression_impl(parser *p, bool type_argument) {
    if (!enter(p)) {
        return 0;
    }
    size_t start = p->pos;
    ludo_node_id id = binary(p, 1);
    if (!p->exhausted && peek(p) == LUDO_TOK_KW_RESCUE) {
        size_t op = take(p);
        if (type_argument && type_name_expression(p, id) &&
            (peek(p) == LUDO_TOK_COMMA || peek(p) == LUDO_TOK_RBRACKET)) {
            ludo_node_id fallible = node(p, LUDO_AST_FALLIBLE_TYPE, op);
            add(p, fallible, id);
            id = finish(p, fallible, start);
            goto done;
        }
        ludo_node_id rescue = node(p, LUDO_AST_RESCUE, op);
        add(p, rescue, id);
        if (peek(p) == LUDO_TOK_KW_RETURN || peek(p) == LUDO_TOK_KW_BREAK ||
            peek(p) == LUDO_TOK_KW_CONTINUE) {
            ludo_token_kind kind = peek(p);
            add(p, rescue,
                leaf(p, kind == LUDO_TOK_KW_RETURN  ? LUDO_AST_RETURN
                        : kind == LUDO_TOK_KW_BREAK ? LUDO_AST_BREAK
                                                    : LUDO_AST_CONTINUE));
        } else if (peek(p) == LUDO_TOK_IDENT && ahead(p) == LUDO_TOK_KW_THEN) {
            size_t name = take(p);
            (void)take(p);
            ludo_node_id handler = node(p, LUDO_AST_HANDLER, name);
            add(p, handler, block(p, false));
            expect(p, LUDO_TOK_KW_END, handler);
            add(p, rescue, finish(p, handler, name));
        } else {
            add(p, rescue, expression(p));
        }
        id = finish(p, rescue, start);
    }
done:
    p->depth--;
    return id;
}

static ludo_node_id expression(parser *p) { return expression_impl(p, false); }

static ludo_node_id expression_argument(parser *p) { return expression_impl(p, true); }

static ludo_node_id pattern(parser *p) {
    if (!enter(p)) {
        return 0;
    }
    size_t start = p->pos;
    ludo_node_id id;
    if (literal(peek(p))) {
        id = leaf(p, LUDO_AST_PATTERN_LITERAL);
    } else if (eat(p, LUDO_TOK_UNDERSCORE)) {
        id = node(p, LUDO_AST_PATTERN_DISCARD, start);
    } else if (eat(p, LUDO_TOK_LBRACE)) {
        id = node(p, LUDO_AST_PATTERN_STRUCT, start);
        do {
            size_t before = p->pos;
            ludo_node_id field = node(p, LUDO_AST_PATTERN_FIELD, before);
            expect(p, LUDO_TOK_IDENT, field);
            expect(p, LUDO_TOK_EQ, field);
            add(p, field, pattern(p));
            add(p, id, finish(p, field, before));
            progress(p, before, id);
            if (!eat(p, LUDO_TOK_COMMA) || peek(p) == LUDO_TOK_RBRACE) {
                break;
            }
        } while (!p->exhausted);
        expect(p, LUDO_TOK_RBRACE, id);
    } else if (eat(p, LUDO_TOK_IDENT)) {
        if (eat(p, LUDO_TOK_LPAREN)) {
            id = node(p, LUDO_AST_PATTERN_VARIANT, start);
            do {
                add(p, id, pattern(p));
            } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
            expect(p, LUDO_TOK_RPAREN, id);
        } else {
            id = node(p, LUDO_AST_PATTERN_IDENT, start);
        }
    } else {
        id = error(p, "pattern expected");
    }
    p->depth--;
    return finish(p, id, start);
}

static ludo_node_id condition(parser *p) {
    if (peek(p) == LUDO_TOK_IDENT && ahead(p) == LUDO_TOK_COLON_EQ) {
        size_t start = take(p);
        (void)take(p);
        ludo_node_id id = node(p, LUDO_AST_BIND_CONDITION, start);
        add(p, id, expression(p));
        return finish(p, id, start);
    }
    return expression(p);
}

static ludo_node_id bindings(parser *p) {
    size_t start = p->pos;
    ludo_node_id id = node(p, LUDO_AST_BINDINGS, start);
    do {
        if (peek(p) == LUDO_TOK_UNDERSCORE) {
            add(p, id, leaf(p, LUDO_AST_DISCARD));
        } else {
            add(p, id, identifier(p));
        }
    } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
    return finish(p, id, start);
}

static bool let_ahead(const parser *p) {
    size_t pos = p->pos;
    if (token_at(p, pos).kind != LUDO_TOK_IDENT && token_at(p, pos).kind != LUDO_TOK_UNDERSCORE) {
        return false;
    }
    pos = significant(p, pos + 1);
    if (token_at(p, pos).kind == LUDO_TOK_COLON && peek(p) == LUDO_TOK_IDENT) {
        return true;
    }
    while (token_at(p, pos).kind == LUDO_TOK_COMMA) {
        pos = significant(p, pos + 1);
        if (token_at(p, pos).kind != LUDO_TOK_IDENT &&
            token_at(p, pos).kind != LUDO_TOK_UNDERSCORE) {
            return false;
        }
        pos = significant(p, pos + 1);
    }
    return token_at(p, pos).kind == LUDO_TOK_COLON_EQ;
}

/* CallStatement's expression may carry a rescue tail, but an arbitrary
   arithmetic expression is only legal as a function tail. */
static bool call_expression(const parser *p, ludo_node_id id) {
    while (id != 0 && p->nodes[id].kind == LUDO_AST_RESCUE) {
        id = p->nodes[id].first_child;
    }
    return id != 0 && p->nodes[id].kind == LUDO_AST_CALL;
}

static ludo_node_id simple_statement(parser *p, bool tail) {
    if (!enter(p)) {
        return 0;
    }
    size_t start = p->pos;
    ludo_node_id id;
    if (eat(p, LUDO_TOK_KW_RETURN)) {
        id = node(p, LUDO_AST_RETURN, start);
        if (expr_start(peek(p))) {
            do {
                add(p, id, expression(p));
            } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
        }
    } else if (eat(p, LUDO_TOK_KW_BREAK)) {
        id = node(p, LUDO_AST_BREAK, start);
    } else if (eat(p, LUDO_TOK_KW_CONTINUE)) {
        id = node(p, LUDO_AST_CONTINUE, start);
    } else if (eat(p, LUDO_TOK_KW_DEFER)) {
        id = node(p, LUDO_AST_DEFER, start);
        add(p, id, simple_statement(p, false));
    } else if (let_ahead(p)) {
        id = node(p, LUDO_AST_LET, start);
        add(p, id, bindings(p));
        if (eat(p, LUDO_TOK_COLON)) {
            add(p, id, type_expr(p, FOLLOW_TYPE));
            expect(p, LUDO_TOK_EQ, id);
        } else {
            expect(p, LUDO_TOK_COLON_EQ, id);
        }
        add(p, id, expression(p));
    } else {
        ludo_node_id value = expression(p);
        ludo_token_kind kind = peek(p);
        if (assignment_operator(kind)) {
            size_t op = take(p);
            id = node(p, kind == LUDO_TOK_EQ ? LUDO_AST_ASSIGN : LUDO_AST_COMPOUND_ASSIGN, op);
            add(p, id, value);
            if (value != 0 &&
                (p->nodes[value].kind == LUDO_AST_BINARY ||
                 p->nodes[value].kind == LUDO_AST_UNARY || p->nodes[value].kind == LUDO_AST_CAST ||
                 p->nodes[value].kind == LUDO_AST_RESCUE)) {
                add(p, id, error(p, "assignment requires a postfix place"));
            }
            add(p, id, expression(p));
        } else if (tail && peek(p) == LUDO_TOK_KW_END) {
            id = node(p, LUDO_AST_TAIL, start);
            add(p, id, value);
        } else {
            id = node(p, LUDO_AST_EXPR_STMT, start);
            add(p, id, value);
            if (!call_expression(p, value)) {
                add(p, id, error(p, "only a call is an expression statement"));
            }
        }
    }
    p->depth--;
    return finish(p, id, start);
}

/* The condition is consumed once. A single THEN lookahead decides whether an
   IF after a simple statement is the next compound statement or a modifier.
   statement() can return that pair as a short sibling chain. */
static ludo_node_id if_statement(parser *p, size_t start, ludo_node_id first_condition) {
    ludo_node_id id = node(p, LUDO_AST_IF, start);
    size_t branch_start = start;
    ludo_node_id branch_condition = first_condition;
    for (;;) {
        ludo_node_id branch = node(p, LUDO_AST_BRANCH, branch_start);
        add(p, branch, branch_condition);
        expect(p, LUDO_TOK_KW_THEN, branch);
        add(p, branch, block(p, false));
        add(p, id, finish(p, branch, branch_start));
        branch_start = p->pos;
        if (p->exhausted || !eat(p, LUDO_TOK_KW_ELSEIF)) {
            break;
        }
        branch_condition = condition(p);
    }
    if (eat(p, LUDO_TOK_KW_ELSE)) {
        ludo_node_id branch = node(p, LUDO_AST_BRANCH, branch_start);
        add(p, branch, block(p, false));
        add(p, id, finish(p, branch, branch_start));
    }
    expect(p, LUDO_TOK_KW_END, id);
    return finish(p, id, start);
}

static ludo_node_id statement(parser *p, bool tail) {
    if (!enter(p)) {
        return 0;
    }
    size_t start = p->pos;
    ludo_node_id id;
    if (eat(p, LUDO_TOK_KW_IF)) {
        ludo_node_id first_condition = condition(p);
        id = if_statement(p, start, first_condition);
    } else if (eat(p, LUDO_TOK_KW_WHILE)) {
        id = node(p, LUDO_AST_WHILE, start);
        add(p, id, condition(p));
        expect(p, LUDO_TOK_KW_DO, id);
        add(p, id, block(p, false));
        expect(p, LUDO_TOK_KW_END, id);
    } else if (eat(p, LUDO_TOK_KW_FOR)) {
        id = node(p, LUDO_AST_FOR, start);
        add(p, id, bindings(p));
        expect(p, LUDO_TOK_KW_IN, id);
        add(p, id, expression(p));
        expect(p, LUDO_TOK_KW_DO, id);
        add(p, id, block(p, false));
        expect(p, LUDO_TOK_KW_END, id);
    } else if (eat(p, LUDO_TOK_KW_UNSAFE)) {
        id = node(p, LUDO_AST_UNSAFE, start);
        expect(p, LUDO_TOK_KW_DO, id);
        add(p, id, block(p, false));
        expect(p, LUDO_TOK_KW_END, id);
    } else if (eat(p, LUDO_TOK_KW_MATCH)) {
        id = node(p, LUDO_AST_MATCH, start);
        add(p, id, expression(p));
        expect(p, LUDO_TOK_KW_DO, id);
        bool any = false;
        while (!p->exhausted && !block_end(peek(p))) {
            size_t before = p->pos;
            ludo_node_id arm = node(p, LUDO_AST_ARM, before);
            add(p, arm, pattern(p));
            expect(p, LUDO_TOK_KW_THEN, arm);
            add(p, arm, block(p, false));
            expect(p, LUDO_TOK_KW_END, arm);
            add(p, id, finish(p, arm, before));
            progress(p, before, id);
            any = true;
        }
        if (!any) {
            add(p, id, error(p, "match requires an arm"));
        }
        expect(p, LUDO_TOK_KW_END, id);
    } else {
        id = simple_statement(p, tail);
        if (peek(p) == LUDO_TOK_KW_IF || peek(p) == LUDO_TOK_KW_UNLESS) {
            size_t op = take(p);
            ludo_node_id following_condition = condition(p);
            if (token_at(p, op).kind == LUDO_TOK_KW_IF && peek(p) == LUDO_TOK_KW_THEN) {
                ludo_node_id next = if_statement(p, op, following_condition);
                if (id != 0) {
                    p->nodes[id].next_sibling = next;
                }
                goto done; /* simple_statement() already fixed the first statement's span */
            }
            ludo_node_id modifier = node(p, LUDO_AST_MODIFIER, op);
            add(p, modifier, id);
            add(p, modifier, following_condition);
            id = modifier;
        }
    }
    id = finish(p, id, start);
done:
    p->depth--;
    return id;
}

static ludo_node_id block(parser *p, bool tail) {
    size_t start = p->pos;
    ludo_node_id id = node(p, LUDO_AST_BLOCK, start);
    while (!p->exhausted && !block_end(peek(p))) {
        if (declaration_start(peek(p))) {
            /* Preserve a following top-level declaration after a missing end.
               Its declaration keyword is a better boundary than a newline. */
            add(p, id, error(p, "declaration in block or missing end"));
            break;
        }
        size_t before = p->pos;
        add(p, id, statement(p, tail));
        progress(p, before, id);
    }
    return finish(p, id, start);
}

static ludo_node_id attributes(parser *p) {
    /* Temporary owner: the declaration takes this chain without copying nodes. */
    ludo_node_id first = 0;
    ludo_node_id last = 0;
    while (!p->exhausted && peek(p) == LUDO_TOK_HASH) {
        size_t start = take(p);
        size_t name = p->pos;
        ludo_node_id id = node(p, LUDO_AST_ATTRIBUTE, name);
        expect(p, LUDO_TOK_IDENT, id);
        if (eat(p, LUDO_TOK_LPAREN)) {
            do {
                if (literal(peek(p))) {
                    add(p, id, leaf(p, LUDO_AST_LITERAL));
                } else {
                    add(p, id, identifier(p));
                }
            } while (!p->exhausted && eat(p, LUDO_TOK_COMMA));
            expect(p, LUDO_TOK_RPAREN, id);
        }
        (void)finish(p, id, start);
        if (id != 0) {
            if (last == 0) {
                first = id;
            } else {
                p->nodes[last].next_sibling = id;
            }
            last = id;
        }
    }
    return first;
}

static ludo_node_id declaration(parser *p) {
    size_t start = p->pos;
    bool pub = eat(p, LUDO_TOK_KW_PUB);
    ludo_node_id attrs = attributes(p);
    size_t keyword = p->pos;
    ludo_token_kind kind = peek(p);
    (void)take(p);
    size_t name = p->pos;
    ludo_node_id id;
    if (kind == LUDO_TOK_KW_TYPE) {
        id = node(p, LUDO_AST_TYPE_DECL, name);
        expect(p, LUDO_TOK_IDENT, id);
        if (peek(p) == LUDO_TOK_LBRACKET) {
            add(p, id, generic_params(p));
        }
        expect(p, LUDO_TOK_EQ, id);
        add(p, id, type_body(p));
    } else if (kind == LUDO_TOK_KW_CONST || kind == LUDO_TOK_KW_PERSIST) {
        id = node(p, kind == LUDO_TOK_KW_CONST ? LUDO_AST_CONST : LUDO_AST_PERSIST, name);
        expect(p, LUDO_TOK_IDENT, id);
        if (kind == LUDO_TOK_KW_PERSIST || peek(p) == LUDO_TOK_COLON) {
            expect(p, LUDO_TOK_COLON, id);
            add(p, id, type_expr(p, FOLLOW_TYPE));
        }
        if (kind == LUDO_TOK_KW_CONST || peek(p) == LUDO_TOK_EQ) {
            expect(p, LUDO_TOK_EQ, id);
            add(p, id, expression(p));
        }
    } else if (kind == LUDO_TOK_KW_FN) {
        id = node(p, LUDO_AST_FN, name);
        expect(p, LUDO_TOK_IDENT, id);
        signature(p, id, true, FOLLOW_BODY);
        add(p, id, block(p, true));
        expect(p, LUDO_TOK_KW_END, id);
    } else if (kind == LUDO_TOK_KW_FRAME) {
        id = node(p, LUDO_AST_FRAME, keyword);
        add(p, id, parameters(p));
        add(p, id, block(p, false));
        expect(p, LUDO_TOK_KW_END, id);
    } else if (kind == LUDO_TOK_KW_EXTERN) {
        if (peek(p) == LUDO_TOK_STRING) {
            ludo_node_id library = leaf(p, LUDO_AST_LITERAL);
            expect(p, LUDO_TOK_KW_FN, library);
            id = node(p, LUDO_AST_EXTERN_FN, p->pos);
            add(p, id, library);
            expect(p, LUDO_TOK_IDENT, id);
            signature(p, id, false, FOLLOW_TYPE);
        } else {
            ludo_node_id class_name = identifier(p);
            id = node(p, LUDO_AST_EXTERN_ITEM, p->pos);
            add(p, id, class_name);
            expect(p, LUDO_TOK_IDENT, id);
            if (eat(p, LUDO_TOK_EQ)) {
                size_t path = p->pos;
                expect(p, LUDO_TOK_STRING, id);
                add(p, id, node(p, LUDO_AST_LITERAL, path));
            }
        }
    } else {
        id = error(p, "declaration expected");
    }
    if (id != 0) {
        if (pub) {
            p->nodes[id].flags |= LUDO_AST_PUBLIC;
        }
        if (attrs != 0 && kind != LUDO_TOK_KW_TYPE && kind != LUDO_TOK_KW_FN) {
            add(p, id, error(p, "attributes require type or fn"));
        }
        /* Attributes precede the semantic children even though their ids were
           minted earlier. Node index is identity, not traversal order. */
        if (attrs != 0) {
            ludo_node_id last = attrs;
            while (p->nodes[last].next_sibling != 0) {
                last = p->nodes[last].next_sibling;
            }
            p->nodes[last].next_sibling = p->nodes[id].first_child;
            if (p->nodes[id].last_child == 0) {
                p->nodes[id].last_child = last;
            }
            p->nodes[id].first_child = attrs;
        }
    }
    return finish(p, id, start);
}

static const char *lex_error_code(ludo_lex_error error) {
    switch (error) {
    case LUDO_LEX_OK:
        return NULL;
    case LUDO_LEX_INVALID_UTF8:
        return "bootstrap:lex-invalid-utf8";
    case LUDO_LEX_UNEXPECTED_CHAR:
        return "bootstrap:lex-unexpected-character";
    case LUDO_LEX_UNTERMINATED_STRING:
        return "bootstrap:lex-unterminated-string";
    case LUDO_LEX_BAD_ESCAPE:
        return "bootstrap:lex-bad-escape";
    case LUDO_LEX_MALFORMED_NUMBER:
        return "bootstrap:lex-malformed-number";
    }
    return NULL;
}

static bool text_is(const parser *p, size_t pos, const char *text) {
    ludo_token token = token_at(p, pos);
    size_t length = strlen(text);
    return token.length == length && memcmp(p->unit->file.source + token.offset, text, length) == 0;
}

static ludo_node_id module(parser *p) {
    size_t start = p->pos;
    ludo_node_id id = node(p, LUDO_AST_MODULE, start);
    if (peek(p) == LUDO_TOK_HASH) {
        size_t next = significant(p, p->pos + 1);
        ludo_token hash = token_at(p, p->pos);
        /* ch1 §11.1.1: the marker is the one spelling `#explicit`. A gap makes it
           no marker, and no attribute either, so it gets its own error. */
        if (token_at(p, next).kind == LUDO_TOK_IDENT && text_is(p, next, "explicit")) {
            if (hash.offset + hash.length == token_at(p, next).offset) {
                ludo_node_id marker = node(p, LUDO_AST_EXPLICIT, take(p));
                (void)take(p);
                add(p, id, finish(p, marker, start));
            } else {
                add(p, id, error(p, "#explicit takes no space after #"));
                (void)take(p);
                (void)take(p);
            }
        }
    }
    while (!p->exhausted && peek(p) != LUDO_TOK_EOF) {
        size_t before = p->pos;
        if (peek(p) == LUDO_TOK_KW_LIBRARY || peek(p) == LUDO_TOK_KW_USE) {
            bool library = peek(p) == LUDO_TOK_KW_LIBRARY;
            (void)take(p);
            ludo_node_id item = node(p, library ? LUDO_AST_LIBRARY : LUDO_AST_USE, p->pos);
            expect(p, library ? LUDO_TOK_IDENT : LUDO_TOK_STRING, item);
            add(p, id, finish(p, item, before));
        } else if (eat(p, LUDO_TOK_KW_IMPL)) {
            ludo_node_id impl = node(p, LUDO_AST_IMPL, before);
            add(p, impl, named_type(p));
            expect(p, LUDO_TOK_COLON, impl);
            add(p, impl, constraints(p));
            add(p, id, finish(p, impl, before));
        } else if (declaration_start(peek(p))) {
            add(p, id, declaration(p));
        } else {
            add(p, id, statement(p, false));
        }
        progress(p, before, id);
    }
    /* Lex errors include trivia, so they cannot be discovered only by parser
       consumption. Each is a ch7 diagnostic and an explicit root error node. */
    for (size_t i = 0; !p->exhausted && i < p->unit->file.token_count; i++) {
        ludo_token token = token_at(p, i);
        const char *code = lex_error_code(token.error);
        if (code != NULL) {
            diagnostic(p, i, code, ludo_lex_error_name(token.error));
            add(p, id, node(p, LUDO_AST_ERROR, i));
        }
    }
    if (id != 0) {
        p->nodes[id].span = (ludo_span){p->unit->file.id, 0, p->unit->file.length};
    }
    return id;
}

ludo_frontend_status ludo_session_create(ludo_session **out) {
    *out = NULL;
    ludo_arena arena = {0};
    ludo_session *session = ludo_arena_alloc(&arena, sizeof *session);
    if (session == NULL) {
        return LUDO_FRONTEND_OUT_OF_MEMORY;
    }
    *session = (ludo_session){.arena = arena};
    *out = session;
    return LUDO_FRONTEND_OK;
}

void ludo_session_destroy(ludo_session *session) {
    if (session == NULL) {
        return;
    }
    parsed_unit *unit = session->files;
    while (unit != NULL) {
        ludo_arena_destroy(&unit->arena);
        unit = unit->next;
    }
    ludo_arena_destroy(&session->check_arena);
    /* The session itself lives in this arena. Do not touch it after release. */
    ludo_arena arena = session->arena;
    ludo_arena_destroy(&arena);
}

ludo_frontend_status ludo_parse_file(ludo_session *session, const char *name, const char *source,
                                     size_t length, const ludo_parsed_file **out) {
    *out = NULL;
    parsed_unit *unit = ludo_arena_alloc(&session->arena, sizeof *unit);
    if (unit == NULL || session->count == SIZE_MAX) {
        return LUDO_FRONTEND_OUT_OF_MEMORY;
    }
    *unit = (parsed_unit){0};
    unit->file = (ludo_parsed_file){
        .id = session->count + 1, .name = name, .source = source, .length = length};
    /* Count once, store once, retaining every trivia token. No source copies. */
    ludo_lexer lexer;
    ludo_lexer_init(&lexer, source, length);
    size_t count = 1;
    while (ludo_lex_next(&lexer).kind != LUDO_TOK_EOF) {
        if (count == SIZE_MAX) {
            return LUDO_FRONTEND_OUT_OF_MEMORY;
        }
        count++;
    }
    if (count > SIZE_MAX / sizeof(ludo_token)) {
        return LUDO_FRONTEND_OUT_OF_MEMORY;
    }
    ludo_token *tokens = ludo_arena_alloc(&unit->arena, count * sizeof *tokens);
    if (tokens == NULL) {
        return LUDO_FRONTEND_OUT_OF_MEMORY;
    }
    ludo_lexer_init(&lexer, source, length);
    for (size_t i = 0; i < count; i++) {
        tokens[i] = ludo_lex_next(&lexer);
    }
    unit->file.tokens = tokens;
    unit->file.token_count = count;
    parser p = {.unit = unit, .failure = LUDO_FRONTEND_OUT_OF_MEMORY};
    /* Reserve id 0. Its bytes are never exposed as a node. */
    (void)node(&p, LUDO_AST_ERROR, 0);
    p.pos = significant(&p, 0);
    ludo_node_id root = module(&p);
    if (p.exhausted) {
        ludo_arena_destroy(&unit->arena);
        return p.failure;
    }
    unit->file.ast = (ludo_ast){p.nodes, p.node_count, root};
    unit->file.diagnostics = p.diagnostics;
    unit->file.diagnostic_count = p.diagnostic_count;
    unit->next = session->files;
    session->files = unit;
    session->count++;
    *out = &unit->file;
    return LUDO_FRONTEND_OK;
}
