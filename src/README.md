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

- The C11 subset and coding standard, and the ASan/UBSan and parser-fuzzer CI
  ADR-0020 wants running from the first commit. ADR-0046 §10 leaves CI
  unspecified until there is code to run it against.
- `zig cc` as the build tool — named by ADR-0020, not yet configured.
- Whether the interpreter survives as a `ludo eval` surface, or is genuinely
  discarded (#49).
- The frontend's actual API — [#130](https://github.com/ludo-lang/ludo/issues/130).
  The seam above says where the boundary is, not what crosses it. #96 put one
  floor under it and designed nothing: a sibling `interp/` consumes the AST, so
  **the AST crosses**. Its exported shape is #130's to decide.
