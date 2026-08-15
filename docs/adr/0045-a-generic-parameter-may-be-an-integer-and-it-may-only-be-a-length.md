---
status: accepted
---

> **Amended by [spec ch5 §4.3](../spec/05-runner.md):** §8's reference-program line
> `persist name: TextBuf[32]` **gains an initialiser** — `= {}` — because chapter 5 resolved
> the #9 contradiction §1 and §8 left by requiring one at every `persist` declaration. The
> integer parameter, its annotation and the three restrictions are untouched.

# A generic parameter may be an integer, and it may only be a length

[#84](https://github.com/adamico/ludo/issues/84), split out of
[#81](https://github.com/adamico/ludo/issues/81) / [ADR-0043](0043-a-string-is-a-literal-and-constructed-text-is-bytes.md) §5,
which ships `TextBuf[N] = struct { bytes: [N]u8, len: usize }` and cannot declare it without
this feature. [ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §9
put this ticket ahead of spec chapter 1, on the ground that whether `TextBuf[64]` is a legal
type expression is a **production in the type sublanguage** and the grammar cannot be written
around it.

[#11](https://github.com/adamico/ludo/issues/11) settled generics as monomorphised, nominal
and constraint-bounded, and said nothing about whether a parameter may be a **value** rather
than a type. [#15](https://github.com/adamico/ludo/issues/15) blesses `[N]T`, but that is a
builtin, so the integer in a type expression was already there and had no user-facing door.

## 1. The parameter is admitted, and it is fenced at its only use

A user-declared generic **may** take a compile-time integer parameter. The parameter is
declared with its type and used positionally:

```ludo
TextBuf[N: usize] = struct { bytes: [N]u8, len: usize }

persist name: TextBuf[32]
```

Three restrictions travel with it, and they are the whole of the decision's cost:

1. **The only admitted value type is `usize`.** Not `i32`, not `bool`, not an enum, not a
   float, not a `string`. One type means no signedness case, no width matrix, and no question
   about what a negative length denotes.
2. **A parameter may appear only as an array length, bare, or as an argument to another
   generic's integer parameter.** `[N]u8` is legal. `[N*2]u8` and `[N+1]u8` are **not**.
   `TextBuf[N]` inside another generic's body is legal, because it forwards the same bare
   name.
3. **A parameter is not a value in expression position.** `N` cannot be read, compared,
   printed or passed to a function. Code that wants the capacity writes `buf.bytes.len`, which
   #15 already gives on every fixed array.

Restriction 2 is the load-bearing one and everything cheap about this decision follows from
it. **With no arithmetic on a parameter in type position, there is no type-level expression to
decide the equality of.** Two instantiations name the same type iff their arguments fold to
the same integer — a comparison of two `usize` literals, performed after
[ADR-0021](0021-const-eval-folds-an-expression-and-never-runs-a-program.md) §1 has already
folded them. This is what separates the feature from Rust's const generics, where `[N+1]` and
`[1+N]` are the entrance to a general decidability problem, and where the answer was years of
`generic_const_exprs` still not stabilised. The fence is not a staging post toward that; it is
the reason the feature is admissible at all.

`struct Grid[W: usize, H: usize] { cells: [W*H]Tile }` is therefore illegal, and the
replacement is `[H][W]Tile` — a nested fixed array, which restriction 2 permits and which is
the better layout to write anyway.

## 2. What was actually added, and what was already there

The delta is one thing: **a parameter slot may be declared as a value rather than a type.**
Everything else this feature appears to need was already in the corpus:

- The **integer in a type expression** is #15's `[N]T`.
- The **const-expression at the use site** is ADR-0021's floor, which already folds
  `const POOL = MAX_ENEMIES * 2` for `[POOL]Enemy`. `TextBuf[POOL]` reuses it verbatim, and
  the same floor rules the argument: literals, constants, arithmetic, comparison, bitwise and
  casts, **no calls**, so non-termination stays impossible by construction.
- The **bracket list** is #11's generic parameter list and #15's `List[int]` spelling.

Which is why the argument from #81 — *the type sublanguage already carries an integer, so this
exposes an existing capability rather than minting one* — is accepted, and why the counter
(*it is a genuine widening of #11*) is accepted too, and paid for in §5 rather than denied.

## 3. Declaration is annotated; the use site is not

`TextBuf[N: usize]` at the declaration, `TextBuf[32]` at the use. The annotation is not
optional and there is no inference of a parameter's kind from how the body uses it.

Without it, `Buf[N]` and `List[T]` are the same spelling under two different rules, told apart
only by reading the body — the exact shape ADR-0021 §4 rejected when it made `const` a keyword
rather than inferring const-ness from a binding's position, and the same locality argument:
[#5](https://github.com/adamico/ludo/issues/5)'s R1 wants the property visible at the
declaration, not recovered from elsewhere. It is also
[ADR-0029](0029-explicit-marks-a-module-and-forbids-a-spelling-that-omits-a-name.md)'s floor
read at the type level: a spelling that omits a name is what `#explicit` forbids, and `[N]`
alone omits the kind.

The use site stays positional and unannotated, matching `List[int]`. A caller writing
`TextBuf[32]` is passing an argument, and #11 already fixed that arguments are positional.

**ADR-0043 §5 is narrowed by this**: it wrote the declaration as `TextBuf[N]`. The shipped
declaration is `TextBuf[N: usize]`. Every use site in that ADR is unchanged.

## 4. Inference, checking, and why the oracle is untouched

A generic **function** may take an integer parameter under the same rules, and it is inferred
at the call site from the argument's type:

```ludo
fn append[N: usize](buf: !TextBuf[N], text: []u8)

name!.append($.input.text())   -- N is 32, inferred; never written
```

Inference is inside the signature radius — the parameter appears in an argument's type, so the
call site plus the signature determine it, which is R1 exactly and the same bound #15 used for
its target-typed literal.

**#11 Q11's split of checking from codegen survives with nothing added.** The body of a
generic is type-checked **once**, with `N` opaque; restriction 3 is what makes *opaque*
sufficient, because there is no operation on `N` for the checker to have an opinion about, and
restriction 2 means the only place it appears is a length the checker carries symbolically.
So the [#22](https://github.com/adamico/ludo/issues/22) oracle still never monomorphises, and
monomorphisation stays a pure codegen cost off the answer-a-question path.

**Integer parameters are not constraint-bounded, and this is a narrow, stated exception to
#11.** A constraint exists so a body can be checked once against a promise; a value parameter
promises nothing because no operation on it is permitted. Its `usize` annotation is its whole
bound. Recorded here so that the next reader of #11's *constraint-bounded* does not find an
unexplained hole.

Two instantiations of a generic differing only in `N` are two monomorphisations. Nothing is
specialised on a particular `N` — #11 already rejected specialisation, and nothing here
reopens it.

## 5. What it costs against #24

**Type sublanguage: +1 production** (a value parameter declaration in a generic parameter
list). **Zero keywords and zero operators**, which is [#24](https://github.com/adamico/ludo/issues/24)'s
binding unit — `:` is already the core grammar's annotation operator, `usize` is an existing
type name, and the bracket list already exists. The core grammar is untouched, and the
quarantine #24 call 3 fences is respected: this lands wholly on the type side of the line.

It is **semantics-bearing, not sugar**, so under #24 call 5 it pays by naming the failure class
it deletes, permanently: **hand-maintained cursor arithmetic over a fixed buffer.** ADR-0043 §5
wrote that idiom out in full —

```ludo
written := format(name![name_len..<32], "{}", typed)
name_len += written.len
```

— and rejected it on sight as `snprintf` plus offset arithmetic, the one line in the whole text
story a Lua user would stop at. Its bugs are the classic ones: the off-by-one at the boundary,
the cursor advanced by the wrong quantity, the truncation nobody checked. Without an integer
parameter the alternatives are that idiom or a single spec-chosen capacity — the spec deciding
every game's name-field length.

**No new [#19](https://github.com/adamico/ludo/issues/19) property.** This is a compile-time
type rule; every conforming implementation demonstrates it by compiling the reference program
or failing to. P1–P15 stands.

## 6. What is refused, so nobody relitigates it

- **Value parameters of any other type** — `bool`, enums, floats, `string`. Each has a real
  use (a `Vec[3, f32]`-shaped API, a flag-specialised container) and each reintroduces exactly
  what the fence exists to prevent: `bool` parameters are specialisation with a different face,
  and #11 rejected specialisation.
- **Arithmetic on a parameter in type position** — §1 restriction 2, the decidability fence.
- **Reading a parameter as a value** — §1 restriction 3; `buf.bytes.len` already answers it.
- **Blessing `TextBuf` as a compiler-known type** instead of widening user-declared generics.
  This was live and is rejected on #15's own precedent: `List[T]` and `Map[K,V]` are ordinary
  library generics precisely so that nothing in the container story is magic a user cannot
  imitate. A `TextBuf` a beginner could use but not write would be the first aggregate on the
  map with that property, and [#4](https://github.com/adamico/ludo/issues/4)'s naming-failure
  finding punishes exactly the stdlib-shaped thing with no user-facing rule behind it.
- **Refusing outright**, withdrawing `TextBuf`. This was the ticket's stated fallback and it
  disturbs nothing else in ADR-0043 — which is what made it genuinely available. It loses to
  the failure class in §5.

## 7. Errors

An argument that is not const-evaluable, not a `usize`, or that overflows, is a **compile
error** — ADR-0021 §5's rule applied unchanged: never a wrapped value, never a
[#18](https://github.com/adamico/ludo/issues/18) fault.

`N = 0` is **legal and has no special case.** `TextBuf[0]` is a buffer that truncates
everything appended to it; `[0]u8` is already a legal fixed array under #15. A rule forbidding
it would be a rule to write, test and explain, in exchange for outlawing a program nobody
writes by accident.

## 8. The reference program line

Per ADR-0044 §7, the line that cannot be written without this decision:

```ludo
persist name: TextBuf[32]
```

— the text-entry program of ADR-0043 §5, which enters the reference program with chapter 3.

## The three lenses

- **Simplicity — positive at the use site, one cost at the declaration.** `TextBuf[32]` reads
  as *a text buffer of 32* to anyone, and a Lua user meets only that form; capacity in a
  bracket is not a concept, it is a number. The cost is real and lands on the author of a
  generic: `[N: usize]` is a second thing that can appear in a bracket list, and the rule *no
  arithmetic there* has to be learned when it bites.
- **Robustness — the decision's whole justification.** It deletes the cursor-arithmetic class
  in §5 outright, and it puts capacity in the type, where a mismatch is a compile error rather
  than a truncation nobody notices.
- **Agent-friendliness — positive, with the same conflict ADR-0043 named.** Capacity is visible
  at every mention of the type, which is R1, and there is no constructor to call and no
  capacity argument to get wrong. Against it: an agent carrying Rust habits will write
  `[N+1]u8`, or `TextBuf[N*2]` inside a generic body. Both are **compile errors, not silent wrong
  answers** — the same shape as ADR-0043's `string` conflict, and the same reason it is
  acceptable.
