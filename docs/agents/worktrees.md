# Worktrees

This repository is cloned as a **bare repo with sibling worktrees**, not as a single
checkout. Every branch you work on is its own directory.

```
ludo/
├── .bare/          the bare repository — never edit anything here
├── .git            a file, not a directory: `gitdir: ./.bare`
├── main/           worktree for branch `main`
└── netx-js/        worktree for branch `netx-js`
```

## Read your own worktree, never a sibling

"The repo root" is ambiguous here — there are as many roots as there are worktrees.
Whenever a skill or doc says to read `CONTEXT.md`, `docs/adr/` or anything else "at
the repo root", it means **the root of the worktree you are working in**.

This matters because ADRs, the spec and `CONTEXT.md` are versioned content. A branch
may be *editing* them; reading `main/`'s copy from a feature worktree would silently
hide exactly the change under review.

## Directory names mirror branch names

One branch, one directory, same name — so a path is never a second lookup. Branch
names containing `/` flatten with `-`:

| branch | directory |
| --- | --- |
| `main` | `main/` |
| `netx-js` | `netx-js/` |
| `proto/07-syntax-candidates` | `proto-07-syntax-candidates/` |

No grouping directories (`feature/`, `research/`). They only start earning their
keep past roughly ten live worktrees.

## Worktrees are created on demand

Most branches on `origin` have no local worktree, and that is the normal state. Add
one when you actually start work:

```sh
git worktree add ../proto-07-syntax-candidates proto/07-syntax-candidates
```

and remove it when the branch merges:

```sh
git worktree remove ../proto-07-syntax-candidates
```

## Ignored files do not follow you

A fresh worktree starts without any git-ignored file — local settings, scratch
directories, build output. Recreate them per worktree, or commit them if they are
genuinely shareable. **Do not symlink them between worktrees:** a branch would then
be unable to report its own state honestly.
