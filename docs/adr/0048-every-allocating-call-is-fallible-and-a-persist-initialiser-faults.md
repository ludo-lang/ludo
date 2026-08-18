---
status: accepted
---

> **Extended by [ADR-0050](0050-removal-names-its-cost-at-the-call-site-and-descending-iteration-earns-an-operator.md):**
> §10 handed removal to #105, and #105 is now resolved. Nothing here is reversed — §2's set,
> §3–§6's fallibility, §5's symmetry and §9's runner ownership all stand — and the departure
> surface §10 left open is added: `remove`, `swap_remove`, `pop` and `truncate` on `List`,
> `remove(k) -> ?V` on `Map`. **§4 turns out to be load-bearing beyond what it was written
> for**: the withheld `capacity()` is what makes `Map` tombstones unobservable, and so is what
> licenses removal keeping #15 Q12's insertion order.

> **Absorbed by [spec ch3](../spec/03-memory.md):** the chapter is normative; this ADR
> keeps the argument — fallible allocating calls and the mandated container set are transcribed there; the `persist` initialiser fault is chapter 5's.

# Every allocating call is fallible, and a `persist` initialiser faults instead

[#82](https://github.com/ludo-lang/ludo/issues/82), the container-set gap that spec chapter 3
([#87](https://github.com/ludo-lang/ludo/issues/87)) records as the one thing it could not
transcribe. The ticket carried four questions and arrived with one already answered:
[ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md) §9 states
that its **view/growth seam** — *is `v := xs.items()` legal, and what happens across a `push`* —
is settled there in full, and hands #82 the set's membership and the growth/failure surface.
This ADR does not re-author §3; it consumes it.

## 1. The membership test, stated before it is applied

The corpus has been admitting containers on *"a game reaches for it"*, which is unfalsifiable —
every structure is one some game reaches for. Three tests were weighed and the middle one wins:

**A type is mandated when it cannot be built in the safe layer out of what is already mandated.**
Ties break on **silence**: ship it when a hand-rolled version fails in a way no compile error
catches.

`List` and `Map` pass, and pass for a stated reason rather than by assumption: both need an
allocator, and both need the single-buffer release that
[ADR-0042](0042-an-allocator-is-a-struct-of-function-pointers-the-runner-owns.md) §2 puts inside
`unsafe` because *the safe layer has no spelling for it*. That is the line, and it is the line
the corpus was already using without writing down.

**Rejected: "the reference program needs it."** Too tight. The reference program is one program,
and pinning the mandated stdlib to its accidents freezes the surface on a sample of size one.
**Rejected: silence alone.** It is the tie-breaker, not the test — it admits a ring buffer and a
dozen other structures a stdlib does not owe.

## 2. The set is exactly `List` and `Map`; a ring buffer is blessed, and no `Set` ships

**Mandated: `List[T]` and `Map[K,V]`**, as [#15](https://github.com/adamico/ludo/issues/15) has
them — library generics over the two core aggregates plus an allocator.

**Blessed but unmandated: a ring buffer**, in ADR-0042 §9's sense (ours, own repo, blessed by
reference, nothing in the mandated surface names it). It **fails** §1's test outright — it is
`[N]T` plus a head index and a count, a plain struct over the mandated fixed array, needing no
allocator and no `unsafe`. It wins the tie-breaker: the wrap arithmetic and the
full-versus-empty ambiguity when head meets tail are silently wrong data, not diagnostics. And it
earns its place *more* once §3 makes `List.push` fallible, because a fixed-capacity structure
whose `push` never allocates is the escape hatch from the ceremony §3 imposes — input history,
trailing-N samples, an event queue drained per frame.

**No `Set[K]`.** #15 mints no unit type, so a blessed `Set` would either drag one in — a
[#24](https://github.com/adamico/ludo/issues/24) grammar cost paid for a naming preference — or
be a hand-written wrapper over `Map`. It fails §1's test cleanly, being expressible over the
mandated `Map` with no `unsafe` at all. The spelling is `Map[K, bool]`.

**The one cost, named:** `Map[K, bool]` has two states where a set has one, and *present but
`false`* versus *absent* is a quiet bug class. It is accepted rather than dismissed: the fix is a
`distinct` wrapper in the program that wants set semantics, which is the same move
#11 used for units, and it is cheaper than a unit type in the core.

## 3. The trilemma, and `push` is fallible

Three things cannot all hold:

1. `push` is infallible in the loop body.
2. Exhaustion is not a run-time fault.
3. A `List` can grow.

**(2) is fixed and not up for trade.** A `List` whose over-push faults is a run-time crash landing
on the most common line a beginner writes, which is exactly the
[#11](https://github.com/adamico/ludo/issues/11) criterion-5 veto — *a veto on any feature whose
failure mode is a bad error message* — and flow-insensitively the compiler cannot see the missing
`reserve` above the loop.

**So (1) breaks: `push` is fallible.**

```ludo
for e in spawns do
  xs!.push(e) rescue return
end
```

**This is not a new tax; it is [#10](https://github.com/adamico/ludo/issues/10) holding.** That
ticket already faced this complaint in general form and ruled: *"There is no ignoring layer —
must-use is universal. The beginner's softener is **brevity, not permission** — one declared error
enum per file, an elided error set, and `rescue return` / `rescue 0` being shorter than the Lua
`if err then return end` they replace."* A fallible `push` is that ruling reaching the container.
It also leaves ADR-0042 §3 — the ADR that deliberately made exhaustion a value — untouched.

**Rejected: breaking (3) by fixing capacity at construction.** `List.with_capacity(alloc, n)`
fallible once, no growth thereafter. Over-push still needs a disposition, and every disposition is
a fault (out), a fallible call (this section), or a returned `?` that must-use turns back into
ceremony. The horn does not break.

**Rejected: growth is an explicit fallible `reserve` and over-push is a bug** in #10's
OOB-index class. This was the position this ADR's author held entering the session, on the ground
that *past what you reserved* is the same programmer error as an out-of-bounds index. It dies on
(2): only a statically-bounded push is checkable, and a `push` inside a loop is precisely the case
no flow-insensitive check bounds, so it degrades to a run-time fault wearing a compile-error's
name. [#6](https://github.com/adamico/ludo/issues/6) forbids the escape — layering varies spelling
and never semantics, so a beginner cannot be given an auto-growing `List` while a veteran gets the
reserved one.

**The price, stated plainly:** `rescue` now appears on the most-written call in the language.

## 4. `reserve` ships and is additional; `capacity` does not ship

`len` was never in question — it is the loop bound, and #15's `[]T` views carry it.

**`reserve(n)` ships, and `n` is *additional*:** ensure room for `n` beyond `len`, idempotent, a
no-op when already satisfied. Once `push` is fallible, `reserve` is no longer load-bearing for
*failure*; it is load-bearing for *timing*. A game that must not allocate mid-frame pre-sizes at
load, and the map's standing preference puts frame stability above everything it trades against.
The absolute reading — *ensure total capacity `n`* — was rejected because it makes the caller do
arithmetic against `len`, which is the arithmetic that wants the accessor the next paragraph cuts.

**`capacity()` does not ship.** It exposes the growth policy as observable surface, a program can
branch on it, and the first program that does freezes the policy forever. The eight candidate uses
were enumerated rather than waved away, because the claim *every use dissolves* is the kind that
is false until checked:

| Use | Disposition |
| --- | --- |
| Debug HUD / memory headroom | Real — but wants **bytes across an allocator**, not elements in one list. ADR-0042 §9's **checker allocator** is the instrument. |
| Building your own container over `List` | Real — but ADR-0042 §2 points veterans at `unsafe` over the allocator's `[]u8`, not at layering on `List`. |
| Caller-side amortization (`capacity - len < n`) | **Dissolves** under this section's additional, idempotent `reserve`. |
| Test asserting a loop did not allocate | Real and important — and answered better by the checker allocator, which counts allocations for the whole frame rather than one container. |
| Shrink-to-fit (`len * 4 < capacity`) | **Dead.** ADR-0042 §2 has no per-allocation `free`; release is `reset()`. There is no capacity to return. |
| Serialization / save sizing | Uses `len`. |
| View-versus-clone choice | Uses `len`. |
| `reserve(capacity() * 2)` | An **anti-use** — reimplementing at the call site the policy the container owns. This is the one that freezes it. |

Two survive, and both have a better-aimed instrument already blessed. **The asymmetry decides it:**
adding `capacity()` later is purely additive, while removing it after
[ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) freezes
the surface is a breaking change to the most-used generic in the language.

## 5. `Map` is symmetric with `List`, including `reserve`

`Map` allocates on insert exactly as `List` does on push, so `map!.insert(k, v)` is fallible in the
same shape, and `Map` gets `reserve` too.

The objection considered was that a hash map's pre-sizing interacts with load factor and rehashing
in a way a list's does not, so the knob would over-promise. That argues for what `reserve`
*documents*, not for its absence: *room for `n` more entries without reallocating* is a promise a
hash map keeps by sizing for its load factor, and a game pre-sizing a spatial hash at load needs it
for the same frame-stability reason a list does. **Asymmetry between the two mandated containers is
a thing every user must learn and nothing pays for it** — #15 already moves them together on
move-on-assignment, `clone(allocator)`, `Iter[T]` and structural `==`.

## 6. Every allocating call is fallible, with exactly one carve-out

`clone(allocator)` and `List.from(alloc, ...)` allocate, so both are fallible.

**One sentence, no exceptions to memorise: a call that allocates can fail.** Any carve-out by *call*
is a second rule and a hole in the guarantee. The tempting one — construction is unrecoverable
anyway, so let it fault — was rejected because *"unrecoverable anyway"* is the program's judgement
to make, and ADR-0042 §3's entire argument is that the program decides what to do about exhaustion.

The single carve-out is by **position**, not by call, and §7 is what it costs.

## 7. A `persist` initialiser cannot `rescue`, so it faults

This is the section that narrows ADR-0042 §3.

§6 makes every allocating constructor fallible. A `persist` declaration is not inside a function:
[#17](https://github.com/adamico/ludo/issues/17) §2 confines it to the entry file,
[#26](https://github.com/adamico/ludo/issues/26) makes its initialiser cold-start-only, and
ADR-0042 §8 makes `heap` legal in exactly that position and nowhere else. So:

```ludo
persist enemies: !List[Enemy] = List.from(heap, {})
```

`List.from` has type `List[Enemy] rescue OutOfMemory`; the declaration's type is `!List[Enemy]`.
#10 makes must-use universal and propagation explicit at every call site, so this binds a
`T rescue E` where a `T` is declared and does not typecheck. **All three of #10's exits are
unavailable:**

- **`rescue return`** — there is no function. Nothing to return from.
- **Propagate** — there is no signature. `persist` declares a type, not an error set.
- **`rescue <fallback>`** — available, and a trap.

**The fallback deserves the honest treatment**, because the easy claim — *it is impossible* — is
false. A zero-capacity `List` allocates nothing (null buffer, `len` 0), so `List.empty()` is
genuinely infallible and this compiles:

```ludo
persist world: !World = World.new(heap) rescue World.empty()
```

`World.new` is `fn new(a: Allocator) -> World rescue OutOfMemory` — it calls `List.from`, and a
constructor cannot `rescue return` a half-built `World`, so per §8 it propagates. The author then
writes a fallback world to satisfy the binding, **the game starts**, the frame loop runs, and it
renders an empty level. That is silent wrong behaviour under unchanged familiar notation —
[#4](https://github.com/adamico/ludo/issues/4)'s worst-measured class and criterion 5's exact
prohibition. *The fallback that is easiest to write is the one that hides the failure.*

**The decision: allocation failure in a `persist` initialiser is a
[#18](https://github.com/adamico/ludo/issues/18) fault**, whose report names the declaration.

**This narrows ADR-0042 §3**, which reads *"Allocation failure is a `rescue`-able error, never a #18
fault."* The narrowing is scoped and the reason is not a change of mind about that ADR: §3's
deciding case is a *running* game handling a design condition — the spawner hitting a cap the
designer chose. A `persist` initialiser is definitionally not that. It is cold start, in the entry
file, before the first frame, with no program yet to decide anything and nothing on screen to
degrade. The carve-out is by **position**, the same shape as ADR-0042 §8's own confinement of
`heap` to this one already-exceptional site.

**Rejected: banning containers from `persist`.** A non-starter — #17's own worked example is a
`World` that holds lists.

## 8. `rescue` stays local, and `OutOfMemory` is not viral

A function containing a fallible `push` does **not** have to declare an error set. `rescue return`
and `rescue break` discard locally; only a function that chooses to *propagate* writes
`rescue OutOfMemory` in its signature.

This is #10 working as written — `rescue` is *"purely local, no stack involvement"*, and
`local f = $.io.open(path) rescue return` is #10's own example in a function that declares nothing.

**Rejected: making a containing function surface it.** That is Zig's inferred error set, which #10
rejected by name: a callee adding a variant silently changes your signature, criterion 4's exact
prohibition. **Rejected: making `OutOfMemory` a variant every error set implicitly absorbs.** An
implicitly-absorbed variant is a hole in *nominal and closed*, and it is precisely why ADR-0042 §3
added the variant to the mandated facade sets **by hand** rather than minting the rule.

## 9. The containers are runner-owned

`List` and `Map` implementations live outside the reloadable image, alongside the allocator
implementations.

This follows from ADR-0042 §7 rather than deciding anything new. A `List` captures an allocator that
is a struct of function pointers, and §7 put those pointers outside the reloadable image precisely
so `persist world: !World` survives #17 §3's transitive check. If the container's own code —
including the `unsafe` release seam of ADR-0042 §2 — sat inside the image, §3's check has a second
thing to trip on.

**The alternative worth answering: containers need no release seam at all**, because with no `free`
the old buffer is abandoned. True for a pure arena, where abandonment costs nothing and `reset()`
reclaims it. False for the mandated `heap`, where abandoning every superseded buffer is a leak that
grows with every reload. The seam stays.

## 10. What is handed on

- **Removal semantics are a new ticket, not an addendum here.** Two edges are underivable from the
  corpus: whether `List` removal is swap-remove (O(1), reorders — what games use for unordered
  entity lists) or shifting `remove(i)` (O(n), order-preserving), or both under separate names; and
  the sharper one, that #15 mandates **insertion-order iteration** for `Map` while removal forces
  either tombstones (slots leaked until a compaction nobody asked for) or a swap into the hole
  (which breaks the ordering guarantee). That is a genuine collision between two closed decisions
  and deserves its own argument, not a closing paragraph in a ticket about membership.
- **Spec chapter 3** ([`docs/spec/03-memory.md`](../spec/03-memory.md)) transcribes §1–§9 as
  clauses citing this ADR, and its lines 11–13 — *"Nothing here names a `List`, a `Map` or a pool:
  the dynamic container set is the one thing chapter 3 records as unspecified (#82)"* — stop being
  true and must be rewritten.
- **The reference program** gains container use it currently has none of: a fallible `push` with a
  `rescue`, a `reserve` before a spawn batch, and a `persist` container declaration exercising §7.

## 11. The stamp list

- **ADR-0042 §3** — *"never a #18 fault"* is **narrowed** by §7: allocation failure in a `persist`
  initialiser is a fault. Every other position is unchanged.
- **ADR-0042 §9** — the blessed-but-unmandated list gains the **ring buffer** (§2). Additive; the
  mandated list is unchanged.
- **ADR-0047 §9** — its hand-off to #82 is discharged. No change to ADR-0047.
- **#15** — the container set it left open is closed at `List` + `Map` (§2); its `push`, `clone` and
  `from` become fallible (§3, §6). Recorded in [`SOURCES.md`](SOURCES.md), an issue being
  unstampable.

## 12. Costs, properties and the three lenses

**#24: zero grammar delta.** No new keyword, operator or literal form. `reserve`, `push` and
`insert` are ordinary calls; the ring buffer is an unmandated library type; the `persist` fault is a
compile-time rejection plus an existing #18 report. This is the sixth consecutive zero.

**No new [#19](https://github.com/adamico/ludo/issues/19) property**, per ADR-0030 §8's requirement
to record the absence. P1–P15 stands. The `persist` fault is observable, but it is #18's pause
already asserted, reached through a new door.

**Simplicity — the weakest lens here, and the cost is real.** *A call that allocates can fail* is one
sentence a Lua user can hold. What they cannot hold as easily is that `xs!.push(e)` — the single most
ordinary line in the language — needs a `rescue`, when in Lua `t[#t+1] = v` needs nothing. #10's
brevity answer applies but does not erase it.

**Robustness — decisive, as usual.** Exhaustion is a value everywhere a program can act on it, and a
fault in the one position where the only available handler is a trap. No silent empty world, no
leaked buffers on the mandated heap, no policy frozen by an accessor nobody needed.

**Agent-friendliness — strong.** Two rules with no exceptions to look up: *every allocating call is
fallible*, and *`persist` is the one position where that becomes a fault*. Both are decidable from
the text in front of the agent, and the second is a position it can see. The absence of `capacity()`
removes a surface an agent would otherwise be tempted to branch on.

**The lens conflict, named:** simplicity wanted the infallible `push` and would have paid for it with
a fault. Robustness refuses, because that fault lands on the beginner's most common line, and
criterion 5 vetoes features whose failure mode is a bad error message. Robustness wins and simplicity
pays in `rescue` keystrokes — the same trade #10 made, reaching the container for the first time.
