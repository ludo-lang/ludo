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

static const char *interp_status_name(ludo_interp_status status) {
    switch (status) {
    case LUDO_INTERP_OK:
        return "ok";
    case LUDO_INTERP_OUT_OF_MEMORY:
        return "out of memory";
    case LUDO_INTERP_FAULT:
        return "fault";
    case LUDO_INTERP_UNCHECKED:
        return "unchecked";
    case LUDO_INTERP_BAD_ARGUMENT:
        return "bad argument";
    }
    return "unknown";
}

/* ch5 8.3 has driver/ resolve every declared asset before top level runs. A
   headless run opens no file -- the stub host draws and decodes nothing -- so
   resolving a name is minting it an id. What a declared asset is on disk is
   the runner's question, not this one's. */
typedef struct bindings {
    ludo_image_binding *images;
    size_t image_count;
    ludo_storage_binding *slots;
    size_t slot_count;
    char **names;
    size_t name_count;
} bindings;

static void bindings_free(bindings *b) {
    for (size_t i = 0; i < b->name_count; i++) {
        free(b->names[i]); /* ludo-allow-malloc */
    }
    free(b->names);  /* ludo-allow-malloc */
    free(b->images); /* ludo-allow-malloc */
    free(b->slots);  /* ludo-allow-malloc */
}

static bool bindings_resolve(const ludo_checked_file *checked, bindings *b) {
    const ludo_ast *ast = &checked->file->ast;
    size_t externs = 0;
    for (ludo_node_id it = ast->nodes[ast->root].first_child; it != 0;
         it = ast->nodes[it].next_sibling) {
        externs += ast->nodes[it].kind == LUDO_AST_EXTERN_ITEM ? 1u : 0u;
    }
    *b = (bindings){0};
    size_t room = externs != 0 ? externs : 1u;
    b->images = malloc(room * sizeof *b->images); /* ludo-allow-malloc */
    b->slots = malloc(room * sizeof *b->slots);   /* ludo-allow-malloc */
    b->names = calloc(room, sizeof *b->names);    /* ludo-allow-malloc */
    if (b->images == NULL || b->slots == NULL || b->names == NULL) {
        return false;
    }
    for (ludo_node_id it = ast->nodes[ast->root].first_child; it != 0;
         it = ast->nodes[it].next_sibling) {
        const ludo_node_fact *fact = &checked->facts[it];
        if (ast->nodes[it].kind != LUDO_AST_EXTERN_ITEM || fact->state != LUDO_CHECK_CHECKED) {
            continue;
        }
        size_t length = 0;
        const char *text = ludo_node_text(checked->file, it, &length);
        char *name = malloc(length + 1); /* ludo-allow-malloc */
        if (name == NULL) {
            return false;
        }
        memcpy(name, text, length);
        name[length] = '\0';
        b->names[b->name_count++] = name;
        ludo_extern_class class_id = checked->types[fact->type].extern_class;
        if (class_id == LUDO_EXTERN_IMAGE) {
            b->images[b->image_count] = (ludo_image_binding){name, (uint32_t)b->image_count + 1u};
            b->image_count++;
        } else if (class_id == LUDO_EXTERN_STORAGE) {
            b->slots[b->slot_count] = (ludo_storage_binding){name, (uint32_t)b->slot_count + 1u};
            b->slot_count++;
        }
        /* Fonts and shaders are band 6's: the evaluator refuses them first. */
    }
    return true;
}

typedef struct report {
    const ludo_parsed_file *file;
    size_t regions;
} report;

/* One line per region: where it starts, what it is, and how many nodes in it
   never ran. Line and column are 1-based, the column in bytes. */
static void print_region(void *context, ludo_node_id id, size_t unreached) {
    report *r = context;
    const ludo_parsed_file *file = r->file;
    const ludo_ast_node *node = &file->ast.nodes[id];
    size_t line = 1;
    size_t column = 1;
    for (size_t i = 0; i < node->span.offset && i < file->length; i++) {
        if (file->source[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }
    (void)printf("unreached %s:%zu:%zu %s", file->name, line, column,
                 ludo_ast_kind_name(node->kind));
    if (ludo_ast_kind_has_token_text(node->kind) && node->token < file->token_count) {
        ludo_token token = file->tokens[node->token];
        (void)putchar(' ');
        print_text(file->source + token.offset, token.length);
    }
    (void)printf(" (%zu node%s)\n", unreached, unreached == 1 ? "" : "s");
    r->regions++;
}

/* Runs the program against every stub table and unions what it executed. */
static bool run_tables(const ludo_checked_file *checked, const bindings *b, bool *executed) {
    size_t nodes = checked->file->ast.count;
    for (size_t t = 0; t < LUDO_STUB_TABLE_COUNT; t++) {
        ludo_stub_state state;
        ludo_host host;
        ludo_host_constants constants;
        ludo_interp *in = NULL;
        ludo_stub_host(&state, &host, ludo_stub_tables[t]);
        ludo_stub_constants(&constants);
        constants.images = b->images;
        constants.image_count = b->image_count;
        constants.storage_slots = b->slots;
        constants.storage_slot_count = b->slot_count;
        ludo_interp_status status = ludo_interp_create(checked, &host, &constants, &in);
        (void)printf("run %s: top level %s", ludo_stub_tables[t]->name, interp_status_name(status));
        if (in == NULL) {
            (void)putchar('\n');
            return false;
        }
        /* Two frames, because one cannot tell a persist that survives from one
           that is re-initialised (#134). */
        for (int f = 1; status == LUDO_INTERP_OK && f <= 2; f++) {
            ludo_interp_status turn = ludo_interp_frame(in, ludo_stub_frame(&state));
            (void)printf("; frame %d %s", f, interp_status_name(turn));
            ludo_stub_next_frame(&state);
        }
        (void)putchar('\n');
        const bool *bits = ludo_interp_executed(in);
        for (size_t n = 0; n < nodes; n++) {
            executed[n] = executed[n] || bits[n];
        }
        ludo_interp_destroy(in);
    }
    return true;
}

/* #49's hole-finder (#139): run headless against every stub table, union the
   execution bits, and print what never ran. A report, not a gate: it exits 0
   whatever it finds, 1 if the program has diagnostics and cannot run, 2 on a
   host failure or a malformed exempt list. */
static int coverage(const char *path, const char *exempt_path) {
    size_t length = 0;
    size_t exempt_length = 0;
    char *source = read_file(path, &length);
    char *exempt_text = read_file(exempt_path, &exempt_length);
    ludo_coverage_exempt exempt;
    size_t bad_line = 0;
    if (source == NULL || exempt_text == NULL) {
        (void)fprintf(stderr, "ludo: cannot read %s\n", source == NULL ? path : exempt_path);
        free(source);      /* ludo-allow-malloc */
        free(exempt_text); /* ludo-allow-malloc */
        return 2;
    }
    if (!ludo_coverage_exempt_parse(exempt_text, exempt_length, &exempt, &bad_line)) {
        (void)fprintf(stderr, "%s:%zu: not `node|subtree <kind> <reason>`\n", exempt_path,
                      bad_line);
        free(source);      /* ludo-allow-malloc */
        free(exempt_text); /* ludo-allow-malloc */
        return 2;
    }
    ludo_session *session = NULL;
    const ludo_parsed_file *file = NULL;
    const ludo_checked_file *checked = NULL;
    bindings b = {0};
    bool *executed = NULL;
    ludo_node_id *stack = NULL;
    int status = 2;
    if (ludo_session_create(&session) != LUDO_FRONTEND_OK ||
        ludo_parse_file(session, path, source, length, &file) != LUDO_FRONTEND_OK ||
        ludo_check_file(session, file, &checked) != LUDO_FRONTEND_OK) {
        (void)fputs("ludo: frontend resource exhausted\n", stderr);
        goto done;
    }
    if (checked->diagnostic_count != 0) {
        status = print_diagnostics(checked->diagnostics, checked->diagnostic_count) ? 1 : 2;
        goto done;
    }
    executed = calloc(file->ast.count, sizeof *executed); /* ludo-allow-malloc */
    stack = malloc(file->ast.count * sizeof *stack);      /* ludo-allow-malloc */
    if (executed == NULL || stack == NULL || !bindings_resolve(checked, &b) ||
        !run_tables(checked, &b, executed)) {
        (void)fputs("ludo: interpreter resource exhausted\n", stderr);
        goto done;
    }
    report r = {file, 0};
    size_t total = ludo_coverage_unreached(&file->ast, executed, &exempt, file->ast.root, stack,
                                           print_region, &r);
    (void)printf("%zu unreached node%s in %zu region%s\n", total, total == 1 ? "" : "s", r.regions,
                 r.regions == 1 ? "" : "s");
    status = ferror(stdout) ? 2 : 0;
done:
    bindings_free(&b);
    free(stack);    /* ludo-allow-malloc */
    free(executed); /* ludo-allow-malloc */
    ludo_session_destroy(session);
    free(exempt_text); /* ludo-allow-malloc */
    free(source);      /* ludo-allow-malloc */
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
    if (argc == 4 && strcmp(argv[1], "coverage") == 0) {
        return coverage(argv[2], argv[3]);
    }
    if (argc != 1) {
        (void)fputs("usage: ludo [tokens FILE | ast FILE | check FILE | coverage FILE EXEMPT]\n",
                    stderr);
        return 2;
    }
    (void)printf("ludo %s (interp %s)\n", ludo_frontend_version(), ludo_interp_version());
    return 0;
}
