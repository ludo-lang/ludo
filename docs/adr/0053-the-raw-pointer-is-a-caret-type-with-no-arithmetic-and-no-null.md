---
status: accepted
---

# The raw pointer is a caret type with no arithmetic and no null

[#104](https://github.com/ludo-lang/ludo/issues/104), found while transcribing spec chapter 3
([#87](https://github.com/ludo-lang/ludo/issues/87)). The corpus mandated a type it never spelled:

- [#8](https://github.com/ludo-lang/ludo/issues/8) call 6 — *"Raw pointers are a **distinct,
  greppable type**; `&x`, deref, manual alloc/free are legal only inside `unsafe`."*
- #8 §8's guarantee table, transcribed at chapter 3 §17, is stated against that type existing
  (*"Anything in `unsafe` — no guarantees"*).
- Chapter 3 §16.2 transcribes what `unsafe` permits and has to name the type.

And no source spelled it. `grammar.ebnf` had no pointer production; chapter 1 §9's type sublanguage
admitted `!T`, `?T`, `[N]T`, `[]T`, `fn(...) -> R`, `T rescue E` and `Name[Args]`, none of them a
pointer. `&x` had no expression production either — no prefix `&` in `UnaryExpr`, no dereference in
`Suffix`.

Chapter 3 §16.5 declined to repair it in place, and chapter 2 §19.3 is the reason: a pointer type is
a type-sublanguage production plus an expression production, which chapter 1 §13.8 charges and
§13.6, §13.7 and §13.12 publish. A repair that moves a published budget is a decision, so
[ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §6's
repair-in-the-spec-text rule does not reach it. **This is that ADR.**

## 1. There is one pointer type, and it does no arithmetic

`^T` is the whole family. It does not index, it does not offset, and **there is no many-pointer**
(`[*]T` in Zig's split, which distinguishes the single-element pointer from the one you may walk).

The many-pointer's job is *an address plus a separately-tracked length*, and that is what a view
already is. Chapter 3 §9.6 makes the point concrete: `alloc` and `grow` return **`?[]u8`**, not a
pointer, so the case a many-pointer exists to serve — a container holding heap storage — is served
by the type the allocator already hands back. Adding `[*]T` would buy a second production, a second
set of index rules and a second thing to explain, to serve a case that has a type.

**Offsetting moves to the integer route** (§5): cast to `usize`, do the arithmetic, cast back. This
is chapter 3 §8.4's argument for declining bit fields, applied to addresses — *a program writes the
shifts* — and the element size becomes visible code rather than a scale factor hidden in an
operator.

**Named cost: an offset computation is three lines rather than one, and the author supplies the
element size.** Inside `unsafe`, where §16.3 already says there are no guarantees, that visibility
is worth more than the brevity.

## 2. The sigil is `^`, and the dereference is postfix

`^` is the one glyph the language had not spent. The alternatives each make one glyph denote two
entities, which is the ground chapter 1 §9.7 rejected infix `!` on and §14.8 rejected Lua's `~=` on:

- **`*`** is multiplication (chapter 1 §13.6), so a prefix-`*` dereference sits in the expression
  grammar beside a binary `*`.
- **`~`** carries both xor and unary complement already (chapter 1 §7.4).
- **`!`** is the mutation mark, and §4 below depends on it staying exactly that.

The cost is real and worth stating: **`*T` is what a C-fluent veteran expects**, and `^T` will read
as unfamiliar to the audience most likely to write it. Accepted, because this is `unsafe`-only
surface, and a spelling that makes a reader look twice is doing the job #8 §6 asked for when it said
*greppable*.

**The dereference is the postfix suffix `p^`** (chapter 1 §7.10b), not a prefix form. It is an
alternative inside the existing `Suffix` production, so it chains left to right with the suffixes
the language already has — `p^.field`, `p^[i]`, `p^.f(x)` — where a prefix `^p` would force
`(^p).field` the moment a field follows. It also costs no production, by the same rule `>..` did
not (chapter 1 §13.12).

## 3. Address-of is prefix `&x`, over places only

#8 call 6 writes `&x`, and `&` collides with nothing in prefix position — it is the bitwise
conjunction, binary only. It is a new alternative in `UnaryExpr` over a terminal the budget has
already paid for, so it costs **zero** operators (chapter 1 §13.6.2).

**The operand MUST be a place.** `&f()`, `&(a + b)` and `&literal` are compile errors naming the
expression, with the fix named: *bind it first*. §16.3's *no guarantees inside `unsafe`* is a
statement about what an address lets you do; it is not a licence for the compiler to materialise
storage for a temporary and then owe somebody a lifetime for it. This also keeps the round trip
symmetric — `&` takes a place and `^` yields one — which is what makes §4 work.

## 4. It carries no `!`, and it is not nullable

**No `!` in the type.** [#11](https://github.com/ludo-lang/ludo/issues/11) Q3 makes `!` a property
of the place, not a type constructor, and chapter 1 §9.2 states it: `!T` and `T` are the same type.
Spelling a mutable pointer `^!T` would make `!` a type constructor for exactly one type, and every
reader would have to learn that the mark means one thing in the language and another in `unsafe`.

Instead **the place rule reaches through the dereference**, because `p^` is a place and chapter 1
§7.10 already puts the mark on the root binding with the suffix chain following it. So writing
through a pointer is **`p!^ = v`**, and `p^! = v` is an error by §7.10a with the fix already named.
The pointer needed no rule of its own.

**Not nullable.** [#9](https://github.com/ludo-lang/ludo/issues/9) makes every type non-nullable and
`?T` the only absence concept, and chapter 2 §9.1 says there is no null of any spelling. The raw
pointer is **not** the exception. A maybe-absent pointer is `?^T`, paying chapter 3 §8.5.1's
always-present tag inside `unsafe` as everywhere else.

The carve-out was tempting and is worth less than it looks. §8.5.1 already committed to no niche
optimisation, so `?^T` is a tag plus an address and its size is computable from the declaration
alone — which is exactly the property §8.6 promises [#29](https://github.com/ludo-lang/ludo/issues/29)
at the C boundary. A null pointer would put a value into the language that §9.1 says does not exist,
and #29 would then have two absence concepts to marshal instead of one. Chapter 2 §9.1.1 states the
negative explicitly rather than leaving it to inference, because a reader who knows C arrives
expecting a null and a silence would read as an oversight.

## 5. Three casts, `unsafe`-only

`^T as usize`, `usize as ^T`, `^T as ^U`. Each is total in chapter 2 §2.9's sense — reinterpreting
an address never faults — and each requires the block. `usize` is the landing type because chapter 2
§2.4 defines it as the width of an address and nothing else in the language is.

These exist because §1 removed arithmetic. Without them §1 would have deleted a capability rather
than relocating it. Every other pointer cast is declined on chapter 2 §2.9.7's ground — `^T as bool`,
`^T` to a fixed-width integer, anything from a float — because declining a spelling is cheaper than
defining a total mapping nobody should write. `^T as ^U` is not optional: #29's boundary needs it.

## 6. The budget, and why core is charged

**One core operator. 40 → 41, total 73 → 74.** Against Lua 5.4 the overrun widens from +32.7% to
**+34.5%**; against LuaJIT, from +49.0% to +51.0%. Recorded as chapter 1 §13.9.1 **crossing 3**.
Production counts do not move: all three forms are alternatives inside productions that already
exist.

`^` appears in both grammar sections — `^T` in the type sublanguage, `p^` in the core grammar — and
chapter 1 §13.5 rule 4 counts such a terminal once, in the section that introduces it. **Core is
charged**, so the type-sublanguage figure stays at 6 and the headline percentage is the one that
moves. Charging it to the type sublanguage would have protected the comparison table, and that is
precisely the *"putting it in the cheaper bucket to protect the percentage"* move §13.9.1 crossing 2
refused for `impl`. Every other both-sections terminal is already charged to core, and `!` is the
exact analogue.

**The failure class deleted**, as chapter 1 §13.8 tier 2 requires a semantics-bearing production to
name permanently, is **the address laundered as an integer**. Until this crossing, a container
writing `unsafe` internals had one place to keep an address: a `usize` field. `usize` is also the
type of every length, index, capacity and offset in the language, so an address stored in one is
indistinguishable **by type, by grep and by eye** from arithmetic that must never be dereferenced.
There is no diagnostic and no crash. The bug is the **default outcome** in crossing 1's sense — the
author is not picking a lax spelling over a strict one, because the strict one did not exist. #8 §6
asked for *distinct and greppable* and the corpus mandated it while providing nothing to be distinct
from; this is the operator that makes the demand true.

## 7. What this ADR stamps and what it leaves

**Stamped:**

- **#8 call 6's raw-pointer clause is now spelled**, at chapter 1 §9.4a, §7.10b and §7.10c. #8 is
  otherwise unaffected and remains authoritative for the rest of the `unsafe` design.
- **Chapter 3 §16.5's decline is closed**, and struck rather than deleted so a reader arriving from
  §19.3 or §20 can see the boundary moved.
- **Chapter 3 §16.2's first two bullets are superseded** by the spelled forms, and gain a third for
  the casts.

**Left open, and not this ADR's:**

- **The spelling of uninitialised memory**, §16.2's remaining unspelled permission. It looks
  adjacent — same clause, same block — and it is a different mechanism: a question about the
  initialisation obligation under #9, reaching chapter 3 §5.7 and §5.9 and chapter 5 §4.3.1, none of
  which the pointer touches. Bundling would put two failure classes in one §13.9.1 row, which
  crossing 2's *one mechanism, one register row* reasoning refuses. Filed as
  [#118](https://github.com/ludo-lang/ludo/issues/118).
- **A view's runtime representation.** Now that a pointer type exists it is expressible as
  `{ ptr: ^T, len: usize }`, and this ADR declines to say so. Chapter 3 §8.6 hands view marshalling
  to #29 by name, and §8.5.1's argument holds: a representation C depends on cannot be un-fixed.
  Chapter 3 §16.2.3 records the silence as deliberate.
- **Whether a blessed container's implementation is spec text at all.** #104 noted this is the point
  at which the pointer stops being optional. It is still not answered here.

## 8. What this is not

This does not soften the safe layer, and nothing outside `unsafe` changed meaning. The guarantee
table at chapter 3 §17 is untouched — it was always stated against a pointer type existing, and it
now names one. No safe-layer program can produce a `^T`: `&x` and the casts require the block, and
the only way to obtain one otherwise is from a declaration a program wrote itself.

It also does not make the pointer the container primitive. Chapter 3 §9.6's `?[]u8` is still what
`alloc` returns, §9.4's *release is `reset()`* still stands, and the encapsulation shape at §16.4 is
unchanged. What the pointer buys is a **name** for what a container was already holding.
