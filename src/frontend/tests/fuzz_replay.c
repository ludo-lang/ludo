/* Replays files through the fuzz target without libFuzzer, so the committed
   corpus is a regression suite under any CC and inside `make check`'s
   sanitizers (#131). Each argument is one input. */
#include <stdint.h>
#include <stdio.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/* A test binary may hold file-scope state (docs/agents/c-standard.md). The
   corpus and the reference program sit far below it. */
static uint8_t input[1 << 20];

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb");
        if (f == NULL) {
            (void)fprintf(stderr, "fuzz_replay: cannot read %s\n", argv[i]);
            return 1;
        }
        size_t n = fread(input, 1, sizeof input, f);
        int truncated = !feof(f);
        (void)fclose(f);
        if (truncated) {
            (void)fprintf(stderr, "fuzz_replay: %s is larger than %zu bytes\n", argv[i],
                          sizeof input);
            return 1;
        }
        (void)LLVMFuzzerTestOneInput(input, n);
    }
    (void)printf("ok (%d input(s))\n", argc - 1);
    return 0;
}
