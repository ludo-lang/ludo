# Domain Docs

How the engineering skills should consume this repo's domain documentation when exploring the codebase.

## Before exploring, read these

- **`CONTEXT.md`** at your worktree's root, or
- **`CONTEXT-MAP.md`** at your worktree's root if it exists — it points at one `CONTEXT.md` per context. Read each one relevant to the topic.
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

## A marked gap may not outlive its answer

There is a third sibling, and it points at the spec rather than at the corpus. A spec chapter may
write down a **marked gap** rather than block on it (ADR-0044 §8) — but the ticket that gap files
**owns the repair**. Per [ADR-0049](../adr/0049-a-marked-gap-and-its-repair-are-one-obligation.md),
resolving such a ticket is not done when its ADR lands; it is done when **the chapter text, the
chapter's `coverage/` file and the reference program no longer say the gap is open**, all in the
**same commit** as the answer.

This is the stamp rule applied to a second artifact, for the same reason: a stale normative clause
is an unstamped ADR wearing different clothes — true when written, false now, with nothing in the
text warning the reader. Agents read the spec, not the ADR graph.

So a ticket filed by a marked gap says so **in its own body from the day it is filed**: *…and
repair chapter N's clause, coverage rows and gap list.* The one thing that may split into separate
work is a large **reference-program** extension, and only with the split recorded on the map; the
clause repair never splits.

## The spec is where you read the rule; `STATUS.md` says whether an ADR still is

[ADR-0051](../adr/0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md)
makes a landed spec chapter the **only normative surface**. An ADR a chapter has absorbed keeps
the argument and loses the authority, and says so in a stamp of its own — same one-line mechanism
as amendment, in the chapter's commit, owed **whether or not the chapter changed the rule**:

```markdown
> **Absorbed by [spec §6.2](../spec/06-the-facades.md):** the chapter is normative; this ADR
> keeps the argument.
```

Until chapters 4–8 land, **[`docs/adr/STATUS.md`](../adr/STATUS.md)** is the bridge: one row per
ADR, `ADR-NNNN | status | one-clause gist | declared edges | suspected unstamped targets`. Check a
source against it before citing the source, the way you already check
[`SOURCES.md`](../adr/SOURCES.md) before transcribing an issue.

Two properties are load-bearing and easy to erode:

- It is **derived**. Every row restates an edge the tree already carries — a stamp, a supersession
  line, a `coverage/` entry. Never write a row that is true only in `STATUS.md`; fix the tree and
  let the row follow.
- It is **temporary**. Each landing chapter converts rows to `absorbed-by-ch-N`. When the last
  chapter lands the file is deleted. Do not grow it into a reference; that is the spec's job.

## Record an induced hole in the chapter that opened it

A [hole](../../CONTEXT.md#hole) found by transcription is the scanner working and needs no
bookkeeping — it is a finite pass over a fixed corpus. An
**[induced hole](../../CONTEXT.md#induced-hole)** — one opened by *writing a repair* rather than by
reading — is feedback, and is the only class that can diverge.

So when resolving a ticket opens a new hole, add one line to the `coverage/` file of the chapter it
lands in, naming the ticket whose repair opened it. Nothing else: the ledger answers a **trend**
question, not a per-item one. A short list across chapters means hole arrival is a throughput
problem and the answer is to keep going. A list that grows chapter over chapter means a loop — and
names the repairs driving it, which no amount of auditing would tell you.

The ledger starts with chapter 4. Chapters 1–3 are not backfilled: reconstructing cause for closed
tickets is archaeology, and a trend needs the chapters still to come.

## A convention here ships with a checker

The stamp rule failed silently twice ([#72](https://github.com/ludo-lang/ludo/issues/72)) because
it depended on the author remembering. Per ADR-0051 §6: **a convention added to this directory
ships with a mechanical checker, or with an explicit note saying why it cannot have one.** A rule
an agent can forget and nothing can detect is not a rule.

The first one exists: **`python3 tools/check-stamps.py`** (#97). Run it before committing an ADR.
It checks that every amendment an ADR *declares* is stamped on its target, that stamps resolve to
files that exist, that a stamp's label and link agree, and that an amendment always points
backwards.

It is a **floor**. An ADR that amends something without declaring it is invisible to it — that
case is the one that actually bit us (#72) and stays human, assigned to the spec chapter covering
the ADR. Its first run over the corpus produced nine findings of which one was real; the eight
false positives came from reading every reference under an amendments heading rather than the
bullet's target, which is now the rule the script applies.

This replaces, rather than adds to, the standing-audit instinct. There are **no recurring corpus
sweeps**: re-reading fifty ADRs on a cadence to usually find nothing is the re-derivation cost paid
on a timer. The corpus gets one bootstrap sweep for `STATUS.md` and no more.

## Flag ADR conflicts

If your output contradicts an existing ADR, surface it explicitly rather than silently overriding:

> _Contradicts ADR-0007 (event-sourced orders) — but worth reopening because…_
