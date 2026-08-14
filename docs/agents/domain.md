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

### The convention has failed silently, so each spec chapter re-checks its own sources

Two ADRs skipped their stamps without anything noticing ([#72](https://github.com/ludo-lang/ludo/issues/72)), which means the rule depends on the author remembering. Rather than one corpus-wide audit, **a spec chapter checks the stamps on the ADRs it covers before transcribing them**, and records the result in its `coverage/` file. Chapters 2–8 discharge the audit incrementally over the ADRs that matter, which are the ones a chapter is about to turn into normative text.

The check is two-directional, and both directions have produced findings:

- **Missing stamps** — an ADR that changes, narrows or corrects one of your sources without stamping it. Add the stamp retroactively; it is additive metadata, not prose, so the immutability rule is not in tension with it.
- **Stamps not owed** — a later ADR that only *cites* your source as precedent, or that retracts its own misapplication of the source's rule. No stamp. Record the withdrawal so the next author doesn't re-derive it. (ADR-0026 *confirms* ADR-0025 §6; ADR-0043 §4's "that cut was wrong" retracts its own misreading of ADR-0017 §7, whose rule reaches only higher-order functions.)

An amendment whose target is an **issue** cannot be stamped. That is what the next section is for.

## An issue citation may not be the whole truth

The corpus cites issues and ADRs interchangeably, and [ADR-0033](../adr/0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md) §1 found this to be the common cause of every Tier-1 finding in the corpus audit: *a claim is checked against the issue that produced it rather than against the ADR that superseded it.* The citations are true but not always complete.

**[`docs/adr/SOURCES.md`](../adr/SOURCES.md) lists the issues that are still the sole authority for something, with no ADR** — the rows an author cannot derive without proving a negative across the whole corpus — and names, for each, what later ADRs moved. Check a source against it before transcribing the source into a spec chapter.

The two failures are siblings: a stamp tells you an ADR moved, and `SOURCES.md` tells you an issue did.

## Flag ADR conflicts

If your output contradicts an existing ADR, surface it explicitly rather than silently overriding:

> _Contradicts ADR-0007 (event-sourced orders) — but worth reopening because…_
