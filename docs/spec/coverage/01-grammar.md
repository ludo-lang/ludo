# Coverage: chapter 1 — lexical structure and grammar

**Non-normative.** This file is the closing test for
[`../01-grammar.md`](../01-grammar.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §4 below says which
happened in each case.

---

## 1. Sources this chapter covers

Per issue #85: #7 (candidate B), #24 (the grammar budget and its counting
rules), ADR-0029's `#explicit` spelling, #11's type sublanguage syntax, and
ADR-0045's integer generic parameter.

### #7 — Write the same small game in three candidate syntaxes

| Source clause | Landed |
|---|---|
| Candidate B wins: Lua-flavoured but pragmatic | §1.2, §7.1 (keyword feel, Lua precedence order) |
| Type sublanguage owns `[...]` and nothing else | §9.1, §9.8 |
| `{...}` stays bare — no type named at the point of use | §3.8, §3.9 |
| The mutation mark `!` on the declaration and on the place | §7.10, §9.2 |
| UFCS reads as Lua — `a.f(b)` and `f(a, b)` in one file | §7.12 |
| `$.` works as the reserved root | §2.5 |
| `persist world: World` is a one-line declaration | §5.12 |
| Surface call 1: the colon stays in parameter and field declarations | §5.3, §5.8 (`FieldDecl`, `Param`) |
| Surface call 2: `+= -= *= /= %=`, statement not expression, no `++`/`--` | §6.5 |
| Surface call 3: implicit tail return, only an expression in tail position | §6.7, §6.7.1 |
| `#explicit` rejects both 2 and 3 | §11.3 entries via ADR-0029; §11.4 records `+=` and tail return as **permitted**, which supersedes #7 — see §3 below |
| Grammar budget deliberately not settled here | §13 (settled by #24) |

### #24 — Decide whether the spec binds a grammar budget

| Source clause | Landed |
|---|---|
| Call 1: procedural rule + published measurement, not a cap and not silence | §13.1 |
| Call 2: unit is keywords + operators; production count rides along | §13.2, §13.12 |
| Call 3: two fenced budgets, counted separately | §13.3, §13.6, §9.1.2 |
| Call 3: `#explicit` is in neither count, asserted as checkable | §13.4, §13.5 rule 5 |
| Call 4: target is a comparison against Lua/Odin/Go, within ~30% of Lua | §13.7 — **partially transcribed**, see §3 below |
| Call 5: sugar strictly one-in-one-out; semantics-bearing names a failure class | §13.8 |
| Call 5: the tier line is machine-checkable — anything `#explicit` rejects is sugar | §13.8 entry 3 |
| Call 6: on an implementation it is "no vendor syntax extensions", routed to #19 P9 | §13.10 |
| Call 7: home is a normative spec section beside the machine-readable grammar | This chapter and `../grammar.ebnf`; §13.5 |
| Consequence: labelled break rejected | §6.8 |
| Consequence: stdlib root-name companion count, no target | §13.11 (published by chapter 8) |
| Consequence: exceeding the target needs a recorded overrule on the map | §13.9 |
| No reserve pre-allocated for known spenders | Not transcribed — a rule about how the map spends, with no clause an implementation could satisfy. **Explicitly dropped**, see §3 |

### ADR-0029 — `#explicit` marks a module

| Source clause | Landed |
|---|---|
| §1 `#explicit` is kept; it is the enforced floor, not the mechanism | §11 preamble via §11.3's framing |
| §2 One line at the top of a file; no declaration-, directory- or flag-level form | §11.1, `ExplicitMarker` |
| §3 A layer choice never crosses a library boundary, all three directions | §11.2 |
| §4 The rule: forbids a spelling that omits a name, and nothing else | §11.3 |
| §4 The closed list of four | §11.3 entries 1–4 |
| §4 What is explicitly permitted: descriptor field defaults, `+=`, tail return, `?T` | §11.4 |
| §5 No check needs a whole-program view; all three edges resolve at the call site | §11.5 |
| §6 A violation is an error, four spec-assigned codes, no silencing | §11.6 |
| §6 The claim survives: `#explicit` changes zero semantics | §11.7 |
| §7 Correction: `#align(n)` is the one shipped **type-level** attribute | §12.2, §12.3 |
| §7 Correction: `CONTEXT.md` gains an `#explicit` entry | Not this chapter's file. **Explicitly dropped**, see §3 |
| §8 Zero grammar delta — no new keyword, operator or production | §13.4, §12.1 |

### #11 — Set the depth of the type system (syntax half only)

This chapter transcribes #11's **spellings**. Its semantics — checking,
dispatch, inference, the failure-class table — are chapter 2's, and are listed
here as routed rather than dropped.

| Source clause | Landed |
|---|---|
| Generics present, explicit type parameters, constraint-bounded | §10.1, §10.2 |
| A generic body is checked once against its constraints, never per instantiation | §10.2 |
| Sum types mandatory, with `match` | §5.4, §8.1 |
| Pattern language: variants, bindings, nested, struct destructuring, literals | §8.2 |
| No guards | §8.3 |
| Or-patterns and ranges out | §8.4 |
| Mutability is a property of the place; `!T` and `T` are the same type | §7.10, §9.2 |
| Signatures fully annotated; return types never inferred | §5.8 |
| Numeric literals default; the ambiguous case is a named compile error | §3.2 |
| `distinct` and `distinct numeric` | §5.5 |
| UFCS `a.f(b)` is sugar for `f(a, b)` | §7.12 (spelling); resolution rule routed to chapter 2 |
| Nominal newtypes only, no transparent type synonyms | §9.9 |
| No `dyn`, no boxed existential; static dispatch only | Routed to chapter 2 — no syntax |
| No inheritance, no embedding; composition is a named field | Routed to chapter 2 — no syntax |
| No higher-kinded types, no specialisation, no variance | Routed to chapter 2 — no syntax |
| Recursive data via handles or containers; no `Box[T]` | Routed to chapters 2 and 3 — no syntax |
| No type feature is layer-gated | §11.7 (zero semantics) |
| No aliasing rules, no borrow checker, no lifetimes | Routed to chapter 3 — no syntax |

### ADR-0045 — A generic parameter may be an integer, and only a length

| Source clause | Landed |
|---|---|
| §1 A user-declared generic may take a compile-time integer parameter | §10.3, `ValueParam` |
| §1 restriction 1: `usize` is the only admitted value type | §10.3 |
| §1 restriction 2: only as a bare array length, or forwarded | §10.4, §9.5 |
| §1 restriction 3: not a value in expression position | §10.5 |
| §1 `[W*H]Tile` illegal; `[H][W]Tile` is the replacement | §10.4 (the rule; the worked example stays in the ADR) |
| §2 The delta is one thing: a parameter slot may hold a value | §10.3, §13.6 (+1 type-sublanguage production) |
| §2 The const-expression at the use site is ADR-0021's floor, no calls | §9.5, §5.6 |
| §3 Annotated at the declaration, positional at the use site | §10.6 |
| §3 ADR-0043 §5 narrowed: the declaration is `TextBuf[N: usize]` | §10.6; spelling normalised in §14.10 |
| §4 Inferred at a generic function's call site from the argument's type | §10.7 |
| §4 The body is checked once with `N` opaque; the oracle never monomorphises | §10.2, §10.5 |
| §4 Integer parameters are not constraint-bounded — a stated exception | §10.8 |
| §5 +1 type-sublanguage production, zero keywords, zero operators | §13.6 |
| §6 What is refused (other value types, arithmetic, reading, blessing `TextBuf`) | §10.3, §10.4, §10.5 |
| §7 A bad argument is a compile error; `N = 0` is legal with no special case | §10.9 |
| §8 The reference-program line `persist name: TextBuf[32]` | Enters with chapter 3 per ADR-0045 §8; the initialiser tension is flagged at §5.13 and §14.5 |

---

### #101 — Where the mutation mark sits on a sub-view

Resolved after this chapter closed. It amends §7.10 rather than adding a source:
`grammar.ebnf` is untouched, since both spellings already parse as `Suffix*`
chains and the question was which one denotes a writable sub-view.

| Source clause | Landed |
|---|---|
| The mark rides the root binding; the suffix chain follows | §7.10 (restated) |
| The suffix need not be the mutating operation (`w!.frame += 1`) | §7.10 |
| A writable sub-view is `name![a..<b]` | §7.10a |
| `name[a..<b]!` parses but is an error, with the fix named | §7.10a |
| `w.rocks!` was an unsourced exhibit and is removed | §7.10 (see §4 below) |

---

## 2. Sources this chapter draws on without covering

These are not #85's list. Each is owned by a later chapter, and this chapter
takes **only the spelling** from it. Listed so a later chapter's coverage file
does not find its source already half-spent without a record.

| Source | Spelling taken | Clause |
|---|---|---|
| #3 | Types quarantined in their own sublanguage | §9.1 |
| #8 | `defer`, `unsafe`, `!` as the transient mutation mark | §6.9, §6.15, §7.10 |
| #9 | `?T`, `?.`, `or`-unwrap, `:=`-in-condition, statement modifier, no `nil`, bool-only conditions | §3.7, §6.11–§6.14, §7.6, §7.11, §9.3 |
| #10 | `rescue` and its three tails, `T rescue E`, no postfix `?`, no `errdefer` | §6.9, §7.14–§7.17, §9.7 |
| #12, #26 | `frame` as a marked declaration; top level runs in the entry file | §4.3, §5.11 |
| #15 | `{...}` and its item forms, `[N]T`, `[]T`, `..<`, 0-indexing, multi-return, `_` | §3.8, §6.3, §6.6, §7.7, §7.8, §9.4, §5.10 |
| #22 | Parseable without semantic information; a machine-readable grammar ships | `grammar.ebnf` preamble, §8.5, §0.3 |
| #52 / ADR-0017 | Every function top-level and named; `fn(T) -> R` | §5.7, §6.1, §9.6 |
| ADR-0008 | `#vertex`, `#fragment`, the attribute mechanism, extern shader | §12.1, §12.2, §5.16 |
| ADR-0013, ADR-0042 | The entry's parameters `screen: !Target, scratch: !Scratch` | §5.11 |
| ADR-0014 | `library <name>`, `use "<key>"`, visibility as a declaration-site marker | §4.4, §4.5, §5.1 |
| ADR-0015, ADR-0026 | `extern image … = "…"`, `extern storage <name>` | §5.14 |
| ADR-0016 | Float evaluation exactly as written | §3.3 |
| ADR-0018, ADR-0023 | A spec-owned diagnostic is an error; no silencing | §0.2, §11.6 |
| ADR-0019 / #29 | `extern "SDL3" fn`, which may not rename | §5.14 |
| ADR-0021 | `const` as a keyword; the const-eval floor; casts exist | §5.6, §7.4, §7.13, §9.5 |
| ADR-0027 | The compiler is handed a mapping and never searches | §4.5 (routed to chapter 4) |
| ADR-0043 | `string` is the literal type; the hidden NUL; UTF-8 | §1.1, §3.5, §3.6 |

---

## 3. Sources located but not transcribed, with the reason

Required by ADR-0044 §5: a clause is transcribed **or explicitly dropped**, and
the drop is recorded.

1. **#7's "`#explicit` rejects `+=` and the implicit tail return."**
   Not transcribed, because **ADR-0029 §4 supersedes it**: the closed list of
   four permits both, and §11.4 records them as permitted specifically so this
   is not re-litigated. #7 predates the closed list. Recorded here because a
   reader of #7 alone would find the chapter contradicting it.

2. **#24 call 4's Odin and Go columns.** §13.7 publishes the Lua comparison,
   which is the one the target is stated against. The Odin and Go columns are
   **not yet transcribed**: they require counting two more languages'
   grammars to this file's rules, which is research this chapter did not do and
   would have had to invent numbers to fake. Left as a **stated gap** in the
   table rather than filled with a guess ([#93](https://github.com/adamico/ludo/issues/93)).

2a. **#24 call 4's Lua version — under-specified in the source, resolved here.**
   The call says "Lua (~50)" and names no version, and the version is
   load-bearing: **LuaJIT 2.1 — the host LÖVE2D embeds — counts 49, Lua 5.4
   counts 55**, and the ticket's own figure matches LuaJIT rather than the
   current release. §13.7 **fixes the target against Lua 5.4**, on the ground
   that a comparator must have the capability being compared: LuaJIT lacks
   bitwise operators, which ADR-0021 §1 mandates, and that single absence is 5
   of the ~7 tokens by which ludo would exceed a LuaJIT baseline.

   This is a **spec-text repair of an under-specified citation** (ADR-0044 §6),
   not a reversal: #24's decision — a published count against a ~30% target
   relative to Lua — is unchanged, and ludo is compliant, so no ADR is written
   and §13.9's recorded overrule is not owed. The LuaJIT and LÖVE2D rows are
   published beside 5.4 so the choice is auditable and so the **+44.9%
   recognition distance a LÖVE2D user actually faces** is on the record next to
   the +29.1% the spec claims. The first draft picked 5.4 silently; the defect
   was the silence, not the number.

   **Both figures are as-of-closing and have since moved** — to +49.0% and
   +32.7% by §13.9.1 crossings 1 (`>..`) and 2 (`interface` + `impl`). They are
   left as written because this file records *why the comparator is Lua 5.4*,
   and that argument is unaffected by the numerator; §13.7 and §13.9.1 are the
   live figures and this file never was one.

3. **#24's "no reserve is pre-allocated for the four known spenders."**
   Explicitly dropped. It governs how the map spends its budget, and there is no
   clause an implementation could satisfy or violate. §13.9's recorded-overrule
   rule is the part with normative content and it is transcribed.

4. **ADR-0029 §7's `CONTEXT.md` correction.** Explicitly dropped from the
   chapter: it is an instruction to edit the glossary, not a rule about the
   language. Discharged in the repository rather than in the spec — `CONTEXT.md`
   is the authority on its own contents.

5. **ADR-0033's amendment to ADR-0029.** The stale
   `$.graphics.set_canvas({...})` example is not transcribed because §4 of that
   ADR carries the rule and the example was rationale. The amendment itself
   states "the rule and its closed list are unaffected."

---

## 4. Phantom clauses

A **phantom clause** is a source clause cited by the corpus that was never
authored anywhere. Under ADR-0044 §6 the repair goes in the spec text, and an
ADR is written only for a reversal. None of the entries below is a reversal, so
none takes an ADR.

Seven were found. Each is authored in `../01-grammar.md` §14 with its reasoning,
and listed here so the count is legible:

| # | Phantom | Cited by | Repair |
|---|---|---|---|
| 1 | The visibility keyword | ADR-0014 §5 ("this is a new keyword", never named) | `pub`, §14.1 |
| 2 | The sum-type declaration form | #9, #10, #11 all require sum types; none spells the declaration | `type X = enum … end`, §14.2 |
| 3 | The extern shader declaration's spelling | ADR-0008 §8 fixes its shape, never writes it | `extern shader n = "…"`, §14.3 |
| 4 | `unsafe`'s delimiters | #8 call 6 fixes it as per-site and greppable, never delimits it | `unsafe do … end`, §14.4 |
| 5 | The cast spelling | ADR-0021 §1 puts casts in the const-eval floor | `x as T`, §14.6 |
| 6 | The `match` arm form | #11 Q10 fixes the pattern language, never spells an arm | `<pattern> then … end`, §14.7 |
| 7 | The inequality operator | #7 keeps Lua's look; ADR-0021 needs `~` for bitwise | `!=`, §14.8 |

Three further findings that are **not** phantoms but recorded corrections:

- **§14.5 — `persist` without an initialiser.** A live contradiction between #9
  and ADR-0045 §1/§8, **not resolved here**: it is chapter 5's call, the grammar
  is permissive so either resolution is available, and it is flagged so chapter 5
  finds it rather than inheriting it silently.
- **§14.9 — no import alias.** #26's `use "world" as World` is stale prose
  predating ADR-0014 §3 and §7. The chapter admits the single form.
- **§14.10 — `TextBuf`'s declaration.** ADR-0045's inline brace rendering is
  normalised to the `type … = struct … end` form every other type declaration
  uses. The ADR's decision is unaffected.
- **§7.10 — `w.rocks!` was this chapter's own invention.** Found while resolving
  [#101](https://github.com/ludo-lang/ludo/issues/101). It occurred exactly once
  in the corpus — in §7.10's exhibit list — and no source produces it: every
  sourced exhibit marks the **root binding** (`pool!.add(e)`, `list!.push(e)`,
  `xs!.push(e)`, `l!.push(e)`, `w!.frame += 1`, `format(buf!, …)`,
  `get_pixels(img!)`). It is inconsistent with `w!.frame += 1`, which mutates
  `w.frame` and still marks `w`, and it is ambiguous on sight: §7.12's UFCS makes
  `w.rocks` a possible zero-arg call, so `w.rocks!` reads as a mutating call
  named `rocks`. Removed, not reversed — nothing depended on it. This is a
  chapter-authored defect rather than a phantom, since no source cited it.
- **§12.2 — `#align(n)`'s owner column cited the wrong record.** Found while
  transcribing chapter 3 ([#87](https://github.com/ludo-lang/ludo/issues/87)).
  It read *"ADR-0025 as corrected by ADR-0029 §7"* on two counts of confusion:
  the attribute is shipped by **issue [#25](https://github.com/ludo-lang/ludo/issues/25) §3**
  (ADR-0025 is *Fullscreen is a player preference* and decides nothing about
  layout), and the correction ADR-0029 §7 makes is to **ADR-0024 §6**, which
  §12.3 already cites correctly for the field-level decline. Repaired to
  `#25 §3`. The attribute set is unchanged. Recorded in
  [`03-memory.md`](03-memory.md) §0 with the three other sites of the same
  conflation, and in [`../../adr/SOURCES.md`](../../adr/SOURCES.md)'s #25 row so
  the next author is warned before citing it.
