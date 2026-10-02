/* The CLI. Owns the filesystem, the terminal and the process; one consumer of
   frontend/ among several (ADR-0020). */
#include "ludo_frontend.h"
#include "ludo_interp.h"

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

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "tokens") == 0) {
        return dump_tokens(argv[2]);
    }
    if (argc != 1) {
        (void)fputs("usage: ludo [tokens FILE]\n", stderr);
        return 2;
    }
    (void)printf("ludo %s (interp %s)\n", ludo_frontend_version(), ludo_interp_version());
    return 0;
}
