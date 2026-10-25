# The C bootstrap's build (#131).
#
# ADR-0020 calls zig cc a build-time dependency that can be swapped, not a
# codebase we are married to, so CC must stay a one-variable change. `CC ?=`
# does not achieve that: make defines CC=cc as a built-in, so ?= never fires
# and the toolchain silently becomes the host cc. Overriding only the built-in
# keeps `make CC=clang` (and CC from the environment) working while making
# zig cc the actual default.
ifeq ($(origin CC),default)
CC = zig cc
endif

BUILD ?= build

STD = -std=c11
WARNINGS = -Wall -Wextra -Werror -Wswitch-enum -Wconversion -Wshadow -Wvla \
           -Wstrict-prototypes
OPT ?= -O1 -g
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all \
      -fno-omit-frame-pointer
EXTRA ?=

INCLUDES = -Isrc -Isrc/frontend/include -Isrc/interp/include
ALL_CFLAGS = $(STD) $(WARNINGS) $(OPT) $(INCLUDES) $(EXTRA) $(CFLAGS)
# fmod and fmodf: float % (ch2 16.2) evaluated at the operand's own width.
LDLIBS = -lm

# platform/ is the SDL3 host and has no code; it is deliberately not wired
# into the build or CI until it does (#131).
FRONTEND_SRC = src/frontend/frontend.c src/frontend/lexer.c src/frontend/arena.c \
               src/frontend/parser.c src/frontend/ast.c src/frontend/diagnostic.c \
               src/frontend/check.c
INTERP_SRC = src/interp/interp.c src/interp/stub_host.c src/interp/eval.c
DRIVER_SRC = src/driver/main.c

FRONTEND_HEADERS = $(wildcard src/frontend/include/*.h) src/frontend/arena.h src/frontend/session.h
HEADERS = $(FRONTEND_HEADERS) $(wildcard src/interp/include/*.h) src/test.h

LIB_SRC = $(FRONTEND_SRC) $(INTERP_SRC)

TESTS = $(BUILD)/test_frontend $(BUILD)/test_parser $(BUILD)/test_check $(BUILD)/test_interp

# The lexer's committed token dump (#141). Committed rather than generated so a
# change to what the lexer reads is a diff someone reviews. After an edit to
# reference.ludo, `make tokens` regenerates it.
TOKEN_SRC = docs/spec/reference/reference.ludo
TOKEN_DUMP = src/frontend/tests/reference.tokens
AST_DUMP = src/frontend/tests/reference.ast
PARSER_INPUTS = $(wildcard src/frontend/tests/corpus/parser/*) $(FUZZ_INPUTS) \
                $(wildcard docs/spec/reference/libs/vec-utils/*.ludo)

# The fuzz target's committed corpus is the permanent regression suite (#131).
# The reference program's own files are replayed beside it, so they stay seeds
# without being copied in.
FUZZ_SRC = src/frontend/tests/fuzz_lexer.c
FUZZ_CORPUS = src/frontend/tests/corpus/lexer
FUZZ_INPUTS = $(wildcard $(FUZZ_CORPUS)/*) $(wildcard docs/spec/reference/*.ludo)

.PHONY: all test check cross clean format format-check standard tokens tokens-check ast ast-check fuzz fuzz-lexer fuzz-parser

all: $(BUILD)/ludo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/ludo: $(DRIVER_SRC) $(LIB_SRC) $(HEADERS) | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ $(DRIVER_SRC) $(LIB_SRC) $(LDLIBS)

$(BUILD)/test_frontend: src/frontend/tests/test_frontend.c $(FRONTEND_SRC) $(FRONTEND_HEADERS) src/test.h | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/frontend/tests/test_frontend.c $(FRONTEND_SRC)

$(BUILD)/test_parser: src/frontend/tests/test_parser.c $(FRONTEND_SRC) $(FRONTEND_HEADERS) src/test.h | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/frontend/tests/test_parser.c $(FRONTEND_SRC)

$(BUILD)/test_check: src/frontend/tests/test_check.c $(FRONTEND_SRC) $(FRONTEND_HEADERS) src/test.h | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/frontend/tests/test_check.c $(FRONTEND_SRC)

$(BUILD)/test_interp: src/interp/tests/test_interp.c $(LIB_SRC) $(HEADERS) | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/interp/tests/test_interp.c $(INTERP_SRC) $(FRONTEND_SRC) $(LDLIBS)

$(BUILD)/fuzz_replay: src/frontend/tests/fuzz_replay.c $(FUZZ_SRC) $(FRONTEND_SRC) $(FRONTEND_HEADERS) src/test.h | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/frontend/tests/fuzz_replay.c $(FUZZ_SRC) $(FRONTEND_SRC)

# One binary per library; the exit code is the verdict. The corpus replay and
# the token dump ride along, so `make check` runs both under the sanitizers.
test: $(TESTS) $(BUILD)/fuzz_replay $(BUILD)/fuzz_parser_replay tokens-check ast-check
	@for t in $(TESTS); do echo "run $$t"; $$t || exit 1; done
	@echo "run $(BUILD)/fuzz_replay"; $(BUILD)/fuzz_replay $(FUZZ_INPUTS)
	@echo "run $(BUILD)/fuzz_parser_replay"; $(BUILD)/fuzz_parser_replay $(PARSER_INPUTS)

ast: $(BUILD)/ludo
	$(BUILD)/ludo ast $(TOKEN_SRC) > $(AST_DUMP)

ast-check: $(BUILD)/ludo
	@echo "diff $(AST_DUMP)"
	@$(BUILD)/ludo ast $(TOKEN_SRC) > $(BUILD)/reference.ast && \
	  diff -u $(AST_DUMP) $(BUILD)/reference.ast \
	  || { echo "AST dump invalid or out of date: run make ast and review the diff"; exit 1; }

$(BUILD)/fuzz_parser_replay: src/frontend/tests/fuzz_replay.c src/frontend/tests/fuzz_parser.c $(FRONTEND_SRC) $(FRONTEND_HEADERS) | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/frontend/tests/fuzz_replay.c src/frontend/tests/fuzz_parser.c $(FRONTEND_SRC)

tokens: $(BUILD)/ludo
	$(BUILD)/ludo tokens $(TOKEN_SRC) > $(TOKEN_DUMP)

tokens-check: $(BUILD)/ludo
	@echo "diff $(TOKEN_DUMP)"
	@$(BUILD)/ludo tokens $(TOKEN_SRC) | diff -u $(TOKEN_DUMP) - \
	  || { echo "token dump out of date: run make tokens and review the diff"; exit 1; }

# libFuzzer is clang's, not zig cc's, so the fuzz build names its own
# compiler. New inputs land in $(BUILD)/fuzz-corpus; a finding worth keeping is
# minimised and committed to $(FUZZ_CORPUS) by hand.
FUZZ_CC ?= clang
FUZZ_TIME ?= 60
$(BUILD)/fuzz_lexer: $(FUZZ_SRC) $(FRONTEND_SRC) $(FRONTEND_HEADERS) src/test.h | $(BUILD)
	$(FUZZ_CC) $(STD) $(WARNINGS) -O1 -g $(INCLUDES) -fsanitize=fuzzer,address,undefined \
	  -fno-sanitize-recover=all -o $@ $(FUZZ_SRC) $(FRONTEND_SRC)

fuzz: fuzz-lexer fuzz-parser

fuzz-lexer: $(BUILD)/fuzz_lexer
	mkdir -p $(BUILD)/fuzz-corpus
	$(BUILD)/fuzz_lexer -max_total_time=$(FUZZ_TIME) \
	  $(BUILD)/fuzz-corpus $(FUZZ_CORPUS) docs/spec/reference

$(BUILD)/fuzz_parser: src/frontend/tests/fuzz_parser.c $(FRONTEND_SRC) $(FRONTEND_HEADERS) | $(BUILD)
	$(FUZZ_CC) $(STD) $(WARNINGS) -O1 -g $(INCLUDES) -fsanitize=fuzzer,address,undefined \
	  -fno-sanitize-recover=all -o $@ src/frontend/tests/fuzz_parser.c $(FRONTEND_SRC)

fuzz-parser: $(BUILD)/fuzz_parser
	mkdir -p $(BUILD)/fuzz-parser-corpus
	$(BUILD)/fuzz_parser -max_total_time=$(FUZZ_TIME) \
	  $(BUILD)/fuzz-parser-corpus src/frontend/tests/corpus/parser $(FUZZ_CORPUS) docs/spec/reference

# The everyday signal, identical on the macOS dev host and in Linux CI.
#
# It builds into its own directory. Sharing $(BUILD) with `test` would make the
# sanitized and unsanitized binaries the same targets: after a plain `make
# test` they are newer than their sources, make skips the rebuild, and `check`
# silently re-runs the unsanitized binaries and reports green.
check:
	$(MAKE) BUILD="$(BUILD)/san" EXTRA="$(SAN) $(EXTRA)" test

# Compile-only cross checks. ADR-0020's single-binary cross-compilation claim
# is untested without them. zig cc only; a swapped CC will not have --target.
cross:
	$(MAKE) cross-each TARGET=aarch64-macos
	$(MAKE) cross-each TARGET=x86_64-windows-gnu

.PHONY: cross-each
cross-each:
	@echo "cross $(TARGET)"
	@for f in $(DRIVER_SRC) $(LIB_SRC); do \
	  $(CC) $(STD) $(WARNINGS) $(INCLUDES) --target=$(TARGET) -c $$f -o /dev/null || exit 1; \
	done

# clang-format is not shipped by zig; CI pins it as a package (see
# docs/agents/c-standard.md).
format:
	clang-format -i `git ls-files '*.c' '*.h'`

format-check:
	clang-format --dry-run -Werror `git ls-files '*.c' '*.h'`

standard:
	python3 tools/check-c-standard.py

clean:
	rm -rf $(BUILD)
