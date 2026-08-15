# Coverage: chapter 2 — types

**Non-normative.** This file is the closing test for
[`../02-types.md`](../02-types.md), required by ADR-0044 §5:

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

Discharged on [#86](https://github.com/ludo-lang/ludo/issues/86) before
transcription and recorded here rather than repeated:

- **ADR-0043** is correctly stamped by ADR-0045 (`TextBuf[N]` →
  `TextBuf[N: usize]`).
- **ADR-0017 is owed no stamp.** The three ADRs naming it only cite it:
  ADR-0024 §1–2 and ADR-0035 use it as precedent, and ADR-0043 §4's "that cut
  was wrong" retracts *its own* misapplication — `concat` is not higher-order,
  so ADR-0017 §7's rule never reached it. The rule stands unchanged.
- **ADR-0045 narrows #11** (integer parameters are not constraint-bounded). The
  target is an issue, so no stamp is possible; recorded in
  [`../../adr/SOURCES.md`](../../adr/SOURCES.md) and transcribed at spec ch1
  §10.8, restated here at §5.7.
- **ADR-0017 §3 and §6 extend #11.** Both are chapter 2 clauses — §6.5 and §7.5.
- **#9 versus ADR-0045 §1/§8 on `persist` without an initialiser** was a known
  live contradiction, deferred to chapter 5 by spec ch1 §14.5 and left there
  (§9.6). **Chapter 5 §4.3 resolved it for #9**, and repaired §9.6, §17.6 and
  §21 here in the same commit ([ch5 coverage §13](05-runner.md)).

---

## 1. Sources this chapter covers

Per issue [#86](https://github.com/ludo-lang/ludo/issues/86): #9, #10, #11,
ADR-0043 and ADR-0017.

### #9 — Decide the fate of nil

| Source clause | Landed |
|---|---|
| `?T` is a distinct type; every other type is non-nullable | §9.1, §2.2 |
| No null of any spelling | §9.1, §18 (spelled at ch1 §3.7) |
| Zero values rejected | §9.3 |
| Keeping `nil` rejected | §9.1, §18 |
| `?T` is sugar over a user-expressible sum type; sum types mandatory | §9.2, §4.1 |
| Absence ≠ failure; two types, never one `Result` | §9.4, §10.13 |
| No auto-flattening: `?(?T)` does not collapse | §9.5, §9.11 |
| Every binding initialises at its declaration | §9.6 |
| Struct literals name every field (no implicit zero) | §9.6, reconciled with #15 Q21's declared defaults |
| Uninitialised memory in `unsafe` only | Routed to chapter 3 (memory) — the safe-layer half is §9.6 |
| A missing key reads as `?V`; no panicking indexing variant; no by-layer difference | §9.7 |
| A stale handle *is* an optional | §9.7, §8.4 |
| `has(k) -> bool` on containers only | §9.7 (signature routed to chapter 3) |
| `or` is unwrap-or | §9.8, §16.3 |
| `?.` short-circuits, yields `?C`, works for UFCS calls | §9.8, §13.6 |
| `:=`-in-condition is its own production; the check is the bind; not in `for` | §9.8 (spelled at ch1 §6.11) |
| No `guard` keyword; the statement modifier covers it; binds into the enclosing scope | §9.8 (spelled at ch1 §6.13) |
| The modified statement must diverge when the left side binds | §9.8 (spelled at ch1 §6.14) |
| `is_some()` / `is_nil()` deliberately absent | §9.9 |
| Conditions are `bool` only; no truthiness | §16.4, §2.2 |
| The three lenses | Rationale. **Explicitly dropped** — §3 |
| Hands `#11` the sum-type mandate, `#10` the clean boundary, `#24` the productions | §4.1, §10.1; the #24 half is ch1 §13 |

### #10 — Choose the error model

| Source clause | Landed |
|---|---|
| Errors are values in the return type | §10.1 |
| The bug/failure line is drawn by the language, not convention | §8.1, §8.2 |
| The bug list, and integer overflow always checked | §8.3 |
| Float arithmetic exempt: infinity and NaN are values | §8.3, §2.5 |
| A stale handle is not a bug; only an `unsafe` deref of a dead handle is | §8.4 |
| A bug cannot be caught in the safe layer | §8.2 |
| `$.assert` / `$.panic` in the stdlib root, always on | §8.5 |
| A sum type with sugar, parallel to `?T`; never multiple return values | §10.1, §10.2 |
| Never collapsed into `?T` | §9.4, §10.13 |
| Error sets nominal, closed, per-function | §10.3 |
| Inferred error sets rejected | §10.4 |
| A universal `Error` type rejected | §10.5 |
| Type spelling `File rescue IOError`; infix `!` and `or` rejected | Spelled at ch1 §9.7; the type's meaning is §10.1 |
| Elision: `File rescue` = the file's one declared set; two candidates is a compile error | §10.6 |
| Nothing inferred across files | §10.6, §11.4 |
| `rescue` is the handler word, purely local, no stack | §10.8 |
| The three tails: propagate, default, bind the error | §10.8 |
| `match` remains the general form | §10.10 |
| must-use makes an unbound fallible value a compile error; `_ = f()` discards | §10.11 |
| Propagation explicit at every call site | §10.12 |
| `or` handles absence, `rescue` handles failure | §10.13 |
| Rejected spellings: postfix `?`, `try`, `raise`/`rescue`-as-exceptions, `catch`/`throw` | Spelled at ch1 §7.16; the no-unwind rule is §10.8, §18 |
| `rescue` is legal only where a fallible value is produced | §10.9 |
| `ensure` rejected; `defer` kept; no `errdefer`; no destructors | §10.18 |
| The allocator owns the OOM policy | §10.17 |
| `?File rescue IOError` composes, no flattening; the blessed stdlib never returns one | §10.14 |
| No ignoring layer; must-use is universal | §10.15 |
| `#explicit` accepts `rescue` | §10.16 |
| Boundary with #18: a bug is one mechanism, no catch, no recover | §8.6 |
| The notation test, and the three lenses | Rationale. **Explicitly dropped** — §3 |
| **How a failure is *produced*** | **Phantom — authored at §10.6.1, §19.5** |

### #11 — Set the depth of the type system (semantics half)

Chapter 1 transcribed #11's **spellings** and routed its semantics here. This
table is the other half; the rows chapter 1 marked "routed to chapter 2" all
land below.

| Source clause | Landed |
|---|---|
| Generics present, monomorphised, constraint-bounded, explicit type parameters | §5.1 |
| A generic body is checked once against its constraints, never per instantiation | §5.2 |
| Checking a call needs only the signature; monomorphisation is codegen | §5.3 |
| Unconstrained templates rejected | §5.5 |
| Boxed/uniform representation rejected | §5.6 |
| No higher-kinded types, no specialisation, no variance (nominal invariance) | §1.6 |
| Explicit nominal interface satisfaction | §6.1, §6.2 |
| Structural satisfaction rejected as *accidental* | §6.2 |
| Static dispatch only; no `dyn`, no boxed existential | §6.3, §6.4 |
| Heterogeneity is what sum types are for | §6.3 |
| Sum types mandatory; the pattern language | §4.1, §4.2 (spelled at ch1 §8.2) |
| No guards, and why exhaustiveness needs it | §4.3, §4.4, §4.5 |
| Or-patterns and ranges out | Spelled at ch1 §8.4 — no semantics to add |
| No inheritance, no embedding; composition is a named field | §1.5 |
| Mutability is a property of the place; `!T` and `T` are the same type | §12.1–§12.3 |
| Immutable is the default | §12.2 |
| Signatures fully annotated; bodies and struct literals inferred | §11.1, §11.3 |
| Return types never inferred | §11.2 |
| Numeric literals default; the ambiguous case is a named compile error | §11.5 (spelled at ch1 §3.2) |
| Global/whole-program inference vetoed; inference failure is an error, not a widening | §11.4, §11.5 |
| `distinct` — opaque, keeps equality and comparison, loses arithmetic | §3.1 |
| `distinct numeric` — `T+T→T`, `T*scalar→T`, no `T*T` | §3.2 |
| Full dimensional algebra rejected; conversions are named functions | §3.4 |
| The blessed quantity type set is a stdlib call | §3.5 |
| UFCS resolves in the module declaring the receiver's type, and nowhere else | §13.1 |
| The importing file's import set never participates | §13.2, §13.3 |
| You cannot UFCS-extend a third-party type | §13.4 |
| Recursive data via handles or containers; no `Box[T]` | §14.2, §14.3 |
| Direct self-inclusion by value is a compile error naming the cycle | §14.1 |
| No type feature is layer-gated | §1.1 |
| Beginners see annotations in signatures | §11.1 |
| No aliasing rules, no borrow checker, no lifetimes | §1.7 |
| Nominal newtypes only, no transparent type synonyms | §1.3, §1.4 |
| No overloading, no implicit-conversion-driven dispatch | §1.8, §2.8 |
| The minimum-type-system table (feature → failure class) | Rationale. **Explicitly dropped** — §3 |
| The three lenses, and the downstream list | Rationale. **Explicitly dropped** — §3 |
| **The interface declaration and satisfaction spellings** | **Phantom — filed, then authored at §6.7** ([#100](https://github.com/ludo-lang/ludo/issues/100) resolved; §6.2.1, §19.3 record the route; chapter 1 §13.9.1 crossing 2 carries the charge) |

### ADR-0017 — The ceiling is a named function pointer

| Source clause | Landed |
|---|---|
| §1 Nothing above a non-capturing function pointer; closures, bound receivers and capture are non-goals | §7.1, §7.2 |
| §1 Capture is supplied by an interface-bounded generic over a struct of named fields | §7.4 |
| §1 The four levels weighed, (a)–(d), and the corroboration from Odin/Zig/Jai | Rationale. **Explicitly dropped** — §3 |
| §2 The type is spelled `fn(T, U) -> R`, one type-sublanguage production | Spelled at ch1 §9.6; §7.1 |
| §3 A `fn` type satisfies a single-function interface, signature matching exactly | §6.5 |
| §3 Not structural satisfaction returning; the fifth privileged-compiler-knowledge use | §6.5, §6.6 |
| §4 Every function top-level and named; no anonymous function literals | §7.3 (spelled at ch1 §5.7, §6.1) |
| §5 Generic functions become values only when explicitly instantiated; no inference from the expected type | §5.8 |
| §6 UFCS never applies to a function value | §7.5, §13.5 |
| §7 The stdlib ships no `map`, `filter` or `reduce`; a higher-order stdlib function must contribute an algorithm | §7.6 |
| §8 A third-party combinator library is possible, unblocked and unpleasant | Rationale, and its two causes are transcribed elsewhere (§13.4, §7.2). **Explicitly dropped** — §3 |
| §9 The lenses, and the named beginner/agent conflict | Rationale. **Explicitly dropped** — §3 |
| §10 #24 delta | Chapter 1 §13.6 (the production) and chapter 8 (the total) |

### ADR-0043 — A string is a literal, and constructed text is bytes

| Source clause | Landed |
|---|---|
| §1 The #17/#15 contradiction, and the two rejected resolutions | Rationale. **Explicitly dropped** — §3 |
| §1 `string` is a view into the runner's constant blob and nothing else | §17.1 |
| §1 Its escape permission is derived, not granted | §17.3 |
| §2 One-way implicit widening to `[]u8` at parameter position | §17.2, §2.8 |
| §2 No runtime bytes can become a `string` | §17.1 |
| §2 No runtime-constructed `string` container key; no player text in a `string` `persist` | §17.4 |
| §3 The spelling is `string`, not `str`; #17 §4 stamped | §2.1, §17.1 |
| §4 `format(dst: ![]u8, "...", ...) -> []u8` | §17.5 |
| §4 Not a macro: a compiler-known signature, the format string a checked literal | §17.5 |
| §4 The hole grammar is `{}` and nothing else | §17.5 |
| §4 Hole types are a closed spec-owned list; the math types excluded | §17.5 |
| §4 Overflow truncates at a scalar boundary and returns what was written | §17.5 |
| §4 Slicing is free, inherited from #15 through the widening | Routed to chapter 3 (`[]T` slicing is #15's) |
| §4 The rejected `Scratch`-destination form | Rationale. **Explicitly dropped** — §3 |
| §5 `TextBuf[N: usize]`: the struct, the four operations, a plain value that lives in `persist` | §17.6 |
| §5 `append` returns nothing and truncates silently | §17.6 |
| §5 `format` kept as the primitive | §17.5, §17.6 |
| §5 The buffer-and-cursor idiom rejected, and the worked text-entry program | Rationale. **Explicitly dropped** — §3 |
| §5 The conditional on integer generic parameters | Discharged by ADR-0045; §17.6 |
| §6 `$.input.text()` returns `[]u8` with a frame lifetime | §17.11 (facade routed to chapter 6) |
| §7 A literal carries a hidden NUL; nothing else does; the guarantee attaches to the blob | §17.9 (spelled at ch1 §3.6) |
| §8 `chars()` survives, `bytes()` deleted; both defined on `[]u8` | §17.7 |
| §8 `[]u8` is not validated UTF-8; an invalid sequence yields U+FFFD, never a fault | §17.8 |
| §9 The narrow view-return clause for a string-construction call | §17.10 |
| §9 The general re-cut is split out | §17.10, §20 → [#83](https://github.com/ludo-lang/ludo/issues/83) |
| §10 The two split-out tickets | Both closed: #83 is chapter 3's, #84 became ADR-0045 |
| §10 The unspecified `!`-on-a-sliced-place spelling | Routed to chapter 3 (#15's marker grammar); `TextBuf` removes the need |
| §11 The stamp list | Transcription bookkeeping, discharged in the ADRs. **Explicitly dropped** — §3 |
| §12 The #24 delta, the #19 non-property, the three lenses | Chapter 8 (the count) and rationale. **Explicitly dropped** — §3 |

---

## 2. Sources this chapter draws on without covering

These are not #86's list. Each is owned by another chapter, and this chapter
takes only what it needed to state a typing rule. Listed so that chapter's
coverage file does not find its source already half-spent without a record.

| Source | Taken | Clause |
|---|---|---|
| #5 | Criterion 2 (failure in the signature), criterion 3 (one spelling per entity), criterion 4 (no behaviour change without a signature change), criterion 5 (the diagnosability veto) | §10.1, §10.4, §11.4, §13.3 |
| #6 | Layering varies spelling, never semantics | §1.1, §10.15 |
| #8 | Mutation is transient and non-escaping; must-use; the checked-overflow ban on build-mode switches | §8.3, §10.11, §12.4 |
| #15 | Field defaults (Q21); the derived `Eq`/`Hash`/`Clone` as precedent | §9.6, §6.6 |
| #16 | UFCS is pervasive, which is why §7.5's boundary is stated | §7.5, §13 |
| #18 | A bug's runtime semantics are chapter 5's | §8.6 |
| #22 | Type checking never needs the whole program; the oracle never monomorphises | §1.2, §5.3, §4.6 |
| #24 | The payment rule, which is why §19.3 declines to author | §19.3 |
| #26 | `persist` and the entry file | §9.6 (routed to chapter 5, which decided it at ch5 §4.3.1) |
| #29 | The C callback restriction ADR-0017 §4 generalises | §7.3 |
| #82, #83 | The container set and the view-return rule are chapter 3 gaps | §9.7, §17.10 |
| ADR-0008 §6 | Recognise-by-name, as one of the privileged-knowledge uses | §6.6 |
| ADR-0011 §294 | `$.input.text() -> string`, corrected by ADR-0043 §6 | §17.11 |
| ADR-0013, ADR-0042 | The allocator interface owns the OOM policy | §10.17 |
| ADR-0016 §3 | Arithmetic operators on the blessed math types and nowhere else | §16.1 — **and see §3 below** |
| ADR-0016 §6 | Float evaluation exactly as written | §2.5 — the citation read **§7** (*no blessed batch functions*) and was repaired here; the rule is §6's third mandate |
| ADR-0018 | A compile error is reported under chapter 7's envelope | §0.2 |
| ADR-0021 | The const-eval floor, routed here by ch1 §5.6 | §15 — **and see §3 below** |
| #25 (**not** ADR-0025) | Sum-type layout is chapter 3's | §4.8 — the citation was repaired by chapter 3, [`03-memory.md`](03-memory.md) §0 |
| ADR-0029 | The closed list of four, which does not contain `rescue` | §10.16 |
| ADR-0044 §6 | A hole is repaired in the spec text; an ADR only for a reversal | §19 |
| ADR-0045 | The value parameter's semantics, and the narrowing of #11 | §5.7 |

---

## 3. Sources located but not transcribed, with the reason

Required by ADR-0044 §5: a clause is transcribed **or explicitly dropped**, and
the drop is recorded.

1. **Every "three lenses" section, and every rejected-alternative argument.**
   #9, #10, #11, ADR-0017 §9 and ADR-0043 §12 each close with a
   simplicity/robustness/agent-friendliness assessment, and each argues its
   rejections at length. None states a rule an implementation could satisfy or
   violate. **Explicitly dropped** under ADR-0044 §5's *the ADRs are the
   rationale record*, with one exception: where a rejection **is** the rule (no
   `errdefer`, no `dyn`, no guards), the prohibition is transcribed and §18
   collects them.

2. **#11's minimum-type-system table.** Feature → failure class caught. It is
   the argument for the feature set, not a clause about any of them; every
   feature in it is transcribed on its own row above. **Explicitly dropped.**

3. **ADR-0017 §8 — the third-party combinator library.** Recorded in the ADR
   as the honest reason §7 declines `map`/`filter`. Its two mechanical causes —
   no UFCS extension of foreign types, no closures — are transcribed at §13.4
   and §7.2, so nothing normative is lost. **Explicitly dropped.**

4. **ADR-0043 §11's stamp list.** An instruction to amend five other records,
   discharged in those records. Not a rule about the language. **Explicitly
   dropped**, on the same ground chapter 1 dropped ADR-0029 §7's `CONTEXT.md`
   correction.

5. **ADR-0043 §5's worked text-entry program and §4's rejected `Scratch`
   form.** Illustration and rationale; the rules they illustrate are at §17.5
   and §17.6. **Explicitly dropped.**

6. **ADR-0021 — a routing conflict, recorded rather than resolved.**
   Spec ch1 §5.6 and §9.5 route the const-eval floor to *chapter 2*; ADR-0044
   §5's chapter assignment and [#88](https://github.com/ludo-lang/ludo/issues/88)
   put ADR-0021 in *chapter 4*. Chapter 2 transcribes only what a type question
   needs — the floor, and that a non-foldable const position is a compile error
   (§15.1–§15.3) — and §15.4 names the conflict so chapter 4 finds it. **Not a
   phantom clause**: the source exists and is unambiguous; only its home is.
   Chapter 4 should transcribe the remainder and may delete §15 if it prefers to
   carry the floor itself, in which case ch1 §5.6's routing is the defect.

7. **ADR-0016 — a source that was assigned to no chapter, now chapter 6's.**
   §16.1 transcribes ADR-0016 §3's operator rule, because the typing of `+`
   cannot be stated without saying which types have it, and §2.5 takes §6's
   float-evaluation rule (miscited as §7 until
   [#99](https://github.com/ludo-lang/ludo/issues/99)). The rest was transcribed
   nowhere and closed no chapter: the eight chapter tickets (#85–#92) named
   every ADR in the corpus except this one. That was never a drop — it was a
   **gap in the consolidation itself**, recorded here because chapter 2 is where
   it was found.

   **#99 closed it.** The cause was that `$.vec2` is a `$.`-rooted stdlib type
   and *not* a facade, so ADR-0044 §5's eight chapters had no shelf for it.
   **Chapter 6 is rescoped from *the facades* to *the standard library*** — its
   boundary is now *every name under `$.`* rather than an enumeration of five
   facades — and takes ADR-0016 §1, §2, §4, §5 and §7, plus §6's alignment
   mandate. **Chapter 8 takes §6's two conformance halves** (SIMD lowering is
   not required; float evaluation binds every implementation, which §2.5 states
   for the language and chapter 8 states as the obligation). Eight chapters
   still, so ADR-0044 §5 is narrowed in place and not reversed — no ADR
   (ADR-0044 §6).

8. **Cast conversion rules — located as a silence, now closed.** ch1 §7.13
   spells `x as T` and ADR-0021 §1 puts casts in the const-eval floor, so the
   form existed and was const-evaluable; **no source said which conversions are
   legal or what a lossy one does.** It was a design decision rather than a
   transcription defect, so ADR-0044 §6's repair rule did not reach it and it
   was filed as [#98](https://github.com/ludo-lang/ludo/issues/98). **Resolved
   and authored into this chapter as §2.9–§2.9.9**, no ADR: the decision
   reverses nothing, and §2.9 is its only normative home. `as` is **total and
   never faults**; one bit-width rule covers every integer↔integer row; float→int
   truncates toward zero, saturates out of range and maps NaN to `0`; int→float
   and `f64 as f32` round to nearest, ties to even; `distinct` crosses in both
   directions; `char as u32` is legal and `u32 as char` is not, replaced by
   `char.from_u32(x) -> ?char`; `bool` casts do not exist. §2.9.8 defines
   **lossy** as non-injective at the given width, which is what ch4 §12.9's
   build failure quantifies over. #24 delta: **zero** — no keyword, no operator,
   no new stdlib root name.

---

## 4. Phantom clauses

A **phantom clause** is a source clause cited by the corpus that was never
authored anywhere. Under ADR-0044 §6 the repair goes in the spec text, and an
ADR is written only for a reversal. None of the entries below is a reversal, so
none takes an ADR.

**Five were found.** Four are authored in `../02-types.md` §19; the fifth is
declined, and why is the interesting part.

| # | Phantom | Cited by | Repair |
|---|---|---|---|
| 8 | The primitive scalar type set | ch1 §2.6 names "the sized integer and float names" and enumerates none; eight ADRs use them | Authored, §2.1 / §19.1 |
| 9 | `char` | ADR-0043 §4 (a `format` hole type) and §8 (`chars()`), defined by neither | Authored, §2.6 / §19.2 |
| 10 | Variant construction in expression position | #9, #10, #11 all require sum types; ch1 §8.5 spells the bare name **in a pattern only** | Authored, §4.6.1 / §19.4 |
| 11 | How a `?T` or a fallible value is *produced* | #9 and #10 spell handling and never production | Authored, §9.11, §10.6.1 / §19.5 |
| 12 | The interface declaration and its satisfaction | #11 Q2 ships explicit nominal satisfaction; ADR-0017 §3 depends on a single-function interface; `grammar.ebnf` has neither form | **Declined**, §6.2.1 / §19.3 → [#100](https://github.com/ludo-lang/ludo/issues/100) |

The numbering continues chapter 1's seven.

**Why #12 is declined rather than repaired.** ADR-0044 §6's rule is that a hole
is repaired in the spec text because a transcription defect does not clear
ADR-0020's bar. Phantoms 8–11 fit: each costs a name or a checker rule and
**zero** against the ch1 §13.6 grammar count. #12 does not — an interface body
and a satisfaction form add type-sublanguage productions and probably a keyword,
which ch1 §13.8's payment rule charges and §13.6, §13.7 and §13.12 publish. A
repair that moves the published budget is a decision, and it is filed as one.
This is the first time the consolidation has hit that line, and the line is
stated here so the next chapter reads it the same way.

**One further finding that is not a phantom:** §9.6 reconciles #9's *every
struct literal names every field* with #15 Q21's *field defaults are permitted*
(ch1 §5.3). They do not conflict — #9 prohibits an **implicit zero**, not an
author's written default — and the reconciliation is recorded because a reader
of #9 alone would find the chapter contradicting it.
