# Coverage: chapter 7 — diagnostics

**Non-normative.** This file is the closing test for
[`../07-diagnostics.md`](../07-diagnostics.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §9 below says which
happened.

**Chapter 7's boundary is the diagnostic stream itself** — the envelope, its
fields, and the rules governing codes and severity. It is the smallest chapter
in the spec and covers two ADRs, because the corpus put every *individual*
diagnostic in the chapter that states the rule it enforces, and left this
chapter the shape they all share.

---

**Absorbed here:** ADR-0018, ADR-0023.

Read by `tools/check-absorption.py`, which asserts that each carries the
ADR-0051 §4 stamp naming this chapter, and that no ADR claims this chapter
without appearing here (ADR-0055 §4). ADR-0041 §3, and the clauses of ADR-0029, ADR-0037 and ADR-0027 this chapter reaches, belong to other chapters and are not absorbed here.

## 0. Source check (#72, #73)

Run over **this chapter's sources only** — ADR-0018, ADR-0023, and the clauses
of ADR-0041, ADR-0029, ADR-0037 and ADR-0027 this chapter reaches — before
transcription, per `docs/agents/domain.md`.

`python3 tools/check-stamps.py` reports **zero findings over the whole corpus**
before this chapter and zero after. §0.1's finding is again of the silent kind
the checker cannot see, and this time by a route the corpus has not recorded
before: the amendment was **declared**, in the amending ADR's hand-off section
and in `STATUS.md`, and was then discharged by **editing the amended ADR's
prose** instead of stamping it.

### 0.1 Direction 1 — stamps owed and missing

Applied retroactively in this chapter's commit; stamps are additive metadata,
not prose.

| Record stamped | What moved it, and what moved |
|---|---|
| **ADR-0018** | **ADR-0023.** ADR-0023 §7 declares *ADR-0018 §4 gains a pointer here, replacing its parked-fog paragraph*, and `STATUS.md` carries the edge as `amends:0018(§4 pointer)`. ADR-0018's front matter carried no stamp for it. The pointer was instead applied **inside ADR-0018 §4**, in ADR-0023's own commit (`bb33e21`, +5/−3 on ADR-0018): the parked-fog sentence was deleted and replaced with a sentence naming ADR-0023. |

The finding is worth its own paragraph, because it is a **new failure mode for
the convention**, not a repeat of chapters 5 and 6:

- The rule `docs/agents/domain.md` states is *never rewrite the prose of an
  earlier ADR to match a later decision*, and then *add one line under the front
  matter and change nothing else*. This commit did the first and skipped the
  second — an inversion of the rule rather than an omission of it.
- **The checker cannot see it, and the reason is instructive.** The script reads
  an amending ADR's declared amendment section for bullet *keys*; ADR-0023's §7
  hand-off list keys its bullets on **#19** and on **ADR-0018 §4**, and the
  ADR-0018 bullet reads as a pointer being granted rather than as an amendment
  being declared. The edge survived into `STATUS.md` by hand and into the tree
  as a prose edit, and the one place a reader of ADR-0018 looks — its front
  matter — was the only place it did not reach.
- **The prose edit is left in place.** Reverting it would be a second rewrite of
  an ADR's text, and the current sentence is true. What was missing is the
  front-matter signal, and that is what this commit adds.

### 0.2 Direction 2 — stamps **not** owed, recorded so they are not re-derived

- **ADR-0021 §5**, **ADR-0026 §7**, **ADR-0027 §1**, **ADR-0029 §6**,
  **ADR-0037 §4** and **ADR-0038 §5** each **apply** ADR-0018's rules — a
  diagnostic under its envelope, a code under its §5, its never-guess clause.
  Applying a rule is not changing it. No stamp. This is the largest such set in
  the corpus, and it is what a chapter about a *shape* looks like from the
  inside.
- **ADR-0023 §6** records that ADR-0018 §5 *already grants* shape-based
  distinguishability. It is a claim **about** ADR-0018 rather than a change to
  it, so no stamp is owed — and the claim itself does not hold, which is §9.1's
  finding and §13.1's marked gap.
- **ADR-0029 §6** cites **ADR-0023 §1** as the reason `#explicit`'s four
  diagnostics are errors and the word *lint* leaves the spec. Citation as
  precedent; ADR-0023's rule is unchanged. No stamp.
- **ADR-0041 §3** amends **ADR-0018 §11** and the stamp is **declared and
  present**. Nothing owed.
- **ADR-0023 §1** withdraws **#25 §2's** mandated padding diagnostic. The target
  is an issue and cannot be stamped; the row is already in
  [`SOURCES.md`](../../adr/SOURCES.md) and chapter 3 §8.2.2 carries the
  withdrawal. Nothing owed here.

### 0.3 Issue sources against `SOURCES.md` (#73)

- **#53** and **#59** appear in `SOURCES.md` **§3** — the originating table —
  and **not in §1**, so neither is sole authority for anything. Both were fully
  absorbed by their ADR on the day it landed: #53 asked whether a structured
  format is owed and whether one or two, and ADR-0018 answers both; #59 asked
  whether warning configurability is a language surface or a flag, and ADR-0023
  answers it and dissolves its third sub-question (§5). No row is added.
- **#18** — listed in §1 as *authoritative, extended*, with the standing warning
  not to conflate it with ADR-0018. Heeded: this chapter cites **#18 §8 only for
  the fault payload's existence** (§2.3) and routes its contents to chapter 5.
  The fault-kind enum's membership is chapter 5 §6.5.1's — seven members,
  closed by [#113](https://github.com/ludo-lang/ludo/issues/113) — and is
  untouched here.
- **#19** — not in §1; its properties are chapter 8's. This chapter states the
  obligations P5, P9 and P12 test and numbers none of them (ch7 §14).
- **#5** — not in §1. Criteria 4 and 5 are cited at ch7 §4.1, §5.5, §7.1 and
  §12.4; **ADR-0023 §2 is the later and controlling reading of criterion 4** and
  is what §12.4 transcribes, never the criterion's original wording.

`SOURCES.md` is unedited by this chapter. No clause here rests on an issue as
sole authority.

### 0.4 A note on the checker's floor

Three chapters have now found a stamp the script cannot: chapter 6's was an
amendment declared under a heading the regex does not match
([#115](https://github.com/ludo-lang/ludo/issues/115)), chapter 5's was an
amendment nobody declared, and this one was **declared and then discharged in
the wrong artifact**. The three exhaust the ways the convention can fail while
the author believes it succeeded, which is worth recording because each was
found by a different half of the same manual sweep.

---

## 1. ADR-0018 — one diagnostic stream for the compiler and the faulted program

| Source clause | Where it landed |
|---|---|
| §1 The spec owes the fields and the code's meaning | §1.1 |
| §1 The spec does not own the switch, the rendering, colour, ordering or transport | §1.2, §1.4 |
| §1 Message design stays out of scope | §1.3 |
| §1 The in/out split follows #17 and #22 | **Dropped** — precedent for the split, not a rule. §9 |
| §1 A per-implementation format would make an agent learn N formats | **Dropped** — the argument for §2.1, transcribed as the rule alone |
| §2 Fields normative, one mandatory encoding, shape not frozen | §3.1, §3.2, §8.1, §8.3 |
| §2 The two rejected postures (fields-only; a frozen JSON layout) | §8.3 records the frozen-schema rejection as a rule (*no schema is frozen into this spec*); the fields-only rejection is **dropped** as rationale |
| §2 Newline-delimited, not one enclosing array, so a consumer can act on the first error | §8.1, including the reason, which is the rule's operative content |
| §2 Additive-only evolution; a version bump may add optional fields; removal is a new format | §8.2 |
| §3 One shape, two producers; the discriminant is `compile`/`fault` | §2.1, §2.2 |
| §3 The seven required fields | §3.1 |
| §3 A `fault` message additionally carries #18 §8's contents | §2.3, which routes the contents to ch5 §6.5 |
| §3 Rejected: two top-level formats | §2.1's second sentence |
| §4 Severity is the closed enum `error`/`warning`/`note` | §4.1 |
| §4 `note` is not standalone and attaches to a parent's code | §4.2 |
| §4 The first admission that warnings exist; ADR-0023 settled the rest | §4.3, and §12 in full |
| §5 Every mandated diagnostic has a spec-assigned code, stable forever | §5.2 |
| §5 A retired code is tombstoned, never reused | §5.3 |
| §5 Vendor codes live in a distinct vendor-namespaced code space | §5.4 |
| §5 A vendor code may never be the sole reason a build fails | §5.5 |
| §5 The code is what stability matters most for | §5.1 |
| §6 Primary location required: file, byte offset, length | §6.1 |
| §6 Byte offsets rather than line/column, and why | §6.1 |
| §6 Secondary locations optional, each carrying its own `note` | §6.2 |
| §6 The two-place cases: conflicting definitions, #15's move, ADR-0014's duplicate claim as canonical | §6.3, citing ch4 §6.5 |
| §7 The entity is required for semantic diagnostics, optional for lex/parse | §7.1, §7.2 |
| §7 The code says which class the message is in | §7.3 |
| §7 Requiring it unconditionally would force an implementation to guess | §7.4 |
| §8 Message text is required to be present and required to say nothing | §9.1 |
| §8 It is not stable, not versioned, not a key | §9.2 |
| §8 Keeping the field lets a renderer print something while design stays out of scope | **Dropped** — the motive for §9.1, not a further obligation |
| §9 The suggestion is an optional structured field: source range plus replacement | §10.1 |
| §9 A diagnostic MUST omit it rather than emit a low-confidence one | §10.2 |
| §9 Prose "did you mean" is not a suggestion and carries no obligation | §10.3 |
| §9 *Present and does not resolve the diagnostic* is the checkable failure | §10.4, routed to ch8 for the property's wording |
| §9 The 46.6% / 22.6% measured regression | **Dropped** — evidence for the rule; ADR-0018 §9 and #19 P5 keep it |
| §10 The capability is required and no switch is named | §1.4 |
| §10 The machine-readable stream carries messages and nothing else | §11.1 |
| §10 A single interleaved line breaks every consumer and is silent for the producer | §11.2 |
| §11 The `fault` report is emitted in dev and release alike | §2.4 |
| §11 *Release still exits non-zero* | §2.5, **as amended by ADR-0041 §3** — abnormal termination by the host's convention. See §3 below |
| §11 Emitting only in dev would make one program report differently by mode | §2.4's second sentence |
| §11 The dev/release difference stays the runner's liveness (#12) | **Dropped** — ch5 §1.5 owns it |
| §12 The three lenses | **Dropped**, except the one factual consequence: *nothing here appears in the language a beginner writes*, which ch7 §0.6 states as the reason the reference program is not extended |
| §13 #24 delta: no grammar, no stdlib root name, four recorded rejections | **Dropped from the chapter** — ch8's ledger owns the count. The rejections land as rules where they are rules (§8.3, §2.1, §4.1, §7.4) |
| §14 P5 gains a pointer and keeps its test-shaped form | ch7 §14's routing bullet; ch8 owns P5 |
| §14 The follow-on toolchain effort inherits the code namespace, §5's tombstoning and §2's additive rule | §5.7 |
| §14 Whether warnings are configurable is fog, not decided by implication | **Superseded** by ADR-0023, transcribed at §12 |

## 2. ADR-0023 — the spec never mandates a warning

| Source clause | Where it landed |
|---|---|
| Opening: the corpus mandates **zero** warning-severity diagnostics | §12.2 |
| Opening: ADR-0020's bootstrap-compiler warning configuration is a different language | §14's last bullet |
| §1 A spec-owned diagnostic is an error, or it does not exist | §12.1 |
| §1 Stated as a standing rule because it is self-defending | §12.1's second sentence |
| §1 A warning is the middle position with no occupant | §12.10, where the word *lint* is the same finding |
| §1 The `warning` severity stays in ADR-0018's enum; deleting it was rejected | §4.3 |
| §1 Every warning is vendor-space under ADR-0018 §5 | §12.3 |
| §2 Criterion 4's normative reading: identical accept/reject sets, output unconstrained | §12.4 |
| §2 The rejected reading — two compilers produce the *same* diagnostics | **Dropped** — a rejected reading, and §12.4 states the surviving one |
| §2 Warnings sit outside criterion 4 by construction; promotion moves them inside | §12.5 |
| §3 Forbidden: a flag under which the compiler rejects what another accepts, including a vendor `-Werror` | §12.6 |
| §3 Permitted: an external gate reading the stream and exiting non-zero; the compile succeeded and the artifact is byte-identical | §12.7 |
| §3 The gate needs no new spec surface | §12.7's last sentence |
| §3 The spec still names no flags, no CLI, no rendering | §1.2, which states it once for the chapter |
| §4 No `#allow`, no `#deny`, no in-source silencing | §12.8 |
| §4 A per-site attribute could only name a vendor code, putting vendor identifiers in ludo source | §12.8 |
| §4 The two rejected alternatives (spec-codes-only; tolerate-unknown-codes) | **Dropped** — rationale; §12.8's rule admits neither |
| §5 Warning groups are dissolved, not deferred | §12.9 |
| §6 Vendor codes must be distinguishable by shape alone, with no lookup table | §5.6 — **transcribed as an obligation whose discharge is missing**; see §9.1 and ch7 §13.1 |
| §6 It adds no field to the envelope | §3.1's field list, which does not grow |
| §7 P12 is handed to #19 | ch7 §14's routing bullet; ch8 owns P12 |
| §7 ADR-0018 §4 gains a pointer here | **The stamp finding**, §0.1 above |
| The three lenses | **Dropped** — rationale |

## 3. ADR-0041 §3 — the clause of a later ADR this chapter reaches

| Source clause | Where it landed |
|---|---|
| §3 P2 / #18 §4 / ADR-0018 §11 read as *abnormal termination by the host's convention* | §2.5 |
| §3 The browser's convention: the trap surfaces as an exception through the JavaScript host and the frame entry does not return | **Routed** — ch5 §6.3.3 carries it; §2.5 states only the ordering this chapter owns (the message is emitted first) |
| §3 Scoping the clause to hosted targets was rejected | **Dropped** — rationale for a clause ch5 carries |
| §3 The claim lives in three sites, one of them ADR-0018 §11 | §0.1-adjacent bookkeeping; the chapter states the clause once |

## 4. The clauses of other ADRs this chapter reaches

Each **applies** this chapter's rules rather than being transcribed by it. The
chapter cites them where the application is itself a rule a reader would
otherwise have to re-derive.

| Source clause | Where it landed |
|---|---|
| ADR-0029 §6 — four spec-assigned codes for `#explicit`, severity `error`, no silencing | §5.8, §12.10; ch1 §11.6 mints them |
| ADR-0037 §4 — one spec-owned code for a shader declaration against a facade-only target | §5.8; ch6 §9.3 mints it |
| ADR-0027 §1 — an unresolvable-`use` diagnostic may not suggest a name the compiler has not confirmed exists | §10.5; ch4 §7.8 states it in place |
| ADR-0021 §5, ADR-0026 §7, ADR-0038 §5 — each names the envelope in passing | **Not transcribed.** They cite the stream; they add nothing to it |

## 5. What chapters 5 and 8 own, and this chapter routes

| Subject | Owner |
|---|---|
| The fault report's **contents** — fault kind, concrete values, call chain, frame number, `persist` names and types | ch5 §6.5 (#18 §8). This chapter binds only the envelope they ride in |
| The fault-kind enum's **membership**, and the two undeclared extensions | ch5 §6.5.1 — closed at seven by [#113](https://github.com/ludo-lang/ludo/issues/113) |
| The call chain's mode-independence and its price | ch5 §6.5.2 (#18 §9, as amended by ADR-0041 §4) |
| What the paused runner offers, and the `persist` dump | ch5 §6.4 (#18 §6, §7) |
| **P5**, **P9**, **P12** — wording, numbering, and what a harness asserts | ch8 (#19). ch7 §10.4 and §12.6 state the obligations; the properties are not restated |
| The **#24** ledger and this chapter's zero delta | ch8 |
| The concrete **code strings** and the namespace's contents | Neither chapter: the follow-on toolchain effort (ADR-0018 §14; #19 §E), recorded at ch7 §5.7 and §14 |

## 6. Routed, not dropped

- **#5 criterion 5**'s guarantee that every failure is attributable to a named
  entity is chapter 8's to state as a criterion; ch7 §7.1 uses it as the premise
  of a field rule and does not restate the criterion.
- **The two-place diagnostics** — ADR-0014's duplicate claim, #15's
  moved-on-assignment — are stated by chapters 4 and 3. ch7 §6.3 states only
  that the envelope must carry both places.

## 7. What the chapter authored

**One clause, and it is a transcription with a hole named rather than an
invention:** ch7 §5.7's statement that the concrete code strings are not this
spec's. ADR-0018 §14 hands the code namespace to the follow-on toolchain effort
and ADR-0018 §5 mandates that a spec code exists for every mandated diagnostic;
neither says in one place that the spec therefore ships *rules without strings*.
Two chapters route the codes' text here (ch4 §16, ch5 §12), so a reader arriving
from either needs the answer stated rather than assembled. No rule is changed,
so no ADR (ADR-0044 §6).

## 8. Induced holes opened by this chapter

**None.** The chapter mints no spelling, adds no field and changes no rule, so
there is nothing downstream for it to have made unwritable. §9.1's gap is
**inherited, not induced** — it has been open since ADR-0023 landed and this
chapter is the first document obliged to apply it.

## 9. Phantom clauses, and what was explicitly dropped

### 9.1 One finding, and it is a phantom of an unusual kind

**ADR-0023 §6 cites a clause of ADR-0018 §5 that does not exist in the strength
it claims.** §6's rule — a consumer separates spec-owned from vendor-owned codes
**by the code's shape alone, with no lookup table** — is grounded on ADR-0018 §5
*already requiring* it. ADR-0018 §5 requires a **distinct vendor-namespaced code
space** and fixes no shape for either space. Distinctness is a promise about the
*sets*; shape-distinguishability is a property of the *strings*, and the second
does not follow from the first: a vendor free to choose any namespace may choose
one indistinguishable by inspection from the spec's, and satisfy §5 exactly.

This is not the corpus's usual phantom — the rule is not missing, its
**precondition** is. So the treatment differs from the corpus's four previous
phantoms, where the citation was deleted:

- **The obligation is transcribed** (ch7 §5.6), because it is a real decision
  ADR-0023 took and an agent's read-diagnostic loop depends on it.
- **The shape is not authored here.** Fixing a prefix or a grammar for the code
  space is the same act as assigning the codes, which ADR-0018 §14 hands to the
  follow-on toolchain effort. Authoring half of it in a chapter whose §5.7 says
  the other half is not the spec's would leave the spec deciding a namespace it
  does not own.
- **It is filed** as [#117](https://github.com/ludo-lang/ludo/issues/117) and
  marked at ch7 §13.1, per ADR-0044 §8 and ADR-0049. The ticket owns the repair
  of §5.6, §5.7, §13.1 and these rows in one commit.

No citation is deleted: ADR-0023 §6's reasoning is wrong about what ADR-0018 §5
grants, but its rule stands on its own.

**Resolved by [#117](https://github.com/ludo-lang/ludo/issues/117).** The third
bullet above is superseded on its own terms. Fixing a shape was read as *the
same act as assigning the codes*, and it is not: a code is
`<owner>:<local>`, the owner **`ludo`** is reserved to this spec, and any other
owner is a vendor's — which binds both spaces while assigning no code and
naming no vendor. The local parts remain ADR-0018 §14's, unchanged. ch7
§5.6.1–§5.6.5 carry the rule, §5.7 now points at it instead of at a gap, and
§13.1 is struck through as the record. The precondition ADR-0023 §6 assumed is
therefore supplied by *this spec* rather than found in ADR-0018 §5, which is
what a phantom of this kind costs: the missing half is authored, not deleted.

### 9.2 Explicitly dropped, by class

- **Arguments for a rule that is itself transcribed** — ADR-0018 §2's
  incremental-consumer argument (kept, because it *is* the rule's operative
  content), §3's one-reader argument, §9's measured regression, ADR-0023 §1's
  history-of-warning-configurability argument. The rule is normative; the
  derivation is not.
- **Rejected alternatives that produce no rule a program or an implementation
  can violate** — ADR-0018 §2's fields-only posture, §3's two formats, ADR-0023
  §2's same-diagnostics reading, §4's two attribute designs. Where a rejection
  *is* a rule it is transcribed: ch7 §8.3 (no frozen schema), §2.1 (no second
  format), §4.1 (no open severity), §7.4 (no invented entity), §12.8 (no
  attribute), §12.9 (no groups).
- **The three lenses**, in both ADRs, including ADR-0018 §12's *no
  beginner-versus-agent conflict to name*. Recorded in the ADRs; not rules.
- **Corpus and map bookkeeping** — §13's #24 deltas, §14's and §7's hand-off
  lists, ADR-0023's fog-closure note.
- **Precedent citations** — ADR-0018 §1's #17/#22 split, §5's P9 parallel
  (kept only where ch7 §5.5 needs the parallel to make the rule legible).

## 10. Marked gaps

| Gap | Where | Ticket | State |
|---|---|---|---|
| The code space has no stated shape, so ch7 §5.6's by-shape test cannot be applied | ch7 §13.1, §5.6.1–§5.6.5, §5.7 | [#117](https://github.com/ludo-lang/ludo/issues/117) | **Closed** — codes are owner-qualified, `ludo` reserved |

**No gap of this chapter's remains open.** The reference program is unaffected
(§11): the rule binds toolchain output, and no line of a ludo program is written
or not written on the strength of a code's shape.

## 11. The reference program

ADR-0044 §7's last step, and **this chapter adds nothing to it** — the first
chapter for which that is true of its whole subject.

Every rule here binds **toolchain output**. There is no declaration, expression
or call a ludo program can write that exercises the envelope, the severity set,
the code rules, the encoding or the stream: a `compile` message exists only for
a program that does not compile, and the reference program compiles by
construction; a `fault` message is chapter 5's payload, and chapter 5 recorded
that a fault has no line a program can write either.

Recorded rather than silently skipped, because *the reference program is
unchanged* and *the author forgot the last step* look identical in a diff. The
chapter says the same thing at §0.6, so a reader of the spec alone reaches it
too.
