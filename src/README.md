# The C bootstrap prototype

The throwaway frontend-and-interpreter that proves the specification, in C.
Established by [ADR-0020](../docs/adr/0020-the-bootstrap-compiler-is-written-in-c.md)
and scoped by [#49](https://github.com/ludo-lang/ludo/issues/49); it lives here
rather than in a repository of its own per
[ADR-0046](../docs/adr/0046-the-repository-becomes-a-public-monorepo-in-the-ludo-lang-org.md)
§7, which puts the bootstrap in `src/` and reserves `core/` for the standard
library when it exists.

**Nothing is built yet.** This directory is a skeleton: it fixes the one seam
ADR-0020 says cannot be retrofitted, and nothing else. No parser, no
typechecker, no build.

## What this is for

Its output is **holes in the specification**, not a compiler anyone runs
(#49). [ADR-0044](../docs/adr/0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md)
§5 makes it the fastest hole-finder available and gates it on the grammar
chapter, which is done: [`docs/spec/grammar.ebnf`](../docs/spec/grammar.ebnf)
is the normative syntax authority, and this frontend's tests consume that file
directly — one working tree, one path, no vendoring (ADR-0046 §3).

The completion test is a specification test, not a software test: **every
construct the spec mandates has been parsed, typechecked and executed at least
once, and the ambiguities that surfaced are resolved in the spec** (#49).

## The seam

ADR-0020: *"The compiler frontend is a library, not a binary's internals.
Parser, typechecker and diagnostics are embeddable, with no ambient I/O,
error-tolerant parsing and stable source spans; the CLI is one consumer among
several. This is what makes the playground, an LSP and a formatter buildable at
all, and retrofitting it is a frontend rewrite."*

That is why the split exists **from the first commit** rather than when a
second consumer appears.

| | |
|---|---|
| `frontend/` | Lexer, parser, typechecker, diagnostics. A library. **No ambient I/O** — no reading files, no writing to stdout, no assumed threads. Sources arrive as buffers the caller owns; diagnostics leave as values the caller renders. Parsing is error-tolerant and spans stay stable. |
| `interp/` | The tree-walking interpreter. A library, on the **same no-ambient-I/O rule** as `frontend/`; it never links SDL3. It reaches the five facades through a **host interface the caller injects**, and it declares that interface itself. |
| `platform/` | The SDL3 host — the platform layer ([ADR-0057](../docs/adr/0057-the-delegated-platform-layer-is-sdl3.md)). Implements `interp/`'s header; nothing depends on it but the wiring. |
| `driver/` | The CLI. Owns the filesystem, the terminal and the process; one consumer of `frontend/` among several, and the thing that wires the three together. |

ADR-0020 gives the no-ambient-I/O rule two independent reasons, and neither is
the other. The library shape is what makes the playground, an LSP and a
formatter buildable at all — those are native consumers, and they need the seam
whatever happens to the browser. Separately, *"the browser is a shaping
constraint on the toolchain, exactly as
[ADR-0001](../docs/adr/0001-own-the-platform-layer.md) makes it one on the
platform-layer API: no ambient I/O and no assumed threads in the compiler, with
the `wasm32` target itself deferred."* Deferring the target does not defer the
shape it requires.

One reading is authored here rather than quoted: ADR-0020's *"one consumer among
several"* is what forbids a privileged path from `driver/` into `frontend/`, on
the grounds that a seam only one caller can cross is not a seam. ADR-0020 does
not say this in as many words.

## Out of scope here

Recorded so they are not mistaken for omissions. From #49's scope, which the
move to one repository does not touch: **codegen of any kind, the platform
layer, and the runner.** This directory is the prototype #49 describes.

## This directory holds the toolchain, and there is no second repository

Settled by [#95](https://github.com/ludo-lang/ludo/issues/95), which confirmed
ADR-0046 §3's *holds everything* against the four own-repo promises that predate
it. **The keeper compiler is built here**, not in a repo of its own — ADR-0020
keeps one host language so this `frontend/` carries forward, and it reads
`docs/spec/grammar.ebnf` by relative path, both of which break at a repository
boundary. The platform layer is SDL3, a dependency rather than a repository
(ADR-0056, ADR-0057). See `docs/project-structure.md`.

## The interpreter is a sibling, and the platform layer is injected

Settled by [#96](https://github.com/ludo-lang/ludo/issues/96), amending
[ADR-0046](../docs/adr/0046-the-repository-becomes-a-public-monorepo-in-the-ludo-lang-org.md)
§7. The interpreter fitted neither `frontend/` nor `driver/`, and the map's
destination — a **played** reference program — turned that from a tidiness
question into a load-bearing one: `reference.ludo` reaches all five facades, so
the interpreter that reaches the destination does I/O.

It is a **library beside `frontend/`, not inside it**, and it keeps the
no-ambient-I/O rule by taking the platform layer as a **parameter**: the caller
injects a host interface, `interp/` declares that interface, and `platform/`
implements it. ADR-0020's rule reads as *no I/O the caller did not supply*
rather than *no I/O*. One evaluator therefore serves two consumers — the
hole-finder passes an empty host and needs no window, the runner passes the
SDL3 host.

Deferring the facade calls as an **effect list** was the other candidate and
loses on the reference program's own call sites: five drawing calls on the draw
target return nothing, but ten or more must answer mid-expression with a value
the program branches on (`measure_text`, `button_pressed`, the voice handle,
`cursor`, `render_scale`, `$.time.now`, `save1.write`). Deferring those needs a
suspend-and-resume evaluator — too large a machine for a prototype, and
injection answers them with an ordinary call.

## Not yet decided

- Whether the interpreter survives as a `ludo eval` surface, or is genuinely
  discarded (#49).
- The **host interface's surface** — which entry points the header `interp/` declares
  actually has, and at what granularity. #96 fixed its owner and designed nothing;
  [#133](https://github.com/ludo-lang/ludo/issues/133).
- **What the empty host returns**, and therefore whether a headless hole-finding run
  is a real execution or a shape check.
  [#134](https://github.com/ludo-lang/ludo/issues/134).

## The frontend's API is a session, and the AST crosses as concrete structs

Settled by [#130](https://github.com/ludo-lang/ludo/issues/130). The seam above says
where the boundary is; this says what crosses it.

`frontend/` is entered through a **session**: the caller creates one against a module
mapping, adds source buffers, and checks it. Per-file parse and the lexer are exposed
underneath, because a formatter is a per-file consumer — but a program is not a pile of
files, and ch8 P6's incremental re-check needs something that can hold one. The session
is **immutable in the prototype**, with `replace_source` reserved by shape rather than
implemented: per-file arenas, and no cross-file pointers except through the name table.
Memory is a **session-owned arena** holding nodes and spans; source text is never copied
into it, because the caller owns the buffers.

The **AST is public `struct`s and `interp/` includes them directly**, along with the
type table and nothing else. This is deliberately right for the prototype and wrong for
the keeper — opaque node ids behind accessors is what survives a representation change —
so node access goes through a small accessor header, and the hedge is recorded here
rather than rediscovered. **Types are a side table keyed by node index**, so the parse
tree stays the single thing a formatter, an oracle and the interpreter all read, and a
formatter never links the typechecker. A **failed parse returns explicit error nodes**:
invisible recovery would hand `interp/` a tree it believes is well-formed, and P6 needs
a recovered tree to answer from. **Trivia rides the token stream, never the AST.**

A ch7 §6.1 location's `file` is a **session-minted id**, with the caller's name stored
alongside for rendering — the library never holds, or interprets, a path. Diagnostics
leave as ch7 values; one unit renders a message to a **caller-supplied buffer** as JSON,
which is not ambient I/O and is where ch7 §8.1's obligation lands. The newline, the
stream and the human form are `driver/`'s (ch7 §11.1, §1.2). **Diagnostics are the only
channel for errors in the program under compilation**; status codes cover allocation
failure and ch4 §7.4's duplicate root name, and nothing else.

**`driver/` builds the module mapping.** ch4 §7.1 makes it a required input complete
before compilation begins, §7.2 forbids the compiler to search, and §8.1 puts the
`libs/` scan on the runner. This is what makes *no ambient I/O* concrete rather than
aspirational: the library has nothing it could open.

The **oracle** (ch4 §7.9, ch8 P6) is in scope here only as a constraint on the session's
shape — stable spans, a tree surviving errors, a queryable name table. Its query surface
is not designed, because no second consumer exists yet to shape it.

**No globals, no thread-local state, no internal synchronization, and two sessions on
two threads never interact** — ADR-0020's *no assumed threads* made checkable, and the
`wasm32` shape it keeps while deferring the target. The public surface is one umbrella
header per library, a `ludo_` prefix on everything exported, no varargs and no `errno`.
The C11 subset, the coding standard and the sanitizer CI are
[#131](https://github.com/ludo-lang/ludo/issues/131)'s, not this.

## The subset is a ban list, and the compiler enforces it

Settled by [#131](https://github.com/ludo-lang/ludo/issues/131), which supplies the three
things [ADR-0020](../docs/adr/0020-the-bootstrap-compiler-is-written-in-c.md) committed to
and left unspecified. No ADR: nothing is reversed.

**The build is a plain POSIX `Makefile` with `CC ?= zig cc`.** `build.zig` would drag back
the pre-1.0 Zig churn ADR-0020 declined to pay, and CMake reintroduces an install; the `?=`
is what keeps `zig cc` the *swappable build-time dependency* ADR-0020 calls it. The `zig`
version is **pinned** — `zig cc` is clang, clang's warnings drift between releases, and
`-Werror` on a drifting set stops CI on code nobody touched.

**The subset is a ban list, because a whitelist is unenforceable prose.** The warning
configuration is the primary artifact — `-std=c11 -Wall -Wextra -Werror -Wswitch-enum
-Wconversion -Wshadow -Wvla -Wstrict-prototypes` — and the written bans cover only what a
flag cannot see: VLAs, `alloca`, varargs, `errno`, file-scope mutable globals, TLS,
`threads.h`, atomics, `_Generic`, `malloc` outside the arena, unions for type-punning,
`goto` except single-exit cleanup, and function-like macros where an `inline` works.
**`default:` is banned in a switch over a tag enum, and that ban is ADR-0020's "canonical
switch helper"**: a `default:` case is precisely what silences `-Wswitch-enum`, so the ban
does the work and no macro is added. Formatting is `clang-format`, from the same pinned
toolchain.

**CI is Linux-only and path-filtered to `src/**`**: ASan+UBSan over the whole suite per
push, Valgrind and a long fuzz run nightly. **MSan is not adopted** — with a session-owned
arena and no `malloc` ([#130](https://github.com/ludo-lang/ludo/issues/130)) it finds
little ASan does not — **TSan never**, since it contradicts the no-assumed-threads rule
rather than checking it, and **AFL++ is deferred** because libFuzzer ships with the
toolchain already chosen. macOS and Windows are **compile-only cross checks**; without them
ADR-0020's single-binary cross-compilation claim is untested. Development is on macOS and
**CI decides** — `make check` reproduces the everyday signal locally, and Valgrind is
CI-only because it is dead on Apple Silicon.

***From the first commit* means not retrofitted.** The libFuzzer target lands with the
lexer, and its **corpus is committed as the permanent regression suite** — a CI cache is
evictable, and the value of fuzzing a parser is that findings never come back. Tests are
hand-rolled: one assert header, one binary per library, `make test`'s exit code is the
verdict.

The files themselves — `docs/agents/c-standard.md` and its checker
([ADR-0051](../docs/adr/0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md) §6), the
`Makefile`, `.clang-format`, `build.yml` and `nightly.yml` — are the build's first commit,
not a wayfinder session's output. `platform/` stays out of CI until it has code, which
leaves **how CI acquires SDL3** open; it waits on *which OS first*.
