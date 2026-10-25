/* The checker's reject side for band 1's rules (#143). reference.ludo is
   well-formed by construction, so it witnesses no MUST-reject clause; #149
   grows the corpus, and these are the cases the band itself introduced. */
#include "ludo_frontend.h"
#include "test.h"

#include <string.h>

static const char prelude[] = "type Rock = struct\n"
                              "  radius: f32 = 4.0\n"
                              "end\n"
                              "type Shape = enum\n"
                              "  point\n"
                              "  circle(f32)\n"
                              "end\n"
                              "type Id = distinct u32\n";

/* The first check diagnostic's code, or "" for a clean file. */
static const char *verdict(const char *body, char *buffer, size_t capacity) {
    ludo_session *session = NULL;
    const ludo_parsed_file *file = NULL;
    const ludo_checked_file *checked = NULL;
    const char *code = "host-failure";
    size_t prelude_length = sizeof prelude - 1;
    size_t length = prelude_length + strlen(body);
    if (length + 1 > capacity) {
        return code;
    }
    memcpy(buffer, prelude, prelude_length);
    memcpy(buffer + prelude_length, body, strlen(body) + 1);
    if (ludo_session_create(&session) == LUDO_FRONTEND_OK &&
        ludo_parse_file(session, "case.ludo", buffer, length, &file) == LUDO_FRONTEND_OK &&
        ludo_check_file(session, file, &checked) == LUDO_FRONTEND_OK) {
        code = file->diagnostic_count != 0      ? "parse-error"
               : checked->diagnostic_count == 0 ? ""
                                                : checked->diagnostics[0].code;
    }
    ludo_session_destroy(session);
    return code;
}

static void expect(const char *body, const char *code) {
    char buffer[2048];
    const char *got = verdict(body, buffer, sizeof buffer);
    if (strcmp(got, code) != 0) {
        (void)fprintf(stderr, "expected '%s', got '%s' for:\n%s\n", code, got, body);
    }
    LUDO_CHECK(strcmp(got, code) == 0);
}

LUDO_TEST_MAIN({
    /* ch1 5.6.1: an unannotated const takes its initialiser's type, and a
       bare literal has none. */
    expect("const MAX = 64\n", "bootstrap:check-ambiguous-literal");
    expect("const MAX: usize = 64\n", "");
    expect("const MAX: u8 = 300\n", "bootstrap:check-literal-range");
    expect("fn f() -> i32\n  1\nend\nconst MAX: i32 = f()\n", "bootstrap:check-not-constant");

    /* ch1 6.7.2: a function with a return type does not fall off its end. */
    expect("fn f(x: bool) -> i32\n  if x then\n    return 1\n  end\nend\n",
           "bootstrap:check-missing-return");
    expect("fn f(x: bool) -> i32\n  if x then\n    return 1\n  else\n    return 2\n  end\nend\n",
           "");
    expect("fn f(x: bool) -> i32\n  while x do\n    return 1\n  end\nend\n",
           "bootstrap:check-missing-return");

    /* ch2 4.3: exhaustive, naming the uncovered variant. */
    expect("fn f(s: Shape) -> f32\n  match s do\n    point then\n      return 0.0\n    end\n"
           "  end\nend\n",
           "bootstrap:check-non-exhaustive");
    expect("fn f(s: Shape) -> f32\n  match s do\n    point then\n      return 0.0\n    end\n"
           "    other then\n      return 1.0\n    end\n  end\nend\n",
           "");

    /* ch2 9.10: ?T is never used where T is required. */
    expect("fn f(x: ?Rock) -> f32\n  x.radius\nend\n", "bootstrap:check-optional-as-value");
    expect("fn f(x: ?Rock) -> f32\n  x?.radius or 0.0\nend\n", "");

    /* ch2 16.4: conditions are bool only. */
    expect("fn f(n: i32) -> i32\n  return 1 if n\n  0\nend\n", "bootstrap:check-condition");

    /* ch2 5.8: a generic becomes a value only when instantiated. */
    expect("fn first[T](xs: []T) -> ?T\n  return none\nend\n"
           "fn f() -> i32\n  g := first\n  0\nend\n",
           "bootstrap:check-generic-value");

    /* ch2 12.2.1: a local is rebound whole; a parameter is never rebound;
       writing into a place needs the mark, and the mark needs a ! binding. */
    expect("fn f() -> i32\n  n: i32 = 0\n  n += 1\n  n\nend\n", "");
    expect("fn f(n: i32) -> i32\n  n = 1\n  n\nend\n", "bootstrap:check-immutable");
    expect("fn f(r: !Rock)\n  r.radius = 1.0\nend\n", "bootstrap:check-unmarked-mutation");
    expect("fn f(r: Rock)\n  r!.radius = 1.0\nend\n", "bootstrap:check-immutable");
    expect("fn f(r: !Rock)\n  r!.radius = 1.0\nend\n", "");
    expect("fn g(r: !Rock)\n  r!.radius = 1.0\nend\nfn f(r: !Rock)\n  g(r)\nend\n",
           "bootstrap:check-unmarked-mutation");

    /* ch1 3.2 and ch2 3.1: no silent pick, and distinct loses arithmetic. */
    expect("fn f() -> bool\n  1 < 2\nend\n", "bootstrap:check-ambiguous-literal");
    expect("fn f(a: Id, b: Id) -> Id\n  a + b\nend\n", "bootstrap:check-operator");
    expect("fn f(a: Id, b: Id) -> bool\n  a < b\nend\n", "");
    expect("fn f(a: i32, b: i64) -> i32\n  a + b\nend\n", "bootstrap:check-operator");

    /* ch2 4.6.1: a variant is constructed qualified, with its payload. */
    expect("fn f() -> Shape\n  Shape.circle(1.0)\nend\n", "");
    expect("fn f() -> Shape\n  Shape.circle\nend\n", "bootstrap:check-arity");
    expect("fn f() -> Shape\n  Shape.square\nend\n", "bootstrap:check-unknown-variant");

    expect("fn f() -> i32\n  nope\nend\n", "bootstrap:check-unknown-name");
    expect("fn f() -> i32\n  0\nend\nfn f() -> i32\n  1\nend\n", "bootstrap:check-duplicate");
})
