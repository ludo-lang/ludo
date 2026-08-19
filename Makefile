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

# platform/ is the SDL3 host and has no code; it is deliberately not wired
# into the build or CI until it does (#131).
FRONTEND_SRC = src/frontend/frontend.c
INTERP_SRC = src/interp/interp.c
DRIVER_SRC = src/driver/main.c

LIB_SRC = $(FRONTEND_SRC) $(INTERP_SRC)

TESTS = $(BUILD)/test_frontend $(BUILD)/test_interp

.PHONY: all test check cross clean format format-check standard

all: $(BUILD)/ludo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/ludo: $(DRIVER_SRC) $(LIB_SRC) | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ $(DRIVER_SRC) $(LIB_SRC)

$(BUILD)/test_frontend: src/frontend/tests/test_frontend.c $(FRONTEND_SRC) | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/frontend/tests/test_frontend.c $(FRONTEND_SRC)

$(BUILD)/test_interp: src/interp/tests/test_interp.c $(INTERP_SRC) | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ src/interp/tests/test_interp.c $(INTERP_SRC)

# One binary per library; the exit code is the verdict.
test: $(TESTS)
	@for t in $(TESTS); do echo "run $$t"; $$t || exit 1; done

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
