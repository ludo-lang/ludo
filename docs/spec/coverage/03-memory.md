# Coverage: chapter 3 — memory and aggregates

**Non-normative.** This file is the closing test for
[`../03-memory.md`](../03-memory.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §4 below says which
happened in each case.

---

## 0. Source check (#72, #73)

Run over **this chapter's sources only** — #8, #15, #25, ADR-0042 and
ADR-0047 — before transcription, per `docs/agents/domain.md`. Four results, and
the first two are the largest findings this chapter made.

### 0.1 The layout source is #25, not ADR-0025

**Issue [#25](https://github.com/ludo-lang/ludo/issues/25)** is *Decide how a
type declares its memory layout*. **ADR-0025** is *Fullscreen is a player
preference the program can read and set*. They share a number and nothing else,
and `SOURCES.md` §3 confirms it: ADR-0025 originates from **#62**, and #25
originates no ADR at all, which is exactly why it sits in §1's sole-authority
table.

The corpus conflates them in **four places**, every one of them citing
"ADR-0025" while meaning #25:

| Site | Text | Status |
|---|---|---|
| spec ch1 §12.2 | `#align(n)` owner: *"ADR-0025 as corrected by ADR-0029 §7"* | **Repaired** to `#25 §3`; recorded at [`01-grammar.md`](01-grammar.md) §4 |
| spec ch2 §4.8 | *"Sum-type layout … is chapter 3's, per ADR-0025"* | **Repaired** to #25 §10 |
| spec ch2 §6.6, §20 | *"the SoA transform (ADR-0025, chapter 3)"*, *"Layout … (ADR-0025)"* | **Repaired** to #25 |
| ADR-0042 §1, ADR-0026 §83 | *"Every struct is C-compatible by construction (ADR-0025)"* | **Not repaired.** ADR prose is immutable; stamps only |

The ch1 §12.2 row was wrong twice over: the source is #25 §3, and the
correction ADR-0029 §7 makes is to **ADR-0024 §6** — which ch1 §12.3 already
cites correctly for the field-level decline. The attribute set never changed;
only the provenance was misrecorded.

**This is not a phantom clause.** The rule exists, is unambiguous, and is
transcribed at ch3 §8. Only the pointer to it was wrong — which is precisely
the failure mode ADR-0033 §1 names as the common cause of every Tier-1 audit
finding, arriving here through a number collision rather than a supersession.
Recorded in [`../../adr/SOURCES.md`](../../adr/SOURCES.md)'s #25 row with the
same *do not conflate the numbers* warning the #18 row already carries for
ADR-0018.

### 0.2 ADR-0047 stamped nothing, and its own §10 is the list of what it owed

ADR-0047 §10 is titled **The stamp list** and names four records it amends.
**None of them carried a stamp.** This is #72's direction-1 failure in its
purest form: the ADR did the analysis, wrote down what it changed, and never
applied it.

Stamped retroactively in this chapter's commit — stamps are additive metadata,
not prose:

| Record | What ADR-0047 moved |
|---|---|
| **ADR-0042** | §2's `alloc`/`grow` returns are legalised by ADR-0047 §1; §4's `Scratch` rule cites #15 Q10 verbatim and inherits its re-cut; §4's capture hazard is closed for the single-function case by ADR-0047 §4 |
| **ADR-0043** | §9's narrow view-return clause collapses into ADR-0047 §1; §10's marker question is closed by §5 and by [#101](https://github.com/ludo-lang/ludo/issues/101) |
| **ADR-0009** | §111's `get_pixels(image: Image) -> []u32` → `get_pixels(image: !Image) -> []u32` |
| **ADR-0022** | the same restamp; *taking the view is the mutation signal* is unaffected |

**#15 needed no stamp** — the target is an issue, so no stamp is possible, and
`SOURCES.md`'s #15 row already carries ADR-0047's re-cut of Q10 in full.

### 0.3 Stamps not owed, recorded so they are not re-derived

- **ADR-0016 §1 and ADR-0008 §4 cite #25's `#align(n)` as precedent** and change
  nothing about it. ADR-0016 §1 *depends* on it (the blessed math set is aligned
  by declaration), which is use, not amendment.
- **ADR-0024 §6 restates #25's own decline of field-level `#align`** rather than
  moving it, and ADR-0029 §7 corrects ADR-0024's *description* of the attribute
  set — not #25. ADR-0024 carries no ADR-0029 stamp; **that is outside this
  chapter's source set** (ADR-0024 is not a chapter-3 source), and is left for
  whichever chapter covers it rather than swept in here.
- **ADR-0045 does not reach this chapter's sources.** It narrows #11, which is
  chapter 2's, and is already recorded there and in `SOURCES.md`.

### 0.4 Issue-source check against `SOURCES.md` (#73)

All three issue sources are in §1's sole-authority table. Two rows were
incomplete and are amended in this chapter's commit; two further rows outside
this chapter's sources were found stale by the same reading and amended
alongside, per #73's *same commit* rule:

| Issue | Amendment |
|---|---|
| **#8** | Now **authoritative, narrowed**: ADR-0047 §3 narrows *aliasing is deleted* to the **container** level, not the view level, and §8's guarantee table gains four compile-error rows it does not itself list |
| **#25** | Now **authoritative, with two named changes**: §2's mandated padding diagnostic is withdrawn by ADR-0023 §1 (§3.1 below), and §9's column exclusivity is contradicted by ADR-0047 §3 ([#103](https://github.com/ludo-lang/ludo/issues/103)). Plus the §0.1 warning |
| **#17**, **#26** | ADR-0042 §8 respells `$.mem.heap` as a bare `heap` and confines it to `persist` initialisers in the entry file. Neither row recorded it |

### 0.5 ADR-0048's stamp list, checked in both directions

ADR-0048 §11 claims three records. All three landed in the ADR's own commit
(`e84d741`), which is the `docs/agents/domain.md` *same commit* rule holding
rather than being repaired after the fact. Verified, not assumed:

| Claimed | Verified |
|---|---|
| **ADR-0042 §3** narrowed — *never a #18 fault* is false in a `persist` initialiser | Stamp present on ADR-0042, under the existing ADR-0047 stamp, accumulated as a separate line in date order |
| **ADR-0042 §9** extended — the blessed-but-unmandated list gains a ring buffer | Same stamp, second clause |
| **#15** — the container set it left open is closed, and its container calls become fallible | `SOURCES.md` row amended; an issue cannot be stamped, which is what that file is for |

**Stamps not owed, recorded so they are not re-derived:**

- **ADR-0047.** ADR-0048's preamble *consumes* §3 rather than moving it — the
  view/growth seam is cited as settled, and ADR-0047 §9's hand-off to #82 is
  discharged by being answered, not amended. No stamp.
- **ADR-0044 §8.** ADR-0048 closes a marked gap, which is the mechanism §8
  provides working as designed. No stamp. (Whether the *repair* should have
  been owed by #82 rather than a successor ticket is a process question, not a
  clause change.)
- **#10.** ADR-0048 §3 and §8 apply #10's must-use and locality rules to a new
  call; §11.10 is a position with no call site rather than an exception to
  `rescue`. Applying a rule is not narrowing it. No stamp.

### 0.6 ADR-0050's stamp list, checked in both directions

ADR-0050 §14 claims eight records. Two are **stamps owed** and both landed in
the ADR's own commit; the rest are explicit no-stamp findings, recorded here so
they are not re-derived. Verified, not assumed:

| Claimed | Verified |
|---|---|
| **ADR-0048** extended — §10's hand-off to #105 is discharged, and §4's withheld `capacity()` turns out to license §2's tombstones | Stamp present on ADR-0048, above its title, in the `docs/agents/domain.md` form |
| **#15 Q24** reversed twice — a descending operator ships, and *not grammar* is corrected | `SOURCES.md` #15 row amended; an issue cannot be stamped, which is what that file is for. The row's existing ADR-0048 sentence is left intact and the reversal accumulates after it |

**Stamps not owed:**

- **ADR-0047 §3.** ADR-0050 §5 *applies* the mark-kills-views rule to removal
  and states where it stops — an index is not a view. Applying a rule and
  naming its boundary is not narrowing it. No stamp.
- **#15 Q12, Q19.** Unchanged, and both are *paid for* rather than moved: Q12's
  insertion order survives via tombstones, Q19's iteration ban survives with no
  carve-out. Recorded in the `SOURCES.md` row for the reader who expects a
  removal ADR to have disturbed them. No stamp beyond that.
- **ADR-0017 / chapter 2 §7.2, §7.4.** ADR-0050 §12 cites them as the reason
  `retain` is deferred. Being a constraint on someone else's decision is not a
  change. No stamp.
- **ADR-0021 §1.** Its const-eval floor is untouched; only chapter 1's *count*
  moves (§13.6, §13.7, §13.12), and a count is not a clause. No stamp.
- **ADR-0044 §6.** ADR-0050 exists **because** of §6 — a reversal takes an ADR
  — so §6 is working as designed. No stamp.
- **ADR-0049.** Its whole content is the obligation this commit discharges: the
  clause repair, the coverage rows and the ADR land together. §4's carve-out is
  exercised for the reference program, and the split is recorded on the map as
  §4 requires. No stamp.

**One record is filed rather than stamped.**
[#107](https://github.com/ludo-lang/ludo/issues/107) — chapter 3 §12.2's
`for x in !xs` contradicts chapter 1 §7.10's postfix mark. Found while resolving
#105 and **not fixed here**: it is independent of removal, it touches a clause
this commit already edits for another reason, and folding it in would put an
unrelated reversal inside a resolution commit. §12.2's prefix spelling therefore
**survives this commit unrepaired**, which is a known defect with a ticket and
not an oversight.

---

---

## 1. Sources this chapter covers

Per issue [#87](https://github.com/ludo-lang/ludo/issues/87): #8, #15, #25
(**not** ADR-0025 — §0.1), ADR-0042 and ADR-0047, plus
[#101](https://github.com/ludo-lang/ludo/issues/101), which unblocked it.

### #8 — Choose the memory model

| Source clause | Landed |
|---|---|
| Tracing GC excluded on frame pauses | §1.1 |
| Borrow checker excluded; no lifetimes, regions or aliasing analysis | §1.2 |
| Safety is how you normally work, checked at run time — not a compile-time proof | §1.3 |
| §1 Generational handles for references; arenas for allocation | §2.3, §10.1, §9.4 |
| §1 Raw pointers and manual free live only in a marked escape hatch | §16.1, §16.2 |
| §2 `{...}` is a fixed-size value and does not allocate | §5.4 |
| §2 You name an allocator only when something must grow | §1.7, §5.5 |
| §2 Conflict-3 from #6 resolved: the beginner-facing `{}` costs no allocator decision | §5.4. The conflict's history is rationale — **explicitly dropped**, §3 |
| §3 No raw pointers in the safe layer; `&x`-and-store does not exist | §2.1, §18 |
| §3 Lasting links are generational handles | §2.3 |
| §3 Passing is by value; the compiler may pass big values by hidden reference | §2.2 |
| §3 No aliasing to reason about | §1.2 — **and see §3.2**, narrowed by ADR-0047 §3 to the container level (§7.4) |
| §4 In-place mutation only through transient, marked, non-escaping access | §2.4 |
| §4 A writable parameter or a pool's scoped lend, marked at the call site | §2.4 |
| §4 Write access cannot be stored past the moment | §2.4, §18 |
| §5 `defer` next to the acquire; destructors rejected as non-local | §15.1, §15.2 |
| §5 Resource types that must be closed are must-use; a forgotten `defer` is a named compile error | §15.5 — **and see §3.5**, the set is unfixed |
| §5 must-use is a shallow *was this consumed?* check, not lifetime tracking | §15.3 |
| §6 Loud, greppable, per-site `unsafe`; an escape hatch restated at each use site | §16.1 |
| §6 Raw pointers are a distinct, greppable type | §16.2 — **Phantom, declined**, §16.5 / §19.3 → [#104](https://github.com/ludo-lang/ludo/issues/104) |
| §6 `&x`, deref and manual alloc/free are legal only inside `unsafe` | §16.2 |
| §6 Encapsulatable: a veteran wraps unsafe internals behind a safe function | §16.4, §9.4 |
| §7 An allocator is a library interface you pass; the core knows nothing special about it | §9.1 |
| §7 AoS struct pool: fixed capacity, acquire/release, `active` flag, handle = slot | §10.4 |
| §7 Columnar SoA pool: handle = row, batch ops over a whole column | §10.4, §10.6 |
| §7 A general-purpose allocator, and a per-frame scratch arena passed down, never ambient | §9.12, §9.9, §1.6 |
| §7 The swap-in checker allocator is ordinary polymorphism, not a build mode | §1.5, §9.1, §9.13 |
| §7 Seam: the SoA *declaration syntax* is handed to the data-layout fog item | Discharged by #25, whose clauses are transcribed at §8 and §10. **Explicitly dropped** as a routing instruction — §3 |
| §8 The guarantee table | §17 |
| §8(a) No build-mode behaviour switch; bounds checks always on | §1.4 |
| §8(b) Leak detection is a swapped allocator, not a hidden mode | §1.5 |
| §8 Memory leaks are detected, not prevented; *"no crashes," not "no leaks"* | §1.8, §17 |
| §8 Integer overflow always checked | Chapter 2 §8.3 (bug list); the no-build-mode half is §1.4 |
| §8 Out-of-bounds index cannot corrupt | §17; the bug itself is chapter 2 §8.3 |
| §8 Null deref cannot happen | §17, routed to chapter 2 §9.1 |
| Comment: Odin-style implicit `context` is dead; Zig-style explicit passing survives | §1.6 |
| Comment: no builtin container that grows implicitly | §1.7 |
| Comment: memory safety sits in #5's Tier 2, so the agent lens has no veto here | Rationale. **Explicitly dropped** — §3 |
| The three-lens read, and the cross-ticket effects list | Rationale and routing. **Explicitly dropped** — §3 |

### #15 — Decide what replaces Lua's one-table aggregate

| Source clause | Landed |
|---|---|
| Q1 Two core aggregates: nominal `struct` and fixed array `[N]T` | §4.1 |
| Q1 `List[T]` and `Map[K,V]` are library generics over that pair plus an allocator | §4.3, §11.1 |
| Q1 **No tuple type** | §4.2, §18 |
| Q1 Type spellings: prefix `[` an array, postfix `[` after a name a generic argument list | Spelled at ch1 §9.4, §9.8 — no semantics to add |
| Q2, Q13 One literal `{...}`, the expected type decides the meaning | §5.1 |
| Q2, Q22 The expected type comes only from the signature radius, never whole-program inference | §5.2 |
| Q3 A bare `{...}` with no expected type is a compile error naming the missing entity | §5.3 |
| Q2 `{[expr] = v}` for computed or non-string keys | §5.8 |
| Q2 Literals nest recursively | §5.9 |
| Q2 `{x = 1}` is a struct or a `Map` by target — the fix for Luau#745 | §5.1. The Luau comparison is rationale — **explicitly dropped**, §3 |
| Q8 Struct literals are named-field only | §5.6, §18 |
| Q21 Field defaults are allowed; literals may omit only defaulted fields | §5.7 (reconciled with #9 at chapter 2 §9.6) |
| Q15, Q16 `l: List[int] = {1,2,3}` is illegal — construction *is* growth | §5.5 |
| Q15, Q16 A `List` carries its allocator, captured at construction | §11.1 |
| Q10, Q23 `[]T` is a transient non-escaping view | §6.1, §6.3 |
| Q10 *"never returned"* | **Reversed by ADR-0047 §1** — §6.5 |
| Q10 *"parameter position only"* | **Clarified by ADR-0047 §2** to not-stored and not-escaping — §6.3, §6.4 |
| Q10 Never stored in a struct, in `persist`, or captured | §6.3 (unchanged) |
| Q10 It is #8's lend; `!` marks it writable | §6.2, §6.8 |
| Q10 A column **is** a `[]T`, so batch ops are ordinary functions over views | §6.1, §10.6 |
| Q10 `[]T` is what makes `[3]int` and `List[T]` unify at a boundary | §6.1 |
| Q10 `..<` doubles as the slicing operator | §6.11 (spelled at ch1 §7.7) |
| Q14 Containers move on assignment and are not copyable | §3.1, §18 |
| Q14 Sharing goes through views, duplication through explicit `clone(allocator)` | §3.4 |
| Q14 The check is shallow — *was it moved?* — not lifetime tracking | §3.2 |
| Q20 Non-copyability is transitive with no marker; the error names the field and its path | §3.3 |
| Q4 0-indexed, the first deliberate break with Lua | §6.10 (spelled at ch1 §7.8) |
| Q4 `xs.len` is always one past the last valid index | §6.10 |
| Q5 Key types restricted by a `Key` constraint; floats excluded | §13.2 |
| Q18 `Eq`, `Hash`, `Clone` structurally derived and not hand-writable | §13.3 |
| Q18 Custom hashing through a `distinct` wrapper | §13.4 |
| Q18 Derived `Clone` recurses into container fields, so `clone` takes an allocator | §13.5 |
| Q12 Map iteration order is insertion order | §13.6 |
| Q17 `==` is structural elementwise; a `Map` comparison ignores insertion order | §13.1 |
| Q6, Q19 One iteration construct over everything, via a nominal `Iter[T]` constraint | §12.1 |
| Q19 Static dispatch and monomorphisation; no iterator boxing; user types can satisfy it | §12.1 |
| Q19 `for x in xs` binds a copy; `for x in !xs` binds a writable view per element | §12.2 |
| Q19 The loop's lend is exclusive, so structural mutation during iteration is a compile error | §12.2, §17 |
| Q24 Half-open ranges only; no `..=`; ranges are ordinary `Iter[int]` values | §12.3 |
| Q19 `for i, x in xs.pairs()` | §12.4 |
| Q11 Strings immutable UTF-8, not an indexable aggregate; `s.bytes()` / `s.chars()` | **Superseded by ADR-0043** — chapter 2 §17; `bytes()` is deleted by ADR-0043 §8. Recorded, not transcribed — §3.4 |
| Q7 Fixed-arity multiple returns, destructured; no first-class tuple; the group has no type | §14.1, §14.2 |
| Q7 Destructuring must be total; `_` the one greppable discard | §14.3 (spelled at ch1 §6.3) |
| Q7 #10 rejected multiple returns as the *error channel*, not the mechanism | §14.4 |
| Boundary: missing-key semantics stay with #9 | §13.7, routed to chapter 2 §9.7 |
| Boundary: memory layout and C mapping are #25 and #29 | §8, §8.6 |
| The three lenses, and the Teal/Luau evidence | Rationale. **Explicitly dropped** — §3 |

### #25 — Decide how a type declares its memory layout

| Source clause | Landed |
|---|---|
| §1 The pool decides the layout; the struct is layout-agnostic | §8.1 |
| §1 The SoA transform is compiler-derived and cannot be hand-written | §10.5 |
| §1 Satisfaction is a derived constraint, never declared, checkable once per #11 | §10.5 |
| §2 Declaration order guaranteed; no reordering; no `#repr` | §8.2, §18 |
| §2 Padding is C-compatible; every ludo struct is C-ABI-compatible by construction | §8.2, §8.6 |
| §2 Named cost: you order your own fields | §8.2.1 |
| §2 Mitigation: the compiler *reports* the padding and a better order as a diagnostic | §8.2.2 — **withdrawn as a mandate** by ADR-0023 §1; see §3.1 |
| §3 `#align(n)` ships; `#packed` does not; a packed C header is #29's problem | §8.3 |
| §3 `#align` is type-level only; field-level is declined | §8.3 (ch1 §12.3, ADR-0024 §6) |
| §3 What `n` may be | **Phantom — authored at §8.3.1, §19.2** |
| §4 Bit fields are out; packing flags is `distinct` plus visible shifts | §8.4, §18 |
| §5 The SoA split is exactly one level deep — one column per top-level field | §10.7 |
| §5 No striding anywhere, which is what makes every column a genuine `[]T` | §10.7, §6.1 |
| §5 Granularity stays in the user's hands; named cost: hand-flattening | §10.7 |
| §6 Single-element access on a columnar pool materialises a copy | §10.8 |
| §6 `pool.get(h) -> ?Entity`; writes through a whole-value `pool!.set(h, e)` | §10.2, §10.8 |
| §6 In-place mutation exists only through columns | §10.8 |
| §6 The synthesised in-place `!Entity` lend is rejected | §10.8, §18 |
| §6 The difference between the pools is cost, never semantics — and the cost belongs in the spec text | §10.8 |
| §7 A column is named by derived field access — `rocks.pos` is the `[]Vec2` column | §10.6 |
| §7 The derivation is total and mechanical; every field, no opt-out | §10.6 |
| §7 Named cost: `rocks.pos` is `[]Vec2` while `e.pos` is `Vec2` | §10.6 |
| §8 Columnar pools require all fields to be copyable plain data | §10.9 |
| §8 The error names the offending field and points at the AoS pool | §10.9 |
| §9 Exclusivity is per place and distinct columns are distinct places | **Contradicted by ADR-0047 §3** — §10.10, recorded not resolved → [#103](https://github.com/ludo-lang/ludo/issues/103) |
| §9 `rocks.pos!` beside `rocks.vel` accepted | §7.4, §10.6 — granted more broadly by ADR-0047 §3 |
| §9 `rocks.pos!` beside `rocks.pos` is a compile error naming the column | **Not transcribed** — §10.10, §3.2 |
| §9 `pool.each()` lends the whole pool | §10.11 |
| §10 Sum-type layout: tag then payload, smallest tag that fits, alignment the max of the variants | §8.5 |
| §10 There is no niche optimisation | §8.5, §18 |
| §10 `?T` costs a tag, always; any type's size is computable from its declaration alone | §8.5.1 — **and see §3.3** on *tag word* |
| §10 Additive later; not addable back once C code depends on a size | §8.5.1 |
| §10 #9's `?T`, #10's error sets and #11's sum types are one mechanism | §8.5.2 |
| Hand-off: #29 is unblocked and inherits a smaller job | §8.6, §20 |
| The stale-premise correction (*a column is not a `[]f32`*) | Bookkeeping against this ticket's own body. **Explicitly dropped** — §3 |
| The three lenses | Rationale. **Explicitly dropped** — §3 |

### ADR-0042 — An allocator is a struct of function pointers

| Source clause | Landed |
|---|---|
| §1 The three-way contradiction (#8 §7, #11 Q2, ADR-0015) | Rationale. **Explicitly dropped** — §3 |
| §1 An allocator is an ordinary `struct` of non-capturing function pointers | §9.1 |
| §1 A struct is a value, so it is exchangeable at run time — #8 §7 satisfied | §9.1, §1.5 |
| §1 Not a boxed existential; no subtyping, no vtable | §9.1 (chapter 2 §6.3) |
| §1 Rejected: making the language know the type | Rationale. **Explicitly dropped** — §3 |
| §2 The three operations and their signatures | §9.2 |
| §2 Alignment is a parameter, not an attribute | §9.3 |
| §2 No per-allocation `free` in the safe layer; release is `reset()` | §9.4, §18 |
| §2 A container releases a single old buffer inside `unsafe` | §9.4, §16.2 |
| §2 `grow` exists because `List` would otherwise be quadratic; it may fail; the caller copies | §9.5 |
| §3 Exhaustion is a value, never a fault; `alloc`/`grow` return `?[]u8` | §9.6 |
| §3 One nominal `OutOfMemory`; the mandated facade error sets absorb it now | §9.6, §9.6.1 (facade routed to chapter 6) |
| §3 The deciding case: a fixed-capacity pool's cap is a design condition | §9.6 |
| §4 Two types: `Allocator` storable, `Scratch` transient and non-escaping | §9.7 |
| §4 `Scratch` reuses #15's view rule; a restriction on positions, not lifetimes | §9.7 |
| §4 Rejected: one storable type; rejected: the generational arena | Rationale. **Explicitly dropped** — §3 |
| §5 `List[T]` keeps one type parameter; the allocator kind is inferred and carried invisibly | §9.8 |
| §6 `scratch: !Scratch` joins the drawing entry; `!` is required | §9.9 (entry shape routed to chapter 5) |
| §6 Rejected: a program-owned arena | Rationale. **Explicitly dropped** — §3 |
| §7 The mandated implementations are runner-owned, which is what makes reload work | §9.10 |
| §7 A `persist` container backed by a user-written allocator is rejected | §9.10 (the `persist` rule itself is chapter 5's, #17 §3) |
| §7 In a release build the predicate is vacuous and still passes | Routed to chapter 5 with §9.10 |
| §7 Rejected: an allocator as a reload-stable handle | Rationale. **Explicitly dropped** — §3 |
| §8 `Allocator` and `Scratch` are core prelude types, not a sixth facade | §9.11 |
| §8 `$.mem.heap` is respelled a bare `heap` | §9.12 (and `SOURCES.md`'s #17 and #26 rows — §0.4) |
| §8 `heap` is legal only in `persist` initialisers, in the entry file | §9.12 |
| §9 Mandated: the two types, the frame arena, one `heap` | §9.13 |
| §9 Blessed but unmandated: the AoS pool, the SoA pool, the checker allocator | §9.13, §10.4 |
| §10 No new #19 property; zero #24 delta | Chapter 8. **Explicitly dropped** here — §3 |

### ADR-0047 — A returned view is derived from its receiver

| Source clause | Landed |
|---|---|
| Preamble: #15 Q10's rule is false in the corpus; four mandated signatures return a view | §6.5, and the four exhibits at §6.9, §9.2 |
| §1 A view may be returned iff derived from the receiver or the constant blob | §6.5 |
| §1 A function with no receiver cannot return a view | §6.5 |
| §1 The constant blob is the only unconditional lifetime; the widening makes it writable | §6.7 |
| §1 The rule is checked, not asserted; both sources are visible in the callee's body | §6.6 |
| §1 Rejected: the enumerated allow-list; rejected: an `unsafe`-shaped obligation | Rationale. **Explicitly dropped** — §3 |
| §2 A local binding is a legal view position | §6.4 |
| §2 The struct-field, `persist` and capture bans are unchanged | §6.3 |
| §2 A returned view nothing may bind buys nothing — §1 and §2 are one decision | §6.4, §6.5 |
| §3 A view dies when the place it derives from is next used with `!` or moved | §7.1 |
| §3 The fix is a re-take, and it needs no annotation | §7.1 (worked in the chapter's example) |
| §3 The check is flow-insensitive and spans the function body | §7.2 |
| §3 The kill fires at a call site that takes the place as a `!` operand | §7.3 |
| §3 **There is no exclusivity rule**; any number of views may co-exist | §7.4, §18 |
| §3 Rejected: flow-sensitivity; rejected: block-scoped death | Rationale. **Explicitly dropped** — §3 |
| §4 A container constructed from a place derives from it, and the same mark kills it | §7.5 |
| §4 This is the general rule, not a `Scratch` special case | §7.5 |
| §5 Writability flows from the receiver | §6.8 |
| §5 `get_pixels` is restamped `get_pixels(image: !Image) -> []u32` | §6.9 (facade routed to chapter 6) |
| §5 Rejected: marking the return type `-> ![]u32` | §6.8, §18 |
| §5 The slice-marker spelling is left open | **Closed by [#101](https://github.com/ludo-lang/ludo/issues/101)** — §6.12 |
| §6 What this is not: no lifetimes, regions, shared/exclusive, no cross-function check | §7.8 |
| §6 It is the move check's shape | §7.8, §3.2 |
| §7 `alloc` is not reclassified as `unsafe`-shaped | §9.6, §16.2 — a direct `alloc` is ordinary safe code, and the `unsafe` in ADR-0042 §2 is for **release** only (§9.4) |
| §8 ADR-0043 §9 collapses into §1; `format`'s signature is unchanged | §6.9 |
| §9 #82 inherits §3 rather than authoring it; the seam is answered in full | §11.4 |
| §9 Spec chapter 3 transcribes §1–§5 and extends the reference program | This chapter, and [`../reference/reference.ludo`](../reference/reference.ludo) |
| §10 The stamp list | **Discharged in this commit** — §0.2 |
| §11 Zero #24 delta; no new #19 property; the three lenses and the named conflict | Chapter 8 and rationale. **Explicitly dropped** — §3 |

### ADR-0048 — Every allocating call is fallible, and a `persist` initialiser faults

Resolves #82, the marked gap this chapter carried at §11.3. The gap closes and a
narrower one opens at §11.13.

| Clause | Where it landed |
|---|---|
| §1 The membership test: mandated when it cannot be built in the safe layer from what is mandated | §11.3 |
| §1 Ties break on silence | §11.3 |
| §1 Rejected: *the reference program needs it*; rejected: silence as the test itself | Rationale. **Explicitly dropped** — §3 |
| §2 The mandated set is exactly `List[T]` and `Map[K,V]` | §11.3 |
| §2 A ring buffer is blessed but unmandated | §11.5 |
| §2 No `Set[K]`; `Map[K, bool]` is the spelling, with a `distinct` wrapper for set semantics | §11.5 |
| §3 The trilemma, and the infallible `push` is the horn that breaks | Rationale. **Explicitly dropped** — §3 |
| §3 `push` is fallible | §11.6, §11.1 |
| §3 Rejected: over-push as a bug; rejected: fixing capacity at construction | Rationale. **Explicitly dropped** — §3 |
| §4 `reserve(n)` is additional and idempotent | §11.7 |
| §4 Rejected: the absolute reading of `reserve` | Rationale. **Explicitly dropped** — §3 |
| §4 There is no `capacity()`; the checker allocator is the instrument | §11.8 |
| §4 The eight-use enumeration | Rationale. **Explicitly dropped** — §3 |
| §5 `Map` is symmetric with `List`, `reserve` included | §11.9 |
| §6 `clone` and `from` are fallible; every allocating call is fallible | §11.6 |
| §7 A `persist` initialiser faults; all three of #10's exits are unavailable | §11.10 |
| §7 The fallback compiles and is a trap | §11.10 |
| §7 This narrows ADR-0042 §3 | §9.6 (narrowing stated in place), §11.10 |
| §8 `rescue` is local; `OutOfMemory` is not viral | §11.11 |
| §8 Rejected: inferred error sets; rejected: an implicitly-absorbed variant | Rationale. **Explicitly dropped** — §3 |
| §9 The containers are runner-owned; the release seam stays | §11.12 |
| §10 Removal semantics are handed to #105 | **Discharged.** #105 is resolved by ADR-0050; the gap at §11.13 is now normative clauses §11.13–§11.13.9 and left §20 |
| §10 This chapter transcribes §1–§9; the reference program gains container use | This chapter, and [`../reference/reference.ludo`](../reference/reference.ludo) |
| §11 The stamp list | **Verified** — §0.5 |
| §12 Zero #24 delta; no new #19 property; the three lenses and the named conflict | Chapter 8 and rationale. **Explicitly dropped** — §3 |

### ADR-0050 — Removal names its cost at the call site, and descending iteration earns an operator

Resolves #105, the marked gap this chapter carried at §11.13. **The gap closes
and no successor opens in this chapter.** Two of its clauses land in chapter 1
rather than here, which is the routing §0.3 requires: the operator and the
prelude types are *syntax and spelling*, and this chapter owns only what they
mean for containers.

| Clause | Where it landed |
|---|---|
| §1 `remove(i) -> T` shifts, O(n); `swap_remove(i) -> T` reorders, O(1); both ship | §11.13 |
| §1 The reordering is named at the call site — the agent-friendliness argument | §11.13 |
| §1 `pop() -> ?T` | §11.13.1 |
| §2 `Map.remove(k) -> ?V` tombstones; compaction on growth or `reserve` | §11.13.4 |
| §2 §13.6's insertion order does not move; `len` counts live entries | §11.13.4 |
| §2 The tombstone cost is unobservable because §11.8 withholds `capacity()` | §11.13.4 |
| §2 Named cost: a delete-heavy `Map` that never grows holds its slots | §11.13.4 |
| §2 Rejected: swap-into-the-hole | Rationale. **Explicitly dropped** — §3 |
| §3 Removal returns the element; the move-on-assignment argument | §11.13.2 |
| §4 Out-of-range removal faults; empty `pop` is `none` | §11.13.3, §11.13.1 |
| §5 Removal kills views by §7.1 and creates no new rule | §11.13.6 |
| §5 A held index is not protected — a documented consequence | §11.13.7 |
| §5 Rejected: linear indices; generational handles are blessed-unmandated | §11.13.7, and rationale. **Partly dropped** — §3 |
| §6 Removal is structural mutation; §12.2 takes no carve-out | §12.2 |
| §6 The sanctioned cull iterates a range, so no lend exists | §12.2.1 |
| §6 Descending is the correctness condition, not a style | §12.2.1 |
| §7 The stable cull needs no `swap(i, j)` | §11.13.8, §12.2.1 |
| §8 `>..` is the descending range operator | **Chapter 1 §7.7.1** |
| §8 The angle points at the bound it excludes; the length is always excluded | **Chapter 1 §7.7.1** |
| §8 An empty or inverted range iterates zero times | **Chapter 1 §7.7.4** |
| §8 `>..` is a single token and does not collide | **Chapter 1 §7.7.1** |
| §8 #15 Q24 ruled out `..=`, not a descending operator | **Chapter 1 §7.7.1**, and rationale. **Partly dropped** — §3 |
| §9 The operator is grammar; the range is an ordinary value | **Chapter 1 §7.7.2** |
| §9 `Range` and `RevRange` are prelude identifiers | **Chapter 1 §2.9, §7.7.2** |
| §9 Blessed conformance to `Iter[int]`; independent of #100 | §12.5.1, **chapter 1 §7.7.2** |
| §9 Reification collapses the slice special case | **Chapter 1 §7.7.2** |
| §10 `RevRange` does not slice | **Chapter 1 §7.7.3** |
| §11 `truncate(n)` cuts to the first `n` and returns `()` | §11.13.5 |
| §11 `n > len` is a no-op — a bound, not an index | §11.13.5 |
| §11 The buffer survives; `truncate(0)` is the per-frame reuse call | §11.13.5 |
| §11 No `clear()` — one spelling per meaning | §11.13.5 |
| §11 The name is contested and ships on corpus grounds | Rationale. **Explicitly dropped** — §3 |
| §12 `retain` is deferred pending #100, not rejected | §11.13.8 |
| §12 No `swap`, no `List.with_capacity` | §11.13.8 |
| §12 No front-drop or FIFO; ruled out of scope to #108 | §11.13.9 |
| §13 The budget overrule and its tier-2 payment | **Chapter 1 §13.9.1**, and §13.6, §13.7, §13.12 |
| §14 The stamp list | **Verified** — §0.6 |
| §15 Costs and the three lenses | Rationale. **Explicitly dropped** — §3 |

### #101 — Where the mutation mark sits on a sub-view

| Source clause | Landed |
|---|---|
| `name![a..<b]` — the mark rides the root binding | §6.12 (spelled at ch1 §7.10a) |
| `name[a..<b]!` is a hard error with the fix named | §6.12, §18 |
| `xs![0..<k]` kills a view of `xs[k..<n]`, non-overlapping or not; no range reasoning | §7.6 |
| Two disjoint writable windows bound simultaneously are foreclosed | §7.7 → [#102](https://github.com/ludo-lang/ludo/issues/102) |
| `grammar.ebnf` is untouched; both chains already parse | Ch1 §7.10a; nothing for this chapter to add |
| The `w.rocks!` defect | Landed in chapter 1 §7.10 and [`01-grammar.md`](01-grammar.md) §4 |
| ADR-0047 §5 is confirmed, not contradicted | §6.12 |
| The three lenses | Rationale. **Explicitly dropped** — §3 |

---

## 2. Sources this chapter draws on without covering

Each is owned by another chapter, and this chapter takes only what it needed to
state a memory rule. Listed so that chapter's coverage file does not find its
source already half-spent without a record.

| Source | Taken | Clause |
|---|---|---|
| #5 | Criterion 1 (locality, which kills the ambient allocator); criterion 5 (a failure the compiler cannot attribute is vetoed) | §1.6, §3.3, §10.9 |
| #9 | A stale handle is an absence; *uninitialised memory in `unsafe` only*; the missing-key read | §10.2, §16.2, §13.7 |
| #10 | Allocation failure is the allocator's policy; `defer` and the absence of `errdefer` | §9.6.2, §15.1 |
| #11 | Q3 mutability is a property of the place; Q2's ban on the boxed existential; Q13's handle-or-container recursion | §2.5, §9.1, §10.3 |
| #12, #29 | The C ABI as the universal binding surface, and the boundary this chapter hands it | §8.2, §8.6 |
| #16 | ECS is not a language feature; this chapter delivers what it needs and stops | §8.1, §10.4 |
| #17 §3 | A `persist` declaration may not reach a pointer into the reloadable image | §9.10 (routed to chapter 5) |
| #24 | The payment rule, which is why §19.3 declines to author the pointer type | §19.3 |
| #26 | `persist` initialisers are cold-start-only and entry-file-only, which is what confines `heap` | §9.12 |
| #108 | The mandated set has no front-drop or FIFO — a stated absence, backlogged under ADR-0044 §7 rather than fogged | §11.13.9 |
| ADR-0008 §4, ADR-0029 §8 | The attribute mechanism is one production and names cost nothing | §8.3 |
| ADR-0015 | `decode_image(allocator, bytes)` is the mandated signature the allocator type has to fit | §9.1, §9.6.1 |
| ADR-0016 §1 | The blessed math set is aligned by declaration, which is why alignment is a call parameter | §9.3 |
| ADR-0018 | A compile error is reported under chapter 7's envelope | §0.2 |
| ADR-0021 §1 | The const-eval floor an `#align` argument must fold under | §8.3.1 |
| ADR-0023 §1, §2 | A spec-owned diagnostic is an error or it does not exist; accept/reject is flag-invariant | §1.4, §8.2.2 |
| ADR-0024 §6, ADR-0029 §7 | Field-level `#align` is declined permanently | §8.3 |
| ADR-0026 | The storage surface, which is where a *resource type* would come from | §15.5, §20 |
| ADR-0032 | *Ours, own repo, blessed by reference, unmandated* — the treatment the pools and the ring buffer take | §9.13, §11.5 |
| ADR-0043 §1, §2, §4 | The constant blob's lifetime; the one-way widening; *slicing is free*, which is #15's, inherited by `[]u8` | §6.7, §6.11 |
| ADR-0044 §6, §8 | Repair in the spec text; a marked gap may be written down rather than answered | §11.13, §19 |
| ADR-0045 | The array-length position and the `usize` length type | §6.10 (spelled at ch1 §9.5) |
| Chapter 2 | §1.7, §3.1, §6.2, §6.3, §6.6, §8.3, §8.4, §9.1, §9.7, §10.2, §10.11, §10.15, §10.17, §12.1, §14.2, §15.2, §17.2, §17.10 | throughout |

---

## 3. Sources located but not transcribed, with the reason

Required by ADR-0044 §5: a clause is transcribed **or explicitly dropped**, and
the drop is recorded.

1. **Every "three lenses" section, every rejected-alternative argument, and
   every cross-ticket effects list.** #8, #15, #25, ADR-0042 §11 and ADR-0047
   §11 each close with a simplicity/robustness/agent-friendliness assessment,
   and each argues its rejections at length. None states a rule an
   implementation could satisfy or violate. **Explicitly dropped** under
   ADR-0044 §5's *the ADRs are the rationale record*, with the standing
   exception chapter 2 §3.1 named: where a rejection **is** the rule (no
   `#packed`, no bit fields, no niche optimisation, no `-> ![]T`, no
   per-allocation `free`), the prohibition is transcribed and §18 collects them.

2. **#25 §9's compile error — located, contradicted, and not transcribed.**
   #25 §9 rules that *exclusivity is per place and distinct columns are distinct
   places*, and states the negative: `rocks.pos!` beside `rocks.pos` is a
   compile error naming the column. **ADR-0047 §3 is later and general and says
   there is no exclusivity rule between views at all**, on the ground that
   many-readers-or-one-writer is the borrow checker's own rule and that #8
   deleted aliasing at the **container** level, not the view level.

   The two are not reconcilable by reading them harder. ADR-0047 §3 *grants*
   #25 §9's motivating case more broadly — nothing kills either operand of
   `integrate(rocks.pos!, rocks.vel)` — while leaving #25 §9's prohibition
   following from no rule this specification states.

   The chapter transcribes what both agree on (§7.4, §10.6, §10.11) and records
   the disagreement at §10.10. **Not repaired here**: choosing either way
   reverses a decision, and ADR-0044 §6 reserves a reversal for an ADR, so this
   is filed rather than settled — [#103](https://github.com/ludo-lang/ludo/issues/103).
   It is the first time the consolidation has found two sources that
   *contradict* rather than one that is silent, which is why the treatment is
   spelled out here for the chapters that follow.

3. **#25 §10's *"a tag word"* is the same clause stated loosely, and the
   mechanical rule governs.** §10 gives the layout precisely — *tag then
   payload, smallest tag that fits, alignment the max of the variants* — and
   then summarises it as *"`?T` costs a tag word, always"*. The two do not
   agree for a small payload: under the mechanical rule `?u8` is two bytes, not
   two words. §8.5 transcribes the mechanical rule and §8.5.1 states the
   consequence — *`?T` always costs a tag* — without the word *word*.
   **Not a phantom and not a contradiction**: it is one clause and one
   paraphrase of it, and the chapter takes the clause.

4. **#15 Q11's string rules — superseded, not dropped.** *Immutable UTF-8, not
   an indexable aggregate, iterate via `s.bytes()` or `s.chars()`* is
   **superseded by ADR-0043**, which makes `string` a view into the runner's
   constant blob and **deletes `bytes()`** (§8). Chapter 2 §17 carries what
   survives. Recorded here because #15 is this chapter's source and a reader of
   #15 alone would find the language contradicting it; `SOURCES.md`'s #15 row
   already carries the supersession.

5. **The must-use resource-type set — located as a silence, not authored.**
   #8 §5 requires that *"resource types that must be closed are must-use"* and
   names files, sockets and GPU handles as the motivating cases. **The language
   declares none of them**: there is no file type, no socket type, and the
   storage surface (ADR-0026) is chapter 5's declared-slot model with nothing to
   close. §15.5 transcribes the rule and §20 records that its domain is empty
   today. **Not authored**: minting a resource-type concept is a design decision
   with a failure class of its own, not a transcription defect, so ADR-0044 §6's
   repair rule does not reach it. Left unfiled deliberately — nothing presses on
   it, and chapter 5 is where a first resource type would arrive.

6. **ADR-0042 §10 and ADR-0047 §11's #24 and #19 accounting.** Both report zero
   deltas against the grammar budget and mint no experience-contract property.
   Bookkeeping for **chapter 8**, which owns the totals, not a rule about
   memory. **Explicitly dropped**, on the ground chapter 2 §3.4 used for
   ADR-0043 §11's stamp list.

7. **#8 §7's seam instruction.** *"The SoA declaration syntax … is handed to the
   data-layout fog item as a hard requirement"* is a routing instruction, and
   the fog item is #25, whose answer is transcribed at §8 and §10. The
   instruction is discharged, not a rule. **Explicitly dropped.**

8. **A routing gap, recorded rather than resolved: multiple returns.**
   Ch1 §5.10 and §6.3 spell fixed-arity multiple returns and route the
   restriction — *the returned group has no type; legal only in a destructuring
   binding or a `return` tail* — to **chapter 2**. Chapter 2 does not transcribe
   it: its coverage file's only multi-return row is #10's *never the error
   mechanism* (§10.2), which is a different clause. #15 Q7 is **this** chapter's
   source, so the rule lands here at §14, and §14.5 says so in the chapter text.
   **Not a phantom** — the source exists and is unambiguous; only its home was
   wrong, the same shape as chapter 2 §3.6's ADR-0021 routing conflict.

---

## 4. Phantom clauses

A **phantom clause** is a source clause cited by the corpus that was never
authored anywhere. Under ADR-0044 §6 the repair goes in the spec text, and an
ADR is written only for a reversal. None of the entries below is a reversal, so
none takes an ADR.

**Three were found.** Two are authored in `../03-memory.md` §19; the third is
declined.

| # | Phantom | Cited by | Repair |
|---|---|---|---|
| 13 | `Handle[T]` — the type of a generational handle | #8 §1/§3 make it the language's one lasting link; #9 and #25 §6 fix `pool.get(h) -> ?Entity`; spec ch2 §14.2 writes `[]Handle[Node]` from #11 Q13 | Authored, §10.3 / §19.1 |
| 14 | What `n` may be in `#align(n)` | #25 §3 ships the attribute; ch1 §12.2 lists it; ADR-0016 §1 depends on it; no source constrains the argument | Authored, §8.3.1 / §19.2 |
| 15 | The raw-pointer type | #8 §6 requires *"a distinct, greppable type"*; §8's guarantee table and ch3 §16.2 both depend on it existing; no source spells it, and neither `grammar.ebnf` nor ch1 §9 has a pointer production | **Declined**, §16.5 / §19.3 → [#104](https://github.com/ludo-lang/ludo/issues/104) |

The numbering continues chapter 2's twelve.

**Why #15 is declined rather than repaired.** Chapter 2 §19.3 drew the line and
this chapter reads it the same way: a repair that costs a name or a checker rule
is a transcription defect and goes in the spec text; a repair that adds
**grammar productions** moves the counts ch1 §13.6, §13.7 and §13.12 publish,
which ch1 §13.8's payment rule charges, and a decision with a budget consequence
is not a transcription defect. A pointer type needs both a type-sublanguage
production and an expression production for `&x` and the dereference, so it is
the most expensive repair the consolidation has met, and it is filed as one.

Phantoms 13 and 14 pass the same test in the other direction. `Handle[T]` is a
prelude identifier applied to an existing `GenericArgs` (ch1 §9.8) — **zero**
against the budget, and chapter 2 is already spending the name. `#align(n)`'s
argument rule adds no production at all; all three of its constraints are
derived from decisions already taken (the const-eval floor, what an alignment
is, and §8.3's refusal of `#packed`) rather than chosen here, which is why the
repair is one sentence.

**Two further findings that are not phantoms**, recorded so they are not
re-derived as one:

- **The ADR-0025 / #25 conflation** (§0.1) is a **mis-citation**, not a missing
  rule. Four sites pointed at the wrong record while the rule they wanted was
  written, unambiguous, and transcribable. Three of the four are repaired; the
  two in ADR prose are not, because ADRs take stamps and never edits.
- **#25 §9 versus ADR-0047 §3** (§3.2) is a **contradiction**, not a silence —
  the first the consolidation has hit. Both clauses exist and are clear, and
  they disagree. It is filed rather than repaired because the repair is a
  reversal either way.
