# Coverage: chapter 4 — modules and libraries

**Non-normative.** This file is the closing test for
[`../04-modules.md`](../04-modules.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §6 below says which
happened.

---

## 0. Source check (#72, #73)

Run over **this chapter's sources only** — ADR-0014, ADR-0027, ADR-0029,
ADR-0021 and #26 — before transcription, per `docs/agents/domain.md`.

`tools/check-stamps.py` reports zero findings over the whole corpus both before
and after this chapter, which is the floor working as documented: it sees an ADR
that *declares* an amendment, never one that makes a change silently. All four
findings below are of the silent kind and were found by reading every ADR that
cites these four.

### 0.1 Direction 1 — stamps owed and missing

Applied retroactively in this chapter's commit; stamps are additive metadata,
not prose.

| Record stamped | What moved it, and what moved |
|---|---|
| **ADR-0014** | **ADR-0027**, whole. §1 turns §7's *the compiler is given a mapping* into a normative obligation; §2 gives §4 its clarifying line about `$.`; §1 and §4 make §6 checkable and force flatness; §3 adds the `libs/` scan. ADR-0027 opens by naming ADR-0014 §12 and stamped nothing. |
| **ADR-0014** | **ADR-0019 §1**, narrowing §9: the nominal interface is *necessary and insufficient*, so §9's "ADR-0006 R4 becomes a type error" is no longer the end of the obligation. |
| **ADR-0024** | **ADR-0029 §7**, correcting §6's "the one shipped attribute" to "the one shipped **type-level** attribute". The correction was written **inline in ADR-0024 §6** and never stamped in the header, so it was invisible to a reader scanning stamps and to the checker. |
| **ADR-0021** | **ADR-0050 §13/§14**, a **recount**: ADR-0050's own stamp list names "ADR-0021 §1" and no stamp was applied. The budget clause ADR-0021 charges `const` against is §4 rather than §1; the stamp records the recount and says so. No clause of ADR-0021 changes. |

### 0.2 Direction 2 — stamps **not** owed, recorded so they are not re-derived

- **ADR-0015** cites ADR-0014 three times — §12 on locations never named in
  source, ADR-0014's library-location refusal as precedent for rejecting a load
  call, and the asset-path boundary. All three are precedent or derivation, and
  none moves a clause. **One section mis-citation**: ADR-0015 attributes the
  no-crossing-a-library-boundary rule to *ADR-0014 §9* (which is about
  backends); the rule is **§7**. ADR prose is immutable, this is a wrong section
  rather than a wrong decision, and chapter 4 §3.6 cites §3.2 and ADR-0015
  together so the chapter carries the correct pointer.
- **ADR-0018 §6** names ADR-0014's duplicate-claim error as its canonical
  two-place diagnostic. Precedent; transcribed at ch4 §6.5.
- **ADR-0024 §5** consolidates the quiescence predicate that ADR-0014 §9's
  backend re-point relies on. It defines a term ADR-0014 used and never defined;
  no clause of ADR-0014 moves. Transcribed at ch4 §10.8.
- **ADR-0025 §2**, **ADR-0026 §4** and **ADR-0032** cite ADR-0014 for *there is
  no manifest*, *serialization is a library's job*, and *a library cannot learn
  the canvas size*. Precedent in all three.
- **ADR-0032 §6**, **ADR-0043** and **ADR-0045** cite ADR-0021 for the
  const-eval floor, the macro and reflection non-goals, and §5's
  compile-error rule "applied unchanged". Precedent and reuse; no narrowing.
  ADR-0045 §1 does **narrow #11**, which `SOURCES.md` already carries, and
  nothing of ADR-0021.
- **ADR-0029** already carries its stamp from **ADR-0033** (§4's
  `set_canvas({...})` example is stale under ADR-0032 §5; the rule and the
  closed list are unaffected). Correct and complete.
- **ADR-0045 §2** reads ADR-0029's rule at the type level as an argument for
  annotating a generic value parameter. Precedent.
- **ADR-0027** is cited by nothing that changes it.

### 0.3 Issue sources against `SOURCES.md` (#73)

This chapter's only issue source is **#26**, whose §1 row read *authoritative,
with stale prose*. **The row was incomplete**, and it is amended in this
chapter's commit: **ADR-0021 §3 narrows call 2.** #26's "imported modules do
not run top-level code" reads as though a module may hold nothing at its top
level; ADR-0021 §3 calls that "one claim too strong" and permits **constant
declarations**, on the ground that folding is not execution. An amendment whose
target is an issue cannot be stamped, so `SOURCES.md` was the only place this
could be recorded and it was not there. The row also now names #17's
`persist`-is-entry-file-only amendment, which lives in #26's comments.

Chapter 4 transcribes the narrowed pair, not the original: §13.1 (no execution)
and §13.3 (declaration is not execution) together.

---

## 1. ADR-0014 — a library is a directory that claims a name

| Source clause | Landed |
|---|---|
| §1 A file is a module, a directory is a namespace node, a library is a directory | §1.1 |
| §1 One mechanism: no package concept, no second unit, no manifest | §1.2 |
| §1 Rejected: a declared `library` unit; rejected: modules-plus-packages | Rationale. **Explicitly dropped**, see §7 |
| §2 The dependency declaration is in the source; a program names a library in the file that uses it | §1.4 |
| §2 A single-file program has no `use` line and is unaffected | §1.3 |
| §2 The named cost: resolution policy is the build system's | §14.1 |
| §3 The library declares its own name, once, at its root | §2.1, §2.2 |
| §3 The `use` string is a lookup key; the bound identifier is the library's own | §2.3 |
| §3 No rename, alias or re-export; two claims of one root is a hard error with no escape hatch | §2.4, §6.2 |
| §3 The accepted cost: two libraries claiming `math` cannot coexist | §6.3 |
| §3 Rejected: deriving the binding from the directory name | §2.5 |
| §3 Rejected: fully-qualified origin names | Rationale. **Explicitly dropped**, see §7 |
| §3 The declaration is what makes a directory a library; it is source, not metadata | §2.1 |
| §4 `$.` is not compiler-internal; ordinary module set, one resolution path | §9.1 |
| §4 Privileged in exactly two ways: reserved root, in scope with no `use` | §9.2 |
| §4 Rejected: a structurally-known stdlib (three grounds) | §9.4, §9.5 carry the two that bind an implementation; the third (agents treat the namespaces as one) is evidence. **Partly dropped**, see §7 |
| §4 The named cost: the bootstrap must resolve modules before compiling `$.` | Bootstrap sequencing, not a language rule. **Explicitly dropped**, see §7 |
| §5 Visibility is a declaration-site marker; public or private, marked by the author | §5.1 |
| §5 Rejected: no privacy at all; rejected: directory-position privacy | §5.3 states the rule the second rejection produces; the first is rationale. **Partly dropped**, see §7 |
| §5 Re-export is banned | §5.4 |
| §5 This is a new keyword | Named `pub` by **chapter 1 §14.1**; ch4 §5.2 cites it |
| §6 Within one program, a root name denotes exactly one library | §6.1 |
| §6 It is a language-level invariant even though satisfying it is build-system work | §6.1, §14.2 obligation 2 |
| §6 The nominal-identity ground (#11) | §6.4 |
| §6 The named cost: no in-program version pluralism, ever | §6.3 |
| §7 A path is your code, a name is a library | §3.1 |
| §7 A path reference may not cross a library boundary, in either direction | §3.2 |
| §7 Location is not load-bearing; cache, `vendor/`, anywhere | §3.3 |
| §7 The compiler does not search; it is given a mapping | §7.1, §7.2 (via ADR-0027 §1) |
| §7 Provenance is legible at the reference site, and this is not a #5 criterion-3 risk | Rationale for §3.1. **Explicitly dropped**, see §7 |
| §7 The path form itself | **Phantom clause — authored.** ch4 §3.4, §15.1; see §6 |
| §8 `use` is file-scoped; every file that uses a library says so | §4.1 |
| §8 Non-transitive: a program cannot name a dependency's dependencies | §4.2 |
| §8 The named cost: every file repeats its `use` lines | Consequence of §4.1, no separate rule. **Explicitly dropped**, see §7 |
| §9 A backend is a library claiming a root name; target selection is the module set | §10.2 |
| §9 Two backends claiming one name are never in one program | §10.3 |
| §9 A backend satisfies a declared nominal interface; ADR-0006 R4 becomes a type error | §10.4, narrowed by §10.5 |
| §9 The compile-time seam does not replace ADR-0006 R2's runtime struct | §10.8 |
| §10 A library has no identity beyond its name: no version, no hash, no origin | §2.2 |
| §10 A fork *is* the library and the compiler cannot tell | §2.6 |
| §10 Rejected: carrying version or origin in the declaration | Rationale. **Explicitly dropped**, see §7 |
| §11 Editing a library's files is legal and takes effect | §2.7 |
| §11 The boundary governs name resolution, not file authority | §2.7 |
| §11 Two wrong attempts recorded (location carries provenance; the boundary prevents edits) | Rationale, recorded in the ADR so nobody retries. **Explicitly dropped**, see §7 |
| §11 Marking third-party directories is tooling and convention | §14.3 |
| §12 The four obligations handed to the build system | §14.2, items 1–4 |
| Consequence: three keywords lodged against #24 | Chapter 1 §13; ch4 cites no delta of its own |
| Consequence: the companion count is unaffected | Chapter 8's, per ch1 §13.11 |
| Consequence: ADR-0006 R4 becomes a compiler-checked obligation | §10.4 |
| Consequence: the bootstrap must implement resolution first | **Explicitly dropped**, see §7 |
| Consequence: one root name, one library, one nominal identity | §6.1, §6.4 |
| Consequence: acquisition is future work under §12 | §14.1, §14.2 |
| Consequence: third-party directory legibility is out of scope | §14.3 |

## 2. ADR-0027 — the compiler is handed a mapping and never searches

| Source clause | Landed |
|---|---|
| §1 The mapping is a required input, complete before compilation begins | §7.1 |
| §1 No search: no env var, no directory walk, no traversal, no network, no fallback | §7.2 |
| §1 `use` is a lookup, not a search; an absent key is a compile error | §7.2 |
| §1 The never-guess clause applies to the diagnostic | §7.8 |
| §1 Obligation 2 becomes checkable: two entries for one root name are rejected at supply | §7.4 |
| §1 Obligation 3 becomes structural: a total function over a fixed table | §7.5 |
| §1 The spec fixes no file format, flag, registry, version policy or lockfile | §7.10 |
| §2 `$.` is always present, never a key, not shadowable | §9.3 |
| §2 The conformance ground (a swappable `$.` makes conformance a supply-chain question) | Rationale for §9.3. **Explicitly dropped**, see §7 |
| §3 A runner given no mapping scans `libs/` beside the entry file | §8.1 |
| §3 Each immediate subdirectory is a candidate; the `library` claim is the key; one level, no recursion | §8.2 |
| §3 Two subdirectories claiming one name is a loud error | §8.4 |
| §3 The folder name is irrelevant; a fork resolves as a clean checkout does | §8.3, and §3.3 |
| §3 The directory is named literally, because "a conventional directory" is not testable | §8.5 |
| §3 No ludo source ever contains `libs/`; this is runner behaviour | §8.6 |
| §4 One flat program-wide mapping, not a tree | §7.3 |
| §4 §6 already decided it; nested mappings would permit two instances | §7.3 cites §6.1 |
| §4 `libs/` holds the transitive set, flat | §8.7 |
| §4 Non-transitivity is untouched; resolvable is not visible | §4.3 |
| §5 Values are source directories the compiler parses; no artifact, no interface file | §7.6 |
| §5 Separate compilation is not mandated | §7.7 |
| §6 Supplied replaces scanned, never merges; a merge would be order-dependence | §8.8 |
| §7 The resolvable names are the key set and are queryable through the oracle | §7.9 |
| §8 The three lenses; #24 delta zero on every count | Lens analysis and a zero delta. **Explicitly dropped**, see §7 |
| §9 Acquisition stays out of scope | §14.1 |

## 3. ADR-0029 — `#explicit` marks a module

**Transcribed in full by chapter 1 §11**, which owns the marker's spelling and
took the rule with it; `coverage/01-grammar.md` §1 carries the clause-by-clause
table and it is not duplicated here. #88 assigns ADR-0029 to chapter 4, so this
chapter records the routing (§11.1) and transcribes the three module-system
consequences chapter 1 did not need:

| Source clause | Landed |
|---|---|
| §2 The named cost: the one attribute marking a module rather than a declaration | §11.5 |
| §3 A library author marks their own modules; the statement is about their source | §11.3 |
| §3, §5 A library changing its own layer can never break a consumer's build | §11.4 |
| §1–§8, everything else | **Chapter 1 §11.1–§11.7, §12.1–§12.3, §13.4**; see `coverage/01-grammar.md` |

## 4. ADR-0021 — const evaluation folds an expression

| Source clause | Landed |
|---|---|
| §1 Const evaluation ships, at the floor and no further | §12.1 |
| §1 The evaluable subset: literals, constants, arithmetic, comparison, bitwise, casts | §12.2 |
| §1 No calls, no loops, no recursion, no `if`/`match`, no allocation, no I/O | §12.2 |
| §1 Non-termination is impossible by construction; no step limit, fuel or timeout | §12.3 |
| §1 Rejected: pure-function calls, on cost | Rationale. **Explicitly dropped**, see §7 |
| §2 The computed module constant is ruled out; three replacements, none owed by the language | §12.5 |
| §2 The non-goal list: no macros, comptime blocks, codegen, reflection or CTFE | §12.4 |
| §3 A constant declaration is a compile-time entity, not a load-time side effect | §13.3 |
| §3 A library may declare constants; consumers may use them in constant positions | §13.3 |
| §4 `const` is a keyword; one meaning everywhere | §12.6 |
| §4 Const-ness is not inferred from position; the entry file's bindings keep the plain form | §12.7 |
| §4 Rejected: no keyword; rejected: all top-level bindings fold | Rationale. **Explicitly dropped**, see §7 |
| §4 One keyword against #24, paid as a feature | Chapter 1 §13; recounted by ADR-0050 §13 |
| §5 Division by zero, overflow, bad array length, lossy cast each fail the build | §12.9 |
| §5 A `compile`-producer diagnostic with a location; never a fault, never a wrap | §12.9 |
| §5 The generalising rule: never a value run time would not produce, never one at all when ill-defined | §12.10 |
| The three lenses | Rationale. **Explicitly dropped**, see §7 |

Chapter 2 §15.1–§15.3 restates §12.2–§12.3 for the type-facing reader; ch4 §0.6
records that this chapter owns the source and resolves the routing conflict
`coverage/02-types.md` §3 item 6 left open. Chapter 2 §15.4's pointer is
updated to say so.

## 5. #26 — top-level code, as narrowed

| Source clause | Landed |
|---|---|
| Call 2 Imported modules do not run top-level code; module top level is declarations only | §13.1 |
| Call 2 No load-order-dependent initialisation | §13.2 |
| Call 2 The named cost: no computed module constants | §12.5 |
| Call 2 as narrowed by ADR-0021 §3: constants may be declared | §13.3 |
| #17 amendment `persist` may appear only in the entry file; not in modules, not in function bodies | §13.4 |
| #17 amendment The accepted cost: a library cannot own private cross-frame state | §13.5 |
| #17 amendment The aggregate-under-one-root idiom | Documented idiom, explicitly *not* a language rule in the source. **Explicitly dropped**, see §7 |
| Call 1 Top level executes; a file without an entry is a valid program | **Chapter 5** (ch4 §13.6, ch1 §4.3) |
| Call 3 `persist` is the only mutable cross-frame channel | **Chapter 5** |
| Call 4 `persist x: T = <expr>` initialises at its declaration site, cold start only | **Chapter 5** |
| The zero-parameter frame entry, the fixed 60Hz step, overrun policy, render-rate routing | **Chapter 5** (#47/ADR-0013 settled the parameter question) |

---

## 6. Phantom clauses

**One, and it is authored rather than deleted.**

**The relative-path reference has no spelling.** ADR-0014 §7 is titled *A path
is your code; a name is a library*, states that a program reaches its own files
by relative path, and enforces the boundary between the two forms; ADR-0027 §3
and spec ch1 §14.9 both repeat the rule. **No artifact in the corpus writes the
form.** Chapter 1 §4.5 admits `use "<key>"` as "the one import form" and
defines the binding as *the name the target claims for itself* — which a plain
module, having no `library` line, never does. Read literally, the corpus
mandates a reference form that cannot be written and binds a name that cannot
exist.

**Authored** at ch4 §3.4 and recorded at ch4 §15.1: a key beginning `./` or
`../` is a path, the extension is written, and the binding is the file stem.
The choice between authoring and filing follows the precedent chapters 2 and 3
set with [#100](https://github.com/ludo-lang/ludo/issues/100) and
[#104](https://github.com/ludo-lang/ludo/issues/104), which were **filed**
because each moved a published count: this one moves nothing — no keyword, no
operator, no production, no stdlib root name — so the ground for filing does not
apply, and leaving a multi-file program unwritable is the larger cost.

The one substantive question it answers is where the bound name comes from.
ADR-0014 §3 **rejects** name-from-filesystem for libraries, and §15.1 states why
that rejection does not transfer: the argument there is that whoever checks a
library out can rename its directory, which makes the binding a third party's to
control. A program's own filename is the program author's.

### 6.1 Two further gaps, filed rather than authored

Transcription also found that **a multi-file library has no namespace rule** —
ADR-0014 §1 makes a directory a namespace node and §3 puts the `library` line
"at the library root", and nothing says what `vec_utils.foo` reaches — and that
**`pub` has no stated boundary** below the library edge, because ADR-0014 §5
argues the marker entirely across that edge. Both are **filed as
[#111](https://github.com/ludo-lang/ludo/issues/111)**, not authored.

**A third, found after the chapter shipped, is filed as
[#112](https://github.com/ludo-lang/ludo/issues/112)**: ADR-0014 §9 makes a
backend a library claiming a root name and **never says which name**, nor
whether it is reserved the way `$` is (ADR-0027 §2). ADR-0019 §1's phrase *a
claimant of a spec-defined root* presupposes a definition no artifact supplies.
Not authored, for the #24 reason above: reserving a second root name would be an
exception to ADR-0014's own Consequences claim that the companion count is
otherwise unaffected, and that is a budget decision rather than a spelling.

This is the opposite call from §6's, and the difference is the test the chapter
applies: §6's path form was a **spelling for a rule the corpus already fixed**,
with a zero delta against #24. #111 is a **rule nobody has decided** — the
nested answer would be new grammar and the flat answer would not, so authoring
it would be deciding a budget question by drafting. The reference program is
arranged to survive either answer (`libs/vec-utils/scale.ludo`), and ch4 §5.5
and §16 mark the boundary of what is settled.

#### 6.1.1 #111 is resolved, and the deferral was the right call

The gap is closed and the chapter repaired in place, no ADR, **zero #24 delta**
— the nested spelling that would have moved the count is the answer that was
rejected. Filing rather than authoring was vindicated: the resolution turned on
grounds a transcriber had no standing to settle.

| Rule | Where | Why |
|---|---|---|
| A library's surface is its **root file** | **§3.4.1** | Flat would let *adding a file* enlarge the API — §5.3's prohibition one scope up — and would need a collision tie-break. Nested makes a directory name load-bearing, which §2.5 and §3.3 refuse for the root name and never licensed below it, and costs grammar. The root file makes `<library>.<name>` resolve in **exactly one file**, which is #4's naming finding paid directly. |
| The **root file is the file carrying the `library` line** | **§2.2.1** | No stem match, no conventional filename — both would make a filename authoritative one level below the directory name §2.5 already refuses. |
| **At most one `library` line per directory** | **§2.2.2** | Falls out: two claims make §2.2.1's root file ambiguous and §6.1 unstatable. Derived, not decided. |
| **`pub` gates every module boundary uniformly** | **§5.5** | The library-only reading makes `pub` in a non-root file mark something that can never be API — a keyword that is sometimes decorative, the shape §5's own declaration-site argument exists to avoid. |
| **API is `pub` *and* root-file** | **§5.5.1** | Two independent gates that compose; neither acquires a second sense from the other. |
| **No exemption for a plain program** | **§5.5.2** | An exemption would give *private* one meaning inside a library and another outside it — relocating the two-senses defect rather than fixing it. |

**Nothing fixed was reversed**: §3.2's boundary, §3.4's path form, §5.3's
no-position-derived-visibility, §5.4's no-re-export and §2.5's folder-name
irrelevance all hold, and three of them are what *decided* the answer.
**ADR-0014 §1's *a directory is a namespace node* is read as describing the
tree, not the reachable surface** — the one phrase in the corpus that pulled the
other way, and it is named in §3.4.1 rather than left for a reader to trip over.
The reference program stops hedging: `scale.ludo`'s comment now says `pub` is
load-bearing **and** not what makes `halve` API, and the root file's comment
says the way into the surface is to declare something that calls it — §5.4, not
a forwarding form.

## 7. Explicitly dropped

Everything marked *explicitly dropped* above falls into four kinds, and none is
a hole:

1. **Rejected alternatives and their grounds** — ADR-0014's declared `library`
   unit, origin-qualified names, directory-position privacy, versions in the
   declaration, and its two recorded wrong attempts at provenance; ADR-0021's
   pure-function calls, the no-keyword option and the fold-everything option;
   ADR-0027 §2's conformance argument. A rejected option is rationale by
   construction: there is no clause an implementation could satisfy.
2. **Lens analysis** — ADR-0027 §8 and ADR-0021's closing section. Design
   justification, transcribed nowhere by any chapter.
3. **Named costs that restate a rule** — ADR-0014 §8's "every file repeats its
   `use` lines" is §4.1 seen from the author's chair, not a second obligation.
4. **Sequencing and process facts** — ADR-0014's bootstrap-ordering consequence
   (the bootstrap must resolve modules before compiling `$.`) constrains the
   order in which a compiler is *built*, and binds no program and no conforming
   implementation. ADR-0027 §8's zero #24 delta is a budget fact and lives in
   chapter 1 §13.

**Nothing in this chapter's sources is transcribed nowhere and closes no
chapter.** The only cross-chapter debt is the reverse: #26's calls 1, 3 and 4
and the frame entry are **chapter 5's**, named at ch4 §13.6 so chapter 5
inherits them explicitly.

## 8. Repairs made in this chapter's commit

- **Four retroactive stamps** — ADR-0014 (×2), ADR-0024, ADR-0021. See §0.1.
- **`docs/adr/SOURCES.md`** — #26's row gains ADR-0021 §3's narrowing of call 2
  and #17's `persist` scope. See §0.3.
- **Spec ch2 §15.4** — the routing conflict it recorded is resolved: chapter 4
  owns ADR-0021, chapter 2 keeps the type-facing floor as a restatement.
- **`docs/spec/README.md`** — chapter 4 marked written.

**Made after the chapter shipped**, on a reader's question:

- **Ch4 §10.1 pins the sense of "backend"** to `CONTEXT.md` §Backend — one
  platform's implementation of the platform layer, a **triple** of window and
  input, renderer and audio device — and names the collision ADR-0037's stamp on
  ADR-0002 records, where ADR-0002 and ADR-0022 §1 use the word for the
  **renderer component** alone. #74 found that collision was half the term's
  ambiguity; §10 was transcribed without saying which sense it used, which is
  the same defect one level down. §10.2–§10.8 are the former §10.1–§10.7,
  renumbered.

## 9. What chapter 4 hands forward

- **Chapter 5** inherits #26 calls 1, 3 and 4, the entry file, `persist`
  (including ch1 §14.5's unresolved initialiser question), reload, and the
  diagnostic for a statement in an imported module (ch4 §13.6).
- **Chapter 6** inherits everything under `$.`; ch4 §9 fixes the root's status
  and nothing about its contents.
- **Chapter 7** inherits the diagnostic text for the duplicate-claim error
  (ch4 §6.5), the unresolvable-name error (§7.2, §7.8), and the four
  `#explicit` codes.
- ~~**[#111](https://github.com/ludo-lang/ludo/issues/111)** inherits the
  multi-file library namespace and `pub`'s boundary (§6.1).~~ **Resolved**, and
  it repaired ch4 §2, §3 and §5 in place exactly as predicted — §2.2.1, §2.2.2,
  §3.4.1, §5.5, §5.5.1, §5.5.2, recorded at §15.2. See §6.1.1.
- **Chapter 8** inherits ADR-0019's claimant obligations (ch4 §10.5–§10.7) and
  the runner obligations at ch4 §8.
