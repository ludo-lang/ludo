---
status: accepted
---

> **Amended by [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md):**
> §2's `alloc` and `grow` return a view, which #15 Q10 forbade; ADR-0047 §1 makes that
> legal by deriving the view from the receiver. §4's `Scratch` rule cites Q10 verbatim and
> inherits both of ADR-0047's changes to it — a local binding is a legal position (§2), and
> the struct-field, `persist` and capture bans stand. ADR-0047 §4 closes §4's stated capture
> hazard for the single-function case. (Stamp added retroactively while transcribing spec
> chapter 3, per #72.)

> **Amended by [ADR-0048](0048-every-allocating-call-is-fallible-and-a-persist-initialiser-faults.md):**
> §3's *"never a #18 fault"* is narrowed — allocation failure in a `persist` initialiser is a
> fault, because none of #10's three exits exists in that position; every other position is
> unchanged. §9's blessed-but-unmandated list gains a ring buffer.

# An allocator is a struct of function pointers, and the runner owns the ones that ship

Issue #80 was filed on the observation that the word `allocator` appears in a mandated
facade signature — ADR-0015's `$.graphics.decode_image(allocator, bytes: []u8)` — and in
twenty-six ADRs, and that **no document says what the type is**. It appears zero times in
`CONTEXT.md`. The corpus has been spending a type it never minted.

The gap is narrower than it first looks. [#8](https://github.com/adamico/ludo/issues/8) §7
settled that an allocator is *"a library interface you pass"* and named the blessed kit;
[#15](https://github.com/adamico/ludo/issues/15) settled that a `List` **captures** its
allocator at construction. What was never settled is the **shape** — and the shape turned
out to be over-determined by three closed decisions that cannot all hold as written.

## 1. The contradiction the ticket actually resolves

Three settled things collide:

- **#8 §7** promises the swap-in checker allocator works as *"ordinary polymorphism, not a
  build mode"* — a choice made while the program runs.
- **[#11](https://github.com/adamico/ludo/issues/11) Q2** allows **static dispatch only**:
  no `dyn`, no boxed existential.
- **ADR-0015** writes `allocator` as a **bare, unparameterised** type in a mandated
  signature.

A generic type parameter satisfies #11 and defeats ADR-0015, because a monomorphised
signature cannot be the frozen, C-callable surface [#29](https://github.com/adamico/ludo/issues/29)
requires. A boxed existential satisfies #8 and ADR-0015 and is exactly what #11 forbids.

**The resolution is that the third option was always available and nobody wrote it down: an
allocator is an ordinary `struct` whose fields are non-capturing function pointers.**
[#52](https://github.com/adamico/ludo/issues/52) already blessed function pointers, so this
introduces **no new language mechanism**. A struct is a value, so it can be exchanged at run
time — #8 §7 is satisfied. A struct is not a boxed existential — #11 bans a mechanism, not a
data layout that resembles one, and the distinction is real: there is no subtyping, no
object-safety question, and no compiler-generated vtable. Every struct is C-compatible by
construction (ADR-0025), so ADR-0015's signature stands unchanged and a **backend** or a C
host can supply one.

**Rejected: making the language know the type.** A compiler-known allocator would be a
fourth thing the core has to define, and it buys nothing the struct does not already give.

## 2. Three operations, and `free` is not one of them

```
Allocator {
  alloc(size: usize, align: usize) -> ?[]u8
  grow(old: []u8, size: usize)     -> ?[]u8
  reset()
}
```

**Alignment is a parameter, not an attribute.** ADR-0016 mandates specified alignment for the
blessed math set, so a container of `Vec4` must be able to ask for 16 at the call rather than
hope a type attribute reached the allocation site.

**There is no per-allocation `free` in the safe layer.** #8 §8 claims double-free is
*structurally prevented* — "no individual frees; arenas bulk-wipe, pools release by handle" —
and a `free` on this struct would hand every program the operation that claim denies. Release
is `reset()`, all at once.

**`grow` exists because `List` would otherwise be quadratic.** Without it, growth means
allocate-and-copy on every reallocation, and an arena — the allocator a game reaches for
most — can extend its most recent allocation in place for free. `grow` is **allowed to fail**,
and a failing `grow` means the caller copies; an arena implements it as *extend if last,
else fail*, which is the standard trick and needs no bookkeeping.

A container still needs to release a single old buffer, which the safe layer cannot express.
It does so **inside `unsafe`**, which is #8 §6's stated shape — *"a veteran wraps unsafe
internals behind a safe function whose callers never see the word."* The double-free
guarantee survives for every program that is not itself writing a container.

## 3. Exhaustion is a value, not a fault

`alloc` and `grow` return `?[]u8`. Allocation failure is a **`rescue`-able error**, never a
[#18](https://github.com/adamico/ludo/issues/18) fault.

The deciding case is #8 §7's AoS pool, which is **fixed capacity** by design. "The spawner hit
the cap" is a design condition a game handles — spawn nothing, reuse the oldest, play a
sound — and #18's fault exists for programmer error. Making capacity exhaustion a fault would
turn every deliberate limit in a game into a crash.

**One nominal `OutOfMemory` error, and the mandated facade error sets absorb it now.**
ADR-0015 publishes `decode_image(allocator, bytes) -> Image rescue DecodeError`; that call
allocates, so `DecodeError` and its siblings gain the variant **in this ADR** rather than
later. [#10](https://github.com/adamico/ludo/issues/10)'s error sets are closed and #11's
`match` is exhaustive, so adding a variant to a published set is a breaking change to every
call site — cheap now, expensive after the surface is frozen by criterion 4.

## 4. Two types, because one of them must not escape

#15 states that *"a `List` carries its allocator, captured at construction"* — capture is the
design, and it is what keeps `list!.push(e)` argument-free in a loop body. Capture is also
the hazard. Given a per-frame arena, a program can write

```
self.enemies = List.from(scratch, ...)
```

and read it next frame, after the reset. That is use-after-free walking back in through the
one door #8 closed.

**One storable type will not do.** It was considered as "document the hazard" and withdrawn
on inspection: a `List` holding a `[]u8` from a reset arena **is a raw pointer in the safe
layer**, which #8 §3 forbids outright and §8's guarantee table says cannot happen. It is not
a simplicity-for-safety trade; it silently breaks a closed decision.

**The decision: two types.**

- **`Allocator`** — storable. Lives in a struct field, is captured by containers, persists.
- **`Scratch`** — **transient and non-escaping**: parameter position only, never stored in a
  struct, never returned, never outliving the call.

`Scratch`'s rule is **not new machinery**. #15 specifies exactly this rule for `[]T` views —
*"parameter position only, never stored in a struct, never returned, never outliving the
call"* — and it is #8's lend. This ADR applies an existing rule to a second type. It is a
restriction on **positions**, not an analysis of lifetimes: no regions, no borrow checker,
and nothing #2 rejected.

**Rejected: the generational arena.** The serious alternative was one type plus a generation
word per block, with a stale touch caught at run time and surfaced through #18's pause. It is
arguably more faithful to #8's *"checked at runtime — not a compile-time proof"* instinct, and
it keeps one allocator type and a non-generic `List`. It loses on **when you find out**: it
reports a design error — storing frame-scratch state — as a fault at frame 3000, far from the
`List.from` that caused it, where the two-type rule reports it at the construction site and
names the field. #11 calls criterion 5 *"a veto on any feature whose failure mode is a bad
error message"*, and this is the same call #15 made when it turned iterator invalidation into
a compile error rather than documenting it. Its second cost is a check on the container hot
path in a language whose stated priority is frame stability.

## 5. `List[T]` keeps one type parameter

A `List` holding a `Scratch` must itself be non-escaping. **The allocator kind is inferred
from the constructor and carried invisibly; `List[T]` is not respelled `List[T, A]`.**

#15 already built this machinery and deliberately gave it no marker: *"a struct with a
move-only field is move-only, and the error names the field and its path; inferred within the
signature radius, so R1 holds."* Escaping-ness is the same kind of property and rides the same
rail. A second type parameter would land on the most-written generic in the language and be
paid for in every signature that names a `List` — against
[#24](https://github.com/adamico/ludo/issues/24)'s grammar budget, and for almost no benefit,
since #15 makes `[]T` the universal boundary and few functions name a `List` at all.

## 6. `Scratch` is the drawing entry's second parameter

`CONTEXT.md` states the **drawing entry** has **one rule**: *the
entry's parameters are exactly the values only the runner can supply* — which until now meant
a single `screen: !Target`. The frame arena joins it:

```
fn draw(screen: !Target, scratch: !Scratch)
```

**The rule gains a member; it is not amended.** Only the runner can supply memory it resets at
a boundary it alone controls, and ADR-0041 fixes that boundary as the frame **task**. The
arena's lifetime is exactly one entry.

**Rejected: a program-owned arena** built in top-level code and stored in program state. It
keeps the entry at one parameter and costs a program-owned reset call — and a forgotten reset
is a leak that grows every frame with no diagnostic, which is the failure #8 was willing to
tolerate only because a swapped checker allocator finds it. Here the runner can simply do it.

**`!` is required.** #11 makes mutability a property of the place and demands the marker in
all three positions; allocation mutates the arena, and an unmarked `scratch` would be the one
place in the language where mutation happens without the glyph. The short name `Scratch` is
chosen over `ScratchAllocator` because it is written at every call site that passes it down,
and the type it contrasts with is named right beside it.

## 7. The implementations that ship are runner-owned, which is what makes reload work

This is the constraint that nearly overturned §1.

[#17](https://github.com/adamico/ludo/issues/17) §3 rejects any `persist` declaration that can
transitively reach **a pointer into the reloadable image**, and §5 states it flatly: *"a
`persist` declaration may not reach a function value."* Compose that with §1 (an allocator is
a struct of function pointers) and #15 (a `List` captures its allocator) and #17's own worked
example stops compiling:

```
persist world: !World.World = World.new(heap)
```

— because if `World` holds one `List`, that `List` carries function pointers, and §3's
transitive check fires.

**The resolution: the allocator implementations this spec mandates are runner-owned — their
code lives outside the reloadable image.** #17 §3's predicate is already worded as *a pointer
into the reloadable image*, and runner-owned code is not that, so **the check passes with no
change to #17**. §1 survives intact and no new mechanism is added.

The consequence is accepted and stated rather than discovered: **a `persist` container backed
by a user-written allocator is rejected**, because that allocator's code *is* in the swapped
image. §3 already names the offending field path, so the diagnostic is in hand. The line is
defensible — blessed allocators persist, bespoke ones do not — and #17 §6's per-declaration
`unsafe` lifts it for anyone who owns the consequence.

In a release build there is no runner and no swap (#12, #17 §1), so "runner-owned" resolves to
"statically linked, outside any reloadable image", and the predicate is vacuous. #17 §3 keeps
the check on in release for criterion-4 reasons, and it continues to pass.

**Rejected: an allocator as a reload-stable handle** — a `distinct u32` into a runner table,
which #17 §3 confirms would be reload-invariant by construction. It is the strongest
alternative and it fails on cost, not correctness: it reverses §1's encoding, adds a
registration API for user allocators, and puts a table indirection on every allocation to buy
a property §1 already has for free.

## 8. Prelude, not a sixth facade — and `$.mem` is respelled

`CONTEXT.md` records **five** facades. The allocator does not become a sixth. `Allocator` and
`Scratch` are **core prelude types**.

A **facade** is defined as *"a stable front over a tier that is free to
evolve beneath it"*. There is no tier beneath memory; there is no backend to delegate to; and
**core conformance is headless**, so the allocator must exist where no backend does. Putting
memory in the same category as sound and input is a category error that the word *facade*
would then have to absorb.

**#17 §2 and #26 spell `$.mem.heap` in resolved text. The spelling is corrected to a bare
`heap`.** This changes no decision either ticket made — it removes a `$.` prefix that reads as
a facade call and is not one.

**`heap` is legal only in `persist` initialisers, in the entry file.** #8's allocation-by-
absence — *a function that allocates must have received the allocator as a parameter* — is
defeated outright by a globally reachable heap, and it is the mechanism the whole memory model
rests on. #26 already makes `persist` initialisers cold-start-only and #17 §2 already confines
`persist` to the entry file, so the global is reachable in exactly one place that is already
exceptional, and every ordinary function still receives its allocator or does not allocate.

## 9. What the spec mandates, and what is merely ours

Following ADR-0032's treatment of the audio engine tier — *ours, own repo, blessed by
reference, unmandated*:

**Mandated:** the `Allocator` and `Scratch` types, the frame arena handed to the drawing entry,
and one general-purpose `heap`. These are load-bearing on surfaces the spec already publishes —
`decode_image` takes an allocator, the drawing entry hands one over, `persist` initialisers
need one.

**Blessed but unmandated:** #8 §7's **AoS struct pool** and **columnar SoA pool**, and the
**checker allocator**. They are real, they are ours, and nothing in the mandated surface names
them. The checker allocator is unmandated and still delivers #8 §7's promise, because it is an
ordinary value a program passes where it would have passed any other.

## 10. No new experience-contract property, and the reason

Per ADR-0030 §8, the absence is recorded rather than left silent. **No new
[#19](https://github.com/adamico/ludo/issues/19) property is minted. The count stands at
P1–P15.**

#17 §6 already reads *"reload preserves state for every `persist` declaration not marked
`unsafe`"*. A persistent container backed by a mandated allocator is precisely a `persist`
declaration that survives a swap, so the property that would be added here is the one already
asserted, restated in memory's vocabulary. §7's rule is enforced by a **compiler check**, which
is #17 §3's, not a runtime observable a harness could assert.

**Zero [#24](https://github.com/adamico/ludo/issues/24) delta.** No grammar is added: an
allocator is a struct, `Scratch`'s restriction reuses #15's view rule, and `List[T]` keeps its
spelling. This is the fifth consecutive zero.
