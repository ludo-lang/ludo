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

/* Band 2 (#144): interfaces, bounded generics and the failure line. */

#define RANKED                                                                                     \
    "type Ranked[T] = interface\n  fn rank(v: T) -> f32\nend\n"                                    \
    "fn rank(v: Rock) -> f32\n  v.radius\nend\n"
#define BEST                                                                                       \
    "fn best[T: Ranked[T]](xs: []T) -> f32\n  r: f32 = 0.0\n  for x in xs do\n"                    \
    "    r = rank(x)\n  end\n  r\nend\n"
#define ERR "type Err = enum\n  bad\n  worse(i32)\nend\n"
#define COUNT "fn g(n: i32) -> i32 rescue Err\n  return Err.bad if n < 0\n  n\nend\n"

static void check_band2(void) {
    /* ch2 6.7.3: an impl is discharged by name and signature. */
    expect(RANKED "impl Rock: Ranked[Rock]\n" BEST "fn f(xs: []Rock) -> f32\n  best(xs)\nend\n",
           "");
    expect("type Ranked[T] = interface\n  fn rank(v: T) -> f32\nend\nimpl Rock: Ranked[Rock]\n",
           "bootstrap:check-unsatisfied");
    expect("type Ranked[T] = interface\n  fn rank(v: T) -> f32\nend\n"
           "fn rank(v: Rock) -> i32\n  1\nend\nimpl Rock: Ranked[Rock]\n",
           "bootstrap:check-unsatisfied");
    expect(RANKED "impl Rock: Ranked\n", "bootstrap:check-arity");
    expect(RANKED "impl Rock: Ranked[Rock]\nimpl Rock: Ranked[Rock]\n",
           "bootstrap:check-duplicate");
    expect(RANKED "impl f32: Ranked[f32]\n", "bootstrap:check-orphan-impl");
    expect("impl Rock: Shape\n", "bootstrap:check-not-an-interface");

    /* ch2 5.4: satisfaction is declared, never structural -- rank exists
       but nothing says Rock is Ranked. */
    expect(RANKED BEST "fn f(xs: []Rock) -> f32\n  best(xs)\nend\n", "bootstrap:check-unsatisfied");

    /* ch2 6.4: an interface is a constraint only. */
    expect(RANKED "fn f(r: Ranked[Rock])\nend\n", "bootstrap:check-interface-as-type");
    expect("type Marker = interface\nend\nfn f(m: Marker)\nend\n",
           "bootstrap:check-interface-as-type");

    /* ch2 5.5.1: a member shadows the module's name, and two constraints
       providing it are ambiguous. */
    expect(RANKED "type Sized[T] = interface\n  fn rank(v: T) -> f32\nend\n"
                  "fn both[T: Ranked[T] + Sized[T]](x: T) -> f32\n  rank(x)\nend\n",
           "bootstrap:check-ambiguous-member");

    /* ch2 10.6, 10.6.0a: a bare rescue needs exactly one named error set. */
    expect(ERR COUNT "fn h(n: i32) -> i32 rescue\n  g(n) rescue return\nend\n", "");
    expect(ERR "fn h(n: i32) -> i32 rescue\n  n\nend\n", "bootstrap:check-no-error-set");
    expect(ERR COUNT "type Other = enum\n  x\nend\n"
                     "fn k() -> i32 rescue Other\n  1\nend\nfn h() -> i32 rescue\n  1\nend\n",
           "bootstrap:check-ambiguous-error-set");
    expect("fn h() -> i32 rescue Rock\n  1\nend\n", "bootstrap:check-not-an-error-set");
    expect(ERR "fn h() -> Err rescue Err\n  Err.bad\nend\n", "bootstrap:check-ambiguous-fallible");

    /* ch2 10.8-10.13: the tails, and each word handling one thing. */
    expect(ERR COUNT "fn h(n: i32) -> i32\n  g(n) rescue 0\nend\n", "");
    expect(ERR COUNT "fn h(n: i32) -> i32\n  g(n) rescue 0.5\nend\n", "bootstrap:check-mismatch");
    expect(ERR COUNT "fn h(n: i32) -> i32\n  g(n) rescue return\nend\n",
           "bootstrap:check-propagation");
    expect(ERR COUNT "type Other = enum\n  x\nend\n"
                     "fn h(n: i32) -> i32 rescue Other\n  g(n) rescue return\nend\n",
           "bootstrap:check-error-set-mismatch");
    expect(ERR COUNT
           "fn h(n: i32) -> i32\n  m := g(n) rescue e then\n    return 0\n  end\n  m\nend\n",
           "");
    expect(ERR COUNT
           "fn h(n: i32) -> i32\n  k: i32 = 0\n  m := g(n) rescue e then\n    k += 1\n  end\n"
           "  m\nend\n",
           "bootstrap:check-handler-falls-through");
    expect("fn h(n: i32) -> i32\n  n rescue 0\nend\n", "bootstrap:check-not-fallible");
    expect("fn h(n: ?i32) -> i32\n  n rescue 0\nend\n", "bootstrap:check-rescue-on-optional");
    expect(ERR COUNT "fn h(n: i32) -> i32\n  g(n) or 0\nend\n", "bootstrap:check-or-on-fallible");

    /* ch2 10.11: must-use, discharged by one token. */
    expect(ERR COUNT "fn h(n: i32)\n  g(n)\nend\n", "bootstrap:check-unused-fallible");
    expect(ERR COUNT "fn h(n: i32)\n  g(n)\n  _ = n\nend\n", "bootstrap:check-unused-fallible");
    expect(ERR COUNT "fn h(n: i32)\n  _ = g(n)\nend\n", "");

    /* ch2 10.6.1: the returned value's static type picks the side. */
    expect(ERR "fn h(n: i32) -> i32 rescue Err\n  return Err.worse(n) if n < 0\n  n\nend\n", "");
    expect(ERR "fn h(n: i32) -> i32 rescue Err\n  return true\nend\n", "bootstrap:check-mismatch");
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

    check_band2();
})
