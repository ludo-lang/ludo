/* The checker, grown band by band (#129's slicing; #143 is band 1).

   Two passes over one file. The first collects every top-level name and
   resolves every signature, so a body may name a declaration written after it
   (ch4 7.3's flat mapping, order-independent top level). The second checks
   each body against those signatures.

   A construct no band has checked yet yields LUDO_TYPE_PENDING. PENDING
   satisfies every check silently and marks its declaration PENDING, so the
   bands still to come never produce an error here, and an error here is
   always about a construct this checker claims to understand. */
#include "session.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct local {
    const char *name;
    size_t length;
    ludo_node_id def;
    ludo_type_id type;
    bool mutable_place; /* declared with !: its contents may be written */
    bool parameter;     /* never rebound (ch2 12.2.1) */
} local;

typedef struct checker {
    ludo_arena *arena;
    const ludo_parsed_file *file;
    const ludo_ast *ast;
    ludo_node_fact *facts;
    bool *chain;  /* per node: inside a ?. chain (ch2 9.8) */
    bool *nested; /* per variant pattern: a payload pattern that can fail */

    ludo_type *types;
    size_t type_count;
    size_t type_capacity;
    ludo_instance *instances;
    size_t instance_count;
    size_t instance_capacity;
    ludo_diagnostic *diagnostics;
    size_t diagnostic_count;
    size_t diagnostic_capacity;

    ludo_node_id *decls; /* named top-level declarations */
    size_t decl_count;
    size_t decl_capacity;
    bool unresolved_library; /* a library use whose bound name band 4 learns */

    local *locals;
    size_t local_count;
    size_t local_capacity;

    ludo_node_id generics; /* the generic-params of the declaration being checked */
    ludo_type_id result;   /* the function being checked returns this */
    size_t loop_depth;
    bool pending; /* the declaration being checked reached a later band */
    bool exhausted;
} checker;

/* ---- Memory ------------------------------------------------------------- */

static void *grow(checker *c, void *items, size_t count, size_t *capacity, size_t size) {
    if (count < *capacity) {
        return items;
    }
    size_t next = *capacity == 0 ? 16 : *capacity * 2;
    if (next > SIZE_MAX / size) {
        c->exhausted = true;
        return NULL;
    }
    void *fresh = ludo_arena_alloc(c->arena, next * size);
    if (fresh == NULL) {
        c->exhausted = true;
        return NULL;
    }
    if (count != 0) {
        memcpy(fresh, items, count * size);
    }
    *capacity = next;
    return fresh;
}

/* ---- The tree ------------------------------------------------------------ */

static const ludo_ast_node *at(const checker *c, ludo_node_id id) { return &c->ast->nodes[id]; }

static ludo_ast_kind kind_of(const checker *c, ludo_node_id id) {
    return id == 0 ? LUDO_AST_ERROR : at(c, id)->kind;
}

static ludo_node_id first(const checker *c, ludo_node_id id) {
    return id == 0 ? 0 : at(c, id)->first_child;
}

static ludo_node_id next(const checker *c, ludo_node_id id) { return at(c, id)->next_sibling; }

static ludo_node_id child_of_kind(const checker *c, ludo_node_id id, ludo_ast_kind kind) {
    for (ludo_node_id it = first(c, id); it != 0; it = next(c, it)) {
        if (kind_of(c, it) == kind) {
            return it;
        }
    }
    return 0;
}

static size_t child_count(const checker *c, ludo_node_id id) {
    size_t count = 0;
    for (ludo_node_id it = first(c, id); it != 0; it = next(c, it)) {
        count++;
    }
    return count;
}

const char *ludo_node_text(const ludo_parsed_file *file, ludo_node_id id, size_t *length) {
    const ludo_ast_node *node = ludo_ast_get(&file->ast, id);
    if (node == NULL || node->token >= file->token_count) {
        *length = 0;
        return "";
    }
    const ludo_token *token = &file->tokens[node->token];
    *length = token->length;
    return file->source + token->offset;
}

static bool named(const checker *c, ludo_node_id id, const char *name) {
    size_t length = 0;
    const char *text = ludo_node_text(c->file, id, &length);
    return length == strlen(name) && memcmp(text, name, length) == 0;
}

static bool same_name(const checker *c, ludo_node_id a, ludo_node_id b) {
    size_t la = 0;
    size_t lb = 0;
    const char *ta = ludo_node_text(c->file, a, &la);
    const char *tb = ludo_node_text(c->file, b, &lb);
    return la == lb && memcmp(ta, tb, la) == 0;
}

static ludo_token_kind token_kind(const checker *c, ludo_node_id id) {
    size_t token = at(c, id)->token;
    return token < c->file->token_count ? c->file->tokens[token].kind : LUDO_TOK_EOF;
}

/* ---- Diagnostics ----------------------------------------------------------
   Codes live under bootstrap: until ch7 5.7 assigns the spec's (src/frontend
   AST.md). A message is its fixed text followed by the name it is about. */

static void report(checker *c, ludo_node_id at_node, const char *code, const char *message,
                   ludo_node_id subject) {
    size_t subject_length = 0;
    const char *subject_text =
        subject != 0 ? ludo_node_text(c->file, subject, &subject_length) : "";
    size_t message_length = strlen(message);
    /* ": `" before the subject, "`" after it, and the terminator. */
    char *text = ludo_arena_alloc(c->arena, message_length + subject_length + 5);
    ludo_diagnostic *grown = grow(c, c->diagnostics, c->diagnostic_count, &c->diagnostic_capacity,
                                  sizeof *c->diagnostics);
    if (text == NULL || grown == NULL) {
        c->exhausted = true;
        return;
    }
    memcpy(text, message, message_length);
    size_t length = message_length;
    if (subject_length != 0) {
        memcpy(text + length, ": `", 3);
        length += 3;
        memcpy(text + length, subject_text, subject_length);
        length += subject_length;
        text[length++] = '`';
    }
    text[length] = '\0';
    c->diagnostics = grown;
    c->diagnostics[c->diagnostic_count++] =
        (ludo_diagnostic){.code = code, .message = text, .primary = at(c, at_node)->span};
}

/* ---- Types --------------------------------------------------------------- */

static bool same_type(const ludo_type *a, const ludo_type *b) {
    if (a->kind != b->kind || a->decl != b->decl || a->inner != b->inner ||
        a->param_count != b->param_count || a->extern_class != b->extern_class) {
        return false;
    }
    for (size_t i = 0; i < a->param_count; i++) {
        if (a->params[i] != b->params[i]) {
            return false;
        }
    }
    return true;
}

static ludo_type_id intern(checker *c, ludo_type type) {
    for (size_t i = LUDO_TYPE_FIXED_COUNT; i < c->type_count; i++) {
        if (same_type(&c->types[i], &type)) {
            return (ludo_type_id)i;
        }
    }
    if (type.param_count != 0) {
        ludo_type_id *params = ludo_arena_alloc(c->arena, type.param_count * sizeof *params);
        if (params == NULL) {
            c->exhausted = true;
            return LUDO_TYPE_ID_OF(LUDO_TYPE_INVALID);
        }
        memcpy(params, type.params, type.param_count * sizeof *params);
        type.params = params;
    }
    ludo_type *grown = grow(c, c->types, c->type_count, &c->type_capacity, sizeof *c->types);
    if (grown == NULL || c->type_count >= UINT32_MAX) {
        c->exhausted = true;
        return LUDO_TYPE_ID_OF(LUDO_TYPE_INVALID);
    }
    c->types = grown;
    c->types[c->type_count] = type;
    return (ludo_type_id)c->type_count++;
}

static ludo_type_id simple(ludo_type_kind kind) { return LUDO_TYPE_ID_OF(kind); }

static ludo_type_id wrap(checker *c, ludo_type_kind kind, ludo_type_id inner) {
    return intern(c, (ludo_type){.kind = kind, .inner = inner});
}

static const ludo_type *type_at(const checker *c, ludo_type_id id) { return &c->types[id]; }

static ludo_type_kind tkind(const checker *c, ludo_type_id id) { return c->types[id].kind; }

static ludo_type_id pending(checker *c) {
    c->pending = true;
    return simple(LUDO_TYPE_PENDING);
}

/* PENDING and INVALID satisfy everything: the first belongs to a later band,
   the second was already reported. */
static bool soft(const checker *c, ludo_type_id id) {
    ludo_type_kind kind = tkind(c, id);
    return kind == LUDO_TYPE_PENDING || kind == LUDO_TYPE_INVALID;
}

static bool is_integer(const checker *c, ludo_type_id id) {
    ludo_type_kind kind = tkind(c, id);
    return kind >= LUDO_TYPE_I8 && kind <= LUDO_TYPE_USIZE;
}

static bool is_float(const checker *c, ludo_type_id id) {
    ludo_type_kind kind = tkind(c, id);
    return kind == LUDO_TYPE_F32 || kind == LUDO_TYPE_F64;
}

static bool is_numeric(const checker *c, ludo_type_id id) {
    return is_integer(c, id) || is_float(c, id);
}

static bool is_numeric_distinct(const checker *c, ludo_type_id id) {
    return tkind(c, id) == LUDO_TYPE_DISTINCT &&
           (at(c, type_at(c, id)->decl)->flags & LUDO_AST_NUMERIC) != 0;
}

/* ch2 9.11: T is usable where ?T is expected, by static type and without
   flattening, which the exact-match test above it preserves. */
static bool assignable(const checker *c, ludo_type_id from, ludo_type_id to) {
    if (from == to || soft(c, from) || soft(c, to)) {
        return true;
    }
    if (tkind(c, to) == LUDO_TYPE_OPTIONAL) {
        return assignable(c, from, type_at(c, to)->inner);
    }
    return false;
}

/* ---- Names --------------------------------------------------------------- */

/* Prelude identifiers a later band gives meaning to (ch1 2.6-2.9, ch3 9.11,
   10.3). Naming one is PENDING; naming an unknown identifier is an error. */
static const char *const later_prelude[] = {
    "heap",    "Range",  "RevRange", "Allocator", "Scratch", "Target",      "List",  "Map",
    "TextBuf", "Handle", "Iter",     "format",    "char",    "OutOfMemory", "Color",
};

static bool in_later_prelude(const checker *c, ludo_node_id id) {
    for (size_t i = 0; i < sizeof later_prelude / sizeof later_prelude[0]; i++) {
        if (named(c, id, later_prelude[i])) {
            return true;
        }
    }
    return false;
}

static const struct {
    const char *name;
    ludo_type_kind kind;
} primitives[] = {
    {"bool", LUDO_TYPE_BOOL},     {"i8", LUDO_TYPE_I8},   {"i16", LUDO_TYPE_I16},
    {"i32", LUDO_TYPE_I32},       {"i64", LUDO_TYPE_I64}, {"u8", LUDO_TYPE_U8},
    {"u16", LUDO_TYPE_U16},       {"u32", LUDO_TYPE_U32}, {"u64", LUDO_TYPE_U64},
    {"usize", LUDO_TYPE_USIZE},   {"f32", LUDO_TYPE_F32}, {"f64", LUDO_TYPE_F64},
    {"string", LUDO_TYPE_STRING},
};

/* A use's bound name is the path's stem (ch4 3.4) or the name the library
   claims (ch4 2.3); neither is the use node's token, so it is not a lookup
   key here. Band 4 builds the mapping that binds them. */
static bool declares_name(ludo_ast_kind kind) {
    return kind == LUDO_AST_TYPE_DECL || kind == LUDO_AST_CONST || kind == LUDO_AST_FN ||
           kind == LUDO_AST_EXTERN_FN || kind == LUDO_AST_EXTERN_ITEM || kind == LUDO_AST_PERSIST;
}

static ludo_node_id lookup_decl(const checker *c, ludo_node_id name) {
    for (size_t i = 0; i < c->decl_count; i++) {
        if (same_name(c, c->decls[i], name)) {
            return c->decls[i];
        }
    }
    return 0;
}

ludo_node_id ludo_checked_lookup(const ludo_checked_file *checked, const char *name,
                                 size_t name_length) {
    const ludo_ast *ast = &checked->file->ast;
    for (ludo_node_id it = ast->nodes[ast->root].first_child; it != 0;
         it = ast->nodes[it].next_sibling) {
        size_t length = 0;
        const char *text = ludo_node_text(checked->file, it, &length);
        if (declares_name(ast->nodes[it].kind) && length == name_length &&
            memcmp(text, name, length) == 0) {
            return it;
        }
    }
    return 0;
}

static local *lookup_local(checker *c, ludo_node_id name) {
    size_t length = 0;
    const char *text = ludo_node_text(c->file, name, &length);
    for (size_t i = c->local_count; i != 0; i--) {
        local *entry = &c->locals[i - 1];
        if (entry->length == length && memcmp(entry->name, text, length) == 0) {
            return entry;
        }
    }
    return NULL;
}

static ludo_node_id lookup_generic(const checker *c, ludo_node_id name) {
    for (ludo_node_id it = first(c, c->generics); it != 0; it = next(c, it)) {
        if (same_name(c, it, name)) {
            return it;
        }
    }
    return 0;
}

static void bind(checker *c, ludo_node_id def, ludo_type_id type, bool mutable_place,
                 bool parameter) {
    local *grown = grow(c, c->locals, c->local_count, &c->local_capacity, sizeof *c->locals);
    if (grown == NULL) {
        return;
    }
    c->locals = grown;
    size_t length = 0;
    const char *text = ludo_node_text(c->file, def, &length);
    c->locals[c->local_count++] = (local){text, length, def, type, mutable_place, parameter};
    c->facts[def].ref = LUDO_REF_BINDS;
    c->facts[def].type = type;
}

/* ---- Type expressions ---------------------------------------------------- */

static ludo_type_id resolve_type(checker *c, ludo_node_id id);

static ludo_type_id nominal(checker *c, ludo_node_id decl) {
    ludo_node_id body = 0;
    for (ludo_node_id it = first(c, decl); it != 0; it = next(c, it)) {
        body = it;
    }
    if (child_of_kind(c, decl, LUDO_AST_GENERIC_PARAMS) != 0) {
        return pending(c); /* generic types are band 2's (Ranked[T]) */
    }
    ludo_ast_kind kind = kind_of(c, body);
    if (kind == LUDO_AST_STRUCT) {
        return intern(c, (ludo_type){.kind = LUDO_TYPE_STRUCT, .decl = decl});
    }
    if (kind == LUDO_AST_ENUM) {
        return intern(c, (ludo_type){.kind = LUDO_TYPE_ENUM, .decl = decl});
    }
    if (kind == LUDO_AST_DISTINCT) {
        /* The base is settled after the type pass (settle_distinct). */
        return intern(c, (ludo_type){.kind = LUDO_TYPE_DISTINCT, .decl = decl});
    }
    if (kind == LUDO_AST_ERROR) {
        return simple(LUDO_TYPE_INVALID);
    }
    return pending(c); /* an interface or a type-expression body: band 2's */
}

static ludo_type_id resolve_named(checker *c, ludo_node_id id) {
    ludo_node_id head = first(c, id);
    if (child_of_kind(c, id, LUDO_AST_GENERIC_ARGS) != 0) {
        return pending(c); /* List[Rock], TextBuf[32], Ranked[Rock] */
    }
    if (kind_of(c, head) == LUDO_AST_ROOT) {
        ludo_node_id name = next(c, head);
        if (name != 0 && next(c, name) == 0 && named(c, name, "vec2")) {
            return simple(LUDO_TYPE_VEC2);
        }
        return pending(c); /* the rest of $. is band 6's */
    }
    if (head == 0 || next(c, head) != 0) {
        return pending(c); /* a qualified name reaches a module: band 4 */
    }
    ludo_node_id generic = lookup_generic(c, head);
    if (generic != 0) {
        c->facts[id].ref = LUDO_REF_GENERIC;
        c->facts[id].target = generic;
        return c->facts[generic].type;
    }
    ludo_node_id decl = lookup_decl(c, head);
    if (decl != 0 && kind_of(c, decl) == LUDO_AST_TYPE_DECL) {
        c->facts[id].ref = LUDO_REF_DECL;
        c->facts[id].target = decl;
        return c->facts[decl].type;
    }
    if (decl != 0) {
        report(c, head, "bootstrap:check-not-a-type", "this names a value, not a type", head);
        return simple(LUDO_TYPE_INVALID);
    }
    for (size_t i = 0; i < sizeof primitives / sizeof primitives[0]; i++) {
        if (named(c, head, primitives[i].name)) {
            return simple(primitives[i].kind);
        }
    }
    if (in_later_prelude(c, head)) {
        return pending(c);
    }
    report(c, head, "bootstrap:check-unknown-type", "no type has this name", head);
    return simple(LUDO_TYPE_INVALID);
}

static ludo_type_id resolve_fn_type(checker *c, ludo_node_id id) {
    ludo_node_id params = first(c, id);
    ludo_type_id buffer[16];
    size_t count = 0;
    for (ludo_node_id it = first(c, params); it != 0; it = next(c, it)) {
        if (count == sizeof buffer / sizeof buffer[0]) {
            return pending(c);
        }
        buffer[count++] = resolve_type(c, it);
    }
    ludo_node_id result = params != 0 ? next(c, params) : 0;
    ludo_type_id result_type = result != 0 ? resolve_type(c, result) : simple(LUDO_TYPE_UNIT);
    return intern(
        c, (ludo_type){
               .kind = LUDO_TYPE_FN, .inner = result_type, .params = buffer, .param_count = count});
}

static ludo_type_id resolve_type(checker *c, ludo_node_id id) {
    ludo_ast_kind kind = kind_of(c, id);
    ludo_type_id type;
    if (kind == LUDO_AST_NAMED_TYPE) {
        type = resolve_named(c, id);
    } else if (kind == LUDO_AST_OPTIONAL_TYPE || kind == LUDO_AST_VIEW_TYPE) {
        ludo_type_id inner = resolve_type(c, first(c, id));
        type =
            soft(c, inner)
                ? inner
                : wrap(c, kind == LUDO_AST_VIEW_TYPE ? LUDO_TYPE_VIEW : LUDO_TYPE_OPTIONAL, inner);
    } else if (kind == LUDO_AST_MUTABLE_TYPE) {
        /* ch2 12.1: !T and T are the same type; the place records the mark. */
        type = resolve_type(c, first(c, id));
    } else if (kind == LUDO_AST_FN_TYPE) {
        type = resolve_fn_type(c, id);
    } else if (kind == LUDO_AST_ERROR) {
        type = simple(LUDO_TYPE_INVALID);
    } else {
        type = pending(c); /* raw pointers, fixed arrays, fallible types */
    }
    c->facts[id].type = type;
    return type;
}

/* ---- Generics ------------------------------------------------------------ */

static size_t generic_index(const checker *c, ludo_node_id generics, ludo_node_id param) {
    size_t index = 0;
    for (ludo_node_id it = first(c, generics); it != 0; it = next(c, it), index++) {
        if (it == param) {
            return index;
        }
    }
    return SIZE_MAX;
}

static ludo_type_id substitute(checker *c, ludo_type_id type, ludo_node_id generics,
                               const ludo_type_id *args) {
    const ludo_type *t = type_at(c, type);
    if (t->kind == LUDO_TYPE_PARAM) {
        size_t index = generic_index(c, generics, t->decl);
        return index == SIZE_MAX ? type : args[index];
    }
    if (t->kind == LUDO_TYPE_VIEW || t->kind == LUDO_TYPE_OPTIONAL) {
        return wrap(c, t->kind, substitute(c, t->inner, generics, args));
    }
    if (t->kind != LUDO_TYPE_FN) {
        return type;
    }
    ludo_type_id params[16];
    size_t count = t->param_count;
    ludo_type_id inner = substitute(c, t->inner, generics, args);
    for (size_t i = 0; i < count; i++) {
        params[i] = substitute(c, type_at(c, type)->params[i], generics, args);
    }
    return intern(
        c,
        (ludo_type){.kind = LUDO_TYPE_FN, .inner = inner, .params = params, .param_count = count});
}

static bool mentions_param(const checker *c, ludo_type_id type) {
    const ludo_type *t = type_at(c, type);
    if (t->kind == LUDO_TYPE_PARAM) {
        return true;
    }
    if (t->kind == LUDO_TYPE_VIEW || t->kind == LUDO_TYPE_OPTIONAL || t->kind == LUDO_TYPE_FN) {
        if (t->inner != 0 && mentions_param(c, t->inner)) {
            return true;
        }
        for (size_t i = 0; i < t->param_count; i++) {
            if (mentions_param(c, t->params[i])) {
                return true;
            }
        }
    }
    return false;
}

/* Binds the generic's parameters from one argument. False on a mismatch. */
static bool unify(checker *c, ludo_type_id param, ludo_type_id arg, ludo_node_id generics,
                  ludo_type_id *bound) {
    const ludo_type *p = type_at(c, param);
    if (soft(c, arg)) {
        return true;
    }
    if (p->kind == LUDO_TYPE_PARAM) {
        size_t index = generic_index(c, generics, p->decl);
        if (index == SIZE_MAX) {
            return param == arg;
        }
        if (bound[index] == 0) {
            bound[index] = arg;
        }
        return bound[index] == arg;
    }
    const ludo_type *a = type_at(c, arg);
    if (p->kind == LUDO_TYPE_OPTIONAL && a->kind != LUDO_TYPE_OPTIONAL) {
        return unify(c, p->inner, arg, generics, bound); /* ch2 9.11 injection */
    }
    if (p->kind != a->kind) {
        return false;
    }
    if (p->kind == LUDO_TYPE_VIEW || p->kind == LUDO_TYPE_OPTIONAL) {
        return unify(c, p->inner, a->inner, generics, bound);
    }
    if (p->kind == LUDO_TYPE_FN) {
        if (p->param_count != a->param_count) {
            return false;
        }
        for (size_t i = 0; i < p->param_count; i++) {
            if (!unify(c, type_at(c, param)->params[i], type_at(c, arg)->params[i], generics,
                       bound)) {
                return false;
            }
        }
        return unify(c, type_at(c, param)->inner, type_at(c, arg)->inner, generics, bound);
    }
    return param == arg;
}

/* The instantiation cache (ch2 5.1, 5.3), keyed by (decl, type arguments).
   Types are interned, so argument lists compare by id. */
static uint32_t instantiate(checker *c, ludo_node_id decl, const ludo_type_id *args, size_t count,
                            ludo_type_id signature) {
    for (size_t i = 0; i < c->instance_count; i++) {
        const ludo_instance *entry = &c->instances[i];
        if (entry->decl == decl && entry->arg_count == count &&
            memcmp(entry->args, args, count * sizeof *args) == 0) {
            return (uint32_t)(i + 1);
        }
    }
    ludo_type_id *stored = ludo_arena_alloc(c->arena, count * sizeof *stored);
    ludo_instance *grown =
        grow(c, c->instances, c->instance_count, &c->instance_capacity, sizeof *c->instances);
    if (stored == NULL || grown == NULL || c->instance_count >= UINT32_MAX - 1) {
        c->exhausted = true;
        return 0;
    }
    memcpy(stored, args, count * sizeof *stored);
    c->instances = grown;
    c->instances[c->instance_count++] = (ludo_instance){decl, stored, count, signature};
    return (uint32_t)c->instance_count;
}

/* ---- Expressions ---------------------------------------------------------- */

static ludo_type_id check_expr(checker *c, ludo_node_id id, ludo_type_id expected);

static bool is_numeric_literal(const checker *c, ludo_node_id id) {
    if (kind_of(c, id) == LUDO_AST_GROUP) {
        return is_numeric_literal(c, first(c, id));
    }
    if (kind_of(c, id) == LUDO_AST_UNARY && token_kind(c, id) == LUDO_TOK_MINUS) {
        return is_numeric_literal(c, first(c, id));
    }
    ludo_token_kind token = kind_of(c, id) == LUDO_AST_LITERAL ? token_kind(c, id) : LUDO_TOK_EOF;
    return token == LUDO_TOK_INT || token == LUDO_TOK_FLOAT;
}

static bool literal_value(const checker *c, ludo_node_id id, uint64_t *value) {
    return ludo_literal_integer(c->file, id, value);
}

bool ludo_literal_integer(const ludo_parsed_file *file, ludo_node_id id, uint64_t *value) {
    size_t length = 0;
    const char *text = ludo_node_text(file, id, &length);
    unsigned base = 10;
    size_t i = 0;
    if (length > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'b')) {
        base = text[1] == 'x' ? 16u : 2u;
        i = 2;
    }
    uint64_t total = 0;
    for (; i < length; i++) {
        char ch = text[i];
        unsigned digit;
        if (ch == '_') {
            continue;
        }
        if (ch >= '0' && ch <= '9') {
            digit = (unsigned)(ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
            digit = (unsigned)(ch - 'a') + 10u;
        } else {
            digit = (unsigned)(ch - 'A') + 10u;
        }
        if (total > (UINT64_MAX - digit) / base) {
            return false;
        }
        total = total * base + digit;
    }
    *value = total;
    return true;
}

/* strtod wants a terminated buffer and the source is not one; the digit
   separators ch1 3.1 admits are dropped on the way. A float literal longer
   than the buffer reads as infinity, which no literal in a real program is. */
double ludo_literal_float(const ludo_parsed_file *file, ludo_node_id id) {
    size_t length = 0;
    const char *text = ludo_node_text(file, id, &length);
    char buffer[128];
    size_t used = 0;
    for (size_t i = 0; i < length; i++) {
        if (text[i] != '_') {
            if (used + 1 >= sizeof buffer) {
                return HUGE_VAL;
            }
            buffer[used++] = text[i];
        }
    }
    buffer[used] = '\0';
    return strtod(buffer, NULL);
}

static uint64_t integer_max(ludo_type_kind kind) {
    static const uint64_t max[] = {INT8_MAX,   INT16_MAX,  INT32_MAX,  INT64_MAX, UINT8_MAX,
                                   UINT16_MAX, UINT32_MAX, UINT64_MAX, UINT64_MAX};
    if (kind < LUDO_TYPE_I8 || kind > LUDO_TYPE_USIZE) {
        return 0;
    }
    return max[(size_t)kind - (size_t)LUDO_TYPE_I8];
}

/* ch1 3.2: a numeric literal's type comes from its context, and no context is
   a compile error naming the ambiguity, never a silent pick. */
static ludo_type_id check_literal(checker *c, ludo_node_id id, ludo_type_id expected) {
    ludo_token_kind token = token_kind(c, id);
    if (token == LUDO_TOK_KW_TRUE || token == LUDO_TOK_KW_FALSE) {
        return simple(LUDO_TYPE_BOOL);
    }
    if (token == LUDO_TOK_STRING) {
        return simple(LUDO_TYPE_STRING);
    }
    if (expected != 0 && tkind(c, expected) == LUDO_TYPE_OPTIONAL) {
        expected = type_at(c, expected)->inner; /* ch2 9.11: injected after typing */
    }
    if (expected != 0 && soft(c, expected)) {
        return expected;
    }
    if (expected == 0 || !(is_numeric(c, expected))) {
        if (expected == 0) {
            report(c, id, "bootstrap:check-ambiguous-literal",
                   "nothing here fixes this literal's type; annotate it", id);
        } else {
            report(c, id, "bootstrap:check-mismatch", "a numeric literal cannot have this type",
                   id);
        }
        return simple(LUDO_TYPE_INVALID);
    }
    if (token == LUDO_TOK_FLOAT && !is_float(c, expected)) {
        report(c, id, "bootstrap:check-mismatch", "a float literal where an integer is expected",
               id);
        return simple(LUDO_TYPE_INVALID);
    }
    uint64_t value = 0;
    if (token == LUDO_TOK_INT && is_integer(c, expected) &&
        (!literal_value(c, id, &value) || value > integer_max(tkind(c, expected)))) {
        report(c, id, "bootstrap:check-literal-range", "this literal does not fit its type", id);
        return simple(LUDO_TYPE_INVALID);
    }
    return expected;
}

static ludo_type_id check_ident(checker *c, ludo_node_id id, ludo_type_id expected) {
    local *entry = lookup_local(c, id);
    if (entry != NULL) {
        c->facts[id].ref = LUDO_REF_LOCAL;
        c->facts[id].target = entry->def;
        return entry->type;
    }
    ludo_node_id decl = lookup_decl(c, id);
    if (decl != 0) {
        c->facts[id].ref = LUDO_REF_DECL;
        c->facts[id].target = decl;
        ludo_ast_kind kind = kind_of(c, decl);
        if (kind == LUDO_AST_FN && child_of_kind(c, decl, LUDO_AST_GENERIC_PARAMS) != 0) {
            /* ch2 5.8: a generic becomes a value only when instantiated. */
            report(c, id, "bootstrap:check-generic-value",
                   "a generic function is a value only with its type arguments", id);
            return simple(LUDO_TYPE_INVALID);
        }
        if (kind == LUDO_AST_TYPE_DECL) {
            report(c, id, "bootstrap:check-not-a-value", "this names a type, not a value", id);
            return simple(LUDO_TYPE_INVALID);
        }
        if (c->facts[decl].type == 0 || kind == LUDO_AST_PERSIST || kind == LUDO_AST_EXTERN_FN) {
            /* persist and extern fn are later bands'; a const read before its
               own declaration is checked is another const's initialiser. */
            return pending(c);
        }
        return c->facts[decl].type;
    }
    if (named(c, id, "none")) {
        /* ch2 9.2: the prelude's nullary constructor for ?T, typed by context. */
        if (expected != 0 && tkind(c, expected) == LUDO_TYPE_OPTIONAL) {
            c->facts[id].ref = LUDO_REF_VARIANT;
            c->facts[id].index = 0;
            return expected;
        }
        if (expected != 0 && soft(c, expected)) {
            return expected;
        }
        report(c, id, "bootstrap:check-ambiguous-none", "nothing here fixes which ?T this is", id);
        return simple(LUDO_TYPE_INVALID);
    }
    if (in_later_prelude(c, id)) {
        return pending(c);
    }
    report(c, id, "bootstrap:check-unknown-name", "nothing in scope has this name", id);
    return simple(LUDO_TYPE_INVALID);
}

/* ch2 16.1-16.2: arithmetic on the primitives, on distinct numeric as ch2 3.2
   permits, and on the blessed math types; no user type gets an operator. */
static ludo_type_id arithmetic(checker *c, ludo_node_id at_node, ludo_token_kind op,
                               ludo_type_id left, ludo_type_id right) {
    if (soft(c, left) || soft(c, right)) {
        return soft(c, left) ? left : right;
    }
    if (left == right && is_numeric(c, left)) {
        return left;
    }
    if (tkind(c, left) == LUDO_TYPE_VEC2) {
        if ((op == LUDO_TOK_PLUS || op == LUDO_TOK_MINUS) && right == left) {
            return left;
        }
        if (op == LUDO_TOK_STAR && tkind(c, right) == LUDO_TYPE_F32) {
            return left;
        }
    }
    if (is_numeric_distinct(c, left)) {
        if ((op == LUDO_TOK_PLUS || op == LUDO_TOK_MINUS) && right == left) {
            return left;
        }
        if (op == LUDO_TOK_STAR && right == type_at(c, left)->inner) {
            return left;
        }
    }
    report(c, at_node, "bootstrap:check-operator", "this operator is not defined on these types",
           at_node);
    return simple(LUDO_TYPE_INVALID);
}

static bool comparable(const checker *c, ludo_type_id type, bool ordering) {
    if (is_numeric(c, type) || tkind(c, type) == LUDO_TYPE_CHAR ||
        tkind(c, type) == LUDO_TYPE_DISTINCT) {
        return true; /* ch2 3.1: distinct keeps equality and ordering */
    }
    return !ordering && tkind(c, type) == LUDO_TYPE_BOOL;
}

/* Checks a binary operation's operands so that a literal on either side is
   typed by the other (ch1 3.2). The scalar side of vec2 * k is f32. */
static void operands(checker *c, ludo_node_id left, ludo_node_id right, ludo_type_id expected,
                     ludo_token_kind op, ludo_type_id *lt, ludo_type_id *rt) {
    if (is_numeric_literal(c, left) && !is_numeric_literal(c, right)) {
        *rt = check_expr(c, right, 0);
        *lt = check_expr(c, left, *rt);
        return;
    }
    *lt = check_expr(c, left, expected);
    ludo_type_id hint = *lt;
    if (op == LUDO_TOK_STAR && tkind(c, *lt) == LUDO_TYPE_VEC2) {
        hint = simple(LUDO_TYPE_F32);
    } else if (op == LUDO_TOK_STAR && is_numeric_distinct(c, *lt)) {
        hint = type_at(c, *lt)->inner;
    }
    *rt = check_expr(c, right, hint);
}

static ludo_type_id check_binary(checker *c, ludo_node_id id, ludo_type_id expected) {
    ludo_node_id left = first(c, id);
    ludo_node_id right = next(c, left);
    ludo_token_kind op = token_kind(c, id);
    ludo_type_id lt;
    ludo_type_id rt;
    if (op == LUDO_TOK_PLUS || op == LUDO_TOK_MINUS || op == LUDO_TOK_STAR ||
        op == LUDO_TOK_SLASH || op == LUDO_TOK_PERCENT) {
        operands(c, left, right, expected, op, &lt, &rt);
        if ((op == LUDO_TOK_SLASH || op == LUDO_TOK_PERCENT) && !soft(c, lt) &&
            !is_numeric(c, lt)) {
            report(c, id, "bootstrap:check-operator", "this operator is not defined on these types",
                   id);
            return simple(LUDO_TYPE_INVALID);
        }
        return arithmetic(c, id, op, lt, rt);
    }
    if (op == LUDO_TOK_EQ_EQ || op == LUDO_TOK_BANG_EQ || op == LUDO_TOK_LT ||
        op == LUDO_TOK_LT_EQ || op == LUDO_TOK_GT || op == LUDO_TOK_GT_EQ) {
        operands(c, left, right, 0, op, &lt, &rt);
        if (soft(c, lt) || soft(c, rt)) {
            return simple(LUDO_TYPE_BOOL);
        }
        if (lt != rt) {
            report(c, id, "bootstrap:check-mismatch", "the two sides have different types", id);
            return simple(LUDO_TYPE_BOOL);
        }
        if (!comparable(c, lt, op != LUDO_TOK_EQ_EQ && op != LUDO_TOK_BANG_EQ)) {
            (void)pending(c); /* equality on aggregates is not this band's */
        }
        return simple(LUDO_TYPE_BOOL);
    }
    if (op == LUDO_TOK_KW_AND) {
        lt = check_expr(c, left, simple(LUDO_TYPE_BOOL));
        rt = check_expr(c, right, simple(LUDO_TYPE_BOOL));
        if (!assignable(c, lt, simple(LUDO_TYPE_BOOL)) ||
            !assignable(c, rt, simple(LUDO_TYPE_BOOL))) {
            report(c, id, "bootstrap:check-condition", "`and` takes bool operands", id);
        }
        return simple(LUDO_TYPE_BOOL);
    }
    if (op == LUDO_TOK_KW_OR) {
        lt = check_expr(c, left, 0);
        if (soft(c, lt)) {
            return lt;
        }
        if (tkind(c, lt) == LUDO_TYPE_OPTIONAL) {
            /* ch2 9.8: a or b with a: ?T and b: T yields T. */
            ludo_type_id inner = type_at(c, lt)->inner;
            rt = check_expr(c, right, inner);
            if (!assignable(c, rt, inner)) {
                report(c, right, "bootstrap:check-mismatch",
                       "the fallback of `or` must have the optional's inner type", right);
            }
            return inner;
        }
        rt = check_expr(c, right, simple(LUDO_TYPE_BOOL));
        if (tkind(c, lt) != LUDO_TYPE_BOOL || !assignable(c, rt, simple(LUDO_TYPE_BOOL))) {
            report(c, id, "bootstrap:check-condition", "`or` takes bool or ?T operands", id);
        }
        return simple(LUDO_TYPE_BOOL);
    }
    if (op == LUDO_TOK_AMP || op == LUDO_TOK_PIPE || op == LUDO_TOK_TILDE || op == LUDO_TOK_SHL ||
        op == LUDO_TOK_SHR) {
        operands(c, left, right, expected, op, &lt, &rt);
        if (soft(c, lt) || soft(c, rt)) {
            return soft(c, lt) ? lt : rt;
        }
        if (lt != rt || !is_integer(c, lt)) {
            report(c, id, "bootstrap:check-operator", "bitwise operators take matching integers",
                   id);
            return simple(LUDO_TYPE_INVALID);
        }
        return lt;
    }
    return pending(c); /* ..< and >.. yield Range and RevRange: band 3's */
}

static ludo_node_id find_field(const checker *c, ludo_node_id decl, ludo_node_id name,
                               uint32_t *index) {
    ludo_node_id body = child_of_kind(c, decl, LUDO_AST_STRUCT);
    uint32_t i = 0;
    for (ludo_node_id it = first(c, body); it != 0; it = next(c, it), i++) {
        if (same_name(c, it, name)) {
            *index = i;
            return it;
        }
    }
    return 0;
}

static ludo_node_id find_variant(const checker *c, ludo_node_id decl, ludo_node_id name,
                                 uint32_t *index) {
    ludo_node_id body = child_of_kind(c, decl, LUDO_AST_ENUM);
    uint32_t i = 0;
    for (ludo_node_id it = first(c, body); it != 0; it = next(c, it), i++) {
        if (same_name(c, it, name)) {
            *index = i;
            return it;
        }
    }
    return 0;
}

/* A field or component of a value of type receiver, named by the member
   node's token. */
static ludo_type_id select_member(checker *c, ludo_node_id id, ludo_type_id receiver) {
    ludo_type_kind kind = tkind(c, receiver);
    if (soft(c, receiver)) {
        return receiver;
    }
    if (kind == LUDO_TYPE_STRUCT) {
        uint32_t index = 0;
        ludo_node_id field = find_field(c, type_at(c, receiver)->decl, id, &index);
        if (field == 0) {
            return pending(c); /* not a field: a UFCS call, band 2's (ch2 13.1) */
        }
        c->facts[id].ref = LUDO_REF_FIELD;
        c->facts[id].target = field;
        c->facts[id].index = index;
        return c->facts[field].type;
    }
    if (kind == LUDO_TYPE_VEC2 && (named(c, id, "x") || named(c, id, "y"))) {
        c->facts[id].ref = LUDO_REF_COMPONENT;
        c->facts[id].index = named(c, id, "x") ? 0u : 1u;
        return simple(LUDO_TYPE_F32);
    }
    if (kind == LUDO_TYPE_OPTIONAL) {
        /* ch2 9.10: ?T is never used where T is required. */
        report(c, id, "bootstrap:check-optional-as-value",
               "this is ?T: handle the absence with `?.`, `or` or a bind", id);
        return simple(LUDO_TYPE_INVALID);
    }
    /* .len, UFCS, storage methods, and whatever a constraint provides (ch2
       5.5) are later bands'. */
    return pending(c);
}

/* The receiver is a type name: Shape.point, or Shape.circle as a callee. */
static ludo_node_id type_receiver(checker *c, ludo_node_id receiver) {
    if (kind_of(c, receiver) != LUDO_AST_IDENT || lookup_local(c, receiver) != NULL) {
        return 0;
    }
    ludo_node_id decl = lookup_decl(c, receiver);
    return kind_of(c, decl) == LUDO_AST_TYPE_DECL ? decl : 0;
}

/* ch2 4.6.1: a variant is constructed qualified by its type. */
static ludo_type_id check_variant_ref(checker *c, ludo_node_id id, ludo_node_id decl, bool called) {
    ludo_type_id type = c->facts[decl].type;
    if (soft(c, type) || tkind(c, type) != LUDO_TYPE_ENUM) {
        return pending(c); /* associated functions are band 2's */
    }
    uint32_t index = 0;
    ludo_node_id variant = find_variant(c, decl, id, &index);
    if (variant == 0) {
        report(c, id, "bootstrap:check-unknown-variant", "the type declares no such variant", id);
        return simple(LUDO_TYPE_INVALID);
    }
    c->facts[id].ref = LUDO_REF_VARIANT;
    c->facts[id].target = variant;
    c->facts[id].index = index;
    if (!called && first(c, variant) != 0) {
        report(c, id, "bootstrap:check-arity", "this variant carries a payload", id);
        return simple(LUDO_TYPE_INVALID);
    }
    return type;
}

static ludo_type_id check_member(checker *c, ludo_node_id id) {
    ludo_node_id receiver = first(c, id);
    if (kind_of(c, receiver) == LUDO_AST_ROOT) {
        return pending(c); /* $.vec2 is only ever called; the rest is band 6's */
    }
    ludo_node_id decl = type_receiver(c, receiver);
    if (decl != 0) {
        c->facts[receiver].ref = LUDO_REF_DECL;
        c->facts[receiver].target = decl;
        return check_variant_ref(c, id, decl, false);
    }
    if (kind_of(c, receiver) == LUDO_AST_IDENT && lookup_local(c, receiver) == NULL &&
        lookup_decl(c, receiver) == 0 && c->unresolved_library) {
        return pending(c); /* vec_utils.EPSILON: the name a library claims is band 4's */
    }
    ludo_type_id type = check_expr(c, receiver, 0);
    if (c->chain[receiver] && !soft(c, type)) {
        /* Inside a ?. chain the link applies to the present value and the
           whole chain stays optional (ch2 9.8). */
        ludo_type_id link = select_member(c, id, type_at(c, type)->inner);
        c->chain[id] = true;
        return soft(c, link) ? link : wrap(c, LUDO_TYPE_OPTIONAL, link);
    }
    return select_member(c, id, type);
}

static ludo_type_id check_optional_member(checker *c, ludo_node_id id) {
    ludo_node_id receiver = first(c, id);
    ludo_type_id type = check_expr(c, receiver, 0);
    if (soft(c, type)) {
        return type;
    }
    if (tkind(c, type) != LUDO_TYPE_OPTIONAL) {
        report(c, id, "bootstrap:check-mismatch", "`?.` needs a ?T on its left", id);
        return simple(LUDO_TYPE_INVALID);
    }
    c->chain[id] = true;
    ludo_type_id link = select_member(c, id, type_at(c, type)->inner);
    return soft(c, link) ? link : wrap(c, LUDO_TYPE_OPTIONAL, link);
}

/* The root binding of a place: ident, through member, index and group. */
static ludo_node_id place_root(const checker *c, ludo_node_id id, bool *marked) {
    for (;;) {
        ludo_ast_kind kind = kind_of(c, id);
        if (kind == LUDO_AST_MARK) {
            *marked = true;
        } else if (kind == LUDO_AST_IDENT) {
            return id;
        } else if (kind != LUDO_AST_MEMBER && kind != LUDO_AST_BRACKET && kind != LUDO_AST_GROUP &&
                   kind != LUDO_AST_DEREF) {
            return 0;
        }
        id = first(c, id);
    }
}

/* A mark (ch1 7.10) rides a root binding declared with ! (ch2 12.2). */
static ludo_type_id check_mark(checker *c, ludo_node_id id) {
    ludo_node_id operand = first(c, id);
    ludo_type_id type = check_expr(c, operand, 0);
    if (soft(c, type)) {
        return type;
    }
    local *entry = kind_of(c, operand) == LUDO_AST_IDENT ? lookup_local(c, operand) : NULL;
    if (entry == NULL) {
        return pending(c); /* persist and other roots are later bands' */
    }
    if (!entry->mutable_place) {
        report(c, id, "bootstrap:check-immutable",
               "the mark needs a binding or parameter declared with !", operand);
    }
    return type;
}

static ludo_type_id check_args(checker *c, ludo_node_id call, ludo_node_id first_arg,
                               const ludo_type *fn, ludo_node_id params_node) {
    size_t count = 0;
    for (ludo_node_id it = first_arg; it != 0; it = next(c, it)) {
        count++;
    }
    if (count != fn->param_count) {
        if (count < fn->param_count) {
            return pending(c); /* parameter defaults (ch1 5.9) are band 2's */
        }
        report(c, call, "bootstrap:check-arity", "this call passes too many arguments", call);
        return simple(LUDO_TYPE_INVALID);
    }
    ludo_node_id param = first(c, params_node);
    size_t i = 0;
    for (ludo_node_id it = first_arg; it != 0; it = next(c, it), i++) {
        ludo_type_id want = fn->params[i];
        ludo_type_id got = check_expr(c, it, want);
        if (!assignable(c, got, want)) {
            report(c, it, "bootstrap:check-mismatch", "this argument has the wrong type", it);
        }
        /* ch1 7.10: a ! parameter is passed a marked place. */
        if (param != 0) {
            ludo_node_id type_node = first(c, param);
            bool marked = false;
            if (kind_of(c, type_node) == LUDO_AST_MUTABLE_TYPE &&
                (place_root(c, it, &marked) == 0 || !marked)) {
                report(c, it, "bootstrap:check-unmarked-mutation",
                       "a ! parameter takes a marked place: write the mark after the root", it);
            }
            param = next(c, param);
        }
    }
    return fn->inner;
}

static ludo_type_id check_generic_call(checker *c, ludo_node_id call, ludo_node_id callee,
                                       ludo_node_id decl) {
    ludo_node_id generics = child_of_kind(c, decl, LUDO_AST_GENERIC_PARAMS);
    ludo_type_id bound[16] = {0};
    size_t count = child_count(c, generics);
    const ludo_type *fn = type_at(c, c->facts[decl].type);
    if (count > sizeof bound / sizeof bound[0] || soft(c, c->facts[decl].type)) {
        return pending(c);
    }
    size_t arguments = child_count(c, call) - 1;
    if (arguments != fn->param_count) {
        return arguments < fn->param_count ? pending(c) : simple(LUDO_TYPE_INVALID);
    }
    size_t i = 0;
    for (ludo_node_id it = next(c, callee); it != 0; it = next(c, it), i++) {
        ludo_type_id want = type_at(c, c->facts[decl].type)->params[i];
        ludo_type_id got = check_expr(c, it, mentions_param(c, want) ? 0 : want);
        if (!unify(c, want, got, generics, bound)) {
            report(c, it, "bootstrap:check-mismatch", "this argument does not fit the generic", it);
            return simple(LUDO_TYPE_INVALID);
        }
    }
    i = 0;
    for (ludo_node_id it = first(c, generics); it != 0; it = next(c, it), i++) {
        if (bound[i] == 0) {
            report(c, call, "bootstrap:check-uninferred", "no argument fixes this type parameter",
                   it);
            return simple(LUDO_TYPE_INVALID);
        }
        if (soft(c, bound[i])) {
            return bound[i];
        }
    }
    ludo_type_id signature = substitute(c, c->facts[decl].type, generics, bound);
    c->facts[call].instance = instantiate(c, decl, bound, count, signature);
    return type_at(c, signature)->inner;
}

static ludo_type_id check_call(checker *c, ludo_node_id id, ludo_type_id expected) {
    ludo_node_id callee = first(c, id);
    ludo_node_id args = next(c, callee);
    if (kind_of(c, callee) == LUDO_AST_MEMBER) {
        ludo_node_id receiver = first(c, callee);
        if (kind_of(c, receiver) == LUDO_AST_ROOT && named(c, callee, "vec2")) {
            /* ch6 3.5: the positional constructor. */
            static const ludo_type_id components[] = {LUDO_TYPE_ID_OF(LUDO_TYPE_F32),
                                                      LUDO_TYPE_ID_OF(LUDO_TYPE_F32)};
            ludo_type ctor = {.kind = LUDO_TYPE_FN,
                              .inner = simple(LUDO_TYPE_VEC2),
                              .params = components,
                              .param_count = 2};
            c->facts[callee].ref = LUDO_REF_VEC2_NEW;
            return check_args(c, id, args, &ctor, 0);
        }
        ludo_node_id decl = type_receiver(c, receiver);
        if (decl != 0) {
            c->facts[receiver].ref = LUDO_REF_DECL;
            c->facts[receiver].target = decl;
            ludo_type_id type = check_variant_ref(c, callee, decl, true);
            if (soft(c, type)) {
                return type;
            }
            ludo_node_id variant = c->facts[callee].target;
            ludo_type_id payload[16];
            size_t count = 0;
            for (ludo_node_id it = first(c, variant); it != 0; it = next(c, it)) {
                if (count == sizeof payload / sizeof payload[0]) {
                    return pending(c);
                }
                payload[count++] = c->facts[it].type;
            }
            ludo_type ctor = {
                .kind = LUDO_TYPE_FN, .inner = type, .params = payload, .param_count = count};
            return check_args(c, id, args, &ctor, 0);
        }
        return pending(c); /* $.panic, UFCS, associated functions: later bands */
    }
    if (kind_of(c, callee) == LUDO_AST_IDENT && lookup_local(c, callee) == NULL &&
        lookup_decl(c, callee) == 0 && named(c, callee, "some")) {
        /* ch2 9.2: the prelude constructor, written explicitly. */
        if (args == 0 || next(c, args) != 0) {
            report(c, id, "bootstrap:check-arity", "`some` takes exactly one value", callee);
            return simple(LUDO_TYPE_INVALID);
        }
        ludo_type_id inner = expected != 0 && tkind(c, expected) == LUDO_TYPE_OPTIONAL
                                 ? type_at(c, expected)->inner
                                 : 0;
        ludo_type_id got = check_expr(c, args, inner);
        c->facts[callee].ref = LUDO_REF_VARIANT;
        c->facts[callee].index = 1;
        return soft(c, got) ? got : wrap(c, LUDO_TYPE_OPTIONAL, got);
    }
    if (kind_of(c, callee) == LUDO_AST_IDENT && lookup_local(c, callee) == NULL) {
        ludo_node_id decl = lookup_decl(c, callee);
        if (kind_of(c, decl) == LUDO_AST_FN) {
            c->facts[callee].ref = LUDO_REF_DECL;
            c->facts[callee].target = decl;
            if (child_of_kind(c, decl, LUDO_AST_GENERIC_PARAMS) != 0) {
                return check_generic_call(c, id, callee, decl);
            }
            if (soft(c, c->facts[decl].type)) {
                return c->facts[decl].type;
            }
            c->facts[callee].type = c->facts[decl].type;
            return check_args(c, id, args, type_at(c, c->facts[decl].type),
                              child_of_kind(c, decl, LUDO_AST_PARAMS));
        }
        if (decl == 0 && in_later_prelude(c, callee)) {
            return pending(c); /* format(...) */
        }
    }
    if (kind_of(c, callee) != LUDO_AST_IDENT) {
        return pending(c); /* calls through brackets, groups: later bands */
    }
    /* ch2 7.1: a function value is a non-capturing pointer, called directly. */
    ludo_type_id type = check_expr(c, callee, 0);
    if (soft(c, type)) {
        return type;
    }
    if (tkind(c, type) != LUDO_TYPE_FN) {
        report(c, callee, "bootstrap:check-not-callable", "this is not a function", callee);
        return simple(LUDO_TYPE_INVALID);
    }
    return check_args(c, id, args, type_at(c, type), 0);
}

static ludo_type_id check_unary(checker *c, ludo_node_id id, ludo_type_id expected) {
    ludo_node_id operand = first(c, id);
    ludo_token_kind op = token_kind(c, id);
    if (op == LUDO_TOK_KW_NOT) {
        ludo_type_id type = check_expr(c, operand, simple(LUDO_TYPE_BOOL));
        if (!assignable(c, type, simple(LUDO_TYPE_BOOL))) {
            report(c, id, "bootstrap:check-condition", "`not` takes a bool", id);
        }
        return simple(LUDO_TYPE_BOOL);
    }
    if (op == LUDO_TOK_MINUS || op == LUDO_TOK_TILDE) {
        ludo_type_id type = check_expr(c, operand, expected);
        if (soft(c, type)) {
            return type;
        }
        ludo_type_kind k = tkind(c, type);
        bool signed_int = k >= LUDO_TYPE_I8 && k <= LUDO_TYPE_I64;
        bool ok = op == LUDO_TOK_MINUS ? signed_int || is_float(c, type) ||
                                             is_numeric_distinct(c, type) || k == LUDO_TYPE_VEC2
                                       : is_integer(c, type);
        if (!ok) {
            report(c, id, "bootstrap:check-operator", "this operator is not defined on this type",
                   id);
            return simple(LUDO_TYPE_INVALID);
        }
        return type;
    }
    return pending(c); /* &x is unsafe surface, band 3's */
}

static ludo_type_id check_expr_kind(checker *c, ludo_node_id id, ludo_type_id expected) {
    switch (kind_of(c, id)) {
    case LUDO_AST_LITERAL:
        return check_literal(c, id, expected);
    case LUDO_AST_IDENT:
        return check_ident(c, id, expected);
    case LUDO_AST_GROUP:
        return check_expr(c, first(c, id), expected);
    case LUDO_AST_BINARY:
        return check_binary(c, id, expected);
    case LUDO_AST_UNARY:
        return check_unary(c, id, expected);
    case LUDO_AST_MEMBER:
        return check_member(c, id);
    case LUDO_AST_OPTIONAL_MEMBER:
        return check_optional_member(c, id);
    case LUDO_AST_CALL:
        return check_call(c, id, expected);
    case LUDO_AST_MARK:
        return check_mark(c, id);
    case LUDO_AST_ERROR:
        return simple(LUDO_TYPE_INVALID); /* reported by the parser */
    case LUDO_AST_CAST:
    case LUDO_AST_BRACKET:
    case LUDO_AST_DEREF:
    case LUDO_AST_AGGREGATE:
    case LUDO_AST_RESCUE:
    case LUDO_AST_ROOT:
    case LUDO_AST_DISCARD:
    case LUDO_AST_NAMED_ITEM:
    case LUDO_AST_KEYED_ITEM:
    case LUDO_AST_HANDLER:
    case LUDO_AST_FALLIBLE_TYPE:
    case LUDO_AST_MODULE:
    case LUDO_AST_EXPLICIT:
    case LUDO_AST_LIBRARY:
    case LUDO_AST_USE:
    case LUDO_AST_ATTRIBUTE:
    case LUDO_AST_TYPE_DECL:
    case LUDO_AST_CONST:
    case LUDO_AST_FN:
    case LUDO_AST_EXTERN_FN:
    case LUDO_AST_EXTERN_ITEM:
    case LUDO_AST_PERSIST:
    case LUDO_AST_FRAME:
    case LUDO_AST_IMPL:
    case LUDO_AST_GENERIC_PARAMS:
    case LUDO_AST_GENERIC_PARAM:
    case LUDO_AST_CONSTRAINTS:
    case LUDO_AST_PARAMS:
    case LUDO_AST_PARAM:
    case LUDO_AST_RETURNS:
    case LUDO_AST_STRUCT:
    case LUDO_AST_FIELD:
    case LUDO_AST_ENUM:
    case LUDO_AST_VARIANT:
    case LUDO_AST_DISTINCT:
    case LUDO_AST_INTERFACE:
    case LUDO_AST_FN_TYPE:
    case LUDO_AST_NAMED_TYPE:
    case LUDO_AST_GENERIC_ARGS:
    case LUDO_AST_OPTIONAL_TYPE:
    case LUDO_AST_POINTER_TYPE:
    case LUDO_AST_VIEW_TYPE:
    case LUDO_AST_ARRAY_TYPE:
    case LUDO_AST_MUTABLE_TYPE:
    case LUDO_AST_BLOCK:
    case LUDO_AST_LET:
    case LUDO_AST_BINDINGS:
    case LUDO_AST_ASSIGN:
    case LUDO_AST_COMPOUND_ASSIGN:
    case LUDO_AST_EXPR_STMT:
    case LUDO_AST_TAIL:
    case LUDO_AST_RETURN:
    case LUDO_AST_BREAK:
    case LUDO_AST_CONTINUE:
    case LUDO_AST_DEFER:
    case LUDO_AST_MODIFIER:
    case LUDO_AST_IF:
    case LUDO_AST_BRANCH:
    case LUDO_AST_WHILE:
    case LUDO_AST_FOR:
    case LUDO_AST_BIND_CONDITION:
    case LUDO_AST_UNSAFE:
    case LUDO_AST_MATCH:
    case LUDO_AST_ARM:
    case LUDO_AST_PATTERN_IDENT:
    case LUDO_AST_PATTERN_VARIANT:
    case LUDO_AST_PATTERN_STRUCT:
    case LUDO_AST_PATTERN_FIELD:
    case LUDO_AST_PATTERN_LITERAL:
    case LUDO_AST_PATTERN_DISCARD:
        /* Casts, indexing, aggregates and rescue are later bands'. */
        return pending(c);
    }
    return pending(c);
}

static ludo_type_id check_expr(checker *c, ludo_node_id id, ludo_type_id expected) {
    ludo_type_id type = check_expr_kind(c, id, expected);
    c->facts[id].type = type;
    return type;
}

/* ---- Statements ----------------------------------------------------------- */

static void check_block(checker *c, ludo_node_id block);
static void check_statement(checker *c, ludo_node_id id);

static bool is_panic_call(const checker *c, ludo_node_id id) {
    if (kind_of(c, id) != LUDO_AST_CALL) {
        return false;
    }
    ludo_node_id callee = first(c, id);
    return kind_of(c, callee) == LUDO_AST_MEMBER && kind_of(c, first(c, callee)) == LUDO_AST_ROOT &&
           named(c, callee, "panic");
}

static bool block_diverges(const checker *c, ludo_node_id block);

/* ch1 6.7.2: structural, never a walk into a nested block for a value. */
static bool diverges(const checker *c, ludo_node_id id) {
    ludo_ast_kind kind = kind_of(c, id);
    if (kind == LUDO_AST_RETURN || kind == LUDO_AST_ERROR) {
        return true; /* an error node was reported by the parser; not twice */
    }
    if (kind == LUDO_AST_EXPR_STMT) {
        return is_panic_call(c, first(c, id));
    }
    if (kind == LUDO_AST_IF) {
        bool has_else = false;
        for (ludo_node_id branch = first(c, id); branch != 0; branch = next(c, branch)) {
            ludo_node_id head = first(c, branch);
            has_else = kind_of(c, head) == LUDO_AST_BLOCK;
            if (!block_diverges(c, has_else ? head : next(c, head))) {
                return false;
            }
        }
        return has_else;
    }
    if (kind == LUDO_AST_MATCH) {
        bool any = false;
        for (ludo_node_id arm = next(c, first(c, id)); arm != 0; arm = next(c, arm)) {
            any = true;
            if (!block_diverges(c, next(c, first(c, arm)))) {
                return false;
            }
        }
        return any;
    }
    return false; /* a loop never diverges, whatever its condition */
}

static bool block_diverges(const checker *c, ludo_node_id block) {
    ludo_node_id last = 0;
    for (ludo_node_id it = first(c, block); it != 0; it = next(c, it)) {
        last = it;
    }
    return last != 0 && diverges(c, last);
}

static void check_condition(checker *c, ludo_node_id id) {
    if (kind_of(c, id) == LUDO_AST_BIND_CONDITION) {
        (void)pending(c); /* e := expr binds: band 2's absence handling */
        return;
    }
    ludo_type_id type = check_expr(c, id, simple(LUDO_TYPE_BOOL));
    if (!assignable(c, type, simple(LUDO_TYPE_BOOL))) {
        /* ch2 16.4: conditions are bool only, no truthiness. */
        report(c, id, "bootstrap:check-condition", "a condition must be bool", id);
    }
}

static void check_let(checker *c, ludo_node_id id) {
    ludo_node_id bindings = first(c, id);
    ludo_node_id type_node = next(c, bindings);
    ludo_node_id init = next(c, type_node);
    if (init == 0) {
        init = type_node;
        type_node = 0;
    }
    if (child_count(c, bindings) != 1) {
        (void)check_expr(c, init, 0);
        (void)pending(c); /* destructuring a multiple return is band 3's */
        return;
    }
    ludo_node_id name = first(c, bindings);
    ludo_type_id type;
    bool mutable_place = false;
    if (type_node != 0) {
        mutable_place = kind_of(c, type_node) == LUDO_AST_MUTABLE_TYPE;
        type = resolve_type(c, type_node);
        ludo_type_id got = check_expr(c, init, type);
        if (!assignable(c, got, type)) {
            report(c, init, "bootstrap:check-mismatch",
                   "the initialiser does not have the annotated type", name);
        }
    } else {
        type = check_expr(c, init, 0);
        if (tkind(c, type) == LUDO_TYPE_UNIT) {
            report(c, init, "bootstrap:check-mismatch", "this call returns nothing to bind", name);
            type = simple(LUDO_TYPE_INVALID);
        }
    }
    if (kind_of(c, name) == LUDO_AST_IDENT) {
        bind(c, name, type, mutable_place, false);
    }
}

/* ch2 12.2.1: a local is rebound whole without a mark; writing into a place
   needs the mark at its root, and a parameter is never rebound. */
static ludo_type_id check_target(checker *c, ludo_node_id target) {
    if (kind_of(c, target) == LUDO_AST_IDENT) {
        ludo_type_id type = check_expr(c, target, 0);
        local *entry = lookup_local(c, target);
        if (entry != NULL && entry->parameter) {
            report(c, target, "bootstrap:check-immutable",
                   "a parameter is never rebound; a ! parameter is written through its mark",
                   target);
        } else if (entry == NULL && !soft(c, type)) {
            report(c, target, "bootstrap:check-immutable", "only a local binding is rebound",
                   target);
        }
        return type;
    }
    bool marked = false;
    ludo_node_id root = place_root(c, target, &marked);
    ludo_type_id type = check_expr(c, target, 0);
    if (soft(c, type)) {
        return type;
    }
    if (root == 0) {
        report(c, target, "bootstrap:check-not-a-place", "this cannot be assigned to", target);
    } else if (!marked) {
        report(c, target, "bootstrap:check-unmarked-mutation",
               "writing into a place needs the mark after its root", root);
    }
    return type;
}

static void check_assign(checker *c, ludo_node_id id) {
    ludo_node_id target = first(c, id);
    ludo_node_id value = next(c, target);
    if (kind_of(c, target) == LUDO_AST_DISCARD) {
        (void)check_expr(c, value, 0);
        (void)pending(c); /* must-use discharge is band 2's */
        return;
    }
    ludo_type_id type = check_target(c, target);
    if (kind_of(c, id) == LUDO_AST_COMPOUND_ASSIGN) {
        static const ludo_token_kind ops[] = {LUDO_TOK_PLUS, LUDO_TOK_MINUS, LUDO_TOK_STAR,
                                              LUDO_TOK_SLASH, LUDO_TOK_PERCENT};
        ludo_token_kind op = token_kind(c, id);
        size_t index = (size_t)op - (size_t)LUDO_TOK_PLUS_EQ;
        ludo_type_id hint = op == LUDO_TOK_STAR_EQ && tkind(c, type) == LUDO_TYPE_VEC2
                                ? simple(LUDO_TYPE_F32)
                                : type;
        ludo_type_id got = check_expr(c, value, hint);
        if (index < sizeof ops / sizeof ops[0]) {
            (void)arithmetic(c, id, ops[index], type, got); /* ch1 6.5: a = a op b */
        }
        return;
    }
    ludo_type_id got = check_expr(c, value, type);
    if (!assignable(c, got, type)) {
        report(c, value, "bootstrap:check-mismatch", "the value does not have the place's type",
               value);
    }
}

static void check_return(checker *c, ludo_node_id id) {
    ludo_node_id value = first(c, id);
    if (value == 0) {
        if (tkind(c, c->result) != LUDO_TYPE_UNIT && !soft(c, c->result)) {
            report(c, id, "bootstrap:check-mismatch", "this function returns a value", id);
        }
        return;
    }
    if (next(c, value) != 0) {
        (void)pending(c); /* multiple returns are band 3's */
        return;
    }
    ludo_type_id got = check_expr(c, value, c->result);
    if (!assignable(c, got, c->result)) {
        report(c, value, "bootstrap:check-mismatch", "this is not the function's return type",
               value);
    }
}

/* ---- Patterns and match --------------------------------------------------- */

typedef struct coverage {
    bool all;
    bool refutable_nested; /* a payload pattern that could fail: not decided here */
    uint64_t tags;
} coverage;

static void variant_payload(checker *c, ludo_type_id scrutinee, ludo_node_id variant,
                            ludo_type_id *payload, size_t *count) {
    *count = 0;
    if (tkind(c, scrutinee) == LUDO_TYPE_OPTIONAL) {
        payload[(*count)++] = type_at(c, scrutinee)->inner;
        return;
    }
    for (ludo_node_id it = first(c, variant); it != 0 && *count < 16; it = next(c, it)) {
        payload[(*count)++] = c->facts[it].type;
    }
}

static size_t variant_count(const checker *c, ludo_type_id scrutinee) {
    if (tkind(c, scrutinee) == LUDO_TYPE_OPTIONAL) {
        return 2;
    }
    return child_count(c, child_of_kind(c, type_at(c, scrutinee)->decl, LUDO_AST_ENUM));
}

/* The variant a pattern names in the scrutinee's type: none and some for ?T
   (ch2 9.2), otherwise the enum's own. */
static ludo_node_id pattern_variant(const checker *c, ludo_type_id scrutinee, ludo_node_id name,
                                    uint32_t *index, bool *found) {
    *found = false;
    if (tkind(c, scrutinee) == LUDO_TYPE_OPTIONAL) {
        if (named(c, name, "none") || named(c, name, "some")) {
            *found = true;
            *index = named(c, name, "none") ? 0u : 1u;
        }
        return 0;
    }
    ludo_node_id variant = find_variant(c, type_at(c, scrutinee)->decl, name, index);
    *found = variant != 0;
    return variant;
}

static bool sum_type(const checker *c, ludo_type_id type) {
    return tkind(c, type) == LUDO_TYPE_ENUM || tkind(c, type) == LUDO_TYPE_OPTIONAL;
}

/* Binds what a pattern binds and returns whether it can fail. */
static bool check_pattern(checker *c, ludo_node_id id, ludo_type_id type) {
    c->facts[id].type = type;
    ludo_ast_kind kind = kind_of(c, id);
    if (kind == LUDO_AST_PATTERN_DISCARD) {
        return false;
    }
    if (kind == LUDO_AST_PATTERN_IDENT) {
        uint32_t index = 0;
        bool found = false;
        ludo_node_id variant = sum_type(c, type) ? pattern_variant(c, type, id, &index, &found) : 0;
        if (!found) {
            bind(c, id, type, false, false);
            return false;
        }
        /* ch2 4.6: the scrutinee's type declares it, so it is a variant. */
        bool payload = tkind(c, type) == LUDO_TYPE_OPTIONAL ? index == 1 : first(c, variant) != 0;
        if (payload) {
            report(c, id, "bootstrap:check-arity", "this variant carries a payload", id);
        }
        c->facts[id].ref = LUDO_REF_VARIANT;
        c->facts[id].target = variant;
        c->facts[id].index = index;
        return true;
    }
    if (kind != LUDO_AST_PATTERN_VARIANT) {
        if (kind != LUDO_AST_ERROR) {
            (void)pending(c); /* literal and struct patterns are band 3's */
        }
        return true;
    }
    uint32_t index = 0;
    bool found = false;
    if (!sum_type(c, type)) {
        if (!soft(c, type)) {
            report(c, id, "bootstrap:check-mismatch", "a variant pattern on a non-sum type", id);
        }
        return true;
    }
    ludo_node_id variant = pattern_variant(c, type, id, &index, &found);
    if (!found) {
        report(c, id, "bootstrap:check-unknown-variant", "the type declares no such variant", id);
        return true;
    }
    c->facts[id].ref = LUDO_REF_VARIANT;
    c->facts[id].target = variant;
    c->facts[id].index = index;
    ludo_type_id payload[16];
    size_t count = 0;
    variant_payload(c, type, variant, payload, &count);
    if (tkind(c, type) == LUDO_TYPE_OPTIONAL && index == 0) {
        count = 0;
    }
    if (child_count(c, id) != count) {
        report(c, id, "bootstrap:check-arity", "the pattern does not match the payload", id);
        return true;
    }
    size_t i = 0;
    bool refutable = false;
    for (ludo_node_id it = first(c, id); it != 0; it = next(c, it), i++) {
        refutable = check_pattern(c, it, payload[i]) || refutable;
    }
    c->nested[id] = refutable;
    return true;
}

static void name_uncovered(checker *c, ludo_node_id match, ludo_type_id type, uint64_t tags) {
    if (tkind(c, type) == LUDO_TYPE_OPTIONAL) {
        report(c, match, "bootstrap:check-non-exhaustive", "the match does not cover a variant", 0);
        return;
    }
    uint32_t index = 0;
    ludo_node_id body = child_of_kind(c, type_at(c, type)->decl, LUDO_AST_ENUM);
    for (ludo_node_id it = first(c, body); it != 0; it = next(c, it), index++) {
        if (index < 64 && (tags & ((uint64_t)1 << index)) == 0) {
            /* ch2 4.3: the error names the uncovered variants, one each. */
            report(c, match, "bootstrap:check-non-exhaustive",
                   "the match does not cover this variant", it);
        }
    }
}

static void check_match(checker *c, ludo_node_id id) {
    ludo_node_id scrutinee = first(c, id);
    ludo_type_id type = check_expr(c, scrutinee, 0);
    bool decided = sum_type(c, type);
    if (!decided && !soft(c, type)) {
        (void)pending(c); /* matching on integers or structs is band 3's */
    }
    coverage covered = {0};
    for (ludo_node_id arm = next(c, scrutinee); arm != 0; arm = next(c, arm)) {
        ludo_node_id pattern = first(c, arm);
        size_t mark = c->local_count;
        bool refutable = check_pattern(c, pattern, decided ? type : simple(LUDO_TYPE_PENDING));
        if (!refutable) {
            covered.all = true;
        } else if (c->facts[pattern].ref == LUDO_REF_VARIANT) {
            if (kind_of(c, pattern) == LUDO_AST_PATTERN_VARIANT && c->nested[pattern]) {
                covered.refutable_nested = true;
            } else if (c->facts[pattern].index < 64) {
                covered.tags |= (uint64_t)1 << c->facts[pattern].index;
            }
        }
        check_block(c, next(c, pattern));
        c->local_count = mark;
    }
    if (!decided || covered.all) {
        return;
    }
    size_t count = variant_count(c, type);
    uint64_t full = count >= 64 ? UINT64_MAX : (((uint64_t)1 << count) - 1);
    if ((covered.tags & full) != full) {
        if (covered.refutable_nested) {
            (void)pending(c); /* nested exhaustiveness is not decided in this band */
            return;
        }
        name_uncovered(c, id, type, covered.tags);
    }
}

static void check_for(checker *c, ludo_node_id id) {
    ludo_node_id bindings = first(c, id);
    ludo_node_id subject = next(c, bindings);
    ludo_node_id block = next(c, subject);
    ludo_type_id type = check_expr(c, subject, 0);
    size_t mark = c->local_count;
    ludo_type_id element = simple(LUDO_TYPE_PENDING);
    if (tkind(c, type) == LUDO_TYPE_VIEW && child_count(c, bindings) == 1 &&
        kind_of(c, subject) != LUDO_AST_MARK) {
        element = type_at(c, type)->inner;
    } else {
        (void)pending(c); /* lists, ranges, writable and indexed iteration: band 3 */
    }
    for (ludo_node_id it = first(c, bindings); it != 0; it = next(c, it)) {
        if (kind_of(c, it) == LUDO_AST_IDENT) {
            bind(c, it, element, false, false);
        }
    }
    c->loop_depth++;
    check_block(c, block);
    c->loop_depth--;
    c->local_count = mark;
}

static void check_if(checker *c, ludo_node_id id) {
    for (ludo_node_id branch = first(c, id); branch != 0; branch = next(c, branch)) {
        ludo_node_id head = first(c, branch);
        size_t mark = c->local_count;
        if (kind_of(c, head) != LUDO_AST_BLOCK) {
            check_condition(c, head);
            head = next(c, head);
        }
        check_block(c, head);
        c->local_count = mark;
    }
}

static void check_statement(checker *c, ludo_node_id id) {
    switch (kind_of(c, id)) {
    case LUDO_AST_LET:
        check_let(c, id);
        return;
    case LUDO_AST_ASSIGN:
    case LUDO_AST_COMPOUND_ASSIGN:
        check_assign(c, id);
        return;
    case LUDO_AST_EXPR_STMT:
        (void)check_expr(c, first(c, id), 0);
        return;
    case LUDO_AST_TAIL: {
        ludo_node_id value = first(c, id);
        ludo_type_id got = check_expr(c, value, c->result);
        if (!assignable(c, got, c->result)) {
            report(c, value, "bootstrap:check-mismatch", "this is not the function's return type",
                   value);
        }
        return;
    }
    case LUDO_AST_RETURN:
        check_return(c, id);
        return;
    case LUDO_AST_MODIFIER: {
        ludo_node_id statement = first(c, id);
        check_condition(c, next(c, statement));
        check_statement(c, statement);
        return;
    }
    case LUDO_AST_IF:
        check_if(c, id);
        return;
    case LUDO_AST_WHILE: {
        ludo_node_id condition = first(c, id);
        size_t mark = c->local_count;
        check_condition(c, condition);
        c->loop_depth++;
        check_block(c, next(c, condition));
        c->loop_depth--;
        c->local_count = mark;
        return;
    }
    case LUDO_AST_FOR:
        check_for(c, id);
        return;
    case LUDO_AST_MATCH:
        check_match(c, id);
        return;
    case LUDO_AST_BREAK:
    case LUDO_AST_CONTINUE:
        if (c->loop_depth == 0) {
            report(c, id, "bootstrap:check-no-loop", "this is not inside a loop", 0);
        }
        return;
    case LUDO_AST_ERROR:
        return;
    case LUDO_AST_DEFER:
    case LUDO_AST_UNSAFE:
    case LUDO_AST_BLOCK:
    case LUDO_AST_MODULE:
    case LUDO_AST_EXPLICIT:
    case LUDO_AST_LIBRARY:
    case LUDO_AST_USE:
    case LUDO_AST_ATTRIBUTE:
    case LUDO_AST_TYPE_DECL:
    case LUDO_AST_CONST:
    case LUDO_AST_FN:
    case LUDO_AST_EXTERN_FN:
    case LUDO_AST_EXTERN_ITEM:
    case LUDO_AST_PERSIST:
    case LUDO_AST_FRAME:
    case LUDO_AST_IMPL:
    case LUDO_AST_GENERIC_PARAMS:
    case LUDO_AST_GENERIC_PARAM:
    case LUDO_AST_CONSTRAINTS:
    case LUDO_AST_PARAMS:
    case LUDO_AST_PARAM:
    case LUDO_AST_RETURNS:
    case LUDO_AST_STRUCT:
    case LUDO_AST_FIELD:
    case LUDO_AST_ENUM:
    case LUDO_AST_VARIANT:
    case LUDO_AST_DISTINCT:
    case LUDO_AST_INTERFACE:
    case LUDO_AST_FN_TYPE:
    case LUDO_AST_NAMED_TYPE:
    case LUDO_AST_GENERIC_ARGS:
    case LUDO_AST_OPTIONAL_TYPE:
    case LUDO_AST_POINTER_TYPE:
    case LUDO_AST_VIEW_TYPE:
    case LUDO_AST_ARRAY_TYPE:
    case LUDO_AST_MUTABLE_TYPE:
    case LUDO_AST_FALLIBLE_TYPE:
    case LUDO_AST_BINDINGS:
    case LUDO_AST_BRANCH:
    case LUDO_AST_BIND_CONDITION:
    case LUDO_AST_ARM:
    case LUDO_AST_PATTERN_IDENT:
    case LUDO_AST_PATTERN_VARIANT:
    case LUDO_AST_PATTERN_STRUCT:
    case LUDO_AST_PATTERN_FIELD:
    case LUDO_AST_PATTERN_LITERAL:
    case LUDO_AST_PATTERN_DISCARD:
    case LUDO_AST_IDENT:
    case LUDO_AST_ROOT:
    case LUDO_AST_DISCARD:
    case LUDO_AST_LITERAL:
    case LUDO_AST_UNARY:
    case LUDO_AST_BINARY:
    case LUDO_AST_CAST:
    case LUDO_AST_MEMBER:
    case LUDO_AST_OPTIONAL_MEMBER:
    case LUDO_AST_CALL:
    case LUDO_AST_BRACKET:
    case LUDO_AST_MARK:
    case LUDO_AST_DEREF:
    case LUDO_AST_GROUP:
    case LUDO_AST_AGGREGATE:
    case LUDO_AST_NAMED_ITEM:
    case LUDO_AST_KEYED_ITEM:
    case LUDO_AST_RESCUE:
    case LUDO_AST_HANDLER:
        (void)pending(c); /* defer and unsafe are band 3's */
        return;
    }
}

static void check_block(checker *c, ludo_node_id block) {
    size_t mark = c->local_count;
    for (ludo_node_id it = first(c, block); it != 0 && !c->exhausted; it = next(c, it)) {
        check_statement(c, it);
    }
    c->local_count = mark;
}

/* ---- Declarations --------------------------------------------------------- */

/* Types first, so a signature may name a type declared below it; then the
   signatures and constants every body reads; then the bodies. */
typedef enum pass { PASS_TYPES, PASS_SIGNATURES, PASS_BODIES } pass;

/* ch4 12.2's floor, checked structurally: literals, constants, arithmetic,
   comparison, bitwise operators and casts, and nothing else. */
static bool foldable(checker *c, ludo_node_id id) {
    ludo_ast_kind kind = kind_of(c, id);
    if (kind == LUDO_AST_LITERAL || kind == LUDO_AST_ERROR) {
        return true;
    }
    if (kind == LUDO_AST_IDENT) {
        return kind_of(c, lookup_decl(c, id)) == LUDO_AST_CONST;
    }
    if (kind == LUDO_AST_CAST) {
        (void)pending(c); /* casts are band 2's */
        return true;
    }
    if (kind != LUDO_AST_GROUP && kind != LUDO_AST_UNARY && kind != LUDO_AST_BINARY) {
        return false;
    }
    for (ludo_node_id it = first(c, id); it != 0; it = next(c, it)) {
        if (!foldable(c, it)) {
            return false;
        }
    }
    return true;
}

/* ch1 5.6.1: the annotation, or the initialiser's own type. */
static void check_const(checker *c, ludo_node_id id) {
    ludo_node_id first_child = first(c, id);
    ludo_node_id init = first_child;
    ludo_type_id type = 0;
    if (next(c, first_child) != 0) {
        type = resolve_type(c, first_child);
        init = next(c, first_child);
    }
    ludo_type_id got = check_expr(c, init, type);
    if (type != 0 && !assignable(c, got, type)) {
        report(c, init, "bootstrap:check-mismatch",
               "the initialiser does not have the annotated type", id);
    }
    if (!foldable(c, init)) {
        report(c, init, "bootstrap:check-not-constant",
               "a const initialiser folds from literals, constants and operators only", id);
    }
    c->facts[id].type = type != 0 ? type : got;
}

static void check_fn_signature(checker *c, ludo_node_id id) {
    ludo_node_id generics = child_of_kind(c, id, LUDO_AST_GENERIC_PARAMS);
    c->generics = generics;
    for (ludo_node_id it = first(c, generics); it != 0; it = next(c, it)) {
        /* Constrained and value parameters are band 2's: what a constraint
           provides is what a body may use (ch2 5.5), and band 1 has none. */
        c->facts[it].type = first(c, it) != 0
                                ? pending(c)
                                : intern(c, (ludo_type){.kind = LUDO_TYPE_PARAM, .decl = it});
    }
    ludo_type_id params[16];
    size_t count = 0;
    for (ludo_node_id it = first(c, child_of_kind(c, id, LUDO_AST_PARAMS)); it != 0;
         it = next(c, it)) {
        if (count == sizeof params / sizeof params[0]) {
            c->facts[id].type = pending(c);
            return;
        }
        ludo_type_id type = resolve_type(c, first(c, it));
        c->facts[it].type = type;
        params[count++] = type;
    }
    ludo_node_id returns = child_of_kind(c, id, LUDO_AST_RETURNS);
    ludo_type_id result = simple(LUDO_TYPE_UNIT);
    if (returns != 0) {
        result = child_count(c, returns) == 1 ? resolve_type(c, first(c, returns)) : pending(c);
    }
    c->facts[id].type = intern(
        c,
        (ludo_type){.kind = LUDO_TYPE_FN, .inner = result, .params = params, .param_count = count});
}

static void check_fn_body(checker *c, ludo_node_id id) {
    ludo_node_id block = child_of_kind(c, id, LUDO_AST_BLOCK);
    c->generics = child_of_kind(c, id, LUDO_AST_GENERIC_PARAMS);
    c->result = type_at(c, c->facts[id].type)->kind == LUDO_TYPE_FN
                    ? type_at(c, c->facts[id].type)->inner
                    : simple(LUDO_TYPE_PENDING);
    c->local_count = 0;
    for (ludo_node_id it = first(c, child_of_kind(c, id, LUDO_AST_PARAMS)); it != 0;
         it = next(c, it)) {
        bind(c, it, c->facts[it].type, kind_of(c, first(c, it)) == LUDO_AST_MUTABLE_TYPE, true);
    }
    check_block(c, block);
    ludo_node_id last = 0;
    for (ludo_node_id it = first(c, block); it != 0; it = next(c, it)) {
        last = it;
    }
    if (tkind(c, c->result) != LUDO_TYPE_UNIT && !soft(c, c->result) &&
        kind_of(c, last) != LUDO_AST_TAIL && !(last != 0 && diverges(c, last))) {
        report(c, id, "bootstrap:check-missing-return",
               "this function can reach its end without returning a value", id);
    }
    c->local_count = 0;
}

static void check_type_decl(checker *c, ludo_node_id id, pass which) {
    ludo_node_id body = 0;
    for (ludo_node_id it = first(c, id); it != 0; it = next(c, it)) {
        body = it;
    }
    ludo_ast_kind kind = kind_of(c, body);
    if (child_of_kind(c, id, LUDO_AST_GENERIC_PARAMS) != 0 ||
        (kind != LUDO_AST_STRUCT && kind != LUDO_AST_ENUM && kind != LUDO_AST_DISTINCT)) {
        (void)pending(c); /* generic types, interfaces and aliases are band 2's */
        return;
    }
    if (which == PASS_TYPES && kind == LUDO_AST_DISTINCT) {
        (void)resolve_type(c, first(c, body));
        return;
    }
    if (which == PASS_TYPES && kind == LUDO_AST_ENUM) {
        for (ludo_node_id variant = first(c, body); variant != 0; variant = next(c, variant)) {
            for (ludo_node_id payload = first(c, variant); payload != 0;
                 payload = next(c, payload)) {
                (void)resolve_type(c, payload);
            }
        }
        return;
    }
    if (kind != LUDO_AST_STRUCT) {
        return;
    }
    for (ludo_node_id field = first(c, body); field != 0; field = next(c, field)) {
        if (which == PASS_TYPES) {
            c->facts[field].type = resolve_type(c, first(c, field));
            continue;
        }
        ludo_node_id fallback = next(c, first(c, field));
        if (fallback != 0) {
            /* ch1 5.3: a field default is an expression typed by the field. */
            ludo_type_id type = c->facts[field].type;
            ludo_type_id got = check_expr(c, fallback, type);
            if (!assignable(c, got, type)) {
                report(c, fallback, "bootstrap:check-mismatch",
                       "the default does not have the field's type", field);
            }
        }
    }
}

/* ch1 5.14: the classes are image, font, shader and storage. */
static void check_extern_item(checker *c, ludo_node_id id) {
    static const struct {
        const char *name;
        ludo_extern_class class_id;
    } classes[] = {{"image", LUDO_EXTERN_IMAGE},
                   {"font", LUDO_EXTERN_FONT},
                   {"shader", LUDO_EXTERN_SHADER},
                   {"storage", LUDO_EXTERN_STORAGE}};
    ludo_node_id class_name = first(c, id);
    for (size_t i = 0; i < sizeof classes / sizeof classes[0]; i++) {
        if (named(c, class_name, classes[i].name)) {
            c->facts[id].type = intern(
                c, (ludo_type){.kind = LUDO_TYPE_EXTERN, .extern_class = classes[i].class_id});
            return;
        }
    }
    report(c, class_name, "bootstrap:check-extern-class",
           "an extern item is an image, font, shader or storage", class_name);
    c->facts[id].type = simple(LUDO_TYPE_INVALID);
}

static void check_decl(checker *c, ludo_node_id id, pass which) {
    ludo_ast_kind kind = kind_of(c, id);
    if (kind == LUDO_AST_ERROR || kind == LUDO_AST_EXPLICIT || kind == LUDO_AST_LIBRARY ||
        kind == LUDO_AST_ATTRIBUTE) {
        c->facts[id].state = LUDO_CHECK_NOT_A_DECLARATION;
        return;
    }
    c->pending = c->facts[id].state == LUDO_CHECK_PENDING;
    c->generics = 0;
    if (kind == LUDO_AST_TYPE_DECL) {
        if (which != PASS_SIGNATURES) {
            check_type_decl(c, id, which);
        }
    } else if (kind == LUDO_AST_FN) {
        if (which == PASS_SIGNATURES) {
            check_fn_signature(c, id);
        } else if (which == PASS_BODIES) {
            check_fn_body(c, id);
        }
    } else if (kind == LUDO_AST_CONST) {
        if (which == PASS_SIGNATURES) {
            check_const(c, id);
        }
    } else if (kind == LUDO_AST_EXTERN_ITEM) {
        if (which == PASS_TYPES) {
            check_extern_item(c, id);
        }
    } else {
        /* use is the module mapping, band 4's (ch4 2.3, 3.4); persist, frame,
           impl, extern fn and the entry file's top-level statements are the
           later bands'. */
        (void)pending(c);
    }
    c->facts[id].state = c->pending ? LUDO_CHECK_PENDING : LUDO_CHECK_CHECKED;
}

static void collect(checker *c) {
    ludo_node_id root = c->ast->root;
    for (ludo_node_id it = first(c, root); it != 0; it = next(c, it)) {
        if (kind_of(c, it) == LUDO_AST_USE) {
            c->unresolved_library = true;
        }
        if (!declares_name(kind_of(c, it))) {
            continue;
        }
        ludo_node_id earlier = lookup_decl(c, it);
        if (earlier != 0) {
            report(c, it, "bootstrap:check-duplicate", "this name is already declared", it);
            continue;
        }
        ludo_node_id *grown = grow(c, c->decls, c->decl_count, &c->decl_capacity, sizeof *c->decls);
        if (grown == NULL) {
            return;
        }
        c->decls = grown;
        c->decls[c->decl_count++] = it;
    }
    /* Nominal types exist before any signature names them, in any order. A
       distinct's base is resolved by the signature pass that follows. */
    for (size_t i = 0; i < c->decl_count; i++) {
        ludo_node_id decl = c->decls[i];
        if (kind_of(c, decl) == LUDO_AST_TYPE_DECL) {
            c->pending = false;
            c->facts[decl].type = nominal(c, decl);
            c->facts[decl].state = c->pending ? LUDO_CHECK_PENDING : LUDO_CHECK_CHECKED;
        }
    }
}

/* A distinct's inner was not known when collect interned it. */
static void settle_distinct(checker *c) {
    for (size_t i = 0; i < c->decl_count; i++) {
        ludo_node_id decl = c->decls[i];
        ludo_type_id type = c->facts[decl].type;
        if (kind_of(c, decl) == LUDO_AST_TYPE_DECL && tkind(c, type) == LUDO_TYPE_DISTINCT) {
            ludo_node_id body = 0;
            for (ludo_node_id it = first(c, decl); it != 0; it = next(c, it)) {
                body = it;
            }
            c->types[type].inner = c->facts[first(c, body)].type;
        }
    }
}

ludo_frontend_status ludo_check_file(ludo_session *session, const ludo_parsed_file *file,
                                     const ludo_checked_file **out) {
    *out = NULL;
    checker c = {.arena = &session->check_arena, .file = file, .ast = &file->ast};
    size_t nodes = file->ast.count;
    ludo_checked_file *checked = ludo_arena_alloc(c.arena, sizeof *checked);
    c.facts = nodes <= SIZE_MAX / sizeof *c.facts
                  ? ludo_arena_alloc(c.arena, nodes * sizeof *c.facts)
                  : NULL;
    c.chain = ludo_arena_alloc(c.arena, nodes * sizeof *c.chain);
    c.nested = ludo_arena_alloc(c.arena, nodes * sizeof *c.nested);
    c.types = ludo_arena_alloc(c.arena, 64 * sizeof *c.types);
    if (checked == NULL || c.facts == NULL || c.chain == NULL || c.nested == NULL ||
        c.types == NULL) {
        return LUDO_FRONTEND_OUT_OF_MEMORY;
    }
    memset(c.facts, 0, nodes * sizeof *c.facts);
    memset(c.chain, 0, nodes * sizeof *c.chain);
    memset(c.nested, 0, nodes * sizeof *c.nested);
    c.type_capacity = 64;
    for (size_t i = 0; i < LUDO_TYPE_FIXED_COUNT; i++) {
        c.types[i] = (ludo_type){.kind = (ludo_type_kind)i};
    }
    c.type_count = LUDO_TYPE_FIXED_COUNT;
    for (size_t i = 0; i < file->diagnostic_count; i++) {
        ludo_diagnostic *grown = grow(&c, c.diagnostics, c.diagnostic_count, &c.diagnostic_capacity,
                                      sizeof *c.diagnostics);
        if (grown == NULL) {
            return LUDO_FRONTEND_OUT_OF_MEMORY;
        }
        c.diagnostics = grown;
        c.diagnostics[c.diagnostic_count++] = file->diagnostics[i];
    }

    collect(&c);
    ludo_node_id root = file->ast.root;
    for (ludo_node_id it = first(&c, root); it != 0 && !c.exhausted; it = next(&c, it)) {
        check_decl(&c, it, PASS_TYPES);
    }
    settle_distinct(&c);
    for (int which = PASS_SIGNATURES; which <= PASS_BODIES; which++) {
        for (ludo_node_id it = first(&c, root); it != 0 && !c.exhausted; it = next(&c, it)) {
            if (c.facts[it].state != LUDO_CHECK_NOT_A_DECLARATION) {
                check_decl(&c, it, (pass)which);
            }
        }
    }
    if (c.exhausted) {
        return LUDO_FRONTEND_OUT_OF_MEMORY;
    }
    *checked = (ludo_checked_file){
        .file = file,
        .types = c.types,
        .type_count = c.type_count,
        .facts = c.facts,
        .instances = c.instances,
        .instance_count = c.instance_count,
        .diagnostics = c.diagnostics,
        .diagnostic_count = c.diagnostic_count,
    };
    *out = checked;
    return LUDO_FRONTEND_OK;
}
