# The C bootstrap prototype

The throwaway frontend-and-interpreter that proves the specification, in C.
Established by [ADR-0020](../docs/adr/0020-the-bootstrap-compiler-is-written-in-c.md)
and scoped by [#49](https://github.com/ludo-lang/ludo/issues/49); it lives here
rather than in a repository of its own per
[ADR-0046](../docs/adr/0046-the-repository-becomes-a-public-monorepo-in-the-ludo-lang-org.md)
§3.

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
several… retrofitting it is a frontend rewrite."*

That is why the split exists **from the first commit** rather than when a
second consumer appears.

| | |
|---|---|
| `frontend/` | Lexer, parser, typechecker, diagnostics. A library. **No ambient I/O** — no reading files, no writing to stdout, no assumed threads. Sources arrive as buffers the caller owns; diagnostics leave as values the caller renders. Parsing is error-tolerant and spans stay stable. |
| `driver/` | The CLI. Owns the filesystem, the terminal and the process; one consumer of `frontend/`, with no privileged access to it. |

The constraint is [ADR-0001](../docs/adr/0001-own-the-platform-layer.md)'s
browser argument applied one tier up: a frontend with ambient I/O cannot run on
`wasm32`, so the playground, an LSP and a formatter are all foreclosed by the
same defect. The `wasm32` target itself is deferred; the shape it requires is
not.

## Out of scope here

Recorded so they are not mistaken for omissions. From #49, and unchanged by the
move to one repository — a shared repository is not a shared scope:

- The **keeper** compiler. This is the throwaway prototype; ADR-0020 keeps one
  host language for both so the frontend carries forward, but only the
  prototype is built here.
- Codegen of any kind, the platform layer, and the runner.
- The 18-backend conformance matrix, which stays with the platform layer under
  ADR-0001.

## Not yet decided

- The C11 subset and coding standard, and the ASan/UBSan and parser-fuzzer CI
  ADR-0020 wants running from the first commit. ADR-0046 §10 leaves CI
  unspecified until there is code to run it against.
- `zig cc` as the build tool — named by ADR-0020, not yet configured.
- Whether the interpreter survives as a `ludo eval` surface, or is genuinely
  discarded (#49).
- The frontend's actual API. The seam above says where the boundary is, not
  what crosses it.
