# Domain Docs

How the engineering skills should consume this repo's domain documentation when exploring the codebase.

## Before exploring, read these

- **`CONTEXT.md`** at the repo root, or
- **`CONTEXT-MAP.md`** at the repo root if it exists — it points at one `CONTEXT.md` per context. Read each one relevant to the topic.
- **`docs/adr/`** — read ADRs that touch the area you're about to work in. In multi-context repos, also check `src/<context>/docs/adr/` for context-scoped decisions.

If any of these files don't exist, **proceed silently**. Don't flag their absence; don't suggest creating them upfront. The `/domain-modeling` skill (reached via `/grill-with-docs` and `/improve-codebase-architecture`) creates them lazily when terms or decisions actually get resolved.

## File structure

Single-context repo (most repos):

```
/
├── CONTEXT.md
├── docs/adr/
│   ├── 0001-event-sourced-orders.md
│   └── 0002-postgres-for-write-model.md
└── src/
```

Multi-context repo (presence of `CONTEXT-MAP.md` at the root):

```
/
├── CONTEXT-MAP.md
├── docs/adr/                          ← system-wide decisions
└── src/
    ├── ordering/
    │   ├── CONTEXT.md
    │   └── docs/adr/                  ← context-specific decisions
    └── billing/
        ├── CONTEXT.md
        └── docs/adr/
```

## Use the glossary's vocabulary

When your output names a domain concept (in an issue title, a refactor proposal, a hypothesis, a test name), use the term as defined in `CONTEXT.md`. Don't drift to synonyms the glossary explicitly avoids.

If the concept you need isn't in the glossary yet, that's a signal — either you're inventing language the project doesn't use (reconsider) or there's a real gap (note it for `/domain-modeling`).

## When you amend an ADR, stamp the one you amended

An ADR is an immutable record: **never rewrite the prose of an earlier ADR to match a later decision.** The record has to keep saying what was believed and when, because *that this was thought and then corrected* is itself the finding.

But an unstamped ADR is worse than a wrong one — a reader (human or agent) meets the superseded text with no signal that it moved, which is the stale-docs failure mode this project has evidence for. So whenever a new ADR changes, narrows, corrects or overrides any part of an earlier one, **add one line directly under the earlier ADR's front matter and change nothing else**:

```markdown
---
status: accepted
---

> **Amended by [ADR-0007](0007-the-audio-facade-surface.md):** the raw sample push's
> justification is corrected — the caller-authored clip, not the push, is the pixel
> array's analogue.

# The original title stays exactly as it was
```

Rules for the stamp: it is a **forward pointer with a one-clause gist**, never a summary of the new decision; multiple amendments **accumulate as separate lines** in date order rather than being merged; and it goes in the **same commit as the amending ADR**, so the two never disagree in the tree. The amending ADR carries the full argument, including what the earlier one got wrong and why.

## Flag ADR conflicts

If your output contradicts an existing ADR, surface it explicitly rather than silently overriding:

> _Contradicts ADR-0007 (event-sourced orders) — but worth reopening because…_
