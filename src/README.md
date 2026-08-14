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
| `driver/` | The CLI. Owns the filesystem, the terminal and the process; one consumer of `frontend/` among several. |

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

## An open conflict, flagged not settled

Tracked as [#95](https://github.com/ludo-lang/ludo/issues/95), which carries the
repair of `docs/project-structure.md` with it. This section goes when that
closes.

Four prior decisions put artifacts in repositories of their own — ADR-0001,
ADR-0004, ADR-0006 and #49 — and ADR-0046 §3 says `ludo-lang/ludo` *"holds
everything"*. None has been reconciled, and this
README does not reconcile them. The two that bear on this directory:

- **ADR-0001**: *"The platform layer lives in its own repo… stops an
  eighteen-backend conformance matrix from dominating a tracker that is still
  designing a language."* Its first argument — preserving #1's completion test —
  was replaced by ADR-0044 §4, but the tracker argument survives that.
- **#49** excludes the **keeper** compiler from its own scope. ADR-0020 keeps
  one host language for both so the prototype's frontend carries forward, which
  says nothing about where the keeper is built.

Until that is decided, what is in this directory is the prototype and nothing
else — a statement about the work that exists, not a boundary anyone has drawn.

## Not yet decided

- The C11 subset and coding standard, and the ASan/UBSan and parser-fuzzer CI
  ADR-0020 wants running from the first commit. ADR-0046 §10 leaves CI
  unspecified until there is code to run it against.
- `zig cc` as the build tool — named by ADR-0020, not yet configured.
- **Where the tree-walking interpreter lives.** #49 scopes one in, and the
  layout above has no place for it: it is not a lexer, parser, typechecker or
  diagnostic, and it is not the CLI. ADR-0020 does not place it either. A third
  directory beside `frontend/` and `driver/` is the obvious guess and is not a
  decision anyone has made — [#96](https://github.com/ludo-lang/ludo/issues/96).
- Whether the interpreter survives as a `ludo eval` surface, or is genuinely
  discarded (#49).
- The frontend's actual API. The seam above says where the boundary is, not
  what crosses it.
