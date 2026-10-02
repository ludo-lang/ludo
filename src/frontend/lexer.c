/* The lexer (#141). grammar.ebnf §3 is the authority for every token shape
   here; the punctuation set is the quoted terminals of §1 and §2. Tokens are
   formed by longest match (ch1 §7.7.1 states it for `>..`). */
#include "ludo_frontend.h"

#include <string.h>

void ludo_lexer_init(ludo_lexer *lexer, const char *src, size_t len) {
    lexer->src = src;
    lexer->len = len;
    lexer->pos = 0;
}

bool ludo_token_kind_is_trivia(ludo_token_kind kind) {
    return kind == LUDO_TOK_WHITESPACE || kind == LUDO_TOK_COMMENT;
}

bool ludo_token_kind_has_fixed_spelling(ludo_token_kind kind) { return kind >= LUDO_TOK_KW_AND; }

/* The one table of spellings. The keyword and punctuation scanners read it
   back, so a spelling is written once. */
const char *ludo_token_kind_name(ludo_token_kind kind) {
    switch (kind) {
    case LUDO_TOK_EOF:
        return "eof";
    case LUDO_TOK_WHITESPACE:
        return "whitespace";
    case LUDO_TOK_COMMENT:
        return "comment";
    case LUDO_TOK_IDENT:
        return "ident";
    case LUDO_TOK_INT:
        return "int";
    case LUDO_TOK_FLOAT:
        return "float";
    case LUDO_TOK_STRING:
        return "string";
    case LUDO_TOK_ERROR:
        return "error";
    case LUDO_TOK_KW_AND:
        return "and";
    case LUDO_TOK_KW_AS:
        return "as";
    case LUDO_TOK_KW_BREAK:
        return "break";
    case LUDO_TOK_KW_CONST:
        return "const";
    case LUDO_TOK_KW_CONTINUE:
        return "continue";
    case LUDO_TOK_KW_DEFER:
        return "defer";
    case LUDO_TOK_KW_DO:
        return "do";
    case LUDO_TOK_KW_ELSE:
        return "else";
    case LUDO_TOK_KW_ELSEIF:
        return "elseif";
    case LUDO_TOK_KW_END:
        return "end";
    case LUDO_TOK_KW_EXTERN:
        return "extern";
    case LUDO_TOK_KW_FALSE:
        return "false";
    case LUDO_TOK_KW_FN:
        return "fn";
    case LUDO_TOK_KW_FOR:
        return "for";
    case LUDO_TOK_KW_FRAME:
        return "frame";
    case LUDO_TOK_KW_IF:
        return "if";
    case LUDO_TOK_KW_IMPL:
        return "impl";
    case LUDO_TOK_KW_IN:
        return "in";
    case LUDO_TOK_KW_LIBRARY:
        return "library";
    case LUDO_TOK_KW_MATCH:
        return "match";
    case LUDO_TOK_KW_NOT:
        return "not";
    case LUDO_TOK_KW_OR:
        return "or";
    case LUDO_TOK_KW_PERSIST:
        return "persist";
    case LUDO_TOK_KW_PUB:
        return "pub";
    case LUDO_TOK_KW_RESCUE:
        return "rescue";
    case LUDO_TOK_KW_RETURN:
        return "return";
    case LUDO_TOK_KW_THEN:
        return "then";
    case LUDO_TOK_KW_TRUE:
        return "true";
    case LUDO_TOK_KW_TYPE:
        return "type";
    case LUDO_TOK_KW_UNLESS:
        return "unless";
    case LUDO_TOK_KW_UNSAFE:
        return "unsafe";
    case LUDO_TOK_KW_USE:
        return "use";
    case LUDO_TOK_KW_WHILE:
        return "while";
    case LUDO_TOK_KW_DISTINCT:
        return "distinct";
    case LUDO_TOK_KW_ENUM:
        return "enum";
    case LUDO_TOK_KW_INTERFACE:
        return "interface";
    case LUDO_TOK_KW_NUMERIC:
        return "numeric";
    case LUDO_TOK_KW_STRUCT:
        return "struct";
    case LUDO_TOK_UNDERSCORE:
        return "_";
    case LUDO_TOK_DOLLAR:
        return "$";
    case LUDO_TOK_HASH:
        return "#";
    case LUDO_TOK_LPAREN:
        return "(";
    case LUDO_TOK_RPAREN:
        return ")";
    case LUDO_TOK_LBRACKET:
        return "[";
    case LUDO_TOK_RBRACKET:
        return "]";
    case LUDO_TOK_LBRACE:
        return "{";
    case LUDO_TOK_RBRACE:
        return "}";
    case LUDO_TOK_COMMA:
        return ",";
    case LUDO_TOK_DOT:
        return ".";
    case LUDO_TOK_COLON:
        return ":";
    case LUDO_TOK_QUESTION:
        return "?";
    case LUDO_TOK_BANG:
        return "!";
    case LUDO_TOK_CARET:
        return "^";
    case LUDO_TOK_AMP:
        return "&";
    case LUDO_TOK_PIPE:
        return "|";
    case LUDO_TOK_TILDE:
        return "~";
    case LUDO_TOK_PLUS:
        return "+";
    case LUDO_TOK_MINUS:
        return "-";
    case LUDO_TOK_STAR:
        return "*";
    case LUDO_TOK_SLASH:
        return "/";
    case LUDO_TOK_PERCENT:
        return "%";
    case LUDO_TOK_EQ:
        return "=";
    case LUDO_TOK_LT:
        return "<";
    case LUDO_TOK_GT:
        return ">";
    case LUDO_TOK_EQ_EQ:
        return "==";
    case LUDO_TOK_BANG_EQ:
        return "!=";
    case LUDO_TOK_LT_EQ:
        return "<=";
    case LUDO_TOK_GT_EQ:
        return ">=";
    case LUDO_TOK_SHL:
        return "<<";
    case LUDO_TOK_SHR:
        return ">>";
    case LUDO_TOK_PLUS_EQ:
        return "+=";
    case LUDO_TOK_MINUS_EQ:
        return "-=";
    case LUDO_TOK_STAR_EQ:
        return "*=";
    case LUDO_TOK_SLASH_EQ:
        return "/=";
    case LUDO_TOK_PERCENT_EQ:
        return "%=";
    case LUDO_TOK_COLON_EQ:
        return ":=";
    case LUDO_TOK_ARROW:
        return "->";
    case LUDO_TOK_QUESTION_DOT:
        return "?.";
    case LUDO_TOK_RANGE:
        return "..<";
    case LUDO_TOK_REV_RANGE:
        return ">..";
    }
    return "?";
}

const char *ludo_lex_error_name(ludo_lex_error error) {
    switch (error) {
    case LUDO_LEX_OK:
        return "ok";
    case LUDO_LEX_INVALID_UTF8:
        return "invalid-utf8";
    case LUDO_LEX_UNEXPECTED_CHAR:
        return "unexpected-char";
    case LUDO_LEX_UNTERMINATED_STRING:
        return "unterminated-string";
    case LUDO_LEX_BAD_ESCAPE:
        return "bad-escape";
    case LUDO_LEX_MALFORMED_NUMBER:
        return "malformed-number";
    }
    return "?";
}

/* ---- Bytes ------------------------------------------------------------------ */

/* The byte at i, or -1 past the end. Everything below reads through this, so
   no scanner can step off the buffer. */
static int byte_at(const ludo_lexer *lexer, size_t i) {
    if (i >= lexer->len) {
        return -1;
    }
    return (unsigned char)lexer->src[i];
}

static bool is_digit(int c) { return c >= '0' && c <= '9'; }

static bool is_bin_digit(int c) { return c == '0' || c == '1'; }

static bool is_hex_digit(int c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool is_letter(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

static bool is_ident_continue(int c) { return is_letter(c) || is_digit(c) || c == '_'; }

static bool is_whitespace(int c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

static bool is_newline(int c) { return c == '\r' || c == '\n'; }

static unsigned hex_value(int c) {
    if (is_digit(c)) {
        return (unsigned)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return (unsigned)(c - 'a' + 10);
    }
    return (unsigned)(c - 'A' + 10);
}

/* The length of the well-formed UTF-8 sequence at i, or 0 if there is none:
   no overlongs, no surrogates, nothing past U+10FFFF (RFC 3629 §4). */
static size_t utf8_length_at(const ludo_lexer *lexer, size_t i) {
    int b0 = byte_at(lexer, i);
    size_t need;
    int lo = 0x80;
    int hi = 0xBF;
    if (b0 < 0) {
        return 0;
    } else if (b0 < 0x80) {
        return 1;
    } else if (b0 >= 0xC2 && b0 <= 0xDF) {
        need = 2;
    } else if (b0 == 0xE0) {
        need = 3;
        lo = 0xA0;
    } else if (b0 == 0xED) {
        need = 3;
        hi = 0x9F;
    } else if (b0 >= 0xE1 && b0 <= 0xEF) {
        need = 3;
    } else if (b0 == 0xF0) {
        need = 4;
        lo = 0x90;
    } else if (b0 == 0xF4) {
        need = 4;
        hi = 0x8F;
    } else if (b0 >= 0xF1 && b0 <= 0xF3) {
        need = 4;
    } else {
        return 0;
    }
    int b1 = byte_at(lexer, i + 1);
    if (b1 < lo || b1 > hi) {
        return 0;
    }
    for (size_t k = 2; k < need; k++) {
        int b = byte_at(lexer, i + k);
        if (b < 0x80 || b > 0xBF) {
            return 0;
        }
    }
    return need;
}

static void note(ludo_token *token, ludo_lex_error error) {
    if (token->error == LUDO_LEX_OK) {
        token->error = error;
    }
}

/* Step over one scalar of free text (a comment's or a string's), noting
   malformed UTF-8 and stepping one byte past it. */
static size_t step_scalar(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    size_t n = utf8_length_at(lexer, i);
    if (n == 0) {
        note(token, LUDO_LEX_INVALID_UTF8);
        return i + 1;
    }
    return i + n;
}

/* ---- Scanners ---------------------------------------------------------------
   Each takes the token's start and returns its end. */

static size_t scan_comment(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    token->kind = LUDO_TOK_COMMENT;
    i += 2;
    while (byte_at(lexer, i) >= 0 && !is_newline(byte_at(lexer, i))) {
        i = step_scalar(lexer, i, token);
    }
    return i;
}

static size_t scan_word(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    size_t start = i;
    while (is_ident_continue(byte_at(lexer, i))) {
        i++;
    }
    size_t n = i - start;
    token->kind = LUDO_TOK_IDENT;
    if (n == 1 && lexer->src[start] == '_') {
        token->kind = LUDO_TOK_UNDERSCORE;
        return i;
    }
    for (int k = LUDO_TOK_KW_AND; k <= LUDO_TOK_KW_STRUCT; k++) {
        const char *spelling = ludo_token_kind_name((ludo_token_kind)k);
        if (strlen(spelling) == n && memcmp(lexer->src + start, spelling, n) == 0) {
            token->kind = (ludo_token_kind)k;
            break;
        }
    }
    return i;
}

/* grammar.ebnf IntegerLiteral and FloatLiteral. A literal running straight
   into a letter, digit or `_` it cannot use (`0x`, `0b12`, `1e`, `2x`) is one
   malformed token rather than two well-formed ones (grammar §3.3). */
static size_t scan_number(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    token->kind = LUDO_TOK_INT;
    int radix_mark = byte_at(lexer, i + 1);
    if (byte_at(lexer, i) == '0' && (radix_mark == 'x' || radix_mark == 'b')) {
        bool hex = radix_mark == 'x';
        i += 2;
        if (hex ? is_hex_digit(byte_at(lexer, i)) : is_bin_digit(byte_at(lexer, i))) {
            i++;
            for (;;) {
                int c = byte_at(lexer, i);
                if (!(c == '_' || (hex ? is_hex_digit(c) : is_bin_digit(c)))) {
                    break;
                }
                i++;
            }
        } else {
            note(token, LUDO_LEX_MALFORMED_NUMBER);
        }
    } else {
        while (is_digit(byte_at(lexer, i)) || byte_at(lexer, i) == '_') {
            i++;
        }
        if (byte_at(lexer, i) == '.' && is_digit(byte_at(lexer, i + 1))) {
            token->kind = LUDO_TOK_FLOAT;
            i += 2;
            while (is_digit(byte_at(lexer, i)) || byte_at(lexer, i) == '_') {
                i++;
            }
        }
        int e = byte_at(lexer, i);
        if (e == 'e' || e == 'E') {
            size_t j = i + 1;
            if (byte_at(lexer, j) == '+' || byte_at(lexer, j) == '-') {
                j++;
            }
            if (is_digit(byte_at(lexer, j))) {
                token->kind = LUDO_TOK_FLOAT;
                i = j;
                while (is_digit(byte_at(lexer, i))) {
                    i++;
                }
            }
        }
    }
    if (is_ident_continue(byte_at(lexer, i))) {
        note(token, LUDO_LEX_MALFORMED_NUMBER);
        while (is_ident_continue(byte_at(lexer, i))) {
            i++;
        }
    }
    return i;
}

/* grammar.ebnf Escape, from the backslash at i. A `\u{...}` must name a
   Unicode scalar value, since the literal is UTF-8 (ch1 §1.1) and cannot hold a
   surrogate. */
static size_t scan_escape(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    int c = byte_at(lexer, i + 1);
    if (c == 'n' || c == 't' || c == 'r' || c == '0' || c == '\\' || c == '"') {
        return i + 2;
    }
    if (c == 'u') {
        size_t j = i + 2;
        if (byte_at(lexer, j) != '{') {
            note(token, LUDO_LEX_BAD_ESCAPE);
            return j;
        }
        j++;
        unsigned long value = 0;
        size_t digits = 0;
        while (is_hex_digit(byte_at(lexer, j))) {
            if (value <= 0x10FFFFUL) { /* saturates past the range; no overflow */
                value = value * 16UL + hex_value(byte_at(lexer, j));
            }
            digits++;
            j++;
        }
        if (byte_at(lexer, j) != '}') {
            note(token, LUDO_LEX_BAD_ESCAPE);
            return j;
        }
        j++;
        if (digits == 0 || value > 0x10FFFFUL || (value >= 0xD800UL && value <= 0xDFFFUL)) {
            note(token, LUDO_LEX_BAD_ESCAPE);
        }
        return j;
    }
    note(token, LUDO_LEX_BAD_ESCAPE);
    if (c < 0 || is_newline(c)) {
        return i + 1;
    }
    return step_scalar(lexer, i + 1, token);
}

/* grammar.ebnf StringLiteral. An unterminated string stops before the newline,
   which is then whitespace like any other (ch1 §3.4: no string spans a line). */
static size_t scan_string(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    token->kind = LUDO_TOK_STRING;
    i++;
    for (;;) {
        int c = byte_at(lexer, i);
        if (c < 0 || is_newline(c)) {
            note(token, LUDO_LEX_UNTERMINATED_STRING);
            return i;
        }
        if (c == '"') {
            return i + 1;
        }
        if (c == '\\') {
            i = scan_escape(lexer, i, token);
        } else {
            i = step_scalar(lexer, i, token);
        }
    }
}

/* The longest punctuation spelling at i, or 0. `_` is a word, so the search
   starts after it. */
static size_t scan_punct(const ludo_lexer *lexer, size_t i, ludo_token *token) {
    size_t best = 0;
    for (int k = LUDO_TOK_DOLLAR; k <= LUDO_TOK_REV_RANGE; k++) {
        const char *spelling = ludo_token_kind_name((ludo_token_kind)k);
        size_t n = strlen(spelling);
        if (n > best && n <= lexer->len - i && memcmp(lexer->src + i, spelling, n) == 0) {
            best = n;
            token->kind = (ludo_token_kind)k;
        }
    }
    return i + best;
}

ludo_token ludo_lex_next(ludo_lexer *lexer) {
    size_t start = lexer->pos;
    ludo_token token = {LUDO_TOK_EOF, LUDO_LEX_OK, start, 0};
    int c = byte_at(lexer, start);
    size_t end = start;

    if (c < 0) {
        return token;
    } else if (is_whitespace(c)) {
        token.kind = LUDO_TOK_WHITESPACE;
        while (is_whitespace(byte_at(lexer, end))) {
            end++;
        }
    } else if (c == '-' && byte_at(lexer, start + 1) == '-') {
        end = scan_comment(lexer, start, &token);
    } else if (is_letter(c) || c == '_') {
        end = scan_word(lexer, start, &token);
    } else if (is_digit(c)) {
        end = scan_number(lexer, start, &token);
    } else if (c == '"') {
        end = scan_string(lexer, start, &token);
    } else {
        end = scan_punct(lexer, start, &token);
        if (end == start) {
            /* One scalar, or one byte of whatever is not UTF-8. */
            token.kind = LUDO_TOK_ERROR;
            size_t n = utf8_length_at(lexer, start);
            note(&token, n == 0 ? LUDO_LEX_INVALID_UTF8 : LUDO_LEX_UNEXPECTED_CHAR);
            end = start + (n == 0 ? 1 : n);
        }
    }

    token.length = end - start;
    lexer->pos = end;
    return token;
}
