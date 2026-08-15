---
status: accepted
---

# A returned view is derived from its receiver, and mutation kills it

[#83](https://github.com/adamico/ludo/issues/83), split out of
[#81](https://github.com/adamico/ludo/issues/81) / [ADR-0043](0043-a-string-is-a-literal-and-constructed-text-is-bytes.md) §9,
which needed a view-return clause for string construction, stated only a narrow local one, and
filed the general question rather than burying it in a string ticket.

[#15](https://github.com/adamico/ludo/issues/15) Q10 fixes `[]T` as *"a transient non-escaping
view: parameter position only, never stored in a struct, never returned, never outliving the
call."* **That rule is false in the corpus and has been since before it was written.** Four
mandated signatures return a view:

- [ADR-0042](0042-an-allocator-is-a-struct-of-function-pointers-the-runner-owns.md) §2 — `alloc(size: usize, align: usize) -> ?[]u8`
- ADR-0042 §2 — `grow(old: []u8, size: usize) -> ?[]u8`
- [ADR-0009](0009-the-drawing-facades-call-spellings.md) §111, [ADR-0022](0022-the-spec-promises-only-what-is-derivable.md) — `get_pixels(image: Image) -> []u32`
- ADR-0043 §4 — `format(buf: ![]u8, ...) -> []u8`

The re-cut spans the memory model and the graphics facade, which is why it is not a string
question. This ADR states the general rule, and the narrow string clause becomes one instance
of it.

## 1. A returned view is derived from its receiver

**A function may return a `[]T` if and only if the view is derived from the call's receiver, or
from the runner's constant blob.**

All four exhibits pass:

```ludo
fn items(self) -> []T                    -- the receiver's buffer
fn view(self) -> []u8                    -- TextBuf's, ADR-0043 §5
fn get_pixels(image: !Image) -> []u32    -- the image's pixels (§5 restamps the marker)
fn alloc(self, size: usize, align: usize) -> ?[]u8
fn grow(self, old: []u8, size: usize) -> ?[]u8
fn format(buf: ![]u8, ...) -> []u8       -- the buffer the caller passed
```

A function with no receiver to derive from cannot return a view:

```ludo
fn make() -> []u8                        -- error: nothing to derive from
```

**This is tighter than the option #83 weighed**, which asked whether the referent is reachable
from the call's *arguments or receiver* and concluded that `alloc` fails it — its referent being
"memory the caller never named". `alloc`'s referent **is** named: it is reachable from the
allocator, which is the receiver. Narrowing the source to the receiver alone is what lets every
mandated signature pass under one rule instead of three plus an exception.

**The constant blob is the second source, and it is the only lifetime in the language that is
unconditional.** ADR-0043 §2's one-way `string` → `[]u8` widening makes this writable, and it is
safe:

```ludo
fn label(self) -> []u8
  return self.name.view() if self.named
  "unknown"                              -- a view of the constant blob
end
```

Receiver-only would make that function illegal and push its author to a return type that cannot
express the other branch. The blob outlives every frame, every reload and every image swap
(ADR-0043 §1), so admitting it as a source costs nothing.

**The rule is checked, not merely asserted.** Both sources are visible in the callee's own body,
so the check needs no whole-program view and R1 holds. The alternative — an obligation on callee
authors in the shape of ADR-0042 §2's `unsafe` container internals — was rejected: `unsafe`
buys an obligation where the safe layer *has no spelling*, and this rule has one.

**The enumerated allow-list was rejected.** It cannot be written against a stdlib that is not
finished, and it fails agent-friendliness outright: an agent cannot derive membership of a list
it has to have memorised.

## 2. A local binding is a legal view position

#15 Q10's *"parameter position only"* is read as what it was written to exclude — struct fields,
`persist`, container capture — and **not** as a ban on binding a view to a local:

```ludo
px := $.graphics.get_pixels(img!)
for i in 0..<px.len do
  px[i] = 0xff00ff00
end
```

The corpus already requires this. ADR-0009 §288 describes what `create_pixel_image` and
`get_pixels` are: *"a computed `Image` and a mutable `[]u32` view into it"*, and ADR-0022 makes **taking
the view the mutation signal** for upload — a program that may not bind the view cannot write
the loop the primitive exists for. The reference program writes it at
[line 187](../spec/reference/reference.ludo): `line := format(buf!, "{}: {} ({} chars)", label,
secs, glyphs)`.

Everything else Q10 bans is unchanged and unweakened:

```ludo
type Painter = struct
  px: []u32                              -- error: a view in a struct field
end

persist px: []u32 = ...                  -- error: a view in persist
```

**A returned view whose result nothing may bind buys nothing**, so this clause and §1 are one
decision, not two.

## 3. Mutating or moving the origin kills the view

Legalising the binding is what opens the dangling window, so this ADR closes it rather than
handing it on:

```ludo
v := xs.items()
xs!.push(e)                              -- the mark kills v
$.print(v[0])                            -- error: v was invalidated by `xs!` above
```

The fix is a re-take, and it needs no annotation:

```ludo
xs!.push(e)
v := xs.items()
$.print(v[0])
```

The rule in one sentence: **a view dies when the place it derives from is next used with `!` or
moved.** It fires on the markers #15 already makes the caller write, so the kill is always
visible in the caller's own text — including when the place is handed to a function that mutates
it:

```ludo
v := xs.items()
helper(xs!, v)                           -- error: v is invalidated at this call site
```

**The check is flow-insensitive and spans the function body.** A view is dead from the first
`!`-use or move of its origin that appears later in the body, whichever branch that use sits in.
Flow-sensitivity was rejected: it buys convenience and costs the one-sentence explanation, and
the moment *control flow* decides whether a program compiles, an agent can no longer predict the
error from the text in front of it. Block-scoped death was rejected as this rule plus an
arbitrary second kill that fires where nothing is wrong.

**There is no exclusivity rule.** Any number of views of one place may co-exist, readable or
writable; only `!` or a move on the **origin** kills them.

```ludo
a := xs.items()
b := xs.items()                          -- legal, and stays legal
```

Many-readers-or-one-writer is the borrow checker's own rule and produces the errors beginners
cannot read. #8 deleted aliasing at the **container** level, not the view level, and that is
where the guarantee stays.

> **Stamp ([ADR-0052](0052-the-origin-of-a-view-is-the-named-container-and-disjointness-is-never-reasoned-about.md) §2): upheld and generalised, not amended.**
> This section says a mark on *the origin* kills the views, and leaves *what the origin is* to
> the reader. ADR-0052 names it: **the container the view's expression names**, never a part
> selected out of it. So a mark kills views of disjoint ranges and of distinct columns alike, and
> no disjointness is reasoned about in either dimension. That also reverses
> [#25](https://github.com/ludo-lang/ludo/issues/25) §9, which this section contradicted without
> resolving.

## 4. The edge chains through a container

One level is a view of a place. Two levels is a container that captured an allocator:

```ludo
l := List.from(scratch, ...)
scratch!.reset()
l!.push(e)                               -- error: l derives from scratch, killed above
```

`l` is not a view — ADR-0042 §4 permits it, because a `Scratch` in parameter position is exactly
what that ADR allows. But the `!` is right there in the same body, and a rule that catches the
one-liner and misses the version with a container in the middle catches the version nobody
writes. **A container constructed from a place derives from it, and the same mark kills it.**

This is the general rule, not a `Scratch` special case. A container on a storable `Allocator`
never meets a `!reset` in a body where it matters, so the general form already gives the right
answer for both allocator kinds, and ADR-0042 §4's hazard is closed for the single-function
case at no extra cost.

## 5. Writability flows from the receiver

ADR-0043 §10 left *where the `!` marker sits* unspecified and called it #15's marker grammar.
Once views are returnable that can no longer wait: a returned view has to declare whether it is
writable, and `get_pixels(image: Image) -> []u32` carries no mark anywhere while being the
corpus's designated **mutation signal**.

**The receiver carries it. A returned view is writable if and only if the operand it derives
from was marked `!` at the call site.**

```ludo
px := $.graphics.get_pixels(img!)        -- writable, because img is marked
```

**`get_pixels` is restamped to `get_pixels(image: !Image) -> []u32`.** This is a signature change
to a published facade call, stated as a cost rather than discovered later.

Marking the **return type** — `-> ![]u32` — was rejected: it permits a writable view derived
from a receiver that was never marked, which is a hole, and it mints a second channel for a fact
the derivation edge already carries. Leaving it unspecified is no longer available, because this
ADR is what makes the returns legal.

**One spelling is still not decided here.** ADR-0043 §10's open item — whether a writable
sub-view of a place is `name![a..<b]` or `name[a..<b]!` — remains #15's marker grammar. This
section fixes where the mark sits on a **call operand**, which the reference program already
writes as `format(buf!, ...)`; the slice form is consistent with the prefix reading but is not
decided by this ADR.

> **Stamp ([ADR-0052](0052-the-origin-of-a-view-is-the-named-container-and-disjointness-is-never-reasoned-about.md) §1, §6): this paragraph is stale.**
> [#101](https://github.com/ludo-lang/ludo/issues/101) closed the open item. The mark rides the
> **root binding** and the suffix chain follows it, *whatever the suffixes are — field, call,
> index or slice*: a writable sub-view is `name![a..<b]`, a writable column is `rocks!.pos`, and
> the trailing forms parse but are errors. Landed at spec ch1 §7.10 / §7.10a and ch3 §6.12.
> The rest of §5 is unaffected.

## 6. What this is not

The check in §3 and §4 tracks a **derived-from** edge between two locals, and that edge is a
thing a borrow checker is also made of. The charge is answered plainly rather than dodged: it is
one rule a borrow checker has, and this is not a borrow checker.

- **No lifetimes.** Nothing is annotated, named, quantified or inferred.
- **No regions.**
- **No shared/exclusive distinction** (§3).
- **The check never crosses a function boundary.** Every kill is a mark in the body being
  compiled.
- **It is the move check's shape.** #15 Q14 already ships *was it moved?* on containers. A
  view is a second name for a place; using the place mutably kills the other name. #15's own
  words for the move check are *"the same shape as #8's must-use, **not** lifetime tracking"*,
  and they carry unchanged.

#2's finding was that the gap is a **non-lifetime-managed** game language, and #8 rejected a
borrow checker as the mechanism. Neither is spent here.

## 7. `alloc` is not reclassified as `unsafe`-shaped

#83's third door was to make `alloc` the `unsafe`-shaped call every container already wraps, on
the ground that its referent is unnamed. §1 removes the premise — the referent is the receiver's
memory — and the door closes with it.

It would also be expensive. ADR-0042 §3 made exhaustion a `rescue`-able value **precisely** so
that a fixed-capacity pool is ordinary safe code; requiring `unsafe` at every direct `alloc`
would take that back. The `unsafe` in ADR-0042 §2 is for **release**, which the safe layer has
no spelling for. `alloc` now has one.

## 8. ADR-0043 §9 collapses into this rule

§9's narrow clause — *a view returned by a string-construction call is derived from a buffer the
caller passed in, so it names memory the caller already owns and is not an escape* — is §1 with
the receiver spelled as `buf`. It stops being an exception beside the rule and becomes an
instance of it. `format`'s signature is unchanged.

## 9. What is handed on

- **[#82](https://github.com/adamico/ludo/issues/82) inherits §3 rather than authoring it.** Its
  view/growth seam — *is `v := xs.items()` even legal, and what happens across a `push`* — is
  answered here in full. #82 keeps its own questions: whether the set is exactly `List` + `Map`,
  the `reserve`/`capacity` surface, and how `OutOfMemory` reaches the caller of `push`.
- **Spec chapter 3** transcribes §1–§5 as clauses citing this ADR, and extends the reference
  program with a view-returning call and an invalidation case.
- **ADR-0043 §10's slice-marker spelling** stays open, per §5.

## 10. The stamp list

- **#15 Q10** — *"never returned"* is **reversed** by §1; *"parameter position only"* is
  clarified by §2 to mean not-stored and not-escaping, with locals legal. The struct-field,
  `persist` and capture bans are unchanged.
- **ADR-0042 §4** — `Scratch`'s rule cites Q10 verbatim and inherits both changes. §4 of this
  ADR closes its stated capture hazard for the single-function case.
- **ADR-0043 §9** — collapses into §1 (§8).
- **ADR-0009 §111 / ADR-0022** — `get_pixels(image: Image) -> []u32` becomes
  `get_pixels(image: !Image) -> []u32` (§5). ADR-0022's *taking the view is the mutation signal*
  is unaffected and is now spelled at the call site.

## 11. Costs, properties and the three lenses

**#24: zero delta.** No new keyword, no new operator, no new type, no new name. The entire cost
is a new compile-error class over markers the language already writes.

**No new [#19](https://github.com/adamico/ludo/issues/19) property, and the reason is recorded**
(ADR-0030 §8): P1–P15 are run-time properties a backend can be tested against, and every clause
here is a compile-time rejection. A conformance suite for it belongs to the frontend, which is
#19's stated exclusion.

**Simplicity.** Two sentences a Lua user can hold: *a returned view points into its receiver*,
and *touching the receiver with `!` kills it*. Against that, a Lua user has never met a rule that
makes a live-looking local unusable, and this is the first.

**Robustness.** It converts the corpus's outstanding dangling case —
`v := xs.items(); xs!.push(e)` — from silent use-after-free into a compile error, and it does so
for the container-in-the-middle version too (§4). #8 §8's guarantee table gets to keep saying
what it says.

**Agent-friendliness.** No annotations to get wrong, and the error is predictable from the text
alone: flow-insensitivity (§3) is chosen for exactly this, so an agent never has to simulate
branches to know whether its code compiles.

**The lens conflict, named:** simplicity wanted the rule with no invalidation clause — return
views, ban storing them, stop. Robustness refuses, because that is `alloc`/`reset` dangling with
nothing to catch it. Robustness wins, and the price is the language's first *this looks fine and
is rejected* error.
