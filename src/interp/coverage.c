/* The coverage report's two halves that are not I/O (#139): reading the
   committed exempt list out of a buffer, and folding the execution bits into
   regions. driver/ reads the file and prints; the tests read the same file. */
#include "ludo_interp.h"

#include <string.h>

static bool is_space(char c) { return c == ' ' || c == '\t' || c == '\r'; }

/* The next whitespace-delimited word in [*at, end), or an empty one. */
static size_t word(const char *text, size_t *at, size_t end) {
    while (*at < end && is_space(text[*at])) {
        (*at)++;
    }
    size_t start = *at;
    while (*at < end && !is_space(text[*at])) {
        (*at)++;
    }
    return start;
}

static bool parse_line(const char *text, size_t start, size_t end, ludo_coverage_exempt *out) {
    size_t at = start;
    size_t mode = word(text, &at, end);
    size_t mode_length = at - mode;
    if (mode_length == 0 || text[mode] == '#') {
        return true;
    }
    ludo_exemption exemption = LUDO_EXEMPT_NONE;
    if (mode_length == 4 && memcmp(text + mode, "node", 4) == 0) {
        exemption = LUDO_EXEMPT_NODE;
    } else if (mode_length == 7 && memcmp(text + mode, "subtree", 7) == 0) {
        exemption = LUDO_EXEMPT_SUBTREE;
    } else {
        return false;
    }
    size_t name = word(text, &at, end);
    size_t name_length = at - name;
    size_t reason = word(text, &at, end);
    if (name_length == 0 || at == reason) {
        return false; /* a reason per entry is the point of the list */
    }
    for (size_t k = 0; k < LUDO_AST_KIND_COUNT; k++) {
        const char *spelling = ludo_ast_kind_name((ludo_ast_kind)k);
        if (strlen(spelling) == name_length && memcmp(spelling, text + name, name_length) == 0) {
            if (out->kinds[k] != LUDO_EXEMPT_NONE) {
                return false; /* two reasons for one kind, and one of them is stale */
            }
            out->kinds[k] = exemption;
            return true;
        }
    }
    return false;
}

bool ludo_coverage_exempt_parse(const char *text, size_t length, ludo_coverage_exempt *out,
                                size_t *bad_line) {
    memset(out, 0, sizeof *out);
    *bad_line = 0;
    size_t line = 1;
    size_t start = 0;
    while (start < length) {
        size_t end = start;
        while (end < length && text[end] != '\n') {
            end++;
        }
        if (!parse_line(text, start, end, out)) {
            *bad_line = line;
            return false;
        }
        start = end + 1;
        line++;
    }
    return true;
}

static ludo_exemption exemption_of(const ludo_coverage_exempt *exempt, ludo_ast_kind kind) {
    return (size_t)kind < LUDO_AST_KIND_COUNT ? exempt->kinds[kind] : LUDO_EXEMPT_NONE;
}

/* Unreached non-exempt nodes in the subtree at region, region included. The
   stack above base is free: the outer walk never holds a node of this subtree. */
static size_t count_region(const ludo_ast *ast, const bool *executed,
                           const ludo_coverage_exempt *exempt, ludo_node_id region,
                           ludo_node_id *stack, size_t base) {
    size_t top = base;
    size_t missing = 0;
    stack[top++] = region;
    while (top != base) {
        ludo_node_id id = stack[--top];
        const ludo_ast_node *node = &ast->nodes[id];
        if (id != region && node->next_sibling != 0) {
            stack[top++] = node->next_sibling;
        }
        ludo_exemption exemption = exemption_of(exempt, node->kind);
        if (exemption == LUDO_EXEMPT_SUBTREE) {
            continue;
        }
        if (exemption == LUDO_EXEMPT_NONE && !executed[id]) {
            missing++;
        }
        if (node->first_child != 0) {
            stack[top++] = node->first_child;
        }
    }
    return missing;
}

size_t ludo_coverage_unreached(const ludo_ast *ast, const bool *executed,
                               const ludo_coverage_exempt *exempt, ludo_node_id root,
                               ludo_node_id *stack,
                               void (*region)(void *context, ludo_node_id id, size_t unreached),
                               void *context) {
    if (ludo_ast_get(ast, root) == NULL) {
        return 0;
    }
    size_t top = 0;
    size_t total = 0;
    stack[top++] = root;
    while (top != 0) {
        ludo_node_id id = stack[--top];
        const ludo_ast_node *node = &ast->nodes[id];
        /* The sibling goes under the child, so the walk is in source order. */
        if (id != root && node->next_sibling != 0) {
            stack[top++] = node->next_sibling;
        }
        ludo_exemption exemption = exemption_of(exempt, node->kind);
        if (exemption == LUDO_EXEMPT_SUBTREE) {
            continue;
        }
        if (exemption == LUDO_EXEMPT_NONE && !executed[id]) {
            size_t missing = count_region(ast, executed, exempt, id, stack, top);
            total += missing;
            if (region != NULL) {
                region(context, id, missing);
            }
            continue;
        }
        if (node->first_child != 0) {
            stack[top++] = node->first_child;
        }
    }
    return total;
}
