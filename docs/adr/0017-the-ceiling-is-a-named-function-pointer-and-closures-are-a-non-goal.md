---
status: accepted
---

> **Absorbed by [spec ch2](../spec/02-types.md):** the chapter is normative; this ADR
> keeps the argument — the function-pointer type, the interface-bounded generic ceiling and the rejection of closures, function literals and UFCS on function values are transcribed there.

# The ceiling is a named function pointer, and closures are a non-goal

Issue #52 asked whether anything exists above a bare non-capturing function
pointer — capture, closure, bound receiver. The floor was already fixed: ADR-0006
R2 makes the backend seam one struct of non-capturing function pointers, because
a `dlopen`'d backend and a linked web backend must present the same interface and
a resolved symbol *is* a function value, and issue #29 already permits that shape
for callbacks out to C.

The ticket framed the surviving case as *transient* — a comparator passed to a
sort, a predicate passed to a filter — and framed the trade as explicit-context
verbosity versus capture. **That framing was one observation short.** The
transient case is already expressible with zero new features, which changes what
the ceiling question is even asking.

## 1. Nothing above a non-capturing function pointer is added

**Closures are a stated non-goal.** So are bound receivers and any form of
capture.

Issue #11 ships nominal interfaces with explicit satisfaction, and generics that
are monomorphised and constraint-bounded. That is already a higher-order
mechanism:

```
sort[T, C](xs: ![]T, cmp: C) where C: Compare[T]

sort(xs, ByDistance{ from = player.pos })
```

Statically dispatched, no boxing, nothing allocated — and **the captured state is
the struct's named fields**, visible in the literal at the call site. Capture was
the only thing a closure was going to add, and this supplies it under a name.

Four levels were on the table:

- **(a) function pointer plus an explicit generic context parameter** —
  `sort[T,C](xs, cmp: fn(T,T,C) -> int, ctx: C)`, C's `qsort_r` made type-safe
  with no `rawptr` and no `unsafe`. Rejected only against (b), and narrowly: it
  costs a second argument on every higher-order signature and makes the context a
  positional blob rather than named fields.
- **(b) interface-bounded generics** — adopted. One argument, context as named
  fields, and the mechanism already exists so nothing is spent against #24.
- **(c) non-allocating capture-by-value closures** — the tempting one, and
  rejected deliberately rather than skipped. It is (b) with the capture struct
  written for you, which means **the compiler generates a type**. That is what
  #5 criterion 3 (no macro-generated names) and #22 (names knowable without
  expansion) exist to prevent, and the convenience does not buy a failure class
  back.
- **(d) closures with an allocator** — dead on arrival. The escaping case an
  allocator exists for does not exist here: #8 forbids stored write access and
  #17 forbids a function being reachable from `persist`.

**Corroboration, not argument:** every systems language in this map's comparison
set declines closures — Odin, Zig and Jai — and Jai is the one that most
aggressively adds convenience elsewhere. Four independent designers under
near-identical constraints reached the same call.

## 2. The type is spelled `fn(T, U) -> R`

One production in the **type sublanguage**, which #24 fences and counts
separately from the core grammar.

It is the declaration form with the name removed, so there is nothing new to
learn and anyone who can write a signature can write the type. A named generic
`Fn[T, U, R]` would have saved the production, but it buries the return type in a
positional slot and reads badly at arity three and above — a readability loss for
a saving the separate fence was designed to make unnecessary.

## 3. A `fn` type satisfies a single-function interface, by a blessed rule

`sort(xs, compare_ascending)` compiles: a `fn` type nominally satisfies an
interface that declares **exactly one** function, when the signature matches
**exactly**.

Without this, the *stateless* case — the common one — is the most ceremonious:
a zero-field struct plus a satisfaction declaration, to pass what is morally just
a function. That is #6's beginner-verbosity conflict landing on the wrong side,
and it would push people toward informal (a)-style context parameters anyway.

This is **not structural satisfaction returning**. #11 rejected structural
satisfaction because it makes satisfaction *accidental* — any type that happens
to have the right shape silently qualifies. This is a fixed rule about **one
built-in type**, in the same class as ADR-0016 §3's operators on the blessed math
types, #15's derived `Eq`/`Hash`/`Clone` and #25's SoA transform: the **fifth use
of the privileged-compiler-knowledge shape**, not a new mechanism.

Stated cost: one call site now accepts two visibly different things — a struct
literal and a bare function name. Bounded, because both are static, both
monomorphise, and the interface still names what is required.

## 4. Every function is top-level and named — no anonymous function literals

Issue #29 restricted C callbacks to top-level, named, non-capturing functions for
ABI reasons. **That restriction generalises into a language rule.**

An inline `fn(a, b) -> int { ... }` expression is non-capturing and perfectly
implementable, and it is what would make a one-off comparator cheap. It is
rejected on three counts:

- An anonymous function has **no entry in the name table**, so #22's oracle cannot
  answer *what is this* about it, and a fault inside it has no named entity to
  attribute to (criterion 5).
- #4's evidence is that agents fail at **naming things that exist**. That is an
  argument for forcing a name, not for supplying a way to skip one.
- Under #24 it is a new **core-grammar** production and it is semantics-bearing,
  so it must name a failure class it deletes. It deletes none.

**The cost is real and is stated rather than minimised:** a one-off comparator
must be lifted to a named top-level function, which is friction in exactly the
case §3 just made cheap. Recorded as a #24 rejection on the budget's own terms.

## 5. Generic functions become values only when explicitly instantiated

`compare[int]` is a value of type `fn(int, int) -> int`. Bare `compare` is a
compile error naming the missing type argument.

A generic function has no single address; only its instantiations do. **No
inference from the expected type** — deducing the instantiation from a target
function-pointer type is the kind of cleverness criterion 5 vetoes, and the
diagnostic here writes itself.

## 6. UFCS never applies to a function value

If `cmp` is a local holding a `fn(T, T) -> int`, then `a.cmp(b)` is not legal.
Calls on function values are always `cmp(a, b)`.

#11's rule already does not reach this case — `a.f(b)` resolves to `f` declared in
the module declaring `a`'s type, and a local variable is not a declaration in any
module. What this section does is decline to *add* the case: admitting it would
make `a.cmp(b)` mean one thing or another depending on whether a local shadows a
module function, which is the name-origin ambiguity R1 exists to prevent.

It is stated explicitly because #16 made UFCS pervasive, so its **boundary** is
the part an agent needs to know.

## 7. The stdlib ships no `map`, `filter` or `reduce`

`for` is the one spelling for a loop.

Without closures, `xs.map(Double{})` is worse than the loop it replaces in nearly
every case. Combinator style is ergonomic *because* of closures, which is why
Odin and Zig, having none, also ship no combinator library. Shipping them anyway
produces a second spelling for a loop (criterion 3) that is longer than the first.

**The rule, stated so `map` stops being re-proposed: a higher-order stdlib
function must contribute an algorithm, never just a loop shape.** `sort`,
`binary_search` and `partition` qualify — each encodes something a loop does not.
`map` and `filter` do not.

## 8. A third-party combinator library is possible, unblocked and unpleasant

This is recorded because it is the honest reason §7 declines them — not a claim
that nobody could want them.

**Mechanically it works.** A lazy adapter struct `Mapped[I, C]` holding a source
iterator and a transform, satisfying #15's nominal `Iter[U]`, monomorphises with
static dispatch and no boxing. An eager version is
`map(allocator, xs, f) -> List[U]` per #8. No privileged access is required and
nothing in the language blocks it.

**Two things that make combinators pleasant are absent, and both were decided
elsewhere for other reasons:**

- **No UFCS extension of foreign types.** #11 resolves `a.f(b)` in the module
  declaring `a`'s type *and nowhere else*, and `[]T`, `List[T]` and `Iter[T]` are
  stdlib types — so a third-party library **cannot** write `xs.map(...)`. It gets
  prefix `seq.map(xs, F{})` for the first step. Chaining partially recovers,
  because the adapter type is *theirs*: `seq.map(xs, F{}).filter(G{})` works,
  `filter` being declared in the module that declares `Mapped`. The result is a
  first-call-is-prefix, rest-chains asymmetry.
- **No closures**, so every stage costs a named struct or a named top-level
  function.

## 9. The lenses

**Simplicity — strong, by subtraction.** There is no closure, no capture list, no
capture-by-value-versus-reference question, no lifetime question about a captured
value, and no lambda syntax. A beginner meets one concept — a function has a name
and you can pass it — and the interface path is the same generics they already
meet everywhere else.

**Robustness — moderate.** It deletes no bug class outright, and does not claim
to. What it does is refuse three features whose failure modes the map has
committed to not having: a generated type name (§1c), an unnameable entity in a
diagnostic (§4), and an inferred instantiation (§5).

**Agent-friendliness — strong, and this is where the section earns itself.**
Every callable entity has a name in the name table, which is the direct mitigation
for #4's 43.7% naming failure. There is no anonymous construct the oracle cannot
answer questions about. §3's blessed rule keeps the common case writable without
introducing a second way for satisfaction to happen silently.

**The named conflict:** §4 taxes the beginner, and §3 is the softener, so this
ticket lands on both sides of #6's beginner-versus-agent axis in the space of two
sections. The tax is accepted deliberately.

## 10. #24 delta

- **Type sublanguage: +1 production** — `fn(T, U) -> R` (§2).
- **Core grammar: unchanged.** §5's `compare[int]` is the existing generic
  instantiation syntax in expression position, not a new form.
- **Recorded rejections on the budget's own terms:** anonymous function literals
  (§4, semantics-bearing, deletes no failure class) and a UFCS case for function
  values (§6).
- **Stdlib root names: none added.** §7 ships nothing.
