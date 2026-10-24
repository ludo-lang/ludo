/* Concrete prototype tree, accessed through this header (#130). Node ids are
   dense per file; a program-wide side table keys on (source id, node id).
   Zero is absent. Nodes have one parent and ordered children, never cross-file
   pointers. Tokens include trivia; spans borrow the caller's source buffer. */
#ifndef LUDO_AST_H
#define LUDO_AST_H

#include <stdbool.h>
#include <stddef.h>

typedef size_t ludo_node_id;
typedef size_t ludo_source_id;

typedef struct ludo_span {
    ludo_source_id file;
    size_t offset;
    size_t length;
} ludo_span;

typedef enum ludo_ast_kind {
    LUDO_AST_ERROR,
    LUDO_AST_MODULE,
    LUDO_AST_EXPLICIT,
    LUDO_AST_LIBRARY,
    LUDO_AST_USE,
    LUDO_AST_ATTRIBUTE,
    LUDO_AST_TYPE_DECL,
    LUDO_AST_CONST,
    LUDO_AST_FN,
    LUDO_AST_EXTERN_FN,
    LUDO_AST_EXTERN_ITEM,
    LUDO_AST_PERSIST,
    LUDO_AST_FRAME,
    LUDO_AST_IMPL,
    LUDO_AST_GENERIC_PARAMS,
    LUDO_AST_GENERIC_PARAM,
    LUDO_AST_CONSTRAINTS,
    LUDO_AST_PARAMS,
    LUDO_AST_PARAM,
    LUDO_AST_RETURNS,
    LUDO_AST_STRUCT,
    LUDO_AST_FIELD,
    LUDO_AST_ENUM,
    LUDO_AST_VARIANT,
    LUDO_AST_DISTINCT,
    LUDO_AST_INTERFACE,
    LUDO_AST_FN_TYPE,
    LUDO_AST_NAMED_TYPE,
    LUDO_AST_GENERIC_ARGS,
    LUDO_AST_OPTIONAL_TYPE,
    LUDO_AST_POINTER_TYPE,
    LUDO_AST_VIEW_TYPE,
    LUDO_AST_ARRAY_TYPE,
    LUDO_AST_MUTABLE_TYPE,
    LUDO_AST_FALLIBLE_TYPE,
    LUDO_AST_BLOCK,
    LUDO_AST_LET,
    LUDO_AST_BINDINGS,
    LUDO_AST_ASSIGN,
    LUDO_AST_COMPOUND_ASSIGN,
    LUDO_AST_EXPR_STMT,
    LUDO_AST_TAIL,
    LUDO_AST_RETURN,
    LUDO_AST_BREAK,
    LUDO_AST_CONTINUE,
    LUDO_AST_DEFER,
    LUDO_AST_MODIFIER,
    LUDO_AST_IF,
    LUDO_AST_BRANCH,
    LUDO_AST_WHILE,
    LUDO_AST_FOR,
    LUDO_AST_BIND_CONDITION,
    LUDO_AST_UNSAFE,
    LUDO_AST_MATCH,
    LUDO_AST_ARM,
    LUDO_AST_PATTERN_IDENT,
    LUDO_AST_PATTERN_VARIANT,
    LUDO_AST_PATTERN_STRUCT,
    LUDO_AST_PATTERN_FIELD,
    LUDO_AST_PATTERN_LITERAL,
    LUDO_AST_PATTERN_DISCARD,
    LUDO_AST_IDENT,
    LUDO_AST_ROOT,
    LUDO_AST_DISCARD,
    LUDO_AST_LITERAL,
    LUDO_AST_UNARY,
    LUDO_AST_BINARY,
    LUDO_AST_CAST,
    LUDO_AST_MEMBER,
    LUDO_AST_OPTIONAL_MEMBER,
    LUDO_AST_CALL,
    LUDO_AST_BRACKET,
    LUDO_AST_MARK,
    LUDO_AST_DEREF,
    LUDO_AST_GROUP,
    LUDO_AST_AGGREGATE,
    LUDO_AST_NAMED_ITEM,
    LUDO_AST_KEYED_ITEM,
    LUDO_AST_RESCUE,
    LUDO_AST_HANDLER
} ludo_ast_kind;

/* The token names a declaration/member/literal or records an operator; it is
   an index into the file's full token stream. SIZE_MAX means no token.
   flags are spelling metadata: pub on declarations, numeric on distinct,
   prefixed on aggregates. Child roles are documented in src/frontend/AST.md. */
enum { LUDO_AST_PUBLIC = 1, LUDO_AST_NUMERIC = 2, LUDO_AST_PREFIXED = 4 };

typedef struct ludo_ast_node {
    ludo_ast_kind kind;
    ludo_span span;
    size_t token;
    unsigned flags;
    ludo_node_id first_child;
    ludo_node_id last_child;
    ludo_node_id next_sibling;
} ludo_ast_node;

typedef struct ludo_ast {
    const ludo_ast_node *nodes;
    size_t count; /* includes absent slot 0 */
    ludo_node_id root;
} ludo_ast;

static inline const ludo_ast_node *ludo_ast_get(const ludo_ast *ast, ludo_node_id id) {
    return id != 0 && id < ast->count ? &ast->nodes[id] : NULL;
}

static inline ludo_node_id ludo_ast_child(const ludo_ast *ast, ludo_node_id id, size_t index) {
    const ludo_ast_node *node = ludo_ast_get(ast, id);
    ludo_node_id child = node != NULL ? node->first_child : 0;
    while (child != 0 && index != 0) {
        child = ast->nodes[child].next_sibling;
        index--;
    }
    return child;
}

const char *ludo_ast_kind_name(ludo_ast_kind kind);
bool ludo_ast_kind_has_token_text(ludo_ast_kind kind);

#endif
