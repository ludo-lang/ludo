---
status: accepted
---

# Absorption has a complement, and `STATUS.md` is deleted

[#120](https://github.com/ludo-lang/ludo/issues/120), the backfill chapter 8 recorded rather than
ran ([`../spec/coverage/08-conformance.md`](../spec/coverage/08-conformance.md) §8). Chapters 1–4
landed without stamping the ADRs they absorbed, so thirteen-plus ADRs still presented themselves as
authoritative — the failure [ADR-0051](0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md)
§4 exists to prevent, on a second artifact.

The backfill itself is bookkeeping and is done: fifteen ADRs now carry an absorption stamp naming
the chapter that transcribed them — ADR-0014, ADR-0021, ADR-0027 and ADR-0054 (ch4); ADR-0017 and
ADR-0043 (ch2); ADR-0029, ADR-0045 and ADR-0053 (ch1); ADR-0042, ADR-0047, ADR-0048, ADR-0050 and
ADR-0052 (ch3); and ADR-0004 (ch6, plus a second line for ch8's core/full split). Two of those were
outside #120's enumeration: **ADR-0004**, which the ticket flagged as arguable and which is
absorbed on the same test as any other source, and **ADR-0054**, which landed after the ticket was
written and repaired chapter 4 in place without stamping itself.

The decision is the part that was not bookkeeping.

## 1. Three of #120's arguable cases split two ways

#120 asked whether ADR-0001, ADR-0002 and ADR-0004 are chapter 6's. The test is the one every
other stamp used: **does a chapter transcribe a clause of it as a source?** Not *does a chapter
cite it* — chapter 8 §7 cites ADR-0044, ADR-0049 and ADR-0051 while absorbing none of them.

- **ADR-0004 is absorbed.** The mandated facade is chapter 6's and the core/full split is chapter
  8 §2.2–§2.3, both transcribed as rules.
- **ADR-0001 and ADR-0002 are not.** Owning the platform layer rather than binding SDL, and
  adopting the WebGPU model for the renderer, are decisions about an **unmandated** surface. No
  chapter states them, because the spec deliberately does not reach there. Chapter 4 §10 and
  chapter 6 §9 cite ADR-0002 as background; neither transcribes it.

## 2. Absorption has a complement, and it is not a backlog

Nine ADRs will never be absorbed, and the reason is structural rather than pending work:

- **Implementation strategy below the normative surface** — ADR-0001, ADR-0002, ADR-0003. The spec
  mandates a facade and leaves the renderer and platform layer delegated; what we build behind that
  line is ours to decide and is not spec text. ADR-0003's live content reaches the spec only through
  ADR-0008, which is absorbed by chapter 6.
- **Process ADRs that bind authors, not programs** — ADR-0020, ADR-0033, ADR-0044, ADR-0046,
  ADR-0049, ADR-0051. A spec chapter has nowhere to put *the bootstrap compiler is written in C* or
  *a marked gap and its repair are one obligation*. Their operative home is `docs/agents/`, and
  they stay authoritative there.

This is a class the corpus never named, and it is why ADR-0051 §5's operative state — *every row
converted* — was never reachable. It was written as if absorption were total.

## 3. `STATUS.md` is deleted, on §5's stated condition

ADR-0051 §5 gives two formulations of when the bridge dies: the **condition** *when the last
chapter lands*, and the **state** *every row converted*. §2 shows the state is unreachable, so the
condition governs, and it was met by chapter 8.

Deleting loses nothing the file was built to answer. Its question is *given this source I am about
to cite, is it still the authority?* For an absorbed ADR the stamp now answers it, in the document
the author already has open — which is the whole point of §4's mechanism, and strictly better than
a table the author must remember to consult. For the nine of §2 the answer is *yes, unchanged*: they
were never displaced, so there is nothing to warn about. A file whose every row says either *read
the stamp* or *nothing happened* is the file §5 predicted, and it competes with the spec for as long
as it survives.

`.claude/agents/adr-shard.md`, the sharding agent whose only job was writing rows for the one-off
sweep, goes with it.

Amendment and supersession edges stay where they always were — stamped on the ADRs themselves,
checked by `tools/check-stamps.py` — and issue-keyed lookups stay in
[`SOURCES.md`](SOURCES.md), which is not affected by this and is not temporary in the same sense.

## 4. The checker the stamp rule still owes

ADR-0051 §6 requires a convention in `docs/agents/` to ship with a mechanical checker or an explicit
note saying why it cannot have one. The absorption stamp shipped with neither, and #120 is the
evidence of what that costs: four chapters skipped it and nothing detected the skip for four
chapters' worth of work. `tools/check-stamps.py` cannot see it — an absorption is declared nowhere,
and the chapter's `coverage/` file is the only record.

The checker is specifiable and is **not** written here: the shape is *read the source lists in
`docs/spec/coverage/*.md`, assert an absorption stamp on each ADR named there, naming the same
chapter*. It is code, so per [ADR-0046](0046-the-repository-becomes-a-public-monorepo-in-the-ludo-lang-org.md)
§7 it goes behind a branch, a PR and CI, while this ADR and the stamps commit straight to `main`.
Bundling them would put the docs half behind the code half's review cycle for no gain. It is filed
as its own ticket.

The same §6 gap covers the **induced-hole ledger**, which `docs/agents/domain.md` says starts at
chapter 4 and which no chapter kept. That is a second ticket: whether the ledger is reconstructible
from chapters 4–8 at all is a question, not a chore, and answering *no* retires the convention
rather than backfilling it.

## 5. Amendments this ADR makes

- **[ADR-0051](0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md) §5** —
  the operative state *every row converted* is withdrawn as unreachable; `STATUS.md` dies on the
  stated condition, the last chapter having landed.
- **ADR-0051 §6** — the absorption stamp's owed checker is specified here and ticketed, rather than
  left implicit.

Nothing else in ADR-0051 moves: §3's single normative surface and §4's stamp mechanism are what
this ADR applies.
