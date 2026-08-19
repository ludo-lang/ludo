/* Hand-rolled test harness (#131): one assert macro, one binary per library.
   make test runs them and the exit code is the verdict. */
#ifndef LUDO_TEST_H
#define LUDO_TEST_H

#include <stdio.h>

/* The only mutable state a test binary is allowed; file-scope mutable globals
   are banned in library code (docs/agents/c-standard.md), and a test binary is
   not library code. */
static int ludo_test_failures = 0;

#define LUDO_CHECK(cond)                                                                           \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            ludo_test_failures++;                                                                  \
            (void)fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                  \
        }                                                                                          \
    } while (0)

#define LUDO_TEST_MAIN(body)                                                                       \
    int main(void) {                                                                               \
        body;                                                                                      \
        if (ludo_test_failures != 0) {                                                             \
            (void)fprintf(stderr, "%d check(s) failed\n", ludo_test_failures);                     \
            return 1;                                                                              \
        }                                                                                          \
        (void)fputs("ok\n", stdout);                                                               \
        return 0;                                                                                  \
    }

#endif /* LUDO_TEST_H */
