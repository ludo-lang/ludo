# Coverage: chapter 8 — conformance

**Non-normative.** This file is the closing test for
[`../08-conformance.md`](../08-conformance.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §9 below says which
happened.

**Chapter 8's boundary is conformance** — what an implementation must *be*,
never what the language *is*. It is the chapter that consumes the other seven,
so nearly every clause of every other ADR in the corpus is *reached* by it and
almost none is *transcribed* by it. The checklists below are correspondingly
narrow: three ADRs and three issues carry it, and the rest appear as the
normative surface conformance is measured against.

---

## 0. Source check (#72, #73)

Run over **this chapter's sources only** — #19, #24, #54, ADR-0019, ADR-0028,
ADR-0037, ADR-0016 §6, and the clauses of ADR-0004, ADR-0006, ADR-0008,
ADR-0015, ADR-0022, ADR-0025, ADR-0030, ADR-0033, ADR-0034, ADR-0035, ADR-0036,
ADR-0038, ADR-0040 and ADR-0041 this chapter reaches — before transcription, per
`docs/agents/domain.md`.

`python3 tools/check-stamps.py` reports **zero findings over the whole corpus**
before this chapter and zero after.

### 0.1 Direction 1 — stamps owed and missing

**None.** This is the first chapter to find nothing in this direction, and the
reason is structural rather than lucky: chapter 8's three carrying ADRs are all
**late** (ADR-0019, ADR-0028, ADR-0037), and a late ADR has had less corpus
written after it to go stale against. ADR-0028 already carries three stamps
(ADR-0032, ADR-0038, ADR-0040), all applied by their authors.

The finding this chapter *does* have is in the sibling direction — an issue
source that moved and cannot be stamped — and it is §0.3's.

### 0.2 Direction 2 — stamps **not** owed, recorded so they are not re-derived

| Pair | Why no stamp |
|---|---|
| ADR-0037 → ADR-0019 | ADR-0037 §1 cites ADR-0019 §1 as the **precedent** for refusing a roster (*a conformance obligation is stated on one claimant, never on a pair*), and §2 applies ADR-0019 §2's `extern`-visibility finding to `extern shader`. Both use the rule; neither changes it. |
| ADR-0028 → ADR-0019 | ADR-0028 §1 declares `extern` outside the admission test **because** a rule binding it *would be unenforceable and would restate ADR-0019 §3*, and §10 declines a conformance clause on the same ground. A decision to add nothing is not an amendment. |
| ADR-0038 → ADR-0028 | Stamped, and correctly — §6's corpus table gains a row. Recorded here only because the neighbouring ADR-0038 → ADR-0037 pair is **not** stamped and should not be: a render scale changes no rung. |

**One stale sentence found and deliberately not fixed.** ADR-0037 §1's floor
reads *made executable by issue #19's P1–P14*, and ADR-0019's own corollary
reads *P1–P11*. Both were true when written; the list is P1–P17 as of this
chapter. **No stamp and no edit**: an ADR is an immutable record, the drift is
in a count rather than in a rule, and per ADR-0051 §3 the floor is now read at
spec ch8 §3.5, which carries no numeral at all — it names *this spec's normative
surface*, which is what the sentence always meant. Recorded so the next reader
does not file it.

### 0.3 Issue sources against `SOURCES.md` (#73)

Chapter 8's issue sources are **#19**, **#24** and **#54**, plus **#5** and
**#22** as cited criteria.

| Issue | `SOURCES.md` row before | Finding |
|---|---|---|
| #19 | *authoritative, with corrections* — ADR-0041 corrects six sentences about the runner half | **Amended by this chapter.** Two ADRs handed #19 a property in as many words and **neither reached the list**: ADR-0008 §10 and ADR-0015 §8. See §9.1. Neither can be stamped, because the target is an issue — which is precisely what `SOURCES.md` exists for. The row now names both. |
| #24 | *authoritative* — the budget and its counting rules; the comparison table has an open loose end in #93 | Unchanged. Call 6's implementation-facing half lands at ch8 §5.9 and call 5's second consequence at ch8 §9.3, both as #24 wrote them. |
| #54 | §3 row: *#54 → ADR-0019* | Correct, and **incomplete in a way §3 already warns about**: ADR-0019 records only two of #54's four findings, and says so in its own opening — *the reviewer-facing detail … lives in issue #54's resolution*. The ranked reviewer kit is therefore issue-only, and ch8 §3.4 transcribes it from the issue. No row is added: §3's rows say an ADR exists, not that it is exhaustive, and the ADR names the split itself. |
| #5 | *authoritative* — the Tier-1 criteria, unchanged | Unchanged. Criteria 4 and 5 are cited as vetoes at ch8 §2.7, §5.8.1, §5.12.1, §7.2 and §8.6. |
| #22 | *authoritative* — confirmed, not moved, by ADR-0045 | Unchanged. §2's refusal of a millisecond figure is ch8 §1.3's ground; §4's oracle shape is P6; §7's machine-readable grammar is P9's test artifact. |

### 0.4 The third sibling — a marked gap this chapter inherits rather than opens

`docs/agents/domain.md` names a marked gap as the third sibling of the stamp and
of `SOURCES.md`. Chapter 8's §11.1 is unusual among the corpus's marked gaps in
that **it was recorded before it was a gap**: ADR-0033 §9 wrote *the root itself
… is not enumerated anywhere in the corpus* in 2026-08, as a limitation of its
own table rather than as a defect, and nothing consumed the observation until a
chapter was obliged to publish a count. See §10.

---

## 1. #19 — the experience contract

The chapter's largest source, and the only one it transcribes end to end.

### 1.1 The posture (#19 §A)

| Source clause | Where it landed |
|---|---|
| Posture 1 — invariants bind absolutely, latency binds as a measurement | §1.3, §6.1, §6.4 |
| Posture 1's two rejections — shapes only, hard numeric MUSTs | §6.6 |
| Posture 2 — the contract binds every implementation; a batch-only compiler is non-conforming; a minimal third-party compiler cannot claim the name | §1.2 |
| Posture 3 — conformance is binary; no documented-deviation tier | §1.1 (and §5.2.3, which is the one place a later ADR tested it) |
| Posture 4 — a suite is implied; authoring it is a follow-on effort; the out-of-scope line moves from *under review* to decided | §1.4, §10.1, §10.4 |

### 1.2 The properties (#19 §B, and the amendment comments)

| Property | Source | Where it landed |
|---|---|---|
| P1 reload preserves declared state, plus six supporting assertions | #19 P1; #17 | §5.1, §5.1.1 |
| P2 fault pauses, plus the resume clause and four supporting assertions | #19 P2; #18 | §5.2, §5.2.1 |
| P2's release observable, restated for non-process hosts | ADR-0041 §3 | §5.2.2, §5.2.3 |
| P3 single-command run, plus the cache clause | #19 P3; #3; #12 | §5.3, §5.3.1 |
| P4 no unchecked escape path, plus the `unsafe`-is-not-one clause | #19 P4; #8 | §5.4, §5.4.1 |
| P5 machine-readable diagnostics and the never-guess clause | #19 P5; ADR-0018 | §5.5, §5.5.1, §5.5.2 |
| P6 the oracle answers correctly and incrementally; no millisecond figure | #19 P6; #22 §4 | §5.6, §5.6.1 |
| P7 frame overrun degrades, never catches up | #19 P7; #26 | §5.7 |
| P7's premise made permanent, and the composed one-image observable | ADR-0035 §1, §3, §6 | §5.7.1, §5.7.2 |
| P8 mode-independence with three enumerated exceptions | #19 P8; #18 §4, §9; #8 | §5.8, §5.8.1 |
| P9 no vendor syntax extensions | #19 P9; #24 call 6 | §5.9, §5.9.1, §5.9.2 |
| P10 callback fault barrier | #19 P10; #29 §8 | §5.10, §5.10.1 |
| P11 C-held callback pointer survives reload; not a P8 exception | #19 P11; #29 §9 | §5.11, §5.11.1 |
| P12 no flag changes the accepted program set | #19 P12; ADR-0023 §3, §7 | §5.12, §5.12.1 |
| P13 the canvas is fitted, not stretched; `k` unconditional; canvas fixed | #19 P13; ADR-0030 §8; ADR-0031; ADR-0032 | §5.13, §5.13.1, §5.13.2 |
| P14 the window opens maximal and its size persists | #19 P14; ADR-0036 §9 | §5.14, §5.14.1 |
| P15 the render scale quantises and clamps; core-runnable; invariance | #19 P15; ADR-0038 §12 | §5.15, §5.15.1, §5.15.2 |
| The audit-grep convention on bare P-tokens | #19, the P13/P14/P15 notes | §4.5 |

### 1.3 The reference set (#19 §C)

| Source clause | Where it landed |
|---|---|
| Save-to-visible as the measured quantity | §6.1 |
| The three sizes, defined by shape | §6.2 |
| Cold start and whole-project check at the same sizes | §6.3 |
| The 1s target and the obligation to publish a miss | §6.4 |
| The spec owes the specification; the corpus is the suite's | §6.5 |

### 1.4 The hand-offs (#19 §E)

| Source clause | Where it landed |
|---|---|
| The map's out-of-scope line for the suite | §10.4, §1.4 |
| The follow-on effort inherits the suite, the corpus and the code namespace | §10.1 (the namespace half routes to ch7 §5.7) |
| #37 — a project template may not violate P3 | **Dropped.** A constraint on a future ticket, not a rule an implementation can violate. P3 binds it directly. |
| #38 — the console is P2's inspection surface | **Dropped**, same class; §5.8's third exception is where the surface's existence is normative. |
| #33/#28/#32 — no asset-pipeline step before `ludo <file>` | **Dropped as a hand-off**, transcribed as the rule: P3's *no init step required as input* (§5.3) is the obligation those tickets were warned against. |

---

## 2. ADR-0019 — claimants conform alone, and `extern` is the only door

| Source clause | Where it landed |
|---|---|
| §1 an obligation is stated on one claimant, never on a pair | §3.1 |
| §1's ground — ADR-0014 §6 and ADR-0006 R3 mean the pair exists nowhere | §3.2 |
| §1 as a **shape rule for every future obligation** | §3.3 |
| §1's corollary — the nominal interface is necessary and insufficient | §3.4 |
| §2 criterion 4 holds unweakened; no program-visible surface below `$.`; `extern` is the only door and is visible in the signature | §3.9 |
| §2's rejection of softening criterion 4 into a language-only rule | **Dropped** — a rejected alternative producing no rule. The rule that survives is §3.9's, stated positively. |
| §3 the obligation handed forward: a program-visible platform surface must carry `extern`'s treatment or reopen criterion 4 | §3.10 |
| §4 criterion 4 does not reach third-party claimants; the pattern stays legal and unblessed | §3.11 |
| *Consequences* — a rule the suite cannot express is advisory (#56) | §1.5 |
| *Consequences* — no grammar delta; future obligations cite §1; the platform-layer fog inherits §3 | **Dropped** — corpus and map bookkeeping. §9.2's zero-delta row carries the count half. |

---

## 3. ADR-0028 — the admission test

| Source clause | Where it landed |
|---|---|
| §1 what the test governs; it binds spec authorship and no implementation; it defers to #24 on everything else | §8.1 |
| §1 `extern` is untouched | §8.2 |
| §2 limb one — reformulate before you refuse, and why it is first | §8.3 |
| §3 limb two — the return guard, quoted | §8.4 |
| §4 limb three — strict necessity, with the spec-mandated denominator | §8.5 |
| §4's *why the defence lives at the door* — the camera-transform channel cannot be closed | **Dropped** — the argument for the door, not a rule. §8.5's denominator is the rule. |
| §5 environment variance permitted, implementation variance not, quoted | §8.6 |
| §6 the corpus audit table | **Dropped as a table**, cited as ADR-0028 §6's at §8.8. Transcribing six historical adjudications into a normative chapter would freeze a worked example as a rule. |
| §7 display count moves, and the shape is not settled | §8.8, folded with ADR-0040's discharge |
| §8 the test is not what keeps a world hardware-independent; the canvas is; limb three is trivially satisfied in graphics | §8.7 |
| §9 four rejected candidates, including the `PhysicalPixels` quantity type | **Dropped** — rejected alternatives producing no rule. ADR-0028 §9 records them, including the note that `PhysicalPixels` becomes right the day a necessary physical number ships. |
| §10 standing: binds future authorship, adds no #19 property, supersedes ADR-0025 §8's operative half | §8.1; the declination is also §5.19's row |
| §11 the three lenses and the zero delta | **Dropped** — lenses are not rules; §9.2's reconciliation carries the delta |

---

## 4. ADR-0037 — the floor, and the third rung

| Source clause | Where it landed |
|---|---|
| §0 `backend` is the platform triple; ADR-0022 §1's usage is the loose one | **Routed to ch6 §0/§10 and ch4 §10.1**, which pin the sense. Ch8 uses *renderer component* throughout (§3.5, §3.7, §3.8) and needs no clause of its own. |
| §1 no admissible set; the floor is the normative surface, quoted | §3.5 |
| §1 a roster is the shape ADR-0019 §1 outlaws; a capability list is a roster in different clothes | §3.6 |
| §1 WebGL1's exclusion survives as a consequence, not a decree; WebGL2 neither admitted nor refused | §3.7 |
| §2 core ⊂ full ⊂ shader, totally ordered by containment | §2.1, §2.4, §2.5 |
| §2 the facade is analytically specifiable and needs no GPU; the shader paths are not | §2.6 |
| §2's rejection of an orthogonal capability flag | §2.5, stated as the rule (containment is real) |
| §2 ADR-0002's ceiling is scoped to the shader rung | **Routed to ch6 §9**, which owns the shader surface. Ch8 §2.4 fixes only the rung. |
| §2 *why this does not reopen criterion 4* — `extern shader` is an `extern`, visible in the signature; a stage marker is a declaration | §2.7 |
| §3 a software rasterizer is admissible and never mandated | §3.8 |
| §3's wasm-rasterizer argument, and the ~55 Mpx/s fill figure | **Dropped** — evidence that the floor is clearable, not a rule. The negative web obligation is ADR-0004's and is ch6's. |
| §4 a shader declaration against a facade-only target is a compile error naming the declaration, with a spec-owned code | §2.8 (and ch6 §9.3, which mints the diagnostic; ch7 §5.8, which counts it) |
| §4's rejection of a runtime fault and of a silent no-op | §2.8, transcribed as the reasons the rule is a compile error |
| §5 ADR-0022 §1's final sentence restated | §3.7 |
| §6 the reference-implementation temptation, refused; the permitted half — the suite may run on a software renderer | §10.3, §10.2 |
| §7 zero #24 delta; no new property; a rung partitions rather than adds; one diagnostic code, not counted | §2.9, §9.2.1 |
| The three lenses | **Dropped** — recorded in the ADR; not rules |

---

## 5. ADR-0016 §6 — the two conformance halves

Chapter 6 §3.9 routed exactly these two clauses here and transcribed the third
itself.

| Source clause | Where it landed |
|---|---|
| §6 clause 2 — SIMD lowering is not a conformance obligation | §7.1 |
| §6's rejection of MUST-lower (no named entity, no source location) and of MUST-MEASURE (no single observable) | §7.2 — transcribed because both are rules about what a future obligation may be, not merely rejected alternatives |
| §6 clause 3 — float evaluation exactly as written; no reassociation, no implicit FMA, no fast-math, in any mode | §7.3 (language-level statement at ch2 §2.5) |
| §6's *clause 3 is determinism, not performance*; the P8-shaped divergence | §7.4 |
| §6 clause 1 — alignment on the blessed types | **Ch6 §3.8's**; ch8 §7.5 cites it as one of the three things mandated instead of lowering |
| §6's *a naming convention plus a layout guarantee plus an evaluation guarantee*; speed is quality-of-implementation | §7.5 |
| The rest of ADR-0016 | **Ch6 §3 and ch2 §16.1's**; not this chapter's |

---

## 6. #24 — the count, run against the finished spec

| Source clause | Where it landed |
|---|---|
| Call 6's implementation-facing half — no vendor syntax extensions | §5.9 (P9), §5.9.2 |
| Call 5's second consequence — the stdlib root-name companion count, no target, published by this chapter | §9.3, §9.3.1, §9.3.2, §9.3.3 |
| Call 4's comparison target and the Odin/Go gap | §9.2, §9.2.3 — both by pointer to ch1 §13.7, which owns the table |
| Call 1, 2, 3, 5 tier rule, 7 home | **Ch1 §13's**, cited at §9.1 and §9.4. This chapter runs the count; it does not restate the process. |
| The three consequences — labelled break rejected, the stdlib escape route, recorded overrules | **Ch1 §13.8–§13.9.1's.** §9.2 reports the register's outcome and names the register as authoritative. |

**§9.2.1 is the clause the ticket asked for**: the corpus tracked deltas per
decision and never totalled them against one document. The reconciliation was
run over all seven chapters' `coverage/` files and the register at ch1 §13.9.1,
and it balances — no chapter moved the count without a row.

---

## 7. The clauses of other ADRs this chapter reaches

Transcribed as **clauses on existing properties** or as list entries, never as
new rules. Each is cited in the chapter at the clause named.

| Source | Clause | Where |
|---|---|---|
| ADR-0004 | the core/full split | §2.2, §2.3 |
| ADR-0006 R8 | ring survives reload (click-free) → P1; drain-then-silence → P2; both at full, neither a new property | §5.1.3, §5.2.5 |
| ADR-0008 §10 | a failed shader recompile retains the last-good pipeline, reports an error value, does not fault | §5.16 — **numbered here**, see §9.1 |
| ADR-0015 §8 | editing a declared asset reloads it at the next frame boundary; failed re-decode retains last-good; the missing/broken asymmetry | §5.17 — **numbered here**, see §9.1 |
| ADR-0022 | the spec promises only what is derivable; a promise wearing a lab coat | §1.6, §10.3 |
| ADR-0025 §3, §6 | the fullscreen affordance and mode persistence attach to P3 at full | §5.3.2 |
| ADR-0030 §8 | the requirement to record a declined property | §1.6, §5.18, §5.19 |
| ADR-0033 §9 | the reconciled companion base, and its note that the root is not enumerated | §9.3.1, §9.3.4, §11.1 |
| ADR-0034 §6 | nothing owed; *this spec owes the assertions, the follow-on effort owes the harness* | §1.4, §5.19 |
| ADR-0035 §1, §3, §5, §6 | the permanent premise, one present per entry, the declined property | §5.7.1, §5.7.2, §5.19 |
| ADR-0038 §5, §9 | the scale affordance and relaunch persistence attach to P3 at full | §5.3.2, §5.1.3 |
| ADR-0040 | display count admitted and never exercised | §8.8, §5.19 |
| ADR-0041 §3, §9 | P2's failure condition; the two declined properties | §5.2.2, §5.2.3, §5.19 |
| ADR-0044 §5, §8 | the normative-clauses-only posture; the marked-gap mechanism | §0.1, §11 |
| ADR-0049 | a marked gap and its repair are one obligation | §11's preamble; #119's body |
| ADR-0051 §3 | a landed chapter is the only normative surface | §0.4; and §8 below |

---

## 8. Absorption, and the end of `STATUS.md`

Per ADR-0051 §4, this chapter's commit stamps the ADRs it absorbs:

| ADR | Stamp |
|---|---|
| ADR-0019 | absorbed by ch8 §3 |
| ADR-0028 | absorbed by ch8 §8 |
| ADR-0037 | absorbed by ch8 §2–§3 |
| ADR-0016 | a **second** absorption line — §6's two conformance halves are ch8 §7's; the rest was already absorbed by ch6 |

**`STATUS.md` is not deleted by this commit, and the reason is a finding.**
ADR-0051 §5 says the file dies when the last chapter lands. Chapter 8 is the last
chapter, but the file's rows are **not** all converted: ADRs absorbed by chapters
1–4 were never stamped or converted when those chapters landed — ADR-0014,
ADR-0017, ADR-0021, ADR-0027, ADR-0029, ADR-0042, ADR-0043, ADR-0045, ADR-0047,
ADR-0048, ADR-0050, ADR-0052 and ADR-0053 all still read `live` or `amended`
while their content is normative in a landed chapter — and that enumeration is a
floor, not a total: ADR-0001, ADR-0002 and ADR-0004 are arguably chapter 6's on
the same test. Deleting the bridge while
it is the only place that backlog is visible would lose the backlog, not
discharge it.

The backfill is **not chapter 8's**: it is a pass over four chapters' sources,
and it is the same shape as #73's index assembly, which is already a ticket and
already fed by the `coverage/` files. Recorded here so the next reader of
ADR-0051 §5 does not read the surviving file as an oversight.

---

## 9. Phantom clauses, and what was explicitly dropped

### 9.1 One finding, and it is the P13 failure repeated twice

**No phantom clause was found.** Every clause of every source was located. The
finding is the opposite shape — **two rules that exist and were never
delivered** — and it is the third and fourth instance of the failure #70 found:

| Declared | By | Delivered? |
|---|---|---|
| P13 — the canvas is fitted, not stretched | ADR-0030 §8 | **No.** Found by #70, transcribed into #19 by hand, four ADRs already depending on it. |
| A failed shader recompile retains the last-good pipeline | ADR-0008 §10 | **No.** *It lands as a property scoped to full conformance*, stated in a block quote, handed to #19 in the ADR's own consequences list — *fourth property this map has handed it*. #19 never gained it. |
| Editing a declared asset reloads it live | ADR-0015 §8 | **No.** *This attaches to issue #19 as a new property at full conformance, P1's sibling* — precondition, action and observable all given. #19 never gained it. |

They are numbered **P16** and **P17** at ch8 §5.16 and §5.17, in ADR order.

Three things are worth recording about the class rather than the instances:

- **The mechanism is the same one `SOURCES.md` exists for.** An ADR that hands a
  property to an *issue* cannot stamp it, so the only thing linking the two is
  the author remembering to post a comment. #19's row is amended accordingly
  (§0.3).
- **The grep the corpus relies on could not have found these.** #19's P13/P14/P15
  notes are careful never to write a bare P-token for a property that does not
  exist, precisely so an audit can grep for the highest live token. That audit
  finds a *gap in the sequence*; it cannot find a property that was never given a
  number at all. Both of these were invisible to it.
- **P16's rung moved while it was undelivered.** ADR-0008 §10 scoped it to *full*
  against the two-rung split, and ADR-0037 §2 subsequently made ADR-0008's two
  authoring paths **define** the third rung. Ch8 §5.16.2 states the shader rung
  and shows the derivation. This is a consequence of ADR-0037, not a new
  decision, which is why it is a transcription and not a reversal (ADR-0044 §6).

**Nothing here reverses a decision**, so no ADR is written. Both properties were
decided in their own ADRs, in the contract's own precondition/action/observable
form; only the numbering was missing, exactly as with P13.

### 9.2 Explicitly dropped, by class

- **Arguments for a rule that is itself transcribed** — ADR-0019 §1's
  no-artifact-can-perform-the-comparison derivation (kept, because it *is* §3.2's
  operative content), ADR-0028 §4's use-site-cannot-be-defended argument,
  ADR-0037 §3's wasm-rasterizer availability argument and its fill-rate figure,
  #19's DeepSeek-Coder regression (kept at ch8 §5.5.2, because it is the reason
  the never-guess clause is failable rather than advisory).
- **Rejected alternatives producing no rule an implementation can violate** —
  ADR-0019 §2's language-only softening of criterion 4, ADR-0028 §9's four
  candidates, ADR-0037 §2's orthogonal capability flag and §1's capability list
  (the last two are transcribed, at §2.5 and §3.6, because each *is* a rule about
  how a future obligation may be worded).
- **Historical adjudications** — ADR-0028 §6's six-row corpus audit. Cited, not
  transcribed: freezing a worked example into a normative chapter makes the
  example the rule.
- **The three lenses**, in all three carrying ADRs and in #19 §D.
- **Corpus and map bookkeeping** — every `#24 delta` and `no new #19 property`
  section. Their *content* is §5.19 and §9.2.1; their form is not a clause.
- **Precedent citations** — ADR-0037 §0's `backend` collision (ch4 §10.1's and
  ch6's), ADR-0033 §1's aliasing diagnosis (`SOURCES.md`'s).

---

## 10. Marked gaps

| Gap | Where | Ticket |
|---|---|---|
| The `$.` root is enumerated nowhere, so ch8 §9.3's mandated count is a floor rather than a total | ch8 §11.1, §9.3.1, §9.3.4 | [#119](https://github.com/ludo-lang/ludo/issues/119) |

**This is the second *inherited* marked gap, and the two differ in an
instructive way.** Chapter 7 §9.1's gap was inherited too — open since ADR-0023
landed, with ch7 the first document obliged to apply it. This one was **recorded
in advance**: ADR-0033 §9 wrote the absence down as a limitation of its own
table, and it sat unconsumed because nothing needed the root enumerated. Ch7's
inheritance was a defect nobody had noticed; this one was a defect somebody had
already written down and no artifact was yet obliged to read. The remedy is the
same in both cases — a chapter obliged to apply a rule finds what the rule needs
— which is the argument ADR-0044 §5 made for transcription over generation.

**Induced holes opened by this chapter: none.** This chapter authored no repair;
it transcribed, numbered two undelivered properties, and marked one gap.

The ledger across chapters 4–8 therefore reads: **ch4 no section at all, ch5
one, ch6 none, ch7 none, ch8 none.** Two observations on that:

- The **trend answer is the one `docs/agents/domain.md` wants** — hole arrival is
  a throughput problem, not a loop. Nothing here names a repair driving further
  repairs, and the list shrinks rather than grows.
- **Chapter 4 has no ledger section at all**, though `docs/agents/domain.md` says
  *the ledger starts with chapter 4*. It is not a timing accident: ADR-0049,
  ADR-0051 and the checker all landed **before** chapter 4 shipped (`d66a58a`
  precedes `ea16667`). The author simply skipped it, and nothing detected the
  skip — which is ADR-0051 §6's own thesis arriving as evidence against the
  convention that thesis was written to protect. The ledger has no checker and
  `docs/agents/domain.md` claims no exemption for it.

  Recorded, not backfilled. Reconstructing cause for a closed chapter is the
  archaeology the same document declines for chapters 1–3, and a four-point trend
  answers the question a five-point one would. The **convention gap** is the
  durable half and belongs with the other checker work.

---

## 11. The reference program

ADR-0044 §7's last step, and **this chapter adds nothing to it** — the second
chapter for which that is true of its whole subject, after chapter 7.

Every rule here binds an **implementation**: what it must be, at which rung, and
what a harness may fail it on. There is no declaration, expression or call a ludo
program can write that exercises a rung, a property's numbering, a latency
measurement, an admission-test limb or a published count. The two properties this
chapter numbers are no exception — P16 is observed by editing a shader source and
watching the pipeline survive, and P17 by editing an image file, and neither is a
line of ludo.

The one place a reference-program line *is* owed is **not this chapter's**: ch8
§11.1's gap covers `$.assert`, which is a call a program writes and which the
reference program does not yet contain because it has no spelling. That line is
[#119](https://github.com/ludo-lang/ludo/issues/119)'s, and #119's body names it.

Recorded rather than silently skipped, because *the reference program is
unchanged* and *the author forgot the last step* look identical in a diff. The
chapter says the same thing at §0.6, so a reader of the spec alone reaches it
too.

**The spec is now eight chapters, and the destination criterion moves to the
prototype**: ADR-0044 §4 makes the map done when a prototype runs the reference
program, which is work in `ludo-lang/ludo`'s C bootstrap and not work on this
map.
