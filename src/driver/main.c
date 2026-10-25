/* The CLI. Owns the filesystem, the terminal and the process; one consumer of
   frontend/ among several (ADR-0020). */
#include "ludo_frontend.h"
#include "ludo_interp.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The whole file in one caller-owned buffer, which is what the frontend takes
   (#130: sources arrive as buffers the caller owns). NULL on any failure. */
static char *read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return NULL;
    }
    char *buf = NULL;
    long size = -1;
    if (fseek(f, 0, SEEK_END) == 0) {
        size = ftell(f);
    }
    if (size >= 0 && fseek(f, 0, SEEK_SET) == 0) {
        /* driver/ owns the process, and this buffer is the caller-owned source
           #130 hands to the frontend -- not frontend memory, so not the arena's. */
        buf = malloc((size_t)size + 1); /* ludo-allow-malloc */
    }
    if (buf != NULL && fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf); /* ludo-allow-malloc */
        buf = NULL;
    }
    (void)fclose(f);
    *len = (size_t)size;
    return buf;
}

/* Token text as one dump line can carry it: tabs, control bytes and the
   backslash escaped, everything else -- UTF-8 included -- verbatim. */
static void print_text(const char *text, size_t len) {
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\\') {
            (void)fputs("\\\\", stdout);
        } else if (c == '\t') {
            (void)fputs("\\t", stdout);
        } else if (c < 0x20 || c == 0x7F) {
            (void)printf("\\x%02X", c);
        } else {
            (void)putchar(c);
        }
    }
}

/* The token dump (#141): one significant token or comment per line, and no
   positions, so an edit to the source diffs as exactly the tokens it changed.
   Whitespace is left out; the tiling it belongs to is the fuzz target's to
   check, not a reviewer's. Exits 1 if any token carries a lex error. */
static int dump_tokens(const char *path) {
    size_t len = 0;
    char *src = read_file(path, &len);
    if (src == NULL) {
        (void)fprintf(stderr, "ludo: cannot read %s\n", path);
        return 2;
    }
    int status = 0;
    ludo_lexer lexer;
    ludo_lexer_init(&lexer, src, len);
    for (;;) {
        ludo_token t = ludo_lex_next(&lexer);
        if (t.kind == LUDO_TOK_EOF) {
            break;
        }
        if (t.kind == LUDO_TOK_WHITESPACE) {
            continue;
        }
        (void)fputs(ludo_token_kind_name(t.kind), stdout);
        if (!ludo_token_kind_has_fixed_spelling(t.kind)) {
            (void)putchar(' ');
            print_text(src + t.offset, t.length);
        }
        if (t.error != LUDO_LEX_OK) {
            (void)printf("  !%s", ludo_lex_error_name(t.error));
            status = 1;
        }
        (void)putchar('\n');
    }
    free(src); /* ludo-allow-malloc */
    return status;
}

/* Traversal is iterative: long left-associated expressions do not consume the
   driver's native stack. Parent/sibling links are file-local node ids. */
static bool print_ast(const ludo_parsed_file *file) {
    typedef struct visit {
        ludo_node_id id;
        size_t depth;
    } visit;
    if (file->ast.count > SIZE_MAX / sizeof(visit)) {
        return false;
    }
    visit *stack = malloc(file->ast.count * sizeof *stack); /* ludo-allow-malloc */
    if (stack == NULL) {
        return false;
    }
    size_t count = 1;
    stack[0] = (visit){file->ast.root, 0};
    while (count != 0) {
        visit current = stack[--count];
        const ludo_ast_node *node = ludo_ast_get(&file->ast, current.id);
        for (size_t i = 0; i < current.depth; i++) {
            (void)fputs("  ", stdout);
        }
        (void)fputs(ludo_ast_kind_name(node->kind), stdout);
        if (node->flags != 0) {
            (void)printf(" flags=%u", node->flags);
        }
        if (ludo_ast_kind_has_token_text(node->kind) && node->token < file->token_count) {
            ludo_token token = file->tokens[node->token];
            (void)putchar(' ');
            print_text(file->source + token.offset, token.length);
        }
        (void)putchar('\n');
        if (node->next_sibling != 0) {
            stack[count++] = (visit){node->next_sibling, current.depth};
        }
        if (node->first_child != 0) {
            stack[count++] = (visit){node->first_child, current.depth + 1};
        }
    }
    free(stack); /* ludo-allow-malloc */
    return !ferror(stdout);
}

static int dump_ast(const char *path) {
    size_t length = 0;
    char *source = read_file(path, &length);
    if (source == NULL) {
        (void)fprintf(stderr, "ludo: cannot read %s\n", path);
        return 2;
    }
    ludo_session *session = NULL;
    const ludo_parsed_file *file = NULL;
    int status = 2;
    if (ludo_session_create(&session) == LUDO_FRONTEND_OK &&
        ludo_parse_file(session, path, source, length, &file) == LUDO_FRONTEND_OK) {
        status = print_ast(file) ? (file->diagnostic_count == 0 ? 0 : 1) : 2;
        for (size_t i = 0; i < file->diagnostic_count; i++) {
            const ludo_diagnostic *diagnostic = &file->diagnostics[i];
            size_t size = ludo_diagnostic_json(diagnostic, NULL, 0);
            char *json = malloc(size); /* ludo-allow-malloc */
            if (json == NULL) {
                status = 2;
                break;
            }
            (void)ludo_diagnostic_json(diagnostic, json, size);
            (void)fwrite(json, 1, size, stderr);
            (void)fputc('\n', stderr);
            free(json); /* ludo-allow-malloc */
        }
    } else {
        (void)fputs("ludo: parser resource exhausted\n", stderr);
    }
    ludo_session_destroy(session);
    free(source); /* ludo-allow-malloc */
    return status;
}

static bool print_diagnostics(const ludo_diagnostic *diagnostics, size_t count) {
    for (size_t i = 0; i < count; i++) {
        size_t size = ludo_diagnostic_json(&diagnostics[i], NULL, 0);
        char *json = malloc(size); /* ludo-allow-malloc */
        if (json == NULL) {
            return false;
        }
        (void)ludo_diagnostic_json(&diagnostics[i], json, size);
        (void)fwrite(json, 1, size, stderr);
        (void)fputc('\n', stderr);
        free(json); /* ludo-allow-malloc */
    }
    return true;
}

/* Parse and check one file: diagnostics to stderr as NDJSON, then one line per
   top-level declaration a later band still owns. Exit 0 clean, 1 with source
   diagnostics, 2 on a host failure. */
static int check_file(const char *path) {
    size_t length = 0;
    char *source = read_file(path, &length);
    if (source == NULL) {
        (void)fprintf(stderr, "ludo: cannot read %s\n", path);
        return 2;
    }
    ludo_session *session = NULL;
    const ludo_parsed_file *file = NULL;
    const ludo_checked_file *checked = NULL;
    int status = 2;
    if (ludo_session_create(&session) == LUDO_FRONTEND_OK &&
        ludo_parse_file(session, path, source, length, &file) == LUDO_FRONTEND_OK &&
        ludo_check_file(session, file, &checked) == LUDO_FRONTEND_OK) {
        status = checked->diagnostic_count == 0 ? 0 : 1;
        if (!print_diagnostics(checked->diagnostics, checked->diagnostic_count)) {
            status = 2;
        }
        for (ludo_node_id it = file->ast.nodes[file->ast.root].first_child; it != 0;
             it = file->ast.nodes[it].next_sibling) {
            if (checked->facts[it].state == LUDO_CHECK_PENDING) {
                size_t name_length = 0;
                const char *name = ludo_node_text(file, it, &name_length);
                (void)printf("pending %s ", ludo_ast_kind_name(file->ast.nodes[it].kind));
                print_text(name, name_length);
                (void)putchar('\n');
            }
        }
    } else {
        (void)fputs("ludo: frontend resource exhausted\n", stderr);
    }
    ludo_session_destroy(session);
    free(source); /* ludo-allow-malloc */
    return status;
}

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "tokens") == 0) {
        return dump_tokens(argv[2]);
    }
    if (argc == 3 && strcmp(argv[1], "ast") == 0) {
        return dump_ast(argv[2]);
    }
    if (argc == 3 && strcmp(argv[1], "check") == 0) {
        return check_file(argv[2]);
    }
    if (argc != 1) {
        (void)fputs("usage: ludo [tokens FILE | ast FILE | check FILE]\n", stderr);
        return 2;
    }
    (void)printf("ludo %s (interp %s)\n", ludo_frontend_version(), ludo_interp_version());
    return 0;
}
