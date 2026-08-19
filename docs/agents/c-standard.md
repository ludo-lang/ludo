# The C standard for `src/`

Settled by [#131](https://github.com/ludo-lang/ludo/issues/131), which supplies what
[ADR-0020](../adr/0020-the-bootstrap-compiler-is-written-in-c.md) committed to and left
unspecified. This file is **the remainder**: prose bans only what a flag cannot see.

The primary artifact is the warning configuration in the root `Makefile`:

```
-std=c11 -Wall -Wextra -Werror -Wswitch-enum -Wconversion -Wshadow -Wvla -Wstrict-prototypes
```

A ban list nothing checks is the failure this exists to prevent. Everything mechanisable is
mechanised — by the compiler where a flag sees it, by `tools/check-c-standard.py` where a
grep does ([ADR-0051](../adr/0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md)
§6: a convention in `docs/agents/` ships with a checker). What neither can see is reviewed
by a human, and is the short list at the end.

## Scope

Everything under `src/`, with two carve-outs that are not exceptions to the *rules* but to
where the rules bite:

- **`src/driver/`** owns the filesystem, the terminal and the process (ADR-0020). It may
  call the `printf` family and open files; `frontend/` and `interp/` may not.
- **Test binaries** (`src/*/tests/`) and `src/test.h` may hold file-scope mutable state and
  print. They are not library code.

`src/platform/` has no code and is not wired into the build or CI until it does.

## Banned, and the compiler enforces it

Nothing to remember: these fail the build.

| Ban | Enforced by |
|---|---|
| Variable-length arrays | `-Wvla` |
| Non-exhaustive switch over a tag enum | `-Wswitch-enum -Werror` |
| Implicit narrowing or sign conversion | `-Wconversion` |
| Shadowed declarations | `-Wshadow` |
| `()` in place of `(void)` in a prototype | `-Wstrict-prototypes` |
| Anything C11 does not have | `-std=c11` |

## Banned, and a grep enforces it

`tools/check-c-standard.py`, run by `make standard` and by CI.

- **`alloca`** — an unbounded stack allocation with no failure mode.
- **`errno`** — the frontend reports through diagnostics and status codes only (#130); the
  driver reads a return value, never a global.
- **`malloc` / `calloc` / `realloc` / `free` outside the arena** — memory is a
  session-owned arena (#130). The arena implementation is the one place allowed to allocate,
  and it says so with an explicit allow-comment.
- **File-scope mutable globals** — no globals, no shared state, two sessions on two threads
  never interact (#130). A file-scope `const` or `static const` is fine.
- **`threads.h`, `<stdatomic.h>`, `_Thread_local`, `thread_local`** — ADR-0020's *no assumed
  threads*, which is a shape the deferred `wasm32` target still requires.
- **`_Generic`** — type-directed dispatch a reader cannot follow, in a codebase whose whole
  argument is that an agent's mistakes should be visible.
- **`default:` in a switch over a tag enum** — this ban **is** ADR-0020's "canonical switch
  helper". A `default:` case is precisely what silences `-Wswitch-enum`, so banning it makes
  the flag fire on every missed AST node. No macro is added, because a wrapper obscures the
  control flow both agents and humans read most often. A switch over a plain `int`, a `char`
  or a bitmask is not a tag-enum switch and may have a `default:` — write it as
  `default: /* not-a-tag-enum */` so the checker can tell.

## Banned, and only a reader enforces it

The remainder, in review:

- **Varargs functions of our own** — no `<stdarg.h>` in a definition we write. Calling
  `printf` from `driver/` or a test is fine; exporting a `ludo_*` function that takes `...`
  is not, because it defeats every prototype check the flags buy.
- **Unions used for type-punning** — a union is a tagged variant. Reinterpreting bytes goes
  through `memcpy`.
- **`goto` except single-exit cleanup** — one label at the end of a function, jumped to
  forward. Never backwards, never into a block.
- **Function-like macros where an `inline` function works** — a macro that could have been a
  function loses type checking and lies in a debugger. `test.h`'s assert macro is the shape
  that earns one: it needs `__FILE__`, `__LINE__` and the stringised expression.

## The toolchain is pinned

`zig cc` is clang, and clang's warning set drifts between releases: `-Werror` on a drifting
set red-lines CI on code nobody touched. CI pins the exact `zig` version; `src/README.md`
documents the minimum for a development host.

**`clang-format` is pinned separately.** #131 assumed it shipped with the pinned toolchain;
it does not — `zig` exposes `zig cc` and `zig ar`, not `clang-format` — so CI installs a
pinned `clang-format` package instead. The config is `.clang-format` at the root: LLVM base,
4-space indent, 100 columns. `make format` applies it, `make format-check` is what CI runs.

## What CI does

`build.yml`, per push, `ubuntu-latest`, path-filtered to `src/**`, the `Makefile` and itself:
`make check` (ASan+UBSan over the suite, `-fno-sanitize-recover=all`), `make format-check`,
`make standard`, and `make cross` for the macOS and Windows compile-only checks.

`nightly.yml`, on `schedule:`: Valgrind over the suite, and the long fuzz run once the
libFuzzer target lands with the lexer.

**Development is on macOS and CI decides.** `make check` reproduces the everyday signal
locally; Valgrind is CI-only, because it is effectively dead on Apple Silicon and is a
backstop for ASan here rather than a primary.
