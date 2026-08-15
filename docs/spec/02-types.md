# Chapter 2 — Types

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where this chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under chapter 7's envelope (ADR-0018).

**0.3** This chapter owns **what a type is and what it means**: the type set,
type identity, satisfaction, dispatch, inference, absence, failure, and the line
between a failure and a bug. [Chapter 1](01-grammar.md) owns the spelling of
every form named here, and [`grammar.ebnf`](grammar.ebnf) remains the authority
on syntax.

**0.4** This chapter does not own **memory**. Where a type's storage, layout,
lifetime or container semantics are in question, chapter 3 owns them, and the
clause here names the routing.

**0.5** Every clause cites its source. A clause with no citation is a defect.

---

## 1. The shape of the type system

**1.1** There is **one type system**. **No type feature is layer-gated**, and a
value's type means the same thing in every module, at every layer, in every
program. (#11 Q12; #6, which fixes layering as varying how much is spelled out
and never semantics.)

**1.2** Type checking is **local**. Checking any expression MUST require only the
call site and the signatures it names — never a whole-program view and never the
body of a function declared elsewhere. (#22 guarantee; #11 Q4, Q11.)

**1.3** Typing is **nominal throughout**. Two types are the same type only if
they are the same declaration. There is no structural type equivalence anywhere
in the language, and no type is equal to another by having the same shape. (#11,
nominal newtypes only; #11 Q2, which rejects structural interface satisfaction.)

**1.4** There is **no transparent type synonym**. `type X = <TypeExpr>`
(chapter 1 §9.9) declares a new nominal type, distinct from the type its body
names. (#11: a synonym is two spellings for one entity, #5 criterion 3.)

**1.5** There is **no inheritance and no embedding**. There is no subclassing,
no `super`, no implicit vtable, and no Go/Odin-style field promotion.
**Composition is a named field, always**: given `e: Entity` with a field
`transform`, the position is `e.transform.pos` and MUST NOT be reachable as
`e.pos`. (#11 Q9, and the fork locked upstream in #16.)

**1.6** There are **no higher-kinded types, no specialisation and no variance**.
Generic types are **nominally invariant**: `Pool[Enemy]` is not a `Pool[Entity]`
under any subtyping rule, because there is no subtyping rule. (#11 Q1.)

**1.7** There are **no aliasing rules, no borrow checker, no lifetimes and no
aliasing analysis** anywhere in the language. (#11, aliasing confirmation; #8,
which makes mutation transient and non-escaping and lasting links generational
handles.) Chapter 3 owns the memory model those clauses rest on.

**1.8** There is **no overloading and no implicit-conversion-driven dispatch**.
A name denotes one function. (#11, #5 Tier 2 criterion 6.) The one exception is
§16.1's operator set on the blessed math types, which is a fixed rule about
compiler-known types and not a mechanism a user type can enter (ADR-0016 §3).

**1.9** There is **no reflection**, and no type is introspectable at compile time
or at run time. (ADR-0021, which makes compile-time execution a stated non-goal
past its floor; ADR-0043 §4, which reasons from the absence of reflection.)

---

## 2. The primitive types

**2.1** The primitive types are **prelude identifiers**, not keywords
(chapter 1 §2.6), and are exactly:

| Group | Names |
|---|---|
| Boolean | `bool` |
| Signed integers | `i8`, `i16`, `i32`, `i64` |
| Unsigned integers | `u8`, `u16`, `u32`, `u64` |
| Pointer-width unsigned | `usize` |
| Floats | `f32`, `f64` |
| Unicode scalar | `char` |
| Literal text | `string` |

The set is **authored by this chapter** — see §19.1. Every name in it is used
normatively by the corpus and none was ever enumerated.

**2.2** `bool` has exactly two values, `true` and `false` (chapter 1 §3.7). It
is not an integer, no integer converts to it implicitly, and **no other type is
usable where a `bool` is required** (§6.7). (#9, truthiness.)

**2.3** The sized integer types have the width and signedness their names state
and a two's-complement representation. `usize` is the width of an address on the
target and is the type of every length, capacity and index in the language.
(ADR-0045 §1, which admits `usize` and only `usize` as a generic value
parameter's type on the ground that it is the length type; ADR-0043 §5's
`TextBuf` field `len: usize`.)

**2.4** **There is no `isize`.** No clause of the corpus requires a signed
pointer-width type, and adding one is additive later. (Authored — §19.1.)

**2.5** `f32` and `f64` are IEEE-754 binary32 and binary64. **Float arithmetic
is evaluated exactly as written**; an implementation MUST NOT reassociate or
otherwise re-evaluate a float expression (ADR-0016 §6, and chapter 1 §3.3).
**Infinity and NaN are values, not bugs** (§8.3).

**2.6** `char` is a **Unicode scalar value**. It is what `chars()` yields
(§17.7) and is a `format` hole type (§17.5). It is not a byte, and `[]u8` is not
a sequence of `char`. (Authored from ADR-0043 §4 and §8 — see §19.2.)

**2.7** `string` is the type of a string literal **and of nothing else**
(§17.1). (ADR-0043 §1, §3.)

**2.8** **There is no implicit numeric conversion.** A value of one numeric type
is used as another only through an explicit cast, `x as T` (chapter 1 §7.13).
(#11, no implicit-conversion-driven dispatch.) **The one implicit widening in the
language is `string` to `[]u8` at parameter position** (§17.2, ADR-0043 §2).

**2.9** **A cast is total. `x as T` never faults.** Every legal cast yields a
value for every input, so no cast is a fault site and no cast kind appears in
chapter 5's fault enumeration. This is the deliberate counterpart to §8.3's
always-checked integer overflow: arithmetic that loses a value the programmer
did not ask to lose stops the program, and `as` **is** the syntax for asking.
The lens cost is named rather than resolved — a silently narrowing cast survives
to runtime where `+` would have faulted (**robustness**), bought with one total
rule and no second concept (**simplicity**) and with a cast an agent can
evaluate locally against no context beyond the two types (**agent-friendliness**,
#5's locality R1). (#98.)

**2.9.1 Integer to integer, one rule.** The source value is taken as its
two's-complement bit pattern and **truncated or extended to the target's width —
sign-extending if and only if the source type is signed — then interpreted under
the target's signedness.** Narrowing, widening and same-width reinterpretation
are the same operation at three widths; there is no separate reinterpret form.
Consequences: `300 as u8` is `44`, `-1 as u8` is `255`, `200 as i8` is `-56`,
`0xFFFFFFFF as i32` is `-1`, and widening from a signed type preserves the value.
**ADR-0007's mandated noise generator depends on this row**: its `(x as i32)` over
`u32` state is a same-width reinterpretation, and any other reading would make
the spec's own bit-exact formula produce a different sequence on a conforming
implementation. (#98.)

**2.9.2 Float to integer.** The value is **truncated toward zero**. Out of the
target's range it **saturates** to the target's minimum or maximum, and **NaN
converts to `0`**. All three are normative: a rule that says only "truncates"
leaves a backend free to emit whatever the hardware does, which is the freedom
§2.9's totality exists to remove. (#98.)

**2.9.3 Integer to float.** The value is rounded to the nearest representable
value, **ties to even** — IEEE-754's default rounding, consistent with §2.5.
The conversion is silent at runtime even where it does not round-trip. (#98.)

**2.9.4 Float to float.** `f32 as f64` is exact. `f64 as f32` rounds to nearest,
ties to even; a magnitude too large for binary32 yields the corresponding
infinity, and NaN stays NaN. (§2.5, §8.3: infinities and NaN are values.) (#98.)

**2.9.5 `distinct` types.** `as` crosses a `distinct` type in **both**
directions, to and from its base type, which is the "explicit cast" §3.3 already
requires. A `distinct` cast is a change of type only: the representation is the
base type's (§3.1) and no bits move. (#98.)

**2.9.6 `char`.** `c as u32` is legal and total — a Unicode scalar value is
always a `u32` (§2.6). **The reverse is not a cast.** Not every `u32` is a
scalar value, so a total `u32 as char` would have to substitute a replacement
character, which is a silent lie about the input. The conversion is
**`char.from_u32(x) -> ?char`**, an associated function on `char`, and the check
is the bind (#9). `u32 as char` MUST be a compile error. (#98.)

**2.9.7 `bool` casts do not exist**, in either direction. `true as i32` is a
compile error. `bool` is not an integer (§2.2), and a numeric spelling for it
would be #24 sugar buying what `if` already expresses. (#98.)

**2.9.8 The const-eval asymmetry, and what "lossy" means.** Chapter 4 §12.9
makes a **lossy cast fail the build** in a constant-required position, while the
identical expression at runtime is silent under §2.9. This is one meaning of
`as` in two settings, not two meanings: a constant is fully known, so a loss the
compiler can prove is unconditionally a typo, and there is no running program to
carry the value onward. **Lossy means the mapping is not injective at the given
width** — information destroyed, not merely a changed value. Same-width
reinterpretation (§2.9.1) and value-preserving widening are bijective and
const-fold freely, so ADR-0007's formula is const-evaluable; a narrowing that
discards a set bit, a float→integer that truncates a fraction or saturates, and
an integer→float that does not round-trip each fail the build. (#98; chapter 4
§12.9.)

**2.9.9 `#explicit` says nothing about casts.** No lint fires on a cast, in
either layer. A cast is already the most explicit construct in the language —
the operator exists solely to make the conversion visible — so a lint would fire
on every correct use and teach nothing, which is §10.16's shape: `#explicit`
accepts a form whose protected property is already satisfied. This is a
decision, not an omission. (#98; #6.)

---

## 3. `distinct` types

**3.1** `type Name = distinct <Type>` declares a **new nominal type** with the
representation of its base type and none of its operations beyond equality and
comparison. It is **opaque**: it keeps `==`, `!=` and ordering, so it works as a
container key with no ceremony, and it **loses arithmetic**. `EntityId +
EntityId` MUST be a compile error. (#11 Q5–Q7.)

**3.2** `type Name = distinct numeric <Type>` declares a **quantity**. It is
closed under addition, subtraction and scalar multiplication — `T + T -> T`,
`T * scalar -> T` — and **`T * T` does not exist**. An implementation MUST reject
`T * T` naming the type. (#11 Q5–Q7.)

**3.3** A `distinct` type of either kind converts to and from its base type only
through an explicit cast or a named function. There is no implicit conversion in
either direction, which is what makes a `distinct u32` handle a type rather than
an integer. (#11, nominal newtypes only.)

**3.4** **There is no dimensional algebra.** Exponent arithmetic over units,
unification over units, and inferred derived dimensions are **not in this
language**. Conversions between quantities are ordinary named functions. (#11
Q5–Q7, the unit-system finding.)

**3.5** Which quantity types the standard library blesses is **not this
chapter's call**: it is a stdlib-surface question, and **chapter 6 owns it**
(ADR-0016 §4; [#99](https://github.com/ludo-lang/ludo/issues/99)). This chapter
owns only the `distinct numeric` machinery that makes them expressible.

---

## 4. Sum types and `match`

**4.1** Sum types are **mandatory**, not optional depth. (#9, which makes `?T`
sugar over a user-expressible sum type; #10, which requires them for error sets;
#11 Q10.) Chapter 1 §5.4 spells the declaration.

**4.2** A variant MAY carry a positional payload. A value of a sum type is
exactly one variant at a time, and its variant is **not readable except by
`match`** or by the forms §6 and §7 build on `match`. (#11 Q10.)

**4.3** **`match` MUST be exhaustive.** A `match` that does not cover every
variant of the scrutinee's type is a **compile error naming the uncovered
variants**. (#11 Q10; #10, which makes exhaustive `match` the general handling
form.)

**4.4** **There are no guards** (chapter 1 §8.3). Exhaustiveness is therefore
decidable by inspection of the arm patterns alone, and an implementation MUST
NOT require any other analysis to decide it. (#11 Q10.)

**4.5** A wildcard or bare-binding arm covers every variant not covered by an
earlier arm. Adding a variant to a type MUST NOT silently change the behaviour
of a `match` that enumerated the old variants: with no wildcard, that `match`
becomes a compile error. **This is the property the whole feature exists for.**
(#11 Q10.)

**4.6** A bare identifier pattern denotes a **nullary variant of the scrutinee's
type** where the scrutinee's type declares one with that name, and a **fresh
binding** otherwise. The distinction is made during checking, never during
parsing (chapter 1 §8.5). (#11 Q10; #22 guarantee 1.)

**4.6.1** A variant is **constructed** by qualifying it with its type:
`Shape.point`, `Shape.circle(3.0)`. A nullary variant is the qualified name; a
variant with a payload is that name applied to its payload. **A bare variant
name is legal only in a pattern** (§4.6), where the scrutinee's type supplies the
qualification. `none` and `some` are the prelude's own unqualified spellings for
`?T` (chapter 1 §2.7) and are the exception this rule already admits. The
spelling is **authored by this chapter** — see §19.4; it costs nothing against
#24, being an existing `PostfixExpr` chain.

**4.7** `match` is a **statement**, so it has no value and no tail position. A
function whose body ends in a `match` writes `return` in each arm (chapter 1
§6.7). (#7, surface call 3.)

**4.8** Sum-type **layout** — representation, tag placement, and the absence of
niche optimisation — is chapter 3's, per
[#25](https://github.com/ludo-lang/ludo/issues/25) §10. (The citation read
"ADR-0025" until chapter 3 found the conflation; ADR-0025 is *Fullscreen is a
player preference*. See [`coverage/03-memory.md`](coverage/03-memory.md) §0.)

---

## 5. Generics

**5.1** Generics are **present, monomorphised, and constraint-bounded**, with
explicit type parameters. (#11 Q1, Q11.) Chapter 1 §10 spells the parameter list.

**5.2** **A generic body is type-checked exactly once, against its constraints,
and never per instantiation.** An implementation MUST NOT defer any type error
in a generic body to an instantiation site. (#11 Q11.)

**5.3** Type-checking a **call** to a generic requires only the generic's
**signature**. Monomorphisation is a **codegen** obligation and MUST NOT be
required to answer a type question. (#11 Q11; #22, which forbids the oracle from
monomorphising.)

**5.4** A type argument MUST satisfy every constraint its parameter declares.
Failure is a compile error naming the parameter, the argument and the unsatisfied
constraint. (#11 Q1, Q2.)

**5.5** There are **no unconstrained templates**. A generic body MUST NOT use an
operation its parameter's constraints do not provide. (#11 Q1.)

**5.6** There is **no boxed or uniform representation**. `Pool[Entity]` MUST be
a real contiguous `Entity` array, not an array of pointers. (#11 Q1; #8's data
layout control.)

**5.7** A **value parameter** (chapter 1 §10.3–§10.9) is a compile-time `usize`.
Its restrictions are grammatical and static and are stated in chapter 1; the
semantics they rest on are:

- The generic body is checked once with `N` **opaque** — its value is not known
  during checking and no branch may depend on it. (ADR-0045 §4.)
- A value parameter is **not constraint-bounded**; its `usize` annotation is its
  whole bound. (ADR-0045 §4, a stated exception to §5.1.)
- Two instantiations differing only in a value argument are **different types**.
  `TextBuf[32]` and `TextBuf[64]` are unrelated. (Follows from §1.3 and
  ADR-0045 §1.)

**5.8** A **generic function becomes a value only when explicitly
instantiated**. `compare[int]` is a value of type `fn(int, int) -> int`; bare
`compare` is a **compile error naming the missing type argument**. There is
**no inference of the instantiation from an expected function-pointer type**.
(ADR-0017 §5.)

---

## 6. Interfaces

**6.1** An interface is a **named set of function signatures** and is used
**only as a constraint on a generic parameter**. (#11 Q2, and the fork locked in
#16: interfaces are constraints only, with no implementation inheritance and no
hierarchy.)

**6.2** **Satisfaction is explicitly declared.** A type satisfies an interface
only where its declaring module says so. **Structural or implicit satisfaction
does not exist**: a type MUST NOT gain an interface by happening to have
functions of the right shape. (#11 Q2.)

**6.2.1** Both declarations now have a spelling: **§6.7 authors them**, closing
what this section previously recorded as the eighth phantom clause. The forms are
charged as chapter 1 §13.9.1 crossing 2, against the failure class §6.2 names.
([#100](https://github.com/ludo-lang/ludo/issues/100).)

**6.3** **Dispatch is static, always.** There is **no boxed existential, no
`dyn`, and no dynamic dispatch of any kind** in this specification.
Heterogeneity is expressed with a sum type. (#11 Q2.)

**6.4** An interface MUST NOT be used as a type in its own right — not as a
field type, a parameter type, a return type, or an element type. It appears only
in a constraint position. (Follows from §6.3; #11 Q2.)

**6.5** A `fn` type satisfies an interface that declares **exactly one**
function, when the signature matches **exactly**. This is a **fixed rule about
one built-in type** and it does not reintroduce structural satisfaction: no
user-declared type satisfies anything implicitly. (ADR-0017 §3, which extends
#11.)

**6.6** §6.5 is the fifth use of the **privileged-compiler-knowledge shape**,
alongside the blessed math operators (§16.1, ADR-0016 §3), the derived
`Eq`/`Hash`/`Clone` (#15, chapter 3), the SoA transform (#25 §1, chapter 3),
and the shader's recognise-by-name (ADR-0008 §6, chapter 6). It is **not a
mechanism a user can invoke**. (ADR-0017 §3.)

### 6.7 The spellings

Authored by [#100](https://github.com/ludo-lang/ludo/issues/100). §6.1–§6.6 fix
what these forms mean; this section fixes how they are written.

**6.7.1 The interface declaration** is the ordinary type declaration with a
**fifth type body**:

```
type Compare[T] = interface
  fn cmp(a: T, b: T) -> i32
end
```

`InterfaceBody ::= "interface" MemberSig* "end"` is a fifth alternative of
`TypeBody`, so an interface is declared by the same `type X = <body> ... end`
shape as a struct, an enum or a distinct type (chapter 1 §14.2), and inherits
generic parameters, attributes and `pub` from `TypeDecl` with **no rule of its
own**. A declaration form parallel to `type` was rejected on that ground: it
would have restated all four for one body.

**6.7.2 A member is a `fn` header with no body**:
`MemberSig ::= "fn" Identifier GenericParams? "(" ParamList? ")" ReturnType?`
— `FnDecl` with `Attribute*`, `Block`, `TailExpr` and `end` removed. There is
**no receiver and no `Self`.** An interface names the type it constrains through
its own generic parameters, so `T` appears in the signatures, as above. This
follows #11, which ships no `self` and no methods; an implicit receiver would be
the first, and `Self` would be a keyword bought for brevity alone — sugar under
chapter 1 §13.8 rule 3, since `#explicit` rejects it. The cost is that signatures
repeat `T`.

A field-style member (`cmp: fn(T, T) -> i32`) was rejected: it is visually the
`fn` **type**, which is the subject of §6.5 — the one rule a reader is most
likely to confuse with structural satisfaction.

**6.7.3 The satisfaction declaration** is a top-level item with no body:

```
impl Rock: Compare[Rock]
```

`ImplDecl ::= "impl" NamedType ":" ConstraintList`. The functions that discharge
it are **ordinary module-level `fn`s**, matched by name and signature; `impl`
asserts the whole interface and MUST NOT list members, which would duplicate the
interface body and rot against it. A block form was rejected because it would
make `impl` a second place functions can be declared, which is a namespacing
question this language does not otherwise have
([#111](https://github.com/ludo-lang/ludo/issues/111)). The consequence is that
the discharging functions are not visually adjacent to the assertion; chapter 7's
diagnostic carries that weight instead.

Reusing `ConstraintList` lets one `impl` discharge several interfaces —
`impl Rock: Compare[Rock] + Hash[Rock]` — and makes the shape that **states** a
constraint on a generic parameter identical to the shape that **discharges** it.

An attribute form (`#satisfies(Compare[Rock])` on the type) would have cost
**zero** tokens, which is a real pull at chapter 1 §13.9.1's figures, and is
rejected anyway: satisfaction is semantics-bearing, and an attribute is where
this spec puts metadata. A clause on the type header
(`type Rock: Compare[Rock] = struct ... end`) is rejected for putting a foreign
claim where a reader expects the type's own shape, and for degrading as the
interface list grows.

**6.7.3.1 Generic arguments on an `impl` line MUST be written out.** `impl Rock:
Compare[Rock]` is well-formed and `impl Rock: Compare` is not, even where the
argument is inferable. Inference would save four characters and cost a rule with
a boundary an author must learn, and it would make the reflexive case read
differently from `impl Rock: Compare[Vec2]`, which is legal and deliberate.

**6.7.4 An empty interface body is legal.** `type Marker = interface end` is a
**purely nominal constraint**, satisfied by an `impl` and by nothing else. It is
this satisfaction model in its limit case rather than an exception to it, and
banning it would require a rule where permitting it requires none. It cannot be
turned into dynamic dispatch: §6.4 forbids an interface in type position whatever
its body.

**6.7.5 `impl` is a `TopLevelItem`, not a `DeclarationBody`, and MUST NOT carry
`pub`.** Satisfaction is always public. A private satisfaction is unobservable —
its only job is to make a generic in another module accept the type — so a `pub`
marker could only produce an `impl` that silently fails at a foreign call site.
The grammar denies the marker rather than a checker rejecting it. This leans on
[#111](https://github.com/ludo-lang/ludo/issues/111) for what `pub`'s boundary
is; if #111 lands a per-file rather than a per-library boundary, this clause is
revisited and nothing else in §6.7 is.

**6.7.6 The orphan rule.** An `impl` MUST appear in the **declaring module of the
type** (#11 Q8's radius). A local type satisfying a foreign interface is the
ordinary case and is legal; a **foreign type satisfying a local interface is
not**. The mirror image would let two modules declare two conflicting
satisfactions for one type, and a generic's meaning would then depend on which
modules happened to be linked. The consequence is stated plainly rather than
buried: **you cannot make a third-party type satisfy your interface** — wrap it
in a `distinct` type (§5) and `impl` that.

**6.7.7 §6.5's blessed `fn` rule gets no syntactic marker.** A `fn` type
satisfying a one-function interface is a **checker rule only**, with no `impl`
written and none permitted. §6.6 files it as privileged compiler knowledge and
"not a mechanism a user can invoke"; a marker would make it invocable, which is
the structural satisfaction §6.2 forbids, reintroduced by the clause that
disclaims it. Zero grammar delta.

**6.7.8 The budget.** `interface` is a **type-sublanguage** keyword (chapter 1
§2.4, 4 → 5) and `impl` is a **core** keyword (§2.3, 32 → 33). The two are
charged as **one** crossing, chapter 1 §13.9.1 row 2, against the failure class
**accidental satisfaction** — §6.2's rule, which the corpus mandated while
providing no syntax to express it. Chapter 1 §13.6, §13.7 and §13.12 publish the
resulting figures; the core row moves to **+32.7%** against Lua 5.4.

---

## 7. Function types and the ceiling

**7.1** `fn(T, U) -> R` (chapter 1 §9.6) is the function type. Its values are
**non-capturing function pointers** and nothing else. (ADR-0017 §2.)

**7.2** **Closures are a stated non-goal.** There is no capture of any kind, no
capture-by-value and no capture-by-reference form, no bound receiver, and no
partial application. (ADR-0017 §1.)

**7.3** **Every function is top-level and named** (chapter 1 §5.7). There are no
anonymous function literals and no nested function declarations, so **every
callable entity has an entry in the name table**. (ADR-0017 §4, generalising
#29's ABI restriction into a language rule.)

**7.4** Where captured state is wanted, the mechanism is **an interface-bounded
generic parameter whose argument is a struct with named fields** — §6.1 plus
§5.1, with no new feature. (ADR-0017 §1.)

**7.5** **UFCS never applies to a function value.** Where `cmp` is a binding
holding a `fn(T, T) -> int`, `a.cmp(b)` is **not legal**; the call is written
`cmp(a, b)`. (ADR-0017 §6, which extends #11.)

**7.6** **A higher-order standard-library function MUST contribute an algorithm,
never merely a loop shape.** `sort`, `binary_search` and `partition` qualify;
`map`, `filter` and `reduce` do not and **are not in the standard library**.
`for` is the one spelling for a loop. (ADR-0017 §7.)

---

## 8. Failure, absence, and the bug line

**8.1** The language draws the line between an **expected failure** and a **bug**
itself; it is not a convention. (#10 Q1, Q12.)

**8.2** An **expected failure is a value in the return type**, handled or
propagated at the call site (§10). A **bug is not a value, has no type, and
cannot be caught, recovered from, or handled in the safe layer.** (#10 Q1, Q12.)

**8.3** The bugs are, exhaustively for this chapter's sources:

- an out-of-bounds index;
- a failed `$.assert`;
- integer divide-by-zero;
- **integer overflow, always checked** — an implementation MUST NOT switch this
  behaviour on a build mode (#8);
- a use-after-free inside `unsafe`.

**Float arithmetic is exempt**: infinity and NaN are values, because a frame loop
MUST NOT abort on a degenerate normal. (#10 Q1.)

**8.4** **A stale handle is not a bug.** `pool.get(h)` yields `?Entity`, so a
dead handle is an **absence** (§9). Only an `unsafe` dereference of a dead handle
is a bug. (#10 Q1; #9.)

**8.5** User code **can** declare a bug: `$.assert` and `$.panic` are in the
standard-library root and are **always on**. (#10 Q9.)

**8.6** What happens **after** a bug — what a pause means, resumption, the fault
report, dev versus shipped behaviour — is chapter 5's, per #18. This chapter
fixes only that a bug is a single mechanism with no catch and no recover in the
safe layer, because that is what determines what the type system must express.
(#10 Q3.)

---

## 9. Absence: `?T`

**9.1** **`?T` is the only absence concept in the language.** Every type other
than `?T` is **non-nullable**: no value of type `T` is ever absent, and **there
is no null of any spelling** (chapter 1 §3.7). (#9.)

**9.2** **`?T` is sugar over a user-expressible sum type**, not a compiler
built-in with no user-level equivalent. Its constructors are the prelude
identifiers `none` and `some` (chapter 1 §2.7), nameable by the same rules any
other sum type's variants are. (#9.)

**9.3** **Zero values are rejected.** No type has a defined default that stands
in for absence, and absence is never invisible in a signature. (#9.)

**9.4** **Absence is not failure.** `?T` and a fallible type (§10) are two
different types, and `?T` MUST NOT be modelled as a fallible type with an empty
error payload. (#9; #10.)

**9.5** **`?T` does not auto-flatten.** `?(?T)` is a distinct type from `?T`.
Where a generic produces one, the implementation MUST report it rather than
quietly collapsing it. (#9.)

**9.6** **Every binding initialises at its declaration**, and no binding is
observable before it is initialised. (#9.) A struct literal MUST name every
field that has no declared default; **an omitted field is legal only where the
declaration supplies a default** (#15 Q21, chapter 1 §5.3) — #9's prohibition is
on an *implicit* zero, not on an author's written default. The one unresolved
site is `persist` without an initialiser, which chapter 5 decides (chapter 1
§5.13, §14.5).

**9.7** The sources of a `?T`, as fixed by this chapter's sources:

- **A missing container key reads as `?V`.** There is **no panicking indexing
  variant** and no by-layer difference. (#9.)
- **A stale handle reads as `?T`.** (#9; #8.)
- **`has(k) -> bool` exists on containers only**, as a question about the
  container rather than about an optional. (#9.)

Chapter 3 owns the container set that these signatures live on (#82 is a marked
gap there).

**9.8** The handling forms are chapter 1 §6.11–§6.14, §7.6 and §7.11. Their
typing rules:

- **`a or b`** where `a: ?T` and `b: T` yields `T`. (#9.)
- **`a?.f`** short-circuits and yields `?C` for the **whole chain**, where `C` is
  the type of the final link. It applies to UFCS calls as well as field access.
  (#9.)
- **A binding condition `e := <expr>`** requires `<expr>: ?T` and binds `e: T` on
  the taken path. **The check is the bind**: there is no state in which `e` is in
  scope and absent. (#9.)
- **A statement modifier over a binding condition** binds into the **enclosing**
  scope, live on the path where the modified statement did not run, and the
  modified statement MUST diverge (chapter 1 §6.14). (#9.)

**9.9** **A bare-`bool` absence predicate — `is_some()`, `is_nil()` — is
deliberately absent from the language and the standard library.** Every form in
§9.8 binds. (#9.)

**9.10** A value of type `?T` MUST NOT be used where `T` is required. There is no
implicit unwrap, and no unchecked unwrap operator. (#9; follows from §9.1.)

**9.11 Injection.** The converse **is** permitted: a value of type `T` is usable
wherever `?T` is expected, and is present. `return x` in a function returning
`?T` is legal and needs no wrapping call, as is `some(x)` written explicitly.
Injection is **by static type** and it does not flatten (§9.5): a `?T` used where
`?(?T)` is expected is a present `?T`. The rule is **authored by this chapter**
— see §19.5, which authors the same rule for a fallible type.

---

## 10. Failure: fallible types and error sets

**10.1** **Errors are values in the return type.** `T rescue E` (chapter 1 §9.7)
is a **sum type with sugar**, exactly parallel to `?T`. Failure MUST be visible
in the signature. (#10 Q2, Q6, Q19, Q20; #5 Tier 1 criterion 2.)

**10.2** **Multiple return values are never the error mechanism.** A function
does not signal failure by returning an extra value a caller may decline to bind.
(#10 Q2.)

**10.3** **Error sets are nominal, closed and per-function.** The set a function
may fail with is written in its signature and is a declared sum type. (#10 Q2.)

**10.4** **Error sets are never inferred.** A callee adding a variant MUST NOT
change a caller's signature. (#10 Q2; #5 criterion 4.)

**10.5** **There is no universal `Error` type.** (#10 Q2.)

**10.6** **Elision:** `T rescue` with no set named denotes **the enclosing
file's declared error set**. This requires no new declaration form: it is legal
where the file declares **exactly one** sum type used in a `rescue` position.
**Two candidates is a compile error naming both**, and nothing is ever inferred
across files. (#10 Q2; #22.)

**10.6.1** A function whose return type is `T rescue E` **produces a failure by
returning a value of `E`**, and a success by returning a value of `T`. The
injection into the fallible type is by the returned value's **static type**, and
no wrapping call is written at either site. `T` and `E` MUST be distinct types; a
signature in which they are the same type is a **compile error naming the
ambiguity**. The rule is **authored by this chapter** — see §19.5.

**10.7** **Crossing a module boundary requires an explicit conversion** from one
error set to another. This is the named cost of §10.3 and is not softened.
(#10, three lenses.)

**10.8** `rescue` (chapter 1 §7.14) is **purely local** and involves no stack:
there is no unwinding, no handler search, and no non-local exit. Its three tails
type as:

- **a diverging tail** (`rescue return`, `rescue break`, `rescue continue`) —
  the expression yields `T` on the path that continues;
- **a default value** (`rescue 0`) — the tail's expression MUST have type `T`,
  and the whole expression yields `T`;
- **a binding handler** (`rescue err then ... end`) — `err` is bound to the
  error value, typed as the function's error set.

(#10 Q5, Q7, Q16.)

**10.9** `rescue` is legal **only where a fallible value is produced**. Applying
it elsewhere is a compile error naming the expression. (#10, the notation test.)

**10.10** **`match` remains the general handling form** for a fallible value, as
for any sum type (§4). (#10 Q5.)

**10.11** **A fallible value is must-use.** An unbound fallible value is a
**compile error**; discarding costs exactly one greppable token, `_ = f()`
(chapter 1 §2.2). (#10 Q5; #8's must-use rule, which chapter 3 owns in general.)

**10.12** **Propagation is explicit at every call site.** There is no implicit
propagation, **no postfix `?` operator and no `try` prefix** (chapter 1 §7.16).
(#10 Q5, and its rejected spellings.)

**10.13** **`or` handles absence and `rescue` handles failure**, and neither
substitutes for the other. Applying `or` to a fallible value, or `rescue` to a
`?T`, is a compile error naming the type. (#10; #9.)

**10.14** `?T` and a fallible type **compose** — `?File rescue IOError` is a
legal type — **with no flattening** (§9.5). **The blessed standard library never
returns one**: an operation is absent-or-present, or succeeds-or-fails, never
both. (#10 Q13.)

**10.15** **There is no layer that may ignore an error.** Must-use is universal;
no layer relaxes it. (#10 Q14, Q11; #6.)

**10.16** **`#explicit` accepts `rescue`** and does not require `match` in its
place: `rescue` **is** the canonical spelling of propagation, so the property
`#explicit` protects is already satisfied. (#10 Q11; ADR-0029 §4's closed list,
chapter 1 §11.3, which does not list it.)

**10.17** **Allocation failure is the allocator's policy, not the language's.**
A general-purpose allocator aborts on exhaustion — a bug (§8.3) — and an
explicitly fallible allocator returns a fallible value. **Not every allocating
call is fallible.** (#10 Q4.) Chapter 3 owns the allocator interface (ADR-0042).

**10.18** **Cleanup is `defer` only. There is no `errdefer`** (chapter 1 §6.9),
and there are no destructors. Undo-on-failure is written with an explicit success
flag. (#10 Q8, Q17; #8 call 5.)

---

## 11. Inference

**11.1** **Signatures are fully annotated.** Every parameter carries its type and
every return type is written. (#11 Q4; chapter 1 §5.8.)

**11.2** **A return type is never inferred.** (#11 Q4.)

**11.3** **Bodies and struct literals are inferred.** A local binding's type
comes from its initialiser; a literal's field types come from the annotated
target. (#11 Q4.)

**11.4** **There is no global or whole-program inference.** No type ever requires
information beyond the current file and the signatures it names. (#11, #5
criterion 5; #22.)

**11.5** **Inference never widens on failure.** Where a type cannot be
determined, the program is a **compile error naming the ambiguity** — never a
silent pick and never a fallback to a dynamic type. (#11 Q4; chapter 1 §3.2 for
numeric literals.)

---

## 12. Mutability

**12.1** **Mutability is a property of the place, not a type constructor.**
`!T` and `T` are **the same type**, differently accessed (chapter 1 §7.10,
§9.2). (#11 Q3.)

**12.2** **Immutable is the default.** A binding, parameter or field is mutable
only where `!` marks it. (#11 Q3.)

**12.3** Because `!` is not a type constructor, **no variance, propagation or
conversion question arises from it**: there is no `[]!T` versus `[]T`
distinction to decide, and no rule converting between them. (#11 Q3.)

**12.4** Mutation is **transient and non-escaping**; write access is not stored.
(#8; #11 Q3.) Chapter 3 owns what that permits.

---

## 13. UFCS resolution

**13.1** **`a.f(b)` is sugar for `f(a, b)` where `f` is declared in the module
that declares `a`'s type, and nowhere else.** (#11 Q8.)

**13.2** Resolution therefore MUST NOT consult the importing file's import set,
and adding an import MUST NOT change what a UFCS call resolves to. (#11 Q8.)

**13.3** **A UFCS call resolves identically in every file.** One expression means
one thing program-wide. (#11 Q8; #5 criterion 4.)

**13.4** **A third-party type cannot be UFCS-extended.** `my_helper(their_thing)`
stays prefix. This is a stated, permanent cost. (#11 Q8.)

**13.5** UFCS does not apply to a function value (§7.5, ADR-0017 §6).

**13.6** `?.` applies to a UFCS call as well as to field access (§9.8; #9).

---

## 14. Recursive data

**14.1** **Direct self-inclusion by value is a compile error naming the cycle.**
A struct MUST NOT contain itself, directly or through a chain of by-value
fields. (#11 Q13.)

**14.2** Recursive data is expressed through **a handle into a pool**
(`children: []Handle[Node]`) or through **a growable container**
(`children: List[Node]`). Both allocate, and both name the allocator at the
growth point. (#11 Q13; #8.)

**14.3** **There is no `Box[T]`** and no third indirection concept. (#11 Q13.)

---

## 15. Const evaluation

**15.1** A **const expression** is one an implementation MUST fold at compile
time. The positions requiring one are `const` declarations, array lengths, and
generic value arguments (chapter 1 §5.6, §9.5, §10.9).

**15.2** The evaluable subset is **literals, other constants, arithmetic,
comparison and bitwise operators, and casts, and nothing else** — no calls, no
loops, no recursion, no `if` or `match` selection, no allocation, no I/O.
(ADR-0021 §1.)

**15.3** An expression in a const position that is not foldable under §15.2 is a
**compile error**. (ADR-0021 §5.)

**15.4** This chapter records §15.1–§15.3 because chapter 1 §5.6 routed the floor
here. **ADR-0021 is [chapter 4](04-modules.md)'s source and chapter 4 §12 owns
it in full** — the floor, the impossibility of non-termination, the non-goal
list, the ruling-out of the computed module constant, and the failure rule.
§15.2–§15.3 are a restatement of chapter 4 §12.2–§12.3 for the reader arriving
from a constant-required position, not a second rule; §15.1's positions are this
chapter's. The routing conflict recorded at
[`coverage/02-types.md`](coverage/02-types.md) §3 item 6 is resolved this way,
by ownership rather than deletion.

---

## 16. Operators and typing

**16.1** **Arithmetic operators exist on the blessed math types and nowhere
else.** `a + b`, `a - b`, `v * 2.0` and `m * v` are defined on those types; **no
user type ever gets an operator, and there is no way to ask for one.** `+` and
`-` are elementwise on matching types; `*` is vector-scalar, matrix-scalar,
matrix-vector and matrix-matrix; **there is no `/` between two vectors and no
elementwise `*` between two vectors**. (ADR-0016 §3.)

**16.2** Otherwise the arithmetic, comparison, bitwise and logical operators are
defined on the primitive types of §2 and on `distinct numeric` types as §3.2
permits. Comparison yields `bool`. (#11 Q5–Q7; chapter 1 §7.2–§7.5.)

**16.3** **`and`, `or` and `not` take `bool` operands**, except for §9.8's
`or`-as-unwrap, which is selected by the left operand's static type being `?T`.
(#9; chapter 1 §7.5, §7.6.)

**16.4** **Conditions are `bool` only.** There is no truthiness and no coercion;
`if target then` on a `?Entity` is a compile error naming the type. (#9;
chapter 1 §6.12.)

**16.5** The blessed math type set itself — which vectors, which matrices, which
quantities — is **not fixed by this chapter's sources**. It is a stdlib surface
and **chapter 6 owns it**, along with the rest of ADR-0016
([#99](https://github.com/ludo-lang/ludo/issues/99)); §16.1 states the typing
rule and chapter 6 cites it rather than restating it.

---

## 17. Text types

**17.1** **`string` is a view into the runner's constant blob, and nothing
else.** It is the type of a string literal and of no other value. **No runtime
bytes can become a `string`.** (ADR-0043 §1, §3.)

**17.2** **A `string` widens implicitly to `[]u8` at parameter position, and
there is no conversion in the other direction.** (ADR-0043 §2.)

**17.3** Because a `string`'s referent outlives every frame, reload and image
swap, a `string` **may be stored in a struct, `persist`ed, and used as a
container key** — an escape permission **derived** from §17.1, not granted as an
exception to the view rule chapter 3 owns. (ADR-0043 §1.)

**17.4** The consequences are normative and are stated rather than discovered:

- **There is no runtime-constructed `string` container key.** Literal keys only.
- **There is no `persist` field typed `string` holding text a player entered.**
  Player text is stored as bytes (§17.6).

(ADR-0043 §2.)

**17.5** **`format(dst: ![]u8, "...", ...) -> []u8`** writes into the buffer it
is handed and returns a view of exactly what it wrote.

- **It is not a macro.** It is a standard-library call with a
  **compiler-known signature**: the format string MUST be a literal the compiler
  checks, so **a wrong hole count or a wrong argument type is a compile error**,
  never garbage bytes at run time.
- **The hole grammar is `{}` and nothing else** — no width, no precision, no
  alignment.
- **Hole types are a closed, spec-owned list**: integers, floats, `bool`,
  `char`, `string`, `[]u8`. It is a list rather than an interface a user type may
  satisfy, because there is no reflection (§1.9). **The blessed math types are
  excluded.**
- **Overflow truncates at a scalar boundary and returns what was written** —
  never a fault and never a fallible return — so the destination remains valid
  UTF-8.

(ADR-0043 §4.)

**17.6** **`TextBuf[N: usize]`** is a fixed-capacity text buffer: a struct of
`bytes: [N]u8` and `len: usize`, with `append(text: []u8)`, `format(...)`,
`view() -> []u8` and `clear()`. It is a **plain value with no pointer**, so it
lives in `persist` and survives a reload by the same rule as a bare `[N]u8`.
**`append` returns nothing and truncates silently**; `len` and remaining capacity
are readable by a caller who cares. (ADR-0043 §5, as amended by ADR-0045, which
discharges its condition and fixes the declaration's spelling — chapter 1
§14.10.)

**17.7** **Iteration: `chars()` survives, `bytes()` does not.** `bytes()` is the
identity function under §17.2 and **is not in the language**. `chars()` decodes
UTF-8 and yields `char` (§2.6). **Both are defined on `[]u8`**, and `string`
reaches them through the widening. (ADR-0043 §8.)

**17.8** **`[]u8` is not validated UTF-8**, and an invalid sequence yields
U+FFFD when decoded. **Never a fault and never a fallible return.** (ADR-0043
§8.)

**17.9** **String literals are UTF-8 and carry a hidden NUL** outside `len`;
nothing else does (chapter 1 §1.1, §3.6). The guarantee attaches to the blob, not
to the type. (ADR-0043 §7.)

**17.10** **`format`'s returned view is not an escape**: it is derived from a
buffer the caller passed in, so it names memory the caller already owns.
(ADR-0043 §9.) The **general** view-return rule is chapter 3's and is under open
challenge in [#83](https://github.com/ludo-lang/ludo/issues/83).

**17.11** **`$.input.text()` returns `[]u8`, not `string`**, viewing
runner-owned memory valid until the frame entry returns. (ADR-0043 §6.) Chapter 6
owns the facade; this chapter records only the type, because ADR-0011 §294 spells
it `string` and §17.1 forbids that.

---

## 18. What a type may not do

Recorded as a closed list so that a later chapter is not read as having granted
one of them by silence:

| Not in the language | Source |
|---|---|
| null, `nil`, or a zero-value stand-in for absence | #9 |
| truthiness or a bool coercion | #9 |
| an unchecked unwrap | #9 |
| a catchable bug | #10 |
| an inferred or open error set | #10 |
| an exception, an unwind, or a non-local exit | #10 |
| a transparent type synonym | #11 |
| inheritance, embedding, or field promotion | #11 |
| structural interface satisfaction | #11 |
| dynamic dispatch, `dyn`, or a boxed existential | #11 |
| a higher-kinded type, specialisation, or variance | #11 |
| overloading or an implicit conversion (other than §17.2) | #11 |
| a borrow checker, lifetimes, or aliasing analysis | #11 |
| dimensional algebra | #11 |
| a closure, a capture, or a bound receiver | ADR-0017 §1 |
| an anonymous function literal | ADR-0017 §4 |
| an inferred generic instantiation from an expected type | ADR-0017 §5 |
| `map`, `filter` or `reduce` in the standard library | ADR-0017 §7 |
| reflection | ADR-0021, ADR-0043 §4 |
| a `string` built at run time | ADR-0043 §1 |
| `bytes()` on text | ADR-0043 §8 |

---

## 19. Spellings authored by this chapter

Recorded under ADR-0044 §6: **a hole is repaired in the spec text, and an ADR is
written only for a reversal.** Neither entry below reverses a decision, so
neither takes an ADR.

**19.1 The primitive scalar type set.** Chapter 1 §2.6 states that "the sized
integer and float names" are prelude identifiers without enumerating them, and
**no ADR or issue in the corpus ever enumerates them** — while `u8`, `u16`,
`u32`, `u64`, `i16`, `i32`, `i64`, `f32`, `f64`, `usize` and `bool` are all used
normatively across the corpus (ADR-0006 §141 `i16`; ADR-0007 §361 `i32`, §382
`f64`; ADR-0009 §111 `[]u32`; ADR-0016 `f32`; ADR-0042 `distinct u32`;
ADR-0043 §5 `[N]u8`, `usize`, §169 `i64`; ADR-0045 §1 `usize`). §2.1 authors the
set as the closure of what the corpus uses, made regular: the four signed and
four unsigned widths, `usize`, two floats, `bool` and `char`. **`isize` is
excluded** (§2.4) because no corpus site requires it and adding it is additive.
The set costs nothing against #24: these are prelude identifiers, not tokens
(chapter 1 §13.5 rule 6).

**19.2 `char` as a Unicode scalar value.** ADR-0043 §4 lists `char` as a
`format` hole type and §8 keeps `chars()` as the decoding iterator, so the type
is required by two clauses and defined by neither. §2.6 authors it as the Unicode
scalar value that `chars()` yields, which is the only reading under which both
clauses are true.

**19.3 The interface declaration and its satisfaction — recorded here, authored
in §6.7.** #11 Q2 ships explicit nominal interface satisfaction and ADR-0017 §3
depends on a single-function interface, but neither declaration form existed in
`grammar.ebnf` or chapter 1. This is the eighth phantom clause the consolidation
has found and the **only one this chapter did not repair on sight**: the other
seven cost a word, while this one adds type-sublanguage productions and a keyword
in each budget, which chapter 1 §13.8 charges and §13.6, §13.7 and §13.12
publish. A decision with a budget consequence is not a transcription defect, so
ADR-0044 §6 did not reach it and it went out as
[#100](https://github.com/ludo-lang/ludo/issues/100) rather than being written
inline — the same treatment chapter 1 §14.5 gave `persist`. **#100 is now
resolved and §6.7 carries the spellings**; the delay is recorded, not the gap,
because the route matters to whoever next finds a phantom clause with a price
tag on it.

---

**19.4 Variant construction in expression position.** #9, #10 and #11 all
require sum types and chapter 1 §14.2 authored the *declaration*, but **nothing
in the corpus writes a variant into an expression** — chapter 1 §8.5 spells the
bare identifier only in a *pattern*, where the scrutinee supplies the type.
§4.6.1 authors the qualified form `Shape.circle(3.0)` on two grounds: it parses
as an ordinary `PostfixExpr` chain (`Identifier` `.` `Identifier` `(` `)`) so it
costs **zero** against chapter 1 §13.6, and qualification is forced in expression
position because there is no scrutinee to resolve a bare name against. `none` and
`some` stay unqualified because chapter 1 §2.7 already put them in the prelude.

**19.5 Injection into `?T` and into `T rescue E`.** #9 spells how absence is
*handled* and #10 spells how failure is *handled*, and **neither spells how
either is produced**: no clause in the corpus says what a function returning
`?T` writes to return a present value, or what a function returning
`T rescue E` writes to fail. §9.11 and §10.6.1 author one rule for both — a
value of an arm type is usable where the sugared type is expected, injected by
static type — because the two sugars are the same shape over the same mechanism
(#9's `?T` and #10's fallible type are both sum types with sugar), and authoring
them apart would be the corpus's own defect repeated inside one chapter. The
alternative, a named wrapping constructor at every site, is a second spelling for
an entity the type already names (#5 criterion 3), and would make `rescue return`
(chapter 1 §7.14) asymmetric with the `return` that produced the error it
propagates. Zero grammar delta: `return` and its expression list already exist
(chapter 1 §6.6). The failure direction stays visible at the call site regardless,
because §10.11's must-use and §10.12's explicit propagation are unaffected.

## 20. What this chapter does not decide

Recorded so the boundary is legible, and so a later chapter is not read as having
inherited a silence:

- ~~**The interface and satisfaction spellings.**~~ **Closed.**
  [#100](https://github.com/ludo-lang/ludo/issues/100) resolved and the forms are
  authored at §6.7; chapter 1 §13.6, §13.7, §13.9.1 and §13.12 moved with it.
  Kept in this list, struck rather than deleted, because a reader arriving from
  §6.2.1 or §19.3 must be able to see that the boundary moved rather than
  wonder whether they are reading a stale copy.
- **The blessed math type set.** §16.1 transcribes ADR-0016 §3's operator rule
  because the typing of `+` cannot be stated without it, and §2.5 takes §6's
  float-evaluation rule; **the rest of ADR-0016 is chapter 6's**, which is
  rescoped from *the facades* to *the standard library* to hold it
  ([#99](https://github.com/ludo-lang/ludo/issues/99)). ADR-0016 §6's
  conformance halves — SIMD lowering is not required, float evaluation binds
  every implementation — are chapter 8's.
- **The container set.** §9.7's `?V` reads, `has`, `List` and `Pool` are
  chapter 3's, which carries [#82](https://github.com/ludo-lang/ludo/issues/82)
  as a marked gap.
- **Layout.** Sum-type representation, `#align(n)` and the SoA transform are
  chapter 3's ([#25](https://github.com/ludo-lang/ludo/issues/25), **not**
  ADR-0025).
- **The general view-return rule.** §17.10; chapter 3's, and open in
  [#83](https://github.com/ludo-lang/ludo/issues/83).
- **The allocator interface.** §10.17; chapter 3's (ADR-0042).
- **The rest of const evaluation.** §15.4; chapter 4's (ADR-0021).
- **What a bug does at run time.** §8.6; chapter 5's (#18).
- **`persist` without an initialiser.** §9.6; chapter 5's (chapter 1 §14.5).
- **The blessed `distinct numeric` quantity types.** §3.5; a stdlib-surface call
  (ADR-0016 §4), and chapter 6's with the rest of the blessed set.
