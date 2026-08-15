---
status: accepted
---

> **Amended by [ADR-0050](0050-removal-names-its-cost-at-the-call-site-and-descending-iteration-earns-an-operator.md):**
> the grammar-budget count §4 charges `const` against is **recounted** (ADR-0050 §13, §14): `>..`
> takes the core total to +30.9% of Lua 5.4, the first recorded overrule. §4's charge and every
> clause of §1–§5 are unaffected.

# Const evaluation folds an expression and never runs a program

Issue #55 asked whether the language ships compile-time execution at all, and
framed the surviving justification as the **computed module constant** — a
library wanting a lookup table it cannot build at load time, since issue #26
established that an imported module's top level does not execute.

The walls were already up. Issue #5 criterion 3 forbids comptime generating
identifiers; criterion 5 forbids failures the compiler cannot attribute to a
named source location. Issue #11 monomorphises generics and bounds them by
constraints, so comptime has no type-level job left. What #55 had to decide was
whether anything remained.

**Two observations reframed the question**, and both point the same way.

First, **the floor is already forced by decisions taken elsewhere.** Issue #15
ships fixed-size arrays `[N]T` and issue #25 ships `#align(n)`. Both are
constant-required positions. Unless every array length in the language is a
decimal literal — `[64]Enemy`, never `[MAX_ENEMIES]Enemy` — the compiler already
evaluates expressions to constants. "Does const-eval exist" was answered by prior
work; the only open question was how far past the floor it goes.

Second, **the computed lookup table does not survive contact.** It is the case
the whole feature was being justified by, and it dissolves into cheaper answers.

## 1. Const evaluation ships, at the floor and no further

The evaluable subset is **literals, other constants, arithmetic, comparison and
bitwise operators, and casts.** Nothing else. In particular:

- **no calls**, not even to functions the compiler could prove pure;
- **no loops**, no recursion, no `if`/`match` selection;
- no allocation, no I/O, no access to anything the running program would have.

The prize is structural, not aesthetic. **With no calls there is no recursion,
so non-termination is impossible by construction.** The compiler never needs a
step limit, a fuel counter, or a story for "the const evaluator hung" — and #5
criterion 5 is discharged without one, because the failure it exists to prevent
cannot be expressed.

Pure-function calls were the tempting concession and were rejected on cost.
Purity would become an inferred, propagating property the type system has to
carry on every signature, and the moment a call is const-evaluable an agent will
write a recursive one — at which point the termination story comes back and the
step limit with it. `[MAX_ENEMIES * 2]Enemy` is the whole of the demand, and it
needs none of that.

## 2. The computed module constant is not the job — it is ruled out

A library wanting a 256-entry sine table has three answers, and the language owes
it none of them:

- **write the literal** — the table is data, and data belongs in source;
- **generate the source file** with a script, which is what C, Odin and Zig users
  do in practice and which keeps the generator in a language built for
  generating things;
- **build it at run time in the entry file's top level**, which #26 explicitly
  says *does* run, and hold it in `persist`.

The third costs the language nothing new. The first two cost it nothing at all.
None of them requires a second interpreter inside the compiler, which is what
loop-running const evaluation is — and a second interpreter is a second set of
semantics to specify, to test for conformance under #19, and to keep bit-identical
with the first under #51's "float evaluation exactly as written".

**Stated as a non-goal so it stops being re-litigated:** ludo has no macro
system, no comptime blocks, no compile-time code generation, no compile-time
reflection, and no compile-time function execution. It folds constant
expressions. That is all it does, and the list above is the whole reason.

## 3. Library modules may declare constants; #26 bans execution, not declaration

Issue #26 currently reads as though an imported module can hold nothing at its
top level. That is one claim too strong, and #55 separates the two.

**A constant declaration is a compile-time entity, not a load-time side effect.**
The compiler folds it and substitutes the value; nothing runs when the module is
imported, so #26's guarantee is untouched. A library may therefore declare
constants, and its consumers may use them in constant-required positions.

## 4. `const` is a keyword, and the grammar budget is charged for it

Without a keyword, `MAX = 64` in a library and `world = build_world()` in the
entry file are the same spelling under two different rules, told apart only by
which file you are reading. That is uncomfortably close to #5's no-dialect-variance
criterion, and an agent reading a snippet cannot tell which regime it is in.

So `const` is spelled:

```
const MAX_ENEMIES = 64
const POOL = MAX_ENEMIES * 2

enemies: [POOL]Enemy
```

One meaning everywhere — a binding whose initialiser is const-evaluable per §1,
usable in any constant-required position. The entry file's runtime top-level
bindings keep the plain form and gain nothing and lose nothing.

Two cheaper options were on the table and were declined:

- **No keyword, const-ness inferred from position.** Cheapest against #24 and
  rejected on locality: R1 wants the property visible at the declaration, not
  recovered from the file's role.
- **Delete the variance instead** — require *all* top-level bindings everywhere
  to fold, pushing the entry file's computed state into `persist`. Cheaper still,
  and it breaks #26's whole point: the entry file's top level exists precisely to
  build things.

**Against issue #24:** this is one keyword added to the core grammar count, paid
as a feature rather than as sugar, with the locality argument above as its
justification. It adds nothing to the type sublanguage and nothing to the
operator count.

## 5. Failure is a compile error, never a wrapped value and never a fault

Non-termination is off the table by §1. What remains is division by zero, integer
overflow, a negative or over-wide array length, and a lossy cast.

**Every one of them fails the build**, as an ADR-0018 `compile`-producer
diagnostic with a source location. They are not a #18 fault — there is no running
program to fault — and they do not wrap or saturate silently.

The rule that generalises this, and the one an implementer should hold onto:

> **Const evaluation never produces a value the same expression would not produce
> at run time, and never produces one at all when the operation is ill-defined.**

This keeps const-eval from becoming a back door to different arithmetic than the
language's own, which #51 fixed as "float evaluation exactly as written".

## The three lenses

- **Simplicity.** A Lua user reads `const MAX_ENEMIES = 64` correctly on sight.
  There is no second language to learn, because there is no second language:
  const-eval has no syntax of its own, only a restriction on which expressions
  are allowed where.
- **Robustness.** Named array lengths are the point — `[MAX_ENEMIES]Enemy`
  compared against `[MAX_ENEMIES]Health` is a shape the compiler can check and a
  reader can verify. Division by zero and overflow are caught at build time
  rather than shipped.
- **Agent-friendliness.** The keyword makes the property local (R1). The absence
  of macros and generated names keeps every identifier greppable, which is
  directly what #4's evidence says agents fail at — naming things that exist.
  Criterion 5 is satisfied by construction rather than by policy, so there is no
  diagnostic-quality cliff for an agent to fall off.

**Where the lenses conflict:** they do not, here. The one real tension was
between #24's grammar budget and #5's locality criterion, and §4 spends the
keyword and says why.
