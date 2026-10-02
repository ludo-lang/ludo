/* The lexer's libFuzzer target (#141). #131 put it here, with the lexer and
   not retrofitted, and committed its corpus as the permanent regression suite:
   src/frontend/tests/corpus/lexer/. `make fuzz` runs it under libFuzzer; the
   replay harness beside it runs the same entry point over the corpus in every
   `make check`, so a finding never comes back. */
#include "ludo_frontend.h"

#include <stdint.h>
#include <stdlib.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static void require(bool ok) {
    if (!ok) {
        abort();
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    const char *src = (const char *)data;
    ludo_lexer lexer;
    ludo_lexer_init(&lexer, src, size);
    size_t end = 0;
    size_t count = 0;
    for (;;) {
        ludo_token t = ludo_lex_next(&lexer);
        /* The tokens tile the buffer: no gap, no overlap, in order. */
        require(t.offset == end);
        if (t.kind == LUDO_TOK_EOF) {
            require(t.length == 0 && t.offset == size && t.error == LUDO_LEX_OK);
            break;
        }
        require(t.length > 0 && t.length <= size - t.offset);
        require(t.kind != LUDO_TOK_ERROR || t.error != LUDO_LEX_OK);
        /* Context-free: a token's bytes lexed alone are that token again. */
        ludo_lexer alone;
        ludo_lexer_init(&alone, src + t.offset, t.length);
        ludo_token again = ludo_lex_next(&alone);
        require(again.kind == t.kind && again.error == t.error && again.length == t.length);
        end = t.offset + t.length;
        count++;
        require(count <= size);
    }
    /* EOF is sticky. */
    ludo_token after = ludo_lex_next(&lexer);
    require(after.kind == LUDO_TOK_EOF && after.offset == size);
    return 0;
}
