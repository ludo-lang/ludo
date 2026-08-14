# Chapter 3 — Memory and aggregates

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where this chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under chapter 7's envelope (ADR-0018).

**0.3** This chapter owns **memory and the aggregates that live in it**: the
memory model, moves, the two core aggregates and their literal, views, view
invalidation, layout, allocators, handles and pools, container semantics,
`defer` and must-use, and what `unsafe` permits. [Chapter 1](01-grammar.md) owns
the spelling of every form named here, and [`grammar.ebnf`](grammar.ebnf)
remains the authority on syntax. [Chapter 2](02-types.md) owns what a type
*means*; this chapter owns where its bytes are and how long they last.

**0.4** This chapter does not own the **C boundary** (#29), the **facade
signatures** that take or return the types named here (chapter 6), `persist`
and reload (chapter 5), or the **diagnostic envelope** (chapter 7). Where a
clause reaches one of them, it names the routing.

**0.5** Every clause cites its source. A clause with no citation is a defect.

**0.6 A note on two numbers.** This chapter's layout source is
**[issue #25](https://github.com/ludo-lang/ludo/issues/25)**, *Decide how a type
declares its memory layout*. It is **not** ADR-0025, which is
*Fullscreen is a player preference* and has nothing to do with memory. The
corpus conflates them in four places; see
[`coverage/03-memory.md`](coverage/03-memory.md) §0. Every layout clause below
cites **#25**.

---

## 1. The memory model

**1.1** There is **no tracing garbage collector**. (#8, which excludes it on
frame pauses.)

**1.2** There is **no borrow checker, no ownership-and-borrowing discipline, no
lifetimes, no regions and no aliasing analysis** anywhere in the language. (#8;
#2, which found a borrow checker does not survive gamedev contact; chapter 2
§1.7.)

**1.3** **Safety comes from how a program is normally written, and is checked
while it runs.** It is not a compile-time proof. The two mechanisms are
**generational handles** for lasting links (§10) and **arenas and pools** for
allocation (§9, §10). (#8 §1.)

**1.4** **There is no build-mode behaviour switch.** Bounds checks are always
on, and integer overflow is always checked (chapter 2 §8.3). An implementation
MUST NOT make either depend on a build mode, an optimisation level or a flag. To
skip a bounds check a program writes `unsafe` **in the source** (§16). (#8 §8(a);
ADR-0023 §2, which makes accept/reject flag-invariant.)

**1.5** **Leak detection is a swapped allocator, not a hidden mode.** An
implementation MUST NOT provide leak detection as a build mode; a checker
allocator is an ordinary value a program passes where it would have passed any
other. (#8 §8(b); ADR-0042 §9.)

**1.6 Allocation by absence.** There is **no ambient allocator, no implicit
context, and no global `new`**. A function that allocates MUST have received an
allocator as a parameter. The single exception is `heap` in a `persist`
initialiser in the entry file (§9.12). (#8, the #5 criterion-1 constraint;
ADR-0042 §8.)

**1.7** **No builtin container grows implicitly.** Growth names an allocator at
the growth point, always. (#8, the constraint that makes §1.6 hold; #15.)

**1.8** **Memory leaks are detected, not prevented.** This is the one hazard in
§17's table that the safe layer does not close, and it is the stated price of
having no GC. (#8 §8.)

---

## 2. Values, references, and in-place mutation

**2.1** **There are no raw pointers in the safe layer.** `&x`-and-store does not
exist, and no safe-layer construct produces a storable reference to a place.
(#8 §3.)

**2.2** **Passing a value to a function is by value.** An implementation MAY
pass a large value by hidden reference where the difference is unobservable;
this is a permitted optimisation and never a semantic difference. (#8 §3.)

**2.3** A **lasting link between values is a generational handle** into a pool
(§10), never a stored reference. (#8 §1, §3.)

**2.4** **In-place mutation happens only through transient, marked,
non-escaping access**: a function's writable parameter, or a pool's scoped lend.
The access is marked at the call site with `!` (chapter 1 §7.10), and **write
access MUST NOT be stored** or outlive the moment it was granted. (#8 §4;
#11 Q3.)

**2.5** `!T` and `T` are the same type, differently accessed; `!` is not a type
constructor (chapter 2 §12.1, chapter 1 §9.2). Nothing in this chapter converts
between them, because there is nothing to convert.

**2.6** **There is no destructor and no scope-based destruction.** Cleanup is
`defer` (§15). (#8 §5.)

---

## 3. Moves

**3.1** **A container moves on assignment, and containers are not copyable.**
(#15 Q14.)

**3.2** **Use of a moved-from binding is a compile error naming the binding and
the move site.** The check is shallow — *was it moved?* — and is **not lifetime
tracking**; it is the same shape as must-use (§15.3). (#15 Q14.)

**3.3** **Move-only is transitive and has no marker to write.** A struct with a
move-only field is move-only, and the compile error names the field and its
path. The property is **inferred within the signature radius**, so locality
holds. (#15 Q20.)

**3.4** **Duplication is an explicit `clone(allocator)`; sharing goes through a
view** (§6). There is no implicit deep clone and no shallow alias. (#15 Q14.)

---

## 4. The two core aggregates

**4.1** There are **exactly two compiler-known aggregates**: the nominal
**`struct`** and the **fixed array `[N]T`**, whose length is part of its type.
(#15 Q1; chapter 1 §5.3, §9.4.)

**4.2** **There is no tuple type.** (#15 Q1.) The one construct that resembles
one — fixed-arity multiple returns — has no type at all (§14).

**4.3** **`List[T]` and `Map[K,V]` are library generics** over §4.1's pair plus
an allocator, not core aggregates. (#15 Q1.)

**4.4** Lua's one table is replaced by **one literal syntax, not one type**
(§5). The promise is kept at the surface and paid for in the type system.
(#15.)

---

## 5. The literal `{...}`

**5.1** **One literal form `{...}` serves every aggregate, and the expected type
decides its meaning.** `{x = 1}` is a struct or a `Map[string, int]` according
to the target. (#15 Q2, Q13.)

**5.2** **The expected type comes only from the signature radius** — a
parameter, a declared binding, a field, or a return type. It is **never** whole
-program inference. (#15 Q2, Q22; chapter 2 §11.4.)

**5.3** **A bare `{...}` with no expected type is a compile error naming the
missing entity.** An implementation MUST NOT default it. (#15 Q3.)

**5.4** **A `{...}` literal is a fixed-size value and does not allocate.** This
is what lets a beginner write `{1, 2, 3}` with no allocator decision while
§1.6's allocation-by-absence holds. (#8 §2.)

**5.5** Because construction *is* growth, **`l: List[int] = {1, 2, 3}` is a
compile error naming what is missing**. A `List` is constructed from a literal
by handing it to something that owns storage. (#15; #8 §2.)

**5.6** **Struct literals are named-field only.** There is no positional struct
literal. (#15 Q8.)

**5.7** **Field defaults are permitted, and a literal may omit only defaulted
fields.** (#15 Q21; chapter 1 §5.3; chapter 2 §9.6, which reconciles this with
#9's prohibition on an *implicit* zero.)

**5.8** **`{[expr] = v}` is the computed- or non-string-key form.** (#15 Q2.)

**5.9** **Literals nest recursively**: `grid: [2][2]int = { {1, 2}, {3, 4} }`.
(#15 Q2.)

---

## 6. Views: `[]T`

**6.1** **`[]T` is a view: a non-owning, non-storable second name for a place.**
It is the **universal boundary** — a function written against `[]T` accepts a
`[N]T`, a `List[T]` and an SoA column (§10.6) alike, with no cross-container
API. (#15 Q10, Q23.)

**6.2** A view is **not new machinery**: it is #8 §4's lend, and `!` marks it
writable. (#15 Q10.)

**6.3** **A view MUST NOT be stored.** It MUST NOT be a struct field, MUST NOT
be a `persist` declaration, and MUST NOT be captured by a container. Each is a
compile error naming the field or declaration. (#15 Q10, unchanged by
ADR-0047 §2.)

**6.4** **A local binding is a legal view position.** #15 Q10's *"parameter
position only"* means not-stored and not-escaping; it is not a ban on binding a
view to a local. (ADR-0047 §2.)

**6.5 The return rule.** **A function MAY return a `[]T` if and only if the view
is derived from the call's receiver, or from the runner's constant blob.** A
function with no receiver to derive from MUST NOT return a view, and doing so is
a compile error. (ADR-0047 §1.)

**6.6** Both sources in §6.5 are visible in the callee's own body, so **the rule
is checked, not asserted**, and the check requires no whole-program view.
(ADR-0047 §1.)

**6.7** The constant blob is admissible as a source because it **outlives every
frame, every reload and every image swap** — the only unconditional lifetime in
the language. A `string` reaches `[]u8` through chapter 2 §17.2's widening, so a
returned literal is a legal view. (ADR-0047 §1; ADR-0043 §1, §2.)

**6.8 Writability flows from the receiver.** **A returned view is writable if
and only if the operand it derives from was marked `!` at the call site.** There
is **no `-> ![]T` return-type marker**: marking the return type would permit a
writable view derived from an unmarked receiver. (ADR-0047 §5.)

**6.9** Two consequences of §6.8 are recorded here and owned elsewhere:

- **`get_pixels` is restamped `get_pixels(image: !Image) -> []u32`.** The facade
  is chapter 6's; the signature change is ADR-0047 §5's and is recorded here
  because §6.8 is what causes it. (ADR-0047 §5, amending ADR-0009 §111 and
  ADR-0022.)
- **`format(dst: ![]u8, ...) -> []u8`** is an instance of §6.5 with the receiver
  spelled `buf`, not an exception beside it. (ADR-0047 §8; chapter 2 §17.10.)

**6.10** **Indexing is 0-based** (chapter 1 §7.8), so **`xs.len` is always one
past the last valid index**, which is what makes §6.11's range form correct by
construction. (#15 Q4.)

**6.11** **Slicing is `xs[a..<b]`**, using the half-open range operator, and
there is no inclusive form (chapter 1 §7.7). A slice of a view, an array or a
column is a view. **Slicing is free and allocates nothing.** (#15 Q10, Q24;
ADR-0043 §4, whose *slicing is free* clause is #15's, inherited by `[]u8`
through the widening.)

**6.12** **A writable sub-view is `name![a..<b]`** — the mark rides the root
binding and the suffix chain follows (chapter 1 §7.10, §7.10a). **`name[a..<b]!`
parses but is a compile error**, with the fix named: *the mutation mark belongs
on the root: write `name![a..<b]`*.
([#101](https://github.com/ludo-lang/ludo/issues/101); ADR-0047 §5.)

---

## 7. A view dies when its origin is touched

**7.1 The rule.** **A view dies when the place it derives from is next used with
`!` or moved.** Use of a dead view is a compile error naming the view and the
mark that killed it. (ADR-0047 §3.)

```ludo
v := xs.items()
xs!.push(e)                 -- the mark kills v
$.print(v[0])               -- error: v was invalidated by `xs!` above
```

**7.2** **The check is flow-insensitive and spans the function body.** A view is
dead from the first `!`-use or move of its origin that appears later in the
body, **whichever branch that use sits in**. An implementation MUST NOT make
acceptance depend on control flow. (ADR-0047 §3.)

**7.3** **The kill fires at a call site too**, where the place is handed to a
function as a `!` operand: `helper(xs!, v)` invalidates `v` at that call.
(ADR-0047 §3.)

**7.4** **There is no exclusivity rule between views.** Any number of views of
one place MAY co-exist, readable or writable; only `!` or a move on the
**origin** kills them. Many-readers-or-one-writer is the borrow checker's rule
and is not this language's. #8 deleted aliasing at the **container** level, not
the view level. (ADR-0047 §3.)

**7.5 The edge chains through a container.** **A container constructed from a
place derives from it, and the same mark kills it.** (ADR-0047 §4.)

```ludo
l := List.from(scratch, ...)
scratch!.reset()
l!.push(e)                  -- error: l derives from scratch, killed above
```

**7.6** **A writable sub-view kills views of disjoint ranges of the same
origin.** `xs![0..<k]` kills a bound view of `xs[k..<n]`, overlapping or not:
the origin is `xs`, and §7.1 is unchanged and unextended. **No range reasoning
is required or permitted.**
([#101](https://github.com/ludo-lang/ludo/issues/101).)

**7.7** Consequently **two disjoint writable windows into one buffer cannot be
bound simultaneously**. This is a stated cost, not an oversight; the capability
is filed as [#102](https://github.com/ludo-lang/ludo/issues/102).
([#101](https://github.com/ludo-lang/ludo/issues/101).)

**7.8 What §7 is not.** It tracks a *derived-from* edge between two locals in
one body. Recorded as a closed list so it is not read as a borrow checker
arriving by instalments: **no lifetimes** (nothing annotated, named, quantified
or inferred), **no regions**, **no shared/exclusive distinction** (§7.4), and
**the check never crosses a function boundary** — every kill is a mark in the
body being compiled. It is the move check's shape (§3.2). (ADR-0047 §6.)

---

## 8. Layout

**8.1 Layout belongs to the container, not the type.** A `struct` declaration is
**layout-agnostic**: the same `Entity` is usable packed in one pool and columnar
in another, and no declaration selects between them. (#25 §1.)

**8.2 Declaration order is guaranteed and padding is C-compatible, always.** An
implementation MUST NOT reorder fields, and **there is no `#repr` attribute**.
**Every ludo struct is therefore C-ABI-compatible by construction.** (#25 §2.)

**8.2.1** The **named cost** is that a program orders its own fields, and a bad
order silently wastes memory. (#25 §2.)

**8.2.2** #25 §2's mitigation — *the compiler reports the padding and a better
order as a diagnostic* — is **not mandated by this specification**. A
spec-owned diagnostic is an error or it does not exist, and this one is not an
error, so it is **vendor-space**: an implementation MAY emit it, MUST NOT make
it the sole reason a program fails to compile, and no program's acceptance
depends on it. (ADR-0023 §1, §2, which is later than #25 and general; the
padding rule itself, §8.2, is untouched. Recorded in
[`coverage/03-memory.md`](coverage/03-memory.md) §3.)

**8.3 `#align(n)` is the one layout attribute, and it is type-level.** It marks
a type (chapter 1 §12.2). **There is no field-level `#align`** (chapter 1 §12.3)
and **there is no `#packed`**: a byte-exact external format is a serialisation
problem served by explicit reads and writes over a byte view, and packing buys
unaligned field access. A real C header that forces a packed struct is #29's
problem at the boundary, not a representation the language carries. (#25 §3;
ADR-0024 §6 and ADR-0029 §7 for the field-level decline.)

**8.3.1** `n` MUST be a const expression under chapter 2 §15.2's floor, MUST be
a power of two, and **MUST NOT be less than the type's natural alignment** — an
`#align` that reduced alignment would be `#packed` under another name, which
§8.3 refuses. A violation is a compile error naming the type and the value. The
rule is **authored by this chapter** — see §19.2.

**8.4 There are no bit fields.** Packing flags into a `u32` is expressible with
`distinct` plus accessor functions, where the shifts are visible code rather
than a layout rule. **Named cost: a program writes the shifts.** (#25 §4.)

**8.5 Sum-type layout is fully specified.** A sum type is laid out as **the tag
followed by the payload**; the tag is **the smallest integer type that holds the
variant count**; the type's alignment is **the maximum of its variants'**.
**There is no niche optimisation**, and an implementation MUST NOT introduce
one. (#25 §10.)

**8.5.1** Consequently **`?T` always costs a tag**, and **any type's size is
computable from its declaration alone**. A niche rule would make the size of
`?T` depend on facts about `T` invisible where `?T` is written, and §8.2 has
made every struct C-visible, so that invisibility would propagate to #29's
boundary. A niche rule is additive later if measurement justifies it and is
**not addable once C code depends on a size**. (#25 §10.)

**8.5.2** `?T` (chapter 2 §9.2) and `T rescue E` (chapter 2 §10.1) are sum types
and are laid out by §8.5. (#25 §10, which names #9's, #10's and #11's sum types
as one mechanism.)

**8.6** **All of §8.1–§8.5 is C-ABI-visible.** This chapter fixes the layout;
the **boundary** — calling convention, view and string marshalling, the packed
-header escape, and how a sum type's always-present tag appears to C — is #29's.
(#25 §2, the hand-off.)

---

## 9. Allocators

**9.1** **An allocator is an ordinary `struct` whose fields are non-capturing
function pointers.** The language core knows nothing special about it: it is not
a compiler-known type, not an interface with a blessed rule, and not a boxed
existential. Because a struct is a value it can be exchanged while the program
runs, which is what makes #8 §7's swap-in checker allocator ordinary
polymorphism rather than a build mode. (ADR-0042 §1; #8 §7; chapter 2 §6.3,
which forbids the existential.)

**9.2 Three operations, and `free` is not one of them.**

```
Allocator {
  alloc(size: usize, align: usize) -> ?[]u8
  grow(old: []u8, size: usize)     -> ?[]u8
  reset()
}
```

(ADR-0042 §2.)

**9.3** **Alignment is a parameter, not an attribute.** A container of a
16-byte-aligned type asks for 16 at the call rather than hoping a type attribute
reached the allocation site. (ADR-0042 §2; ADR-0016 §1, which mandates specified
alignment for the blessed math set.)

**9.4** **There is no per-allocation `free` in the safe layer.** Release is
`reset()`, all at once. This is what makes #8 §8's *double-free is structurally
prevented* true rather than aspirational. A container that must release one old
buffer does so **inside `unsafe`** (§16), which is #8 §6's stated shape: the
guarantee survives for every program that is not itself writing a container.
(ADR-0042 §2.)

**9.5** **`grow` is allowed to fail, and a failing `grow` means the caller
copies.** An arena implements it as *extend if last, else fail*. It exists
because growth would otherwise be quadratic. (ADR-0042 §2.)

**9.6 Exhaustion is a value, not a fault.** `alloc` and `grow` return `?[]u8`,
and allocation failure is handled at the call site, never a chapter 5 fault.
**There is one nominal `OutOfMemory` error.** The deciding case is the
fixed-capacity pool: *the spawner hit the cap* is a design condition a game
handles, and making a deliberate limit a crash is not available. (ADR-0042 §3.)

**9.6.1** The mandated facade error sets absorb `OutOfMemory` — chapter 6's,
recorded here because ADR-0042 §3 is the clause that adds the variant.
(ADR-0042 §3.)

**9.6.2** **Not every allocating call is fallible**: allocation-failure policy
belongs to the allocator, not the language (chapter 2 §10.17). A
general-purpose allocator MAY abort on exhaustion, which is a bug
(chapter 2 §8.3). (#10 Q4.)

**9.7 Two types, because one of them must not escape.**

- **`Allocator`** is **storable**: it lives in a struct field, is captured by a
  container, and persists.
- **`Scratch`** is **transient and non-escaping**, under exactly §6's view rule:
  never stored in a struct, never in `persist`, never captured, never outliving
  the call. A **local binding is a legal `Scratch` position**, by the same
  clarification §6.4 makes for views.

This is a restriction on **positions**, not an analysis of lifetimes: no
regions, no borrow checker. (ADR-0042 §4, as clarified by ADR-0047 §2 and §10.)

**9.8** **`List[T]` keeps one type parameter.** The allocator kind is inferred
from the constructor and carried invisibly; the type is **not** respelled
`List[T, A]`. Escaping-ness rides the same rail as #15's move-only inference
(§3.3) and has no marker. (ADR-0042 §5.)

**9.9** The frame entry's second parameter is `scratch: !Scratch`, and the `!`
is required because allocation mutates the arena. The entry's shape is
**chapter 5's** (chapter 1 §5.11); recorded here because ADR-0042 §6 is the
clause that adds it. (ADR-0042 §6.)

**9.10** **The allocator implementations this specification mandates are
runner-owned**: their code lives outside the reloadable image, which is what
lets a container backed by one appear in a `persist` declaration. **A `persist`
container backed by a user-written allocator is rejected**, and the diagnostic
names the offending field path. The `persist` reachability rule itself is
chapter 5's (#17 §3). (ADR-0042 §7.)

**9.11** **`Allocator` and `Scratch` are core prelude types, not a sixth
facade.** There is no tier beneath memory and no backend to delegate to, and
core conformance is headless, so the allocator must exist where no backend does.
(ADR-0042 §8.)

**9.12** **The general-purpose allocator is spelled `heap`, not `$.mem.heap`**,
and **`heap` is legal only in a `persist` initialiser, in the entry file**. A
globally reachable heap would defeat §1.6 outright. (ADR-0042 §8.)

**9.13 What is mandated, and what is merely blessed.** **Mandated**: the
`Allocator` and `Scratch` types, the frame arena handed to the entry, and one
general-purpose `heap`. **Blessed but unmandated**, on ADR-0032's treatment —
ours, own repo, blessed by reference: the **AoS struct pool**, the **columnar
SoA pool** (§10.4) and the **checker allocator**. Nothing in the mandated
surface names them. (ADR-0042 §9; #8 §7.)

---

## 10. Handles and pools

**10.1** **A lasting link is a generational handle into a pool.** A handle to a
released object is caught when it is used; it can never produce a bad
dereference in the safe layer. (#8 §1, §3.)

**10.2** **A stale handle reads as absence, not as a bug.** `pool.get(h)` yields
`?Entity`, and only an `unsafe` dereference of a dead handle is a bug.
(#9; #25 §6; chapter 2 §8.4, §9.7.)

**10.3** **`Handle[T]`** is the type of a handle to a `T` in a pool. It is a
prelude generic type; it is **opaque** — it carries equality and comparison and
no arithmetic, exactly as a `distinct` type does (chapter 2 §3.1) — and it is
**copyable plain data**, so it may be stored in a struct, in a `persist`
declaration and in a container. The spelling is **authored by this chapter** —
see §19.1.

**10.4 The two blessed pools** (unmandated, §9.13):

- an **AoS struct pool** — fixed capacity, acquire and release, an `active`
  flag, **handle = slot**;
- a **columnar SoA pool** — **handle = row**, with batch operations over a whole
  column.

(#8 §7.)

**10.5 The SoA transform is compiler-derived and cannot be hand-written.**
Satisfaction is a **derived constraint** computed by the compiler, never
declared, and satisfiable by generic bodies checked once against it. A
hand-written column split would be a second place to update when a field is
added. This joins the narrow privileged-compiler-knowledge class (#15 Q18's
derived `Eq`/`Hash`/`Clone`, chapter 2 §6.6). (#25 §1.)

**10.6 A column is named by derived field access.** For `rocks: Pool[Entity]`
with `pos: Vec2` on `Entity`, **`rocks.pos` is the `[]Vec2` column**. The
derivation is **total and mechanical** — every field of `T`, no exceptions, no
opt-out. **Named cost: `rocks.pos` is a `[]Vec2` while `e.pos` is a `Vec2`** —
one spelling at two types, disambiguated by whether the receiver is a pool.
(#25 §7.)

**10.7 The SoA split is exactly one level deep — one column per top-level
field.** `pos.x` alone is not a column. This is what makes every column a
genuine `[]T` with **no striding anywhere**, which is in turn what makes §6.1's
universal boundary true. **Named cost: scalar columns require flattening the
struct by hand**, which is a choice made at the declaration a program already
writes. (#25 §5.)

**10.8 Single-element access on a columnar pool materialises a copy.** There is
no `Entity` in memory to point at, so `get` copies, a single-element write is a
whole-value `pool!.set(h, e)`, and **true in-place mutation exists only through
columns**. A synthesised in-place `!Entity` lend that scattered writes back at
scope exit is rejected: it is the invisible non-local control flow #8 deleted
destructors for. **The difference between the two pools shows up as cost, never
as semantics**, and the cost is stated: *a random-access read-modify-write loop
over an SoA pool is slower than over an AoS pool*. Choosing the pool to match
the access pattern is the decision the two pools exist to allow. (#25 §6.)

**10.9 A columnar pool requires every field of its element type to be copyable
plain data.** This is a compiler-checked derived constraint; violating it is a
compile error naming the offending field and pointing at the AoS pool. A column
of containers would make §10.8's materialising `get` either a deep clone or a
**move out of the pool** — a destructive read. **Named cost: an entity owning a
`List` cannot go in a columnar pool.** (#25 §8; #15 Q14 for move-on-assignment.)

**10.10 Column disjointness is unresolved and is not stated as a rule here.**
#25 §9 makes the canonical batch op `integrate(rocks.pos!, rocks.vel)` legal by
declaring that **exclusivity is per place and distinct columns are distinct
places**, with `rocks.pos!` beside `rocks.pos` a compile error naming the
column. ADR-0047 §3 is later and general, and it **removes exclusivity between
views entirely** (§7.4), which grants the batch op more broadly and leaves #25
§9's compile error following from no rule this specification states. **What both
agree on is transcribed: the canonical batch op is legal** (§7.4, §10.6). The
disagreement is recorded rather than resolved, because resolving it either way
reverses a decision and ADR-0044 §6 reserves that for an ADR. Filed as
[#103](https://github.com/ludo-lang/ludo/issues/103); see
[`coverage/03-memory.md`](coverage/03-memory.md) §3.

**10.11** `pool.each()` lends the whole pool. (#25 §9.)

---

## 11. Dynamic containers

**11.1** `List[T]` and `Map[K, V]` are library generics (§4.3), and **a `List`
captures its allocator at construction** — not passed per call — which is what
keeps `list!.push(e)` argument-free in a loop body and puts the allocator choice
in the signature radius at exactly one point. (#15; ADR-0042 §5.)

**11.2** Containers move on assignment (§3.1), duplicate through
`clone(allocator)` (§3.4), iterate by §12, and compare by §13.1.

**11.3 The dynamic container set is unspecified, and this chapter says so rather
than waiting for it.** (ADR-0044 §8;
[#82](https://github.com/ludo-lang/ludo/issues/82).) Open:

- **whether the set is exactly `List` + `Map`**, or whether a set and a ring
  buffer ship, ship as blessed-unmandated, or not at all;
- **the capacity surface** — whether `List` exposes `reserve` and `capacity`,
  which a program that must not allocate mid-frame needs;
- **how `OutOfMemory` reaches the caller of `push`** — whether `push` is
  fallible, or whether there is a pre-reserved infallible path.

**11.4** **The view/growth seam is *not* part of §11.3's gap.** *Is
`v := xs.items()` legal, and what happens across a `push`* is answered in full
by §6.4 and §7.1. (ADR-0047 §9.)

---

## 12. Iteration

**12.1** **There is one iteration construct**: `for x in xs` and `for k, v in m`
over every aggregate (chapter 1 §6.10), via a **nominal `Iter[T]` constraint
with static dispatch and monomorphisation**. There is no iterator boxing and no
`dyn` (chapter 2 §6.3). **A user type can satisfy `Iter[T]`.** (#15 Q6, Q19.)

**12.2** **`for x in xs` binds a copy; `for x in !xs` binds a writable view per
element.** The loop's lend is **exclusive for its duration**, so **structural
mutation of the aggregate during iteration is a compile error** — iterator
invalidation is deleted rather than documented. (#15 Q19.)

**12.3** **Index iteration is `for i in 0..<n`**, and there is no inclusive
range operator: with 0-based indexing `0..<xs.len` is the always-correct shape
while `0..=xs.len` would be out of bounds, and that bug simply has no spelling.
**Ranges are ordinary `Iter[int]` values, not grammar.** (#15 Q24; chapter 1
§7.7.)

**12.4** **`for i, x in xs.pairs()`** is the index-and-value form, using §14's
multiple returns in the loop binding. (#15 Q19.)

**12.5** `Iter[T]` and §13.2's `Key` are **interfaces**, and **the interface
declaration form does not yet exist** (chapter 2 §6.2.1,
[#100](https://github.com/ludo-lang/ludo/issues/100)). The constraint's
*meaning* is fixed here; its *spelling* is inherited from that gap, not created
by it.

---

## 13. Equality, keys, hashing and cloning

**13.1** **`==` is structural and elementwise wherever the elements support
it** — structs, arrays, `List`, `Map`. There is **one rule and no `==`-versus
-`.equals` split**. The `O(n)` is visible from the type, and **a `Map`
comparison ignores insertion order**. (#15 Q17.)

**13.2** **Map key types are restricted by a `Key` constraint** (equality and
hash), satisfied by integers, strings, `distinct` types, and structs of keys.
**Floats are excluded**, because NaN breaks the equality law and the failure is
silent. (#15 Q5.)

**13.3** **`Eq`, `Hash` and `Clone` are structurally derived by the compiler and
cannot be hand-written.** This is a narrow, named exception to chapter 2 §6.2's
explicit nominal satisfaction, and it is **not** the accidental structural
satisfaction #11 rejected: these three are language-known and fixed, not
user-declared shapes an unrelated type might match by coincidence. The
justification is #5's criterion 5: a hash inconsistent with equality is
unattributable corruption that surfaces frames later in unrelated code.
(#15 Q18.)

**13.4** **Custom hashing goes through a `distinct` wrapper** whose fields
define the behaviour, with normalisation as a visible, greppable call at the
boundary. (#15 Q18; chapter 2 §3.1.)

**13.5** **Derived `Clone` clones container fields recursively**, so `clone`
takes an allocator and stays off the implicit path. (#15 Q18; §3.4.)

**13.6** **Map iteration order is insertion order**, specified rather than left
open: unspecified order is a silent-bug generator for replay and lockstep
determinism. **Named cost: a slightly fatter map.** (#15 Q12.)

**13.7** A missing key reads as `?V`, there is **no panicking indexing
variant**, and `has(k) -> bool` exists **on containers only** (chapter 2 §9.7;
#9). The signatures live on §11.3's unspecified set.

---

## 14. Multiple returns

**14.1** **Fixed-arity multiple returns are kept, destructured at the call site,
and there is no first-class tuple.** A returned group **cannot be stored, nested
or passed on**. (#15 Q7.)

**14.2** **The returned group has no type.** Return arity is grammar, there is
no `(int, int)` to name, and **a multi-return call is legal only in a
destructuring binding or a `return` tail**. This is what keeps multiple returns
off §4.1's aggregate list. (#15 Q7; chapter 1 §5.10, §6.3, §6.6.)

**14.3** **Destructuring MUST be total**, and `_` is the one greppable discard
(chapter 1 §6.3). With must-use (§15.3) this means **no returned value is
silently dropped**. (#15 Q7.)

**14.4** **Multiple returns are never the error channel** (chapter 2 §10.2).
#10's rejection was of that one *use*, not of the mechanism. (#15 Q7.)

**14.5** Chapter 1 §5.10 routes this restriction to *chapter 2*, which did not
transcribe it. #15 is **this** chapter's source, so it lands here; the routing
correction is recorded in [`coverage/03-memory.md`](coverage/03-memory.md) §3
rather than resolved silently.

---

## 15. `defer` and must-use

**15.1** **Cleanup of a non-memory resource is `defer`, written next to the
acquire** (chapter 1 §6.9). It runs at scope exit in LIFO order. **There is no
`errdefer` and there are no destructors**; undo-on-failure is written with an
explicit success flag. (#8 §5; #10 Q8, Q17; chapter 2 §10.18.)

**15.2** Destructors are rejected on locality: `}`-time control flow is exactly
the non-local behaviour the language's locality rule fights. (#8 §5.)

**15.3 must-use.** **A value of a must-use type that is not consumed is a
compile error naming it**, and discarding costs exactly one greppable token,
`_ = f()`. The check is a shallow *was this consumed?* question, **not lifetime
tracking**. (#8 §5; chapter 1 §2.2.)

**15.4** **A fallible value is must-use** (chapter 2 §10.11), and **must-use is
universal — no layer may ignore it** (chapter 2 §10.15).

**15.5** **A resource type that must be closed is must-use**, so forgetting the
`defer` is a named compile error rather than a silent leak. **Which types are
resource types is not fixed by this chapter's sources** — see §20. (#8 §5.)

---

## 16. `unsafe`

**16.1** **`unsafe do ... end` is the escape hatch, and it is per-site, loud and
greppable** (chapter 1 §6.15). An escape hatch restated at each use site beats
one that infects a dataflow. (#8 §6.)

**16.2 What `unsafe` permits**, and nothing else changes meaning inside it:

- **raw pointers**, which are a distinct, greppable type;
- **`&x`**, and dereference;
- **manual allocation and free**, including the single-buffer release §9.4
  denies the safe layer;
- **skipping a bounds check** (§1.4);
- **uninitialised memory**, which exists nowhere else in the language.

(#8 §3, §6, §8; #9, whose *uninitialised memory in `unsafe` only* clause lands
here.)

**16.3** **Inside `unsafe` there are no guarantees.** A use-after-free inside
`unsafe` is a bug (chapter 2 §8.3), not a checked failure.

**16.4** **`unsafe` is encapsulatable**: a veteran wraps unsafe internals behind
a safe function whose callers never see the word. This is the shape §9.4's
container release relies on. (#8 §6.)

**16.5** **The raw-pointer type has no spelling.** #8 §6 requires it to be a
distinct greppable type and no source in the corpus spells it. **Recorded, not
authored** — a type-sublanguage production moves chapter 1 §13.6's published
counts, which chapter 2 §19.3 established is a decision rather than a
transcription defect. Filed as
[#104](https://github.com/ludo-lang/ludo/issues/104); see §19.3.

---

## 17. The guarantee table

Normative, and stated as a table because #8 §8 is the clause a conforming
implementation is measured against.

| Hazard | Safe layer | Mechanism |
|---|---|---|
| Stale or dangling reference | Cannot crash or corrupt | A handle to a released object reads as absence (§10.2) |
| Double free | Structurally prevented | No individual frees: arenas bulk-wipe, pools release by handle (§9.4) |
| A forgotten close on a resource | Compile error | must-use (§15.3, §15.5) |
| Null dereference | Cannot happen | There is no null (chapter 2 §9.1) |
| Out-of-bounds index | Cannot corrupt | Always bounds-checked; skipping requires `unsafe` (§1.4) |
| Use of a dangling view | Compile error | §7.1 |
| Use of a moved-from value | Compile error | §3.2 |
| Iterator invalidation | Compile error | §12.2 |
| Memory leak | **Detected, not prevented** | A swapped checker allocator finds it (§1.5, §1.8) |
| Anything inside `unsafe` | No guarantees | §16.3 |

(#8 §8, with the four compile-error rows added from #15 Q14, #15 Q19 and
ADR-0047 §3, which are later clauses in the same table's terms.)

---

## 18. What memory may not do

Recorded as a closed list so that a later chapter is not read as having granted
one of them by silence.

| Not in the language | Source |
|---|---|
| a tracing garbage collector | #8 |
| a borrow checker, lifetimes, regions or aliasing analysis | #8, #11 |
| a raw pointer in the safe layer | #8 §3 |
| `&x`-and-store | #8 §3 |
| a stored write-access | #8 §4 |
| a destructor or scope-based destruction | #8 §5 |
| `errdefer` | #10 Q8 |
| an ambient or context allocator | #8, #5 criterion 1 |
| a global `new`, or a container that grows implicitly | #8 |
| a per-allocation `free` in the safe layer | ADR-0042 §2 |
| a bounds check or overflow check that a build mode removes | #8 §8(a) |
| leak detection as a build mode | #8 §8(b) |
| a copyable container | #15 Q14 |
| a positional struct literal | #15 Q8 |
| a bare `{...}` with no expected type | #15 Q3 |
| a tuple type, or a storable multi-return group | #15 Q1, Q7 |
| a float map key | #15 Q5 |
| a hand-written `Eq`, `Hash` or `Clone` | #15 Q18 |
| an unspecified map iteration order | #15 Q12 |
| an inclusive range operator | #15 Q24 |
| a view in a struct field, in `persist`, or captured | #15 Q10 |
| a returned view with no receiver to derive from | ADR-0047 §1 |
| a `-> ![]T` return-type marker | ADR-0047 §5 |
| `name[a..<b]!` | [#101](https://github.com/ludo-lang/ludo/issues/101) |
| an exclusivity rule between views | ADR-0047 §3 |
| field reordering, `#repr`, `#packed`, field-level `#align` | #25 §2, §3 |
| bit fields | #25 §4 |
| niche optimisation of a sum type | #25 §10 |
| a synthesised in-place lend on a columnar pool | #25 §6 |

---

## 19. Spellings authored by this chapter

Recorded under ADR-0044 §6: **a hole is repaired in the spec text, and an ADR is
written only for a reversal.** No entry below reverses a decision, so none takes
an ADR.

**19.1 `Handle[T]`.** #8 §1 and §3 make a generational handle the language's one
lasting link, #9 and #25 §6 fix `pool.get(h) -> ?Entity`, and chapter 2 §14.2
writes `children: []Handle[Node]` transcribing #11 Q13 — **and no source in the
corpus declares the type**. §10.3 authors it as a prelude generic type, opaque
in exactly the sense chapter 2 §3.1 already defines for `distinct` (equality and
comparison, no arithmetic), and copyable plain data so that §10.9's columnar
constraint admits a handle field, which is the one shape #8's pool model
requires. The set of pool operations that produce and consume it stays
**unmandated** (§9.13), because ADR-0042 §9 put the pools there; the *type* is
mandated because chapter 2 already spends it. **Zero cost against #24**: a
prelude identifier (chapter 1 §13.5 rule 6) applied to existing `GenericArgs`
(chapter 1 §9.8).

**19.2 `#align(n)`'s argument.** #25 §3 ships the attribute and chapter 1 §12.2
lists it, and **nothing anywhere says what `n` may be**. §8.3.1 authors the
three constraints that make the attribute mean what §8.3 says it means: a const
expression (there is no other kind of attribute argument the const-eval floor
admits), a power of two (an alignment is), and never below the natural alignment
— because an `#align` that reduced alignment *is* `#packed`, which §8.3 refuses
outright. Each is derived from a decision already taken rather than chosen here,
which is why the repair is a sentence. **Zero cost against #24**: the attribute
and its argument production already exist (chapter 1 §12.1).

**19.3 The raw-pointer type — recorded, not authored.** #8 §6 requires raw
pointers to be *"a distinct, greppable type"* and §8's guarantee table and §16.2
both depend on them existing, while **no source spells the type**. This is the
second phantom the consolidation has declined, and for chapter 2 §19.3's reason
exactly: a pointer type is a type-sublanguage production, which chapter 1 §13.8
charges and §13.6, §13.7 and §13.12 publish, so the repair moves a published
budget and is therefore a decision rather than a transcription defect. Filed as
[#104](https://github.com/ludo-lang/ludo/issues/104). §16.2 states what the type
must permit once it exists.

---

## 20. What this chapter does not decide

Recorded so the boundary is legible, and so a later chapter is not read as
having inherited a silence.

- **The dynamic container set.** §11.3, and it is stated in the chapter text
  rather than blocking it (ADR-0044 §8).
  [#82](https://github.com/ludo-lang/ludo/issues/82).
- **The raw-pointer type spelling.** §16.5, §19.3.
  [#104](https://github.com/ludo-lang/ludo/issues/104).
- **Column disjointness.** §10.10 — #25 §9 and ADR-0047 §3 disagree, and
  resolving it reverses one of them. [#103](https://github.com/ludo-lang/ludo/issues/103).
- **Which types are must-use resource types.** §15.5. #8 §5 names files, sockets
  and GPU handles as examples; the language declares none of them, and the
  storage surface (ADR-0026) is chapter 5's. No source fixes the set.
- **Two disjoint writable windows into one buffer.** §7.7.
  [#102](https://github.com/ludo-lang/ludo/issues/102).
- **The interface declaration form** that `Iter[T]` and `Key` need. §12.5;
  inherited from chapter 2 §6.2.1.
  [#100](https://github.com/ludo-lang/ludo/issues/100).
- **The C boundary.** §8.6; #29's, and it inherits a far smaller job than it was
  scoped for, because §8.2 makes C compatibility the default rather than a
  feature.
- **`persist` reachability and reload.** §9.10; chapter 5's (#17 §3).
- **The facade signatures** that take an allocator or return a view — §6.9,
  §9.6.1; chapter 6's.
- **The padding diagnostic.** §8.2.2; vendor-space under ADR-0023, and chapter 7
  owns the envelope it would ride in.
- **The blessed math types' alignment.** §9.3 records that ADR-0016 §1 depends
  on `#align`; ADR-0016 itself is assigned to no chapter
  ([#99](https://github.com/ludo-lang/ludo/issues/99)).
