# ludo

A game-development programming language: Lua-simple syntax, Rust/Odin/C++-grade robustness,
layered for beginners, veterans and AI agents. `docs/spec/` is the normative spec; `src/` is
the C bootstrap prototype built against it.

## Build

`make check` is the everyday signal (ASan + UBSan). `CC` defaults to `zig cc`; without zig,
`make CC=clang check`. After editing `docs/spec/reference/reference.ludo`, run `make tokens`
and commit the dump — CI diffs it.

## Before you…

- **write C in `src/`** → `docs/agents/c-standard.md`
- **touch an issue, PR or wayfinder map** → `docs/agents/issue-tracker.md`
- **triage an issue** → `docs/agents/triage-labels.md`
- **explore an area, name a domain term or write an ADR** → `docs/agents/domain.md`
- **start a branch** → `docs/agents/worktrees.md`; read `CONTEXT.md`, ADRs and the spec from
  your own worktree, never `main`'s
