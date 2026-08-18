# Source authority: which issues still decide things

**Non-normative.** Wayfinding, required by [#73](https://github.com/ludo-lang/ludo/issues/73).

[ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md) §1
names the common cause of every Tier-1 finding in the corpus audit:

> a claim is checked against the issue that produced it rather than against the
> ADR that superseded it

The corpus cites issues and ADRs interchangeably, and the citations are *true* —
the issue really did settle the thing. What an author cannot tell from a citation
is whether it is still the **whole** truth. This file answers that, and only
that. It edits nothing and decides nothing.

## How to use it

Before transcribing a clause into a spec chapter, look its source up here.

- **Authoritative** — the issue is still the sole source. Read it and stop.
- **Authoritative, with a named change** — read the issue, then read the named
  ADR clause, which moved part of it.
- **Superseded** — an ADR carries the decision now. The issue is history.

An issue not listed here either produced an ADR directly (see §3) or is cited
too rarely to have caused a mistake yet. Absence is not a guarantee.

**Since the spec landed, §2 answers the question first.** Per ADR-0051 §3 the
spec is the only normative surface, so *where is this decided now* is always a
spec section. Use §2 for the destination and §1 for why the reading moved.

---

## 1. Issues that are the sole authority, with no ADR

These are the ones an author cannot verify without proving a negative across all
46 ADRs. Each of them **originates no ADR** — no ADR opens by naming it as its
ticket — while being cited throughout the corpus.

| Issue | Title | Status | What moved, and where |
|---|---|---|---|
| [#4](https://github.com/ludo-lang/ludo/issues/4) | Gather evidence on how AI agents fail at writing code | authoritative | Research findings. Cited as evidence, never as a rule. |
| [#5](https://github.com/ludo-lang/ludo/issues/5) | Decide what agent-friendliness means for this language | authoritative | The Tier-1 criteria. Unchanged; cited as a veto throughout. |
| [#6](https://github.com/ludo-lang/ludo/issues/6) | Decide whether layers are tiers or profiles | authoritative | The beginner-versus-agent axis. Unchanged. |
| [#7](https://github.com/ludo-lang/ludo/issues/7) | Write the same small game in three candidate syntaxes | authoritative | Candidate B wins. Transcribed in spec ch1 §1.2, §7.1. |
| [#8](https://github.com/ludo-lang/ludo/issues/8) | Choose the memory model | **authoritative, narrowed** | ADR-0042 adds the allocator's *shape*; the model is otherwise untouched. **Narrowed by [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md) §3**: §3's *aliasing is deleted* holds at the **container** level, not the view level — any number of views of one place may co-exist, and only `!` or a move on the origin kills them. §8's guarantee table gains four compile-error rows it does not itself list (dangling view, use-after-move, iterator invalidation), from ADR-0047 §3 and #15 Q14/Q19. |
| [#9](https://github.com/ludo-lang/ludo/issues/9) | Decide the fate of nil | **authoritative, upheld** | `persist` without an initialiser was a live contradiction with ADR-0045 §1/§8. **Resolved for #9 by spec ch5 §4.3**: the initialiser is required, and ADR-0045 §8's line is amended to carry one. Cite spec ch5 §4.3.1, not the contradiction. |
| [#10](https://github.com/ludo-lang/ludo/issues/10) | Choose the error model | authoritative | Errors as values, `rescue`, closed nominal error sets. |
| [#11](https://github.com/ludo-lang/ludo/issues/11) | Set the depth of the type system | **authoritative, narrowed and extended** | **Narrowed** by ADR-0045: integer generic parameters are *not* constraint-bounded, "a narrow, stated exception to #11". **Extended** by ADR-0017 §3 (a `fn` type satisfies a single-function interface by a blessed rule) and ADR-0017 §6 (UFCS never applies to a function value). |
| [#15](https://github.com/ludo-lang/ludo/issues/15) | Decide what replaces Lua's one-table aggregate | **authoritative, under challenge** | Q11's string rule is **superseded by ADR-0043** (`string` is a view into the runner's constant blob). Q10's view rule is **re-cut by [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md)**, which resolved the #83 challenge: *never returned* is reversed (a view may be returned when it derives from the receiver or the constant blob), *parameter position only* is clarified to permit a local binding, and the struct-field, `persist` and capture bans stand. ADR-0043 §1's narrowing is unaffected. **The container set it left open is closed by [ADR-0048](0048-every-allocating-call-is-fallible-and-a-persist-initialiser-faults.md) §2** at exactly `List` + `Map`, with a ring buffer blessed-unmandated and no `Set`; §3/§6 make `push`, `insert`, `clone` and `from` **fallible**, and add `reserve` (additional, idempotent) while withholding `capacity`. **Q24 is reversed by [ADR-0050](0050-removal-names-its-cost-at-the-call-site-and-descending-iteration-earns-an-operator.md) §8–§9**, twice: *there is no inclusive range operator* stands, but it never ruled out a **descending** one, and `>..` ships; and *ranges are ordinary `Iter[int]` values, not grammar* was false in both halves — the operator **is** grammar, and `Iter[int]` is a constraint rather than the value's type, which is now `Range`/`RevRange`. **Q12's insertion order and Q19's iteration ban are unchanged** and are paid for by ADR-0050 §2's tombstones and §6's range loop. |
| [#17](https://github.com/ludo-lang/ludo/issues/17) | Decide how a compiled language delivers state-preserving reload | **authoritative, respelled** | §4's `persist name: str = "boss"` is respelled `string` by ADR-0043 §3, and the type it names is narrowed by ADR-0043 §1. **§2's `$.mem.heap` is respelled a bare `heap` by ADR-0042 §8**, which also confines it to `persist` initialisers in the entry file. The reload semantics are unchanged. |
| [#18](https://github.com/ludo-lang/ludo/issues/18) | Decide what error-as-pause means for a compiled program | **authoritative, extended** | ADR-0018 is a *different* subject (the diagnostic stream, from #53). Do not conflate the numbers. **§8's closed fault-kind enum has been extended twice**, and neither extension can be stamped because the target is an issue: **ADR-0015 §4** adds asset resolution failure explicitly, and **ADR-0048 §7** mandates a fault for a `persist` initialiser's allocation failure that maps onto no existing kind and does not say it is adding one. **Resolved** by [#113](https://github.com/ludo-lang/ludo/issues/113): the enum keeps its closure at **seven** members and its authoritative list moves to **spec ch5 §6.5.1**, so a further kind is a stampable spec edit rather than an unstampable issue amendment (ch5 §6.1.7). §8's five are a superseded prefix. **§4's exit observable is restated** by ADR-0041 §3, and **§9's cost sentence amended** by ADR-0041 §4. |
| [#19](https://github.com/ludo-lang/ludo/issues/19) | Specify the experience contract an implementation must meet | **authoritative, with corrections, and incomplete as it stands** | ADR-0041 corrects six sentences that #78/#79 got wrong about the runner half of this contract. The contract itself stands. **Two properties were decided elsewhere and never reached the list**, the same failure #70 found with P13: **ADR-0008 §10** (a failed shader recompile retains the last-good pipeline, reports an error value, does not fault) and **ADR-0015 §8** (editing a declared asset's file reloads it at the next frame boundary; a failed re-decode retains the last-good asset). Both are stated in their ADRs in precondition/action/observable form and both are handed to #19 in as many words; neither can be stamped, because the target is an issue. **Numbered as P16 and P17 by spec ch8 §5.16–§5.17**, with P16's rung moved from *full* to *shader* by ADR-0037 §2. Cite spec ch8 §5 for the list, never #19's comment thread, which stops at P15. |
| [#22](https://github.com/ludo-lang/ludo/issues/22) | Decide the analyzability guarantees the spec owes an oracle | authoritative | Confirmed, not moved, by ADR-0045 (the oracle still never monomorphises). |
| [#24](https://github.com/ludo-lang/ludo/issues/24) | Decide whether the spec binds a grammar budget | authoritative | The budget and its counting rules. Every ADR's `#24 delta` section reports against it. **Call 4's table is complete** — the Odin and Go rows were counted by [#93](https://github.com/ludo-lang/ludo/issues/93) and land at spec ch1 §13.7. |
| [#25](https://github.com/ludo-lang/ludo/issues/25) | Decide how a type declares its memory layout | **authoritative, with two named changes** | The whole layout chapter: layout belongs to the container, guaranteed declaration order, `#align(n)`, no bit fields, the one-level SoA split, and sum-type layout with no niche optimisation. **§2's mandated padding diagnostic is withdrawn by ADR-0023 §1** — a spec-owned diagnostic is an error or it does not exist, so the padding report is vendor-space; §2's layout rule is untouched. **§9 is reversed in full by [ADR-0052](0052-the-origin-of-a-view-is-the-named-container-and-disjointness-is-never-reasoned-about.md)** — the origin of a column is the **pool**, distinct columns are not distinct places, `rocks!.pos` is the only writable spelling, and the `pool.each()` corollary is deleted as a call the language does not have. Cite ADR-0052 and spec ch3 §7.6/§10.10, never #25 §9. Closed as [#103](https://github.com/ludo-lang/ludo/issues/103). **Do not conflate #25 with ADR-0025**, which is *Fullscreen is a player preference* and decides nothing about memory — ADR-0026 §83, ADR-0042 §1, spec ch1 §12.2 and spec ch2 §4.8/§6.6/§20 all cite "ADR-0025" where they mean **#25**. |
| [#26](https://github.com/ludo-lang/ludo/issues/26) | Decide whether top-level code runs, and how it orders against frames | **authoritative, narrowed, with stale prose** | `use "world" as World` is **stale prose** predating ADR-0014 §3/§7 — there is no import alias (spec ch1 §14.9). The frame-entry parameter question it raised was settled against it by #47 / ADR-0013. **`$.mem.heap` is respelled a bare `heap` by ADR-0042 §8**, and is legal only in a `persist` initialiser in the entry file. **Call 2 is narrowed by [ADR-0021](0021-const-eval-folds-an-expression-and-never-runs-a-program.md) §3**: *modules do not run top-level code* reads as though an imported module may hold nothing at its top level, which is "one claim too strong" — a module **may declare constants**, because a constant is folded at compile time and is not a load-time side effect. The no-execution guarantee is untouched. Spec ch4 §13.1–§13.3. Its own amendment comment from **#17** confines `persist` to the entry file (spec ch4 §13.4–§13.5). **Call 4 gains a rule from [ADR-0048](0048-every-allocating-call-is-fallible-and-a-persist-initialiser-faults.md) §7**: allocation failure in a `persist` initialiser is a #18 fault, not a `rescue`-able value — a change to the position call 4 fixed, which no stamp can carry because the target is an issue. Spec ch5 §4.3, §4.4, §4.6. |
| [#29](https://github.com/ludo-lang/ludo/issues/29) | Specify the C ABI and FFI boundary | authoritative | ADR-0017 §4 **generalises** its callback restriction (top-level, named, non-capturing) into a whole-language rule. The ABI clauses are unchanged. |
| [#60](https://github.com/ludo-lang/ludo/issues/60) | Decide whether the platform layer has a program-visible surface… | **authoritative, partly withdrawn** | Its **blanket ban on program-visible backend state is withdrawn** by ADR-0025 §8; ADR-0028 replaces it with an admission test. Check ADR-0028 before citing #60 on any query. |

## 2. The supersession index

Assembled from the chapters' files in [`../spec/coverage/`](../spec/coverage/)
when chapter 8 closed, per [#73](https://github.com/ludo-lang/ludo/issues/73).

**Read this before §1.** The assembly changed the question it answers. §1 was
written when an ADR was the thing that superseded an issue; since
[ADR-0051](0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md)
§3 the **spec is the only normative surface**, so the answer to *where is this
decided now* is a spec section for every row below — never an issue, and never
an ADR. §1's *what moved* column is still the reason a reading changed, and it is
still worth having; but the destination column here outranks it.

| Issue | Carried now by | Clause-by-clause detail |
|---|---|---|
| [#7](https://github.com/ludo-lang/ludo/issues/7) Three candidate syntaxes | spec ch1 §1.2, §7.1, §9.1, §9.8 | [`coverage/01-grammar.md`](../spec/coverage/01-grammar.md) §1 |
| [#8](https://github.com/ludo-lang/ludo/issues/8) The memory model | spec ch3 — the whole chapter; §8's guarantee table gains four rows it never listed | [`coverage/03-memory.md`](../spec/coverage/03-memory.md) §1 |
| [#9](https://github.com/ludo-lang/ludo/issues/9) The fate of nil | spec ch2 (the type half) and **ch5 §4.3** (the `persist` initialiser, which resolved the contradiction) | [`coverage/02-types.md`](../spec/coverage/02-types.md) §1, [`coverage/05-runner.md`](../spec/coverage/05-runner.md) §13 |
| [#10](https://github.com/ludo-lang/ludo/issues/10) The error model | spec ch2 | [`coverage/02-types.md`](../spec/coverage/02-types.md) §1 |
| [#11](https://github.com/ludo-lang/ludo/issues/11) Depth of the type system | **split**: spellings in spec ch1, semantics in spec ch2 | [`coverage/01-grammar.md`](../spec/coverage/01-grammar.md) §1, [`coverage/02-types.md`](../spec/coverage/02-types.md) §1 |
| [#12](https://github.com/ludo-lang/ludo/issues/12) The standalone shape | spec ch5 §1–§3. **Its runner calls 2–4 originate no ADR**, so before the spec existed the issue was the only record | [`coverage/05-runner.md`](../spec/coverage/05-runner.md) §1 |
| [#15](https://github.com/ludo-lang/ludo/issues/15) What replaces the one-table aggregate | spec ch3 | [`coverage/03-memory.md`](../spec/coverage/03-memory.md) §1 |
| [#17](https://github.com/ludo-lang/ludo/issues/17) State-preserving reload | spec ch5 §3, §4.5.8–§4.5.9, §5.2, §5.9 | [`coverage/05-runner.md`](../spec/coverage/05-runner.md) §3 |
| [#18](https://github.com/ludo-lang/ludo/issues/18) Error-as-pause | spec ch5 §6; **the fault-kind enum's membership is ch5 §6.5.1**, no longer §8's list | [`coverage/05-runner.md`](../spec/coverage/05-runner.md) §4, [`coverage/07-diagnostics.md`](../spec/coverage/07-diagnostics.md) §0.3 |
| [#19](https://github.com/ludo-lang/ludo/issues/19) The experience contract | **spec ch8 §5 — P1–P17.** Never the issue's comment thread, which stops at P15 | [`coverage/08-conformance.md`](../spec/coverage/08-conformance.md) §1 |
| [#24](https://github.com/ludo-lang/ludo/issues/24) The grammar budget | spec ch1 §13 (the rule), **spec ch8 §6** (the count, run against the finished spec) | [`coverage/01-grammar.md`](../spec/coverage/01-grammar.md) §1, [`coverage/08-conformance.md`](../spec/coverage/08-conformance.md) §6 |
| [#25](https://github.com/ludo-lang/ludo/issues/25) Memory layout | spec ch3; **§9 reversed in full** — cite ch3 §7.6/§10.10 and ADR-0052, never #25 §9 | [`coverage/03-memory.md`](../spec/coverage/03-memory.md) §1, §0.1 |
| [#26](https://github.com/ludo-lang/ludo/issues/26) Top-level code and frame order | spec ch4 §13.1–§13.5 (the narrowed pair) and spec ch5 §2, §4.3–§4.6 | [`coverage/04-modules.md`](../spec/coverage/04-modules.md) §5, [`coverage/05-runner.md`](../spec/coverage/05-runner.md) §2 |
| [#54](https://github.com/ludo-lang/ludo/issues/54) The reviewer kit | spec ch8 §3.4. **§3 lists #54 → ADR-0019, and the ADR is deliberately not exhaustive** — it records two of four findings and says so; the ranked kit was issue-only until this chapter | [`coverage/08-conformance.md`](../spec/coverage/08-conformance.md) §0.3 |
| [#101](https://github.com/ludo-lang/ludo/issues/101) The mutation mark on a sub-view | spec ch1 §7.10, spec ch3 | [`coverage/01-grammar.md`](../spec/coverage/01-grammar.md) §1, [`coverage/03-memory.md`](../spec/coverage/03-memory.md) §1 |

### 2.1 Issues cited as grounds, which no clause rests on

[#4](https://github.com/ludo-lang/ludo/issues/4),
[#5](https://github.com/ludo-lang/ludo/issues/5),
[#6](https://github.com/ludo-lang/ludo/issues/6),
[#22](https://github.com/ludo-lang/ludo/issues/22),
[#29](https://github.com/ludo-lang/ludo/issues/29) and
[#60](https://github.com/ludo-lang/ludo/issues/60) are cited across the spec as
**evidence, criteria or vetoes** rather than as the authority for a rule.
Chapters 6 and 7 each checked this explicitly and edited nothing. One catch,
recorded by chapter 7: where a later ADR restates a criterion, **the ADR's
reading controls** — ADR-0023 §2 is what spec ch7 §12.4 transcribes, not #5's
original wording of criterion 4.

### 2.2 What the assembly established

1. **Two chapters produced no rows, and that is the result, not a gap.** Chapters
   6 and 7 found that no clause of theirs rests on an issue as sole authority —
   every rule has an ADR behind it. The failure mode this file exists for is
   concentrated in the **early** issues, before the ADR habit set in.
2. **The rows a chapter could not derive were the ones that mattered.** Every
   amendment made during the assembly came from an author reading the ADRs that
   cite their own sources: ADR-0021 §3's narrowing of #26 (ch4), ADR-0047 §3's
   narrowing of #8 and ADR-0052's reversal of #25 §9 (ch3), ADR-0048 §7's rule
   about #26's call 4 (ch5), and the two properties that never reached #19's list
   (ch8). **None of them could be stamped**, because the target is an issue —
   which is the whole reason this file exists.
3. **Building it up front would have been wrong.** The destination column did
   not exist until the chapters did, and its authority moved out from under the
   ADRs mid-flight (ADR-0051 §3). A table built in 2026-08 would have pointed at
   ADRs and would now be a second stale record to maintain.

## 3. Issues that originate an ADR

For completeness, so that "not listed in §1" is checkable. Each of these opens an
ADR by name, and the ADR is authoritative from that point:

| Issue | ADR |
|---|---|
| #12 | ADR-0001, ADR-0003 (**the platform layer and shaders only** — #12's runner calls 2, 3 and 4 originate no ADR and are transcribed straight into spec ch5 §1–§3) |
| #28 | ADR-0004 |
| #40 | ADR-0005 |
| #42 | ADR-0009, ADR-0015 |
| #51 | ADR-0016 |
| #52 | ADR-0017 |
| #53 | ADR-0018 |
| #54 | ADR-0019 |
| #55 | ADR-0021 |
| #58 | ADR-0022 |
| #59 | ADR-0023 |
| #61 | ADR-0024 |
| #62 | ADR-0025 |
| #63 | ADR-0027 |
| #64 | ADR-0026 |
| #65 | ADR-0028 |
| #66 | ADR-0029 |
| #67 | ADR-0030 |
| #69 | ADR-0034 |
| #70 | ADR-0035 |
| #74 | ADR-0037 |
| #75 | ADR-0038 |
| #79 | ADR-0041 |
| #80 | ADR-0042 |
| #81 | ADR-0043 |
| #83 | ADR-0047 |
| #84 | ADR-0045 |

ADR-0002, 0006, 0007, 0008, 0010, 0011, 0012, 0013, 0014, 0020, 0031, 0032,
0033, 0036, 0039, 0040, 0044 and 0046 open from an earlier ADR or from a session
rather than from a ticket.
