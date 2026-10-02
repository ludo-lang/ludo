/* The frontend's umbrella header. One per library, ludo_ prefix on everything
   exported, no varargs, no errno (#130). The session API is not here yet; the
   lexer is the first stage that is (#141). */
#ifndef LUDO_FRONTEND_H
#define LUDO_FRONTEND_H

#include <stdbool.h>
#include <stddef.h>

/* Compile-time identity of the library, so the gate has something to link. */
const char *ludo_frontend_version(void);

/* ---- The lexer (#141) -------------------------------------------------------

   A pull lexer over a buffer the caller owns: it allocates nothing, copies
   nothing and keeps no state outside the ludo_lexer the caller holds, so the
   session that will own an arena (#130) can store tokens however it likes.

   It is total. Every byte of the input lands in exactly one token, in order,
   and the tokens tile the buffer with no gap and no overlap -- the span
   property #130 paid for, and the invariant the fuzz target checks. A problem
   in the source never stops it: the offending bytes become a token carrying a
   ludo_lex_error, and the parser turns that into a ch7 diagnostic.

   Trivia rides the token stream, never the AST (#130): whitespace and comments
   are tokens like any other, and ludo_token_kind_is_trivia says which. */

/* Grouped: the kinds whose text varies, then the core keywords (grammar.ebnf
   Keyword), the type-sublanguage keywords (TypeKeyword), and the punctuation
   every quoted terminal of grammar.ebnf §1 and §2 spells. */
typedef enum ludo_token_kind {
    LUDO_TOK_EOF,
    LUDO_TOK_WHITESPACE,
    LUDO_TOK_COMMENT,
    LUDO_TOK_IDENT,
    LUDO_TOK_INT,
    LUDO_TOK_FLOAT,
    LUDO_TOK_STRING,
    /* Bytes that start no token: one Unicode scalar, or one byte of malformed
       UTF-8. Always carries an error. */
    LUDO_TOK_ERROR,

    LUDO_TOK_KW_AND,
    LUDO_TOK_KW_AS,
    LUDO_TOK_KW_BREAK,
    LUDO_TOK_KW_CONST,
    LUDO_TOK_KW_CONTINUE,
    LUDO_TOK_KW_DEFER,
    LUDO_TOK_KW_DO,
    LUDO_TOK_KW_ELSE,
    LUDO_TOK_KW_ELSEIF,
    LUDO_TOK_KW_END,
    LUDO_TOK_KW_EXTERN,
    LUDO_TOK_KW_FALSE,
    LUDO_TOK_KW_FN,
    LUDO_TOK_KW_FOR,
    LUDO_TOK_KW_FRAME,
    LUDO_TOK_KW_IF,
    LUDO_TOK_KW_IMPL,
    LUDO_TOK_KW_IN,
    LUDO_TOK_KW_LIBRARY,
    LUDO_TOK_KW_MATCH,
    LUDO_TOK_KW_NOT,
    LUDO_TOK_KW_OR,
    LUDO_TOK_KW_PERSIST,
    LUDO_TOK_KW_PUB,
    LUDO_TOK_KW_RESCUE,
    LUDO_TOK_KW_RETURN,
    LUDO_TOK_KW_THEN,
    LUDO_TOK_KW_TRUE,
    LUDO_TOK_KW_TYPE,
    LUDO_TOK_KW_UNLESS,
    LUDO_TOK_KW_UNSAFE,
    LUDO_TOK_KW_USE,
    LUDO_TOK_KW_WHILE,

    LUDO_TOK_KW_DISTINCT,
    LUDO_TOK_KW_ENUM,
    LUDO_TOK_KW_INTERFACE,
    LUDO_TOK_KW_NUMERIC,
    LUDO_TOK_KW_STRUCT,

    LUDO_TOK_UNDERSCORE,   /* _      the discard token, not an identifier (ch1 §2.2) */
    LUDO_TOK_DOLLAR,       /* $      the reserved stdlib root (ch1 §2.5) */
    LUDO_TOK_HASH,         /* #      attributes and #explicit (ch1 §12.1, §13.5) */
    LUDO_TOK_LPAREN,       /* ( */
    LUDO_TOK_RPAREN,       /* ) */
    LUDO_TOK_LBRACKET,     /* [ */
    LUDO_TOK_RBRACKET,     /* ] */
    LUDO_TOK_LBRACE,       /* { */
    LUDO_TOK_RBRACE,       /* } */
    LUDO_TOK_COMMA,        /* , */
    LUDO_TOK_DOT,          /* . */
    LUDO_TOK_COLON,        /* : */
    LUDO_TOK_QUESTION,     /* ? */
    LUDO_TOK_BANG,         /* ! */
    LUDO_TOK_CARET,        /* ^ */
    LUDO_TOK_AMP,          /* & */
    LUDO_TOK_PIPE,         /* | */
    LUDO_TOK_TILDE,        /* ~ */
    LUDO_TOK_PLUS,         /* + */
    LUDO_TOK_MINUS,        /* - */
    LUDO_TOK_STAR,         /* * */
    LUDO_TOK_SLASH,        /* / */
    LUDO_TOK_PERCENT,      /* % */
    LUDO_TOK_EQ,           /* = */
    LUDO_TOK_LT,           /* < */
    LUDO_TOK_GT,           /* > */
    LUDO_TOK_EQ_EQ,        /* == */
    LUDO_TOK_BANG_EQ,      /* != */
    LUDO_TOK_LT_EQ,        /* <= */
    LUDO_TOK_GT_EQ,        /* >= */
    LUDO_TOK_SHL,          /* << */
    LUDO_TOK_SHR,          /* >> */
    LUDO_TOK_PLUS_EQ,      /* += */
    LUDO_TOK_MINUS_EQ,     /* -= */
    LUDO_TOK_STAR_EQ,      /* *= */
    LUDO_TOK_SLASH_EQ,     /* /= */
    LUDO_TOK_PERCENT_EQ,   /* %= */
    LUDO_TOK_COLON_EQ,     /* := */
    LUDO_TOK_ARROW,        /* -> */
    LUDO_TOK_QUESTION_DOT, /* ?. */
    LUDO_TOK_RANGE,        /* ..<    ascending range (ch1 §7.7) */
    LUDO_TOK_REV_RANGE     /* >..    descending range (ch1 §7.7.1) */
} ludo_token_kind;

/* What went wrong in a token's bytes. A token records the first problem it
   met; the rest of the token is still consumed, so lexing resumes at a sane
   boundary. The ch7 codes these become are unassigned (ch7 §5.7). */
typedef enum ludo_lex_error {
    LUDO_LEX_OK,
    LUDO_LEX_INVALID_UTF8,        /* ch1 §1.1: the file MUST be well-formed UTF-8 */
    LUDO_LEX_UNEXPECTED_CHAR,     /* a scalar that begins no token */
    LUDO_LEX_UNTERMINATED_STRING, /* hit a newline or the end (ch1 §3.4) */
    LUDO_LEX_BAD_ESCAPE,          /* not in grammar.ebnf Escape, or not a scalar value */
    LUDO_LEX_MALFORMED_NUMBER     /* bad digits, or a letter run glued on (grammar §3.3) */
} ludo_lex_error;

/* A span is a byte offset and length into the caller's buffer, which is the
   ch7 §6.1 location shape. The text is never copied. */
typedef struct ludo_token {
    ludo_token_kind kind;
    ludo_lex_error error;
    size_t offset;
    size_t length;
} ludo_token;

typedef struct ludo_lexer {
    const char *src;
    size_t len;
    size_t pos;
} ludo_lexer;

/* The buffer must outlive every token read from it. It need not be
   NUL-terminated and may contain NULs. */
void ludo_lexer_init(ludo_lexer *lexer, const char *src, size_t len);

/* The next token. At the end it returns LUDO_TOK_EOF with length 0, at offset
   len, and keeps returning it. */
ludo_token ludo_lex_next(ludo_lexer *lexer);

bool ludo_token_kind_is_trivia(ludo_token_kind kind);

/* For a kind with one spelling, that spelling ("fn", "..<"); otherwise a
   lowercase name ("ident", "string"). */
const char *ludo_token_kind_name(ludo_token_kind kind);

/* True when the kind always has exactly the text ludo_token_kind_name gives. */
bool ludo_token_kind_has_fixed_spelling(ludo_token_kind kind);

const char *ludo_lex_error_name(ludo_lex_error error);

#endif /* LUDO_FRONTEND_H */
