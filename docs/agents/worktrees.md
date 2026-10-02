# Worktrees

This repository is an **ordinary checkout**: `~/ludo` holds `.git/` and the files of
whatever branch is checked out there, normally `main`. Work that should not happen on
`main` gets its own **git worktree** — a second directory sharing the same repository.

```
ludo/
├── .git/                    the repository
├── docs/ src/ tools/ ...    main's files
└── .claude/worktrees/       one directory per feature worktree; git-ignored
    └── lexer/
```

## Read your own worktree, never main's

"The repo root" means **the root of the worktree you are working in**. Whenever a
skill or doc says to read `CONTEXT.md`, `docs/adr/` or anything else "at the repo
root", read your worktree's copy, not `~/ludo`'s.

This matters because ADRs, the spec and `CONTEXT.md` are versioned content. A branch
may be *editing* them; reading `main`'s copy from a feature worktree would silently
hide exactly the change under review.

## Where worktrees live

Under `.claude/worktrees/`, which is git-ignored, one directory per branch. The
desktop app creates them there itself; by hand:

```sh
git worktree add .claude/worktrees/lexer -b lexer origin/main
```

and remove the worktree when the branch merges:

```sh
git worktree remove .claude/worktrees/lexer
```

A branch name containing `/` flattens with `-` in the directory name
(`proto/07-syntax-candidates` → `proto-07-syntax-candidates`).

## Ignored files do not follow you

A fresh worktree starts without any git-ignored file — local settings, scratch
directories, build output. Recreate them per worktree, or commit them if they are
genuinely shareable. **Do not symlink them between worktrees:** a branch would then
be unable to report its own state honestly.

## Why not a bare repo with sibling worktrees

This clone used that layout until 2026-10-02: `.bare/`, a `.git` file pointing at it,
and one sibling directory per branch (`main/`, `lexer/`, ...). It made the repository's
own root a non-checkout, and tools that key on a folder disagreed with it — the desktop
app opened at `~/ludo` diffed that folder as a work tree and reported every tracked file
deleted, and per-folder agent memory split between `~/ludo` and `~/ludo/main`. An
ordinary checkout gives every tool one root that is also a working tree.
