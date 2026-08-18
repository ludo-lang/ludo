---
status: accepted
---

> **Amended by [ADR-0055](0055-absorption-has-a-complement-and-status-md-is-deleted.md):**
> §5's operative state — *every row converted* — is unreachable, because nine ADRs carry no
> normative clause any chapter could absorb. `STATUS.md` is deleted on §5's stated condition
> instead, and §6's owed checker for the absorption stamp is specified and ticketed.

# The spec is the only normative surface and an absorbed ADR is stamped

Raised in a grilling session about docs maintenance. The observation was about the **process**,
not the language: *whenever a ticket is processed new incoherencies are discovered, which can
create new incoherencies when the ticket is resolved.* The impression behind it was that the
corpus is degrading.

It is not, and the correction matters because the two diagnoses call for opposite work.

## 1. The impression is half right, and the half that is right is new

`CONTEXT.md` already answers the first half. A [hole](../../CONTEXT.md#hole) is *generated rather
than enumerated, and unbounded by construction*; most holes found so far were transcription
defects rather than undecided design ([ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §3).
Chapter transcription surfacing findings is the scanner working. That is not decay, and damping it
would mean scanning less.

But the observation was precise in a way the glossary never covered: incoherencies "can create new
incoherencies **when the ticket is resolved**." A hole in `CONTEXT.md` is discovered by *reading*.
That one is opened by *writing the repair*. The two have opposite prognoses — the first is a
finite pass over a fixed corpus and drains as chapters land; the second is feedback, and is the
only class that can diverge. Nothing recorded a hole's cause, so the two were indistinguishable,
and a run of the second reads exactly like decay.

`CONTEXT.md` now carries **[induced hole](../../CONTEXT.md#induced-hole)** for the second, and the
instrumentation that makes it countable is in `docs/agents/domain.md`.

## 2. The real cost is re-derivation, and it is what makes the queue grow

Three pains hide behind "incoherencies multiply": the open queue never converging, text going
stale after a repair, and the per-ticket cost of establishing what is currently true. The third is
the cause of the first.

[ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md) §1 already named the
mechanism — *a claim is checked against the issue that produced it rather than against the ADR that
superseded it* — and found it to be the common cause of every Tier-1 finding in the corpus audit.
The devices added since each patch one edge of it: amendment stamps say an ADR moved,
[`SOURCES.md`](SOURCES.md) says an issue moved,
[ADR-0049](0049-a-marked-gap-and-its-repair-are-one-obligation.md) says a marked gap may not
outlive its answer. None of them answers the question an author actually asks, which is *given
this source I am about to cite, is it still the authority?* Answering it today means proving a
negative across fifty ADRs, and an author who skips that step files a ticket instead — which is
how re-derivation cost becomes queue growth.

## 3. The decision: the spec chapters are the only normative surface

**A landed spec chapter is where the rule is read. The ADR that produced it becomes history the
moment the chapter absorbs it.**

This is the destination [ADR-0022](0022-the-spec-promises-only-what-is-derivable.md) and ADR-0044
were already pointing at, made explicit and made binding. It is the only arrangement whose steady
state has *one* place to read, and therefore the only one where the question in §2 has a
constant-cost answer.

What it costs: the ADR corpus stops being a live reference. That is a real loss — the corpus
carries argument the spec deliberately does not, and an author who wants to know *why* a rule is
the way it is will still go there. The corpus keeps that job. It loses only the job of saying what
is true now.

## 4. An absorbed ADR is stamped, in the chapter's commit

Absorption is invisible from inside the absorbed document, and this project has direct evidence
that an invisible move is the characteristic failure: two ADRs skipped their amendment stamps and
nothing noticed ([#72](https://github.com/ludo-lang/ludo/issues/72)). *Still true but no longer
where you should read it* misleads an agent exactly as effectively as *false*.

So absorption uses the machinery amendment already uses — one line under the front matter,
nothing else changed, in the **same commit** as the chapter:

```markdown
---
status: accepted
---

> **Absorbed by [spec §6.2](../spec/06-the-facades.md):** the chapter is normative; this ADR
> keeps the argument.

# The original title stays exactly as it was
```

The stamp is owed **whether or not the chapter changed the rule while transcribing it**. The
reader's problem is not whether the rule moved; it is which artifact is authoritative. A chapter
that *did* change the rule owes an amendment stamp as well, and the two accumulate as separate
lines like any other.

## 5. Consequence: an index that is a projection, and that dies

Until chapters 4–8 exist, most of the corpus is unabsorbed and §3 does not yet help. The bridge is
`docs/adr/STATUS.md`: one row per ADR — status, a one-clause gist, declared edges — assembled from
the stamps, supersession lines and `coverage/` files that already exist.

Two properties are load-bearing. It is **derived**, not authored: every row restates an edge the
tree already carries, so it cannot become a third source that goes stale on its own. And it is
**temporary** — each chapter that lands converts rows to `absorbed-by`, and when the last chapter
lands the file says only that the spec is normative, at which point it is deleted. An index built
to survive would compete with the spec, which is the outcome §3 exists to prevent.

It does **not** subsume [#73](https://github.com/ludo-lang/ludo/issues/73), and an earlier draft of
this section wrongly said it did. #73's remaining half is **issue-keyed** — which ADR now carries a
decision an issue is still cited for — and it is a byproduct of chapter work, assembled into
[`SOURCES.md`](SOURCES.md) §2 from the `coverage/` files. `STATUS.md` is **ADR-keyed** and cannot
answer it: an author holding `#42` gets nothing from a table indexed by ADR number. The two are
siblings, like the stamp and `SOURCES.md` before them, and #73 closes with chapter 8 as it always
did.

## 6. Consequence: a rule an agent can forget is not a rule

Every convention in §4 and §5 is one an author can silently skip, and #72 is the proof that they
do. So: **a convention added to `docs/agents/` ships with a mechanical checker, or with an explicit
note saying why it cannot have one.**
[#97](https://github.com/ludo-lang/ludo/issues/97) is the template.

The rejected alternative is a standing corpus-wide audit on a cadence. It re-reads fifty ADRs to
find nothing most of the time, which is the §2 re-derivation cost paid on a timer rather than
removed. The corpus gets **one** sweep to bootstrap `STATUS.md`; after that the checkers hold the
line and sweeps do not recur.
