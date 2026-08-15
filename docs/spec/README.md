# The ludo specification

The consolidated normative document for the ludo language. Established by
[ADR-0044](../adr/0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md)
§5, which found the corpus's real problem: 45 ADRs that cite each other, with no
consolidated normative document anywhere, which is what manufactures **phantom
clauses** — rules three ADRs depend on that nobody ever wrote.

## How to read it

- **The chapters are normative.** They state what a conforming implementation
  MUST do, and nothing else. Each clause cites the ADR or issue that decided it;
  a clause with no citation is a defect.
- **The ADRs are the rationale record** and stay untouched. Where a reader wants
  to know *why*, the citation is the answer.
- **[`grammar.ebnf`](grammar.ebnf) is normative** and is the authority on
  syntax. Prose that disagrees with it is a defect in the prose.
- **A hole is repaired in the spec text.** An ADR is written only when a chapter
  *reverses* a decision (ADR-0044 §6). Spellings authored to fill a phantom
  clause are listed in the chapter that authored them.

## Chapters

| # | Chapter | Status |
|---|---|---|
| 1 | [Lexical structure and grammar](01-grammar.md) | written |
| 2 | [Types](02-types.md) | written |
| 3 | [Memory and aggregates](03-memory.md) | written |
| 4 | [Modules and libraries](04-modules.md) | written |
| 5 | [The program and the runner](05-runner.md) | written |
| 6 | [The standard library](06-stdlib.md) | written |
| 7 | Diagnostics | not started |
| 8 | Conformance | not started |

## Closing test

A chapter closes only when **every clause of every ADR it covers has been
transcribed or explicitly dropped, with a citation both ways** (ADR-0044 §5).
The spec-clause → source direction lives in the chapter. The source → landing
direction lives in [`coverage/`](coverage/), one file per chapter, which is
where phantom clauses are found — by failing to locate a source, never by
reading.

## The reference program

[`reference/reference.ludo`](reference/reference.ludo) is written fresh and
grows one chapter at a time; each chapter's last step extends it, which keeps it
green by construction. Until a frontend exists it is checked by hand.

**The map is done when a prototype runs it** (ADR-0044 §4).
