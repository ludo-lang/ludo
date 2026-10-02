#include "ludo_frontend.h"
#include "test.h"

#include <string.h>

/* The non-whitespace tokens of src, rendered as `kind` for a fixed spelling and
   `kind:text` otherwise, with `!error` appended, joined by spaces. */
static const char *lex(const char *src) {
    static char out[4096];
    size_t used = 0;
    ludo_lexer lexer;
    ludo_lexer_init(&lexer, src, strlen(src));
    out[0] = '\0';
    for (;;) {
        ludo_token t = ludo_lex_next(&lexer);
        if (t.kind == LUDO_TOK_EOF) {
            break;
        }
        if (t.kind == LUDO_TOK_WHITESPACE) {
            continue;
        }
        int n;
        if (ludo_token_kind_has_fixed_spelling(t.kind)) {
            n = snprintf(out + used, sizeof out - used, "%s%s", used ? " " : "",
                         ludo_token_kind_name(t.kind));
        } else {
            n = snprintf(out + used, sizeof out - used, "%s%s:%.*s", used ? " " : "",
                         ludo_token_kind_name(t.kind), (int)t.length, src + t.offset);
        }
        used += (size_t)n;
        if (t.error != LUDO_LEX_OK) {
            n = snprintf(out + used, sizeof out - used, "!%s", ludo_lex_error_name(t.error));
            used += (size_t)n;
        }
    }
    return out;
}

#define EXPECT_LEX(src, want)                                                                      \
    do {                                                                                           \
        const char *got_ = lex(src);                                                               \
        if (strcmp(got_, want) != 0) {                                                             \
            (void)fprintf(stderr, "  lex(%s)\n    got  %s\n    want %s\n", #src, got_, want);      \
        }                                                                                          \
        LUDO_CHECK(strcmp(got_, want) == 0);                                                       \
    } while (0)

static void test_words(void) {
    EXPECT_LEX("fn frame framed _ _x x_1 __",
               "fn frame ident:framed _ ident:_x ident:x_1 ident:__");
    /* grammar.ebnf Keyword and TypeKeyword, impl and interface included (#141). */
    EXPECT_LEX("impl interface distinct enum numeric struct true false",
               "impl interface distinct enum numeric struct true false");
    /* Prelude names are identifiers, not keywords (ch1 §2.6-§2.9). */
    EXPECT_LEX("usize string none some heap Range",
               "ident:usize ident:string ident:none ident:some ident:heap ident:Range");
}

static void test_punctuation(void) {
    EXPECT_LEX("0..<n a >.. b", "int:0 ..< ident:n ident:a >.. ident:b");
    EXPECT_LEX(">> >= > .. ?. ? -> -= - := :", ">> >= > . . ?. ? -> -= - := :");
    EXPECT_LEX("$.graphics #align(16) #explicit",
               "$ . ident:graphics # ident:align ( int:16 ) # ident:explicit");
    EXPECT_LEX("a!=b x! ^p &v ~m |",
               "ident:a != ident:b ident:x ! ^ ident:p & ident:v ~ ident:m |");
}

static void test_comments(void) {
    EXPECT_LEX("x -- note\ny", "ident:x comment:-- note ident:y");
    EXPECT_LEX("a--b", "ident:a comment:--b");
    EXPECT_LEX("-- \xE2\x80\x94 \xC2\xA7\r\nz", "comment:-- \xE2\x80\x94 \xC2\xA7 ident:z");
}

static void test_numbers(void) {
    EXPECT_LEX("0 1_000 0xFF_ff 0b1010_1", "int:0 int:1_000 int:0xFF_ff int:0b1010_1");
    EXPECT_LEX("1.5 2e10 3.0E-2 4e+1 1.x",
               "float:1.5 float:2e10 float:3.0E-2 float:4e+1 int:1 . ident:x");
    EXPECT_LEX("0x 0b2 0X1 1e 2x 0x_1",
               "int:0x!malformed-number int:0b2!malformed-number int:0X1!malformed-number "
               "int:1e!malformed-number int:2x!malformed-number int:0x_1!malformed-number");
}

static void test_strings(void) {
    EXPECT_LEX("\"hi\" \"a\\n\\t\\r\\0\\\\\\\"\" \"\\u{1F600}\"",
               "string:\"hi\" string:\"a\\n\\t\\r\\0\\\\\\\"\" string:\"\\u{1F600}\"");
    EXPECT_LEX("\"\\q\" \"\\u{}\" \"\\u{D800}\" \"\\u{110000}\" \"\\u41\"",
               "string:\"\\q\"!bad-escape string:\"\\u{}\"!bad-escape "
               "string:\"\\u{D800}\"!bad-escape string:\"\\u{110000}\"!bad-escape "
               "string:\"\\u41\"!bad-escape");
    EXPECT_LEX("\"open\nx", "string:\"open!unterminated-string ident:x");
    EXPECT_LEX("\"end\\", "string:\"end\\!bad-escape");
}

static void test_bad_bytes(void) {
    EXPECT_LEX("a @ b", "ident:a error:@!unexpected-char ident:b");
    EXPECT_LEX("\xC3\xA9", "error:\xC3\xA9!unexpected-char");
    EXPECT_LEX("\xFF"
               "a",
               "error:\xFF!invalid-utf8 ident:a");
    EXPECT_LEX("\xC0\x80", "error:\xC0!invalid-utf8 error:\x80!invalid-utf8"); /* overlong */
    EXPECT_LEX("\xED\xA0\x80", "error:\xED!invalid-utf8 error:\xA0!invalid-utf8 "
                               "error:\x80!invalid-utf8"); /* surrogate */
    EXPECT_LEX("\"\xFF\" -- \xFF", "string:\"\xFF\"!invalid-utf8 comment:-- \xFF!invalid-utf8");
    EXPECT_LEX(";", "error:;!unexpected-char"); /* ch1 §1.3: no semicolons */
}

/* A NUL is a byte like any other: the buffer is (pointer, length). */
static void test_spans(void) {
    const char src[] = "x\0y";
    ludo_lexer lexer;
    ludo_lexer_init(&lexer, src, 3);
    ludo_token a = ludo_lex_next(&lexer);
    ludo_token b = ludo_lex_next(&lexer);
    ludo_token c = ludo_lex_next(&lexer);
    ludo_token eof = ludo_lex_next(&lexer);
    LUDO_CHECK(a.kind == LUDO_TOK_IDENT && a.offset == 0 && a.length == 1);
    LUDO_CHECK(b.kind == LUDO_TOK_ERROR && b.error == LUDO_LEX_UNEXPECTED_CHAR && b.offset == 1);
    LUDO_CHECK(c.kind == LUDO_TOK_IDENT && c.offset == 2 && c.length == 1);
    LUDO_CHECK(eof.kind == LUDO_TOK_EOF && eof.offset == 3 && eof.length == 0);
    LUDO_CHECK(ludo_lex_next(&lexer).kind == LUDO_TOK_EOF);
    LUDO_CHECK(ludo_token_kind_is_trivia(LUDO_TOK_COMMENT));
    LUDO_CHECK(!ludo_token_kind_is_trivia(LUDO_TOK_IDENT));
}

LUDO_TEST_MAIN({
    LUDO_CHECK(strcmp(ludo_frontend_version(), "") != 0);
    test_words();
    test_punctuation();
    test_comments();
    test_numbers();
    test_strings();
    test_bad_bytes();
    test_spans();
})
