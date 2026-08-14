---
status: accepted
---

# A marked gap and its repair are one obligation

Raised while resolving [#82](https://github.com/ludo-lang/ludo/issues/82). The observation is
about the **process**, not the language: *chapter transcription generates issues that are
supposed to fix the transcription through another issue, and that is too convoluted.*

It is correct, and the defect is sharper than convolution.

## 1. A marked gap creates an obligation with no owner

[ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §8
lets a spec chapter write a gap down rather than block on it, which is right and is not in
question here. What it never said is **who repairs the chapter when the gap is answered**. In
practice nobody did, so a successor ticket had to be invented after the fact. The observed cycle,
with #82 as the exhibit:

1. Spec chapter 3 transcribes, cannot resolve the container set, **marks the gap** at §11.3 and
   files #82.
2. #82 is grilled and resolved →
   [ADR-0048](0048-every-allocating-call-is-fallible-and-a-persist-initialiser-faults.md).
3. Chapter 3 §11.3 is now **false in the tree** — it says the container set is unspecified.
4. A third ticket (#106) is filed to repair it.

Four hops for one obligation. Worse, hop 4 lands in a **cold session** that must rebuild the
argument hop 2 had fully in hand — the most expensive possible way to transcribe nine clauses
whose author is still holding them.

**And the interval is the real cost.** Between hops 2 and 4 the spec asserts something the corpus
has already decided against. That is the stale-docs failure this project has direct evidence for
([#72](https://github.com/ludo-lang/ludo/issues/72)), arriving through a new door.

## 2. The rule: whoever closes a marked gap repairs the chapter, in the same commit

**A ticket filed by a marked gap is not done when its ADR lands. It is done when the chapter
text, the chapter's `coverage/` file and the reference program no longer say the gap is open.**
The ADR and the normative clauses it produces land in **one commit**.

The gap marker may not outlive its answer, in the tree, at any point.

## 3. This is the stamp rule, not a new one

The project already solved this exact problem for a different artifact.
`docs/agents/domain.md`: *an unstamped ADR is worse than a wrong one* — a reader meets superseded
text with no signal it moved — and therefore the stamp *"goes in the **same commit** as the
amending ADR, so the two never disagree in the tree."*

An unrepaired chapter clause is the identical failure with the identical remedy. §11.3 saying the
container set is unspecified, after ADR-0048 specified it, is exactly an unstamped ADR: true when
written, false now, with nothing in the text to warn the reader. **The convention is extended to
a second artifact rather than invented**, and the argument transfers whole.

The two failures are now three siblings, and `docs/agents/domain.md` already frames the first two
that way: a **stamp** tells you an ADR moved, **`SOURCES.md`** tells you an issue moved, and this
rule stops a **spec chapter** from claiming a question is open after it is closed.

## 4. The carve-out is the reference program, and it is narrow

ADR-0044 §7 makes each chapter extend the reference program, and a gap closing can imply a
substantial extension — ADR-0048 added a `persist` container, a `reserve` and two fallible pushes.

**The clause repair never splits. The reference-program extension may**, where it is large enough
to be its own work, and only with the split recorded on the map.

The asymmetry is deliberate. A stale *clause* asserts a falsehood to every reader; a
reference program that does not yet exercise a new feature merely under-tests it. One is a lie and
the other is a gap in coverage, and only the first is urgent.

## 5. The one-ticket-per-session rule is not strained by this

The map caps a session at one resolved ticket, and repairing a chapter inside the session that
resolved the gap can look like breaking it.

It is not. **The repair was never a second ticket** — it is the first ticket's definition of done,
and it only ever looked like a second one because the protocol split it. A ticket filed by a
marked gap should say so in its own body from the day it is filed: *…and repair chapter N's
clause, coverage rows and gap list.* #82 did not, which is what produced #106.

## 6. Rejected alternatives

**Chapters block until the gap is answered.** This is what ADR-0044 §8 deliberately overturned.
Chapter 3 would still be unwritten, and the consolidation is the thing standing between the corpus
and a grammar file.

**Batch every chapter repair into one pass before the spec ships.** The cheapest to execute and
the worst to live with: it makes staleness the *steady state* rather than a window, across every
chapter at once, for the whole consolidation. #72's evidence is that nobody notices stale text
until something forces a read, and a deferred batch removes the thing that would have forced it.

**A bot or a CI check that fails when a closed issue is cited by an open gap.** Attractive, and
filed rather than built — it is enforcement for a rule that does not exist yet, and this ADR is
the rule. Worth revisiting once the monorepo's CI covers `docs/spec/`.

## 7. Consequences for existing records

- **#82's repair is discharged in the session that resolved it**, retroactively honouring this
  rule: chapter 3 §11.3 and §11.5–§11.13, §9.6's narrowing, §12.2, §20, the coverage tables and
  the reference program.
- **[#106](https://github.com/ludo-lang/ludo/issues/106) is closed as absorbed**, not as
  completed-elsewhere. It is the exhibit this ADR is named for.
- **[#105](https://github.com/ludo-lang/ludo/issues/105) carries the obligation in its body.**
  It is chapter 3's new marked gap (§11.13), so resolving it repairs §11.13, §12.2, §20 and the
  coverage rows in the same commit.

## 8. The stamp list

- **ADR-0044 §8** — its marked-gap bullet is **extended**: a gap may still be written down rather
  than answered, and now the ticket it files owns the repair.
- **ADR-0044 §6** — unchanged and reaffirmed. A gap repair is *repair in the spec text*, which is
  precisely what §6 asks for; this ADR says who does it and when, not whether an ADR is owed.
- **`docs/agents/domain.md`** — gains the third sibling (§3). Not an ADR, so it is edited directly.

## 9. Costs and the three lenses

**The cost, named:** a design session that resolves a marked gap now ends with transcription work
appended to it, and those sessions are already the long ones. The trade is that the transcription
is done by the only session that will ever have the argument in context for free.

**Simplicity.** One rule replacing an unwritten convention: *a gap marker may not outlive its
answer.* Fewer tickets, not more.

**Robustness.** It closes a window in which the spec asserts what the corpus has already decided
against — the failure mode #72 recorded and #82 reproduced.

**Agent-friendliness — the decisive lens.** An agent reading chapter 3 between hops 2 and 4 has no
way to know §11.3 is stale; it is normative text with a citation, and it is wrong. This project's
whole consolidation argument is that agents read the spec and not the ADR graph, and a stale
normative clause is the one defect that argument cannot survive.
