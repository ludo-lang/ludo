---
status: accepted
---

> **Amended by [ADR-0046](0046-the-repository-becomes-a-public-monorepo-in-the-ludo-lang-org.md):**
> §5's topology is corrected — `ludo-lang/ludo` is the whole project as a public monorepo, not
> the bootstrap's own repository beside a separate home for the spec; the schedule is unchanged.

> **Amended by [ADR-0049](0049-a-marked-gap-and-its-repair-are-one-obligation.md):** §8's
> marked-gap bullet is extended — a chapter may still write a gap down rather than block on it,
> and the ticket that gap files now owns repairing the chapter text, coverage rows and gap list
> in the same commit as its answer. §6 is unchanged and reaffirmed.

> **Amended by [ADR-0051](0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md):**
> consolidation is carried to its conclusion — a landed chapter is the only normative surface, and
> the ADR it absorbs is stamped as history in the chapter's commit.

# The map ends at an artifact, and the corpus is consolidated into a spec

Not a ticket. This ADR was written from a session that started as a complaint about
throughput — *the fog seems never-ending; each solved ticket creates one or two more; every
concrete ludo example we write reveals holes we never predicted* — and ended up reversing
[#1](https://github.com/adamico/ludo/issues/1)'s stated destination. It is the first ADR
written under §6's own rule, and it records why the ticket rate is what it is, which is not
what it looked like from inside.

## 1. The premise was false, and the count says so

At the time of writing: **65 issues closed, 12 open.** Of the twelve, **three** are design
questions ([#82](https://github.com/adamico/ludo/issues/82) the dynamic container set,
[#83](https://github.com/adamico/ludo/issues/83) the view-return rule,
[#84](https://github.com/adamico/ludo/issues/84) the integer generic parameter), two are
corpus hygiene (#72, #73), and six are tooling that `docs/project-structure.md` already ruled
out of the map's scope. #1's own *Not yet specified* section is down to **one item**, the
public name, which belongs to #34.

The unspecified area is not expanding. It is nearly gone.

## 2. The two things being called fog

The complaint conflated two quantities that behave differently, and the conflation is the
whole misreading:

- **Fog** — a named area nobody has decided yet. Enumerated in #1, finite, currently one item.
- **Hole** — a contradiction or gap surfaced by writing something concrete. Generated rather
  than enumerated, and **unbounded by construction**.

Holes were being read as evidence about fog. They are evidence about the corpus. Both terms
are now in `CONTEXT.md`.

## 3. What is actually generating tickets

Not design. **The corpus auditing itself.** The recent record is unambiguous:

- #67 found *"integer scale" was never written anywhere*.
- #69 found the texel-centre convention was the **third phantom clause** — cited by three
  ADRs and never authored.
- #70 found #19 stopped at P12, with P13 decided and never posted, four ADRs depending on it.
- #79 found a falsified claim lived in **three** sites, not the two the ticket assumed.
- #81 split two questions out of itself (#83, #84), one of which it is conditional on.

These are not design holes. They are transcription defects, and they have a single cause:
**there are 43 ADRs, with amendments and supersessions, and no consolidated normative
document anywhere in the repository.** `docs/` holds `adr/`, `agents/`, `research/` and
`project-structure.md`. There is no spec. There is no grammar file, though
[#22](https://github.com/adamico/ludo/issues/22) binds the project to ship a machine-readable
one *in the spec*.

A corpus where every clause is reached by following citations between decision records will
manufacture phantom clauses indefinitely, because nothing ever fails when a citation points at
nothing.

## 4. The destination criterion was unreachable by construction

#1 said the map is done when *"an implementer could start a compiler without asking further
design questions."*

That is a claim about the **absence of future questions**, and no amount of grilling can
establish it: every closed ticket enlarges the corpus and licenses new deductions from it, so
the criterion is met only at deductive closure, which never arrives. The observed
one-in-two-out rate is not a symptom of an immature design. It is what this criterion does.

**Replaced with an artifact criterion: the map is done when a prototype runs the reference
program.** Falsifiable, terminating, and it demotes a hole from a map-blocker to a prototype
bug.

The coverage criterion that was the third candidate — *every clause has a citation and no
phantom clauses remain* — is kept, but as the **spec document's** exit criterion rather than
the map's. It is §5's closing test.

## 5. Consolidation comes before the compiler

Two artifacts were candidates for *next*: the spec document, or the C bootstrap that
[#49](https://github.com/adamico/ludo/issues/49) / ADR-0020 already authorised and which does
not yet exist (`ludo-lang` has no repositories).

**The spec goes first**, on evidence already in hand: the browser-semantics research
([#56](https://github.com/adamico/ludo/issues/56),
[#78](https://github.com/adamico/ludo/issues/78)) falsified normative clauses twice, and that
was only *reading*. Transcribing 43 ADRs into one normative document will find phantom clause
four without a C compiler in the way. ADR-0020 also wants a library-shaped frontend from the
first commit, and a frontend wants a grammar to parse against.

The shape:

- **Chaptered `docs/spec/NN-*.md` in this repository.** ADRs are untouched and stay the
  rationale record; the spec carries **normative clauses only** and cites the ADR that decided
  each. Generation from the ADRs was rejected outright — the corpus's problem *is* that ADRs
  cite each other, and a generator preserves exactly that.
- **A chapter closes only when every clause of every ADR it covers has been transcribed or
  explicitly dropped, with a citation both ways.** This is the test that finds phantom
  clauses: they are found by failing to locate a source, never by reading.
- **Eight chapters, in dependency order**: grammar; types; memory and aggregates; modules and
  libraries; the program and the runner; the facades; diagnostics; conformance. The facades
  stay one chapter despite their bulk — they are the best-specified region, and granularity
  buys least where holes are least likely.
- **`docs/spec/grammar.ebnf` is normative**, in EBNF, and prose cites its rule names. Prose
  that disagrees with the grammar is a defect in the prose. EBNF over a PEG because the
  grammar is a spec artifact an implementer reads, not the frontend's implementation; the C
  frontend's test suite consumes the same file.

**The C prototype starts in parallel, gated on the grammar chapter.** `ludo-lang/ludo` gets
its first commit the day `grammar.ebnf` is done — the parser needs no facade chapter, and it
is the fastest hole-finder available.

## 6. A hole is repaired in the spec text, not in a new ADR

The prior discipline sent every hole to a grilling ticket and every ticket to an ADR, which
enlarged the corpus, which produced holes. Three phantom clauses cost three ADRs' worth of
prose to say *this was never written*.

**A hole is repaired directly in the spec text. An ADR is written only when a decision is
reversed.** ADR-0020's own bar — hard to reverse, surprising without context, the result of a
real trade-off — is not met by a transcription defect, which is none of the three. This is
the cheapest available change to the ticket rate.

This ADR exists because §4 clears that bar: it reverses #1's destination.

## 7. The reference program is written fresh, and it is the gate

Every new design ticket must name the line of the reference program that cannot be written
without it. A question that cannot do that goes to a backlog, not the map. This turns *write
a concrete example and holes appear* from an accident into the mechanism.

**It cannot be the one that exists.** `prototypes/07-syntax/b-lua-flavoured.ludo` on
`proto/07-syntax-candidates` is #7's winning candidate, 103 lines, and it predates roughly 35
ADRs. Against the current corpus it breaks six ways: `Option`/`Some`/`None` where #9 fixed
absence as `?T`; `Slice[T]` where #15 spells it `[]T`; a `tick(w, dt, frame_arena)` entry
where #26 removed `dt` and ADR-0013 fixed the entry as `screen: !Target`; `frame_arena:
!Allocator` where ADR-0042 mandates the `Allocator`/`Scratch` split; `hits.free()` where
ADR-0042 states there is **no per-allocation `free` in the safe layer**; and `List.new` on a
container set #82 has not specified.

Six breakages in 103 lines is the complaint's own phenomenon, exhibited. The reference program
therefore lives beside the spec in `docs/spec/reference/` and is **extended by each chapter as
that chapter's last step**, which keeps it green by construction rather than by audit. Until a
frontend parses it, it is checked by hand — #49's completion test is already a spec test done
by hand, and this is the same kind.

## 8. What this costs

Stated rather than discovered:

- **The handover promise weakens.** #1 promised an implementer who need ask nothing. What is
  promised now is a spec that is internally consistent and a prototype that runs the reference
  program — an implementer starting a *second* implementation may still find questions. That
  was always true; it is now written down instead of chased.
- **#82 ships as a marked gap in chapter 3** rather than a resolved design, and the spec says
  so in the text.
- **Consolidation is transcription work**, several chapters of it, with no new decisions in it
  and therefore nothing that feels like progress on the map. The chapter tickets exist so the
  work is visible.

## 9. Consequences for existing records

- **#1's Destination section is rewritten** to §4's criterion, and gains a *Consolidation*
  section carrying §5's eight chapters.
- **`docs/project-structure.md`'s transfer trigger is repaired.** It read *when the map is
  complete — every ticket closed, not merely an empty frontier*, which under §4 names nothing.
  It now reads *when the spec document is complete*.
- **#72 (amendment stamps) and #73 (issue→ADR cross-reference) are repointed at the
  consolidation** and close with it. §5's closing test builds a better cross-reference than
  #73's table as a side effect, and produces #72's audit by construction.
- **The six homeless tooling issues are untouched**, on `project-structure.md`'s existing
  holding-pen decision.
- **#84 is grammar-blocking and is the first ticket**, ahead of chapter 1: whether a generic
  parameter may be a compile-time integer is whether `TextBuf[64]` is a legal type expression,
  which is a production in the type sublanguage. #83 is semantics and lands in chapter 3.

**Zero #24 delta** — nothing here is a language surface. **No new #19 property**: the reason
is stated per ADR-0030 §8, in that this ADR binds the authoring process and adds no observable
a conformance test could name.
