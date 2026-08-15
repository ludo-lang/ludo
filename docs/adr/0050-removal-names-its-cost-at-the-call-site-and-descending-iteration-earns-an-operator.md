---
status: accepted
---

# Removal names its cost at the call site, and descending iteration earns an operator

Resolves [#105](https://github.com/ludo-lang/ludo/issues/105), chapter 3's marked gap at §11.13.
[ADR-0048](0048-every-allocating-call-is-fallible-and-a-persist-initialiser-faults.md) closed the
container set's *membership* and its *growth and failure* surface and deliberately left removal
open. This is removal.

It is longer than its question, because the answer did not stay inside the stdlib. Deciding how an
element leaves a `List` forced a grammar operator, a reified type, and the first recorded overrule
of the grammar budget. §8–§10 are that spill, and §13 pays for it.

## 1. `List` removal is two operations, and both are named

**`remove(i) -> T`** shifts and preserves order, O(n). **`swap_remove(i) -> T`** moves the last
element into the hole, O(1), and reorders. Both ship.

Two names is a real stdlib cost and it is the correct one. ADR-0048 §2 was careful about the class
of API where a call silently does something the caller did not ask for; a single `remove` that
reorders is exactly that, and the reordering is invisible at the call site precisely where it
matters — an entity list someone else's index is pointing into. The agent-friendliness lens decides
it: an agent that writes `swap_remove` has stated the reordering contract **locally**, without
reading the rest of the program, which is the property this project asks of every surface.

**`pop() -> ?T`** removes the last element. It is `?T` and not `T` because an empty list is an
ordinary state rather than a program error (§4).

## 2. `Map` removal is tombstoned, and insertion order does not move

`Map.remove(k) -> ?V`. The entry is **tombstoned**; tombstones are **compacted on the next growth
or `reserve`**.

#15 mandates insertion-order iteration (chapter 3 §13.6) and this ADR does not touch it. The two
candidate implementations were a swap into the hole, which breaks that guarantee outright, and
tombstones, which keep it at the cost of slots held until a compaction.

The tombstone cost is **unobservable by construction**, and ADR-0048 §4 is what makes that true.
There is no `capacity()`, so no program can see the difference between a live slot and a dead one,
branch on it, or come to depend on a compaction schedule. The clause that looked like a problem for
removal — a `len`-versus-slots divergence a program cannot observe — is the clause that licenses
the answer. `len` counts live entries only.

**Named cost:** a delete-heavy `Map` that never grows holds its slots for the life of the map.

## 3. Removal returns the element

`remove` and `swap_remove` return the removed value; `Map.remove` returns `?V`.

Containers move on assignment (#15), so a removal that returned nothing would **destroy** a value
the caller may have wanted, and the workaround — read at the index, then remove — copies where the
removal could have moved. The `?` on the `Map` form is chapter 3 §13.7's legitimate absence, a
missing key, and not §4's program error.

## 4. Out-of-range removal is a fault; an empty `pop` is a value

`remove(i)` and `swap_remove(i)` with `i >= len` **fault**, matching indexing. An index past the
end names a slot that does not exist and admits no sensible reading, and returning `?T` there
invites a caller to swallow a bug. `pop()` on an empty list is `none`, because "the list is empty"
is a state a program is expected to be in.

## 5. Views die by the existing rule; indices do not, and the boundary is stated

Every removal takes its receiver with `!`, so
[ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md) §3 already
kills every outstanding view. **No new clause is created**, and chapter 3 §11.13's replacement text
says so explicitly.

It says so because the **index** case is different and a reader needs the line drawn.
`swap_remove` relocates a live element to a new index, invalidating an index a program is holding
— and an index is not a view, has nothing to derive from, and nothing kills it. This is a
**documented consequence, not a hazard the language addresses.**

The alternative was making indices trackable, which means making them linear: a whole type
discipline, imported for one method, in a language whose entire indexing story is `0..<xs.len`. A
generational-handle container is blessed-unmandated territory (ADR-0048 §2), not the mandated set.
The mitigation that ships is §1's naming.

## 6. Removal during iteration is a compile error, and the loop that culls is not iteration

Chapter 3 §12.2's open pointer resolves to **yes**: removal is structural mutation, it falls under
the existing clause, and §12.2 grows no carve-out.

This was expected to cost a `retain(pred)` and it does not, because the despawn loop was never
iterating the container:

```ludo
for i in rocks.len>..0 do
  rocks!.swap_remove(i) if rocks[i].hp <= 0
end
```

The loop iterates a **range**. It takes no lend on `rocks`, so §12.2's exclusive lend never exists
and there is nothing for the ban to fire on. The ban stays absolute and the despawn loop stays
writable, which is the outcome a carve-out would have bought at the price of a special case.

**Descending is the correctness condition, not a style.** `swap_remove(i)` moves the last element
into slot `i`; iterating upward, that slot is ahead of the cursor and the moved element is silently
skipped. Iterating downward it is behind the cursor, already visited. The direction is what makes
the idiom correct, and §8 is why it is spellable.

## 7. The stable cull needs no exchange primitive

Order-preserving culls pack survivors forward and cut the tail:

```ludo
w := 0
for i in 0..<rocks.len do
  if rocks[i].hp > 0 then
    rocks![w] = rocks[i]
    w += 1
  end
end
rocks!.truncate(w)
```

No `swap(i, j)` ships. Plain-struct elements copy on assignment, so the duplicate left behind is
cut by the `truncate`, and the scratch binding an exchange would need never appears.

## 8. `>..` is the descending range operator — the reversal

`n>..0` iterates `n-1` down to `0`. It joins `RangeExpr` in `grammar.ebnf` at `..<`'s precedence.

Chapter 1 §7.7 said *there is no inclusive range operator*, and that clause was read — including
by this session, at first — as ruling out a **descending** one. It does not. It rules out `..=`,
on the ground that `0..<xs.len` is the always-correct shape and `0..=xs.len` is an out-of-bounds
bug with no spelling. Direction was never decided; an absence was mistaken for a prohibition.

The mirrored form preserves that argument exactly. In `0..<n` the angle sits **beside the bound it
excludes** and points at it; in `n>..0` it does the same. So one sentence covers both — *the angle
points at the bound it excludes* — and the invariant survives whole: **the bound written as the
length is always the excluded one**, making `xs.len>..0` the always-correct descending shape as
`0..<xs.len` is the ascending one. The out-of-bounds spelling has no form in either direction.

An empty or inverted range — `5..<3`, `3>..5` — **iterates zero times** and is neither a fault nor
a compile error. `0..<xs.len` on an empty list is `0..<0`, which must yield zero iterations for
§12.3's always-correct shape to be true at all; an inverted range is the same case by another
route.

`>..` is a single token by maximal munch. It does not collide: `>>` is a distinct munch and there
is no prefix `..`, so `a > ..b` parses as nothing today.

## 9. Ranges are values, and now they have a type

Chapter 1 §7.7 also said *ranges are ordinary `Iter[int]` values, not grammar*. The second half is
false as written — `..<` **is** grammar, a `RangeExpr` production, and §13.6 spends an operator
token on it. What the clause reaches for is a contrast with Lua, where `for i=1,n` is a loop-header
form and no range object exists. The accurate statement, and the repair: **the operator is grammar;
the value it produces is an ordinary type.**

That exposed the real gap. No `Range` type existed anywhere in the spec, so `r := 0..<n` — legal
today under §6.2 and §7.1, an ordinary expression bound to an ordinary name — produced a value the
spec could not name. `Iter[int]` is not the answer: chapter 3 §12.5 makes `Iter` an interface whose
**declaration form does not exist** ([#100](https://github.com/ludo-lang/ludo/issues/100)). The
clause named a constraint in place of a type.

**`Range` and `RevRange` are reified**, as **prelude identifiers** on chapter 1 §2.6's precedent.
A core operator's result type must be nameable without a `use`, and the prelude is where `usize`,
`heap`, `none` and `some` already live, at zero cost against §13.

They satisfy `Iter[int]` by **blessed conformance** — the spec says so; no user-writable
declaration is involved. Chapter 1 §9.6 already makes exactly this move for a `fn` pointer
satisfying a single-function interface. So this follows precedent rather than minting a mechanism,
and — decisively — it means `>..` does **not** block on #100.

Reification also collapses a special case: `xs[2..<5]` stops being slice-shaped syntax and becomes
ordinary indexing by a `Range`.

## 10. `RevRange` does not slice

`xs[n>..0]` is an error. A reversed view needs a direction or a stride in the view representation,
which chapter 3 §7's views do not carry.

Under §9 this is no longer *the mirror does not slice* but a **type fact**: `Range` is admitted in
the index suffix and `RevRange` is not. It is an asymmetry between two sibling types introduced in
the same breath as the symmetry that suggests it, so it earns an explicit clause rather than a
silence — a reader who has just learned *the angle points at the excluded bound* will try it.

## 11. `truncate(n)` ships, and per-frame reuse is what buys it

`truncate(n)` cuts a `List` to its first `n` elements and returns `()`. `n > len` is a **no-op**:
it is a bound — *make `len` at most `n`* — not an index, so §4's reasoning does not carry, and
`n == len` must be legal because it is where §7's compaction loop lands when nothing was culled.

**The buffer survives.** That is the whole point, and it is a ludo-specific point: ADR-0048 §4
withholds `capacity()`, so `truncate` is the only mandated call that drops elements while keeping
the allocation. The dominant use is `truncate(0)` at the top of a frame — a draw list, a
collision-pair list, a spawn queue emptied and rebuilt every frame from a buffer allocated once at
cold start. A `persist` list cannot use `scratch` for this, and building a fresh `List` allocates
every frame.

The stable cull (§7) and length-capped buffers are the other uses. Culling alone would not have
justified it.

**Return type.** `()`, not the dropped elements and not a count. Returning the tail requires
allocating a second `List`, which makes a bookkeeping call fallible under ADR-0048 §3 and hands the
most frequent call in a frame loop an `OutOfMemory` it has no business having. The count is `len`
before minus `n`, computable from a value already in hand — ADR-0048 §4's reasoning by analogy: do
not mint a second way to learn one fact.

**The name.** `truncate` is semantically contested — it describes the operation from the cut end
rather than the kept end — and it ships anyway, on #4's naming-failure grounds and chapter 1
§14.1's precedent for `pub`: it is the highest-corpus spelling for this exact operation, an agent
that has never seen ludo will type it, and a more precise novel synonym is a hallucination
generator. `clear_after(n)` was the considered alternative and is the better description. This is
the agent-friendliness lens outranking naming precision, recorded because the map's Notes require
the conflict to be named rather than resolved silently.

## 12. Rejected, and why

**`retain(pred)`.** The expected answer to §6, and it is not writable. Chapter 2 §7.2 makes
closures a stated non-goal (ADR-0017) — no capture of any kind — so a predicate needing frame state
(*cull everything outside the camera*) must take chapter 2 §7.4's interface-bounded form, and
chapter 3 §12.5 says the interface declaration form does not exist (#100). `retain` is deferred to
the map's fog, not rejected: it becomes writable when #100 lands. §6's range loop is why nothing is
blocked in the meantime.

**`clear()`.** `truncate(0)` is the same meaning. Chapter 1 §7.10a's principle — one spelling per
meaning, a legal synonym gives agents two ways to write one thing — rules it out. Slightly worse to
read; consistent with how this spec has already decided that class of question.

**`swap(i, j)`.** §7 needs no exchange.

**`List.with_capacity`.** Reached for while writing §11's example and it does not exist. ADR-0048
§4 makes `reserve(n)` *the* sizing call, and a capacity constructor is a second sizing spelling
re-opening a closed decision. `List.from(heap, {…})` seeds; a `persist` list wanting room reserves
on its first frame.

**A front-drop or FIFO.** The mandated set has none, so a length-capped trail keeps the oldest
entries where a trail wants the newest. Ruled **out of scope**: #105 asks how an element leaves a
container, and ADR-0048 §2 closed set membership deliberately — re-opening it through a removal
ticket smuggles in a container the set does not have. Filed to the backlog as
[#108](https://github.com/ludo-lang/ludo/issues/108), not to the map's fog, under ADR-0044 §7: no
line of the reference program becomes writable by answering it.

## 13. The grammar budget is overruled, and this is the first time

`>..` takes core operators 39 → 40 and the core total **71 → 72**. Against Lua 5.4's 55 that is
**+30.9%**, outside chapter 1 §13.7's ~30% target. §13.9 requires a **recorded overrule on the
map**, and this ADR is its ground. The budget was already at +29.1%; this is the first clause to
cross, so it sets the precedent for how §13.9 is exercised.

**The payment is tier 2, not tier 1.** §13.8's tier test is machine-checkable and is not a
judgement call: anything `#explicit` rejects is sugar. `#explicit` does not reject `>..`, so it is
semantics-bearing and owes no one-in-one-out deletion — it owes a permanently named deleted failure
class.

**The failure class `>..` deletes: the ascending swap-remove skip.** Culling upward while
swap-removing silently skips elements — no crash, no diagnostic, a rock that survives its own
death. Without a descending range the correct loop has no spelling in this language, so an author
reaches for the ascending one and the bug is the default outcome. That is precisely the shape
§13.8 tier 2 exists to buy.

## 14. The stamp list

- **ADR-0048** — **extended, not amended.** §2's set, §3–§6's fallibility, §4's withheld
  `capacity()`, §5's symmetry and §9's runner ownership all stand; this ADR adds the departure
  surface §10 left open. §4 is load-bearing here in a way it was not written to be (§2, §11).
- **ADR-0047 §3** — **unchanged and reaffirmed.** Removal kills views by the existing rule; §5
  states the index boundary rather than moving it.
- **#15 Q12** (insertion-order iteration) — **unchanged.** §2 pays for it with tombstones.
- **#15 Q19** (structural mutation during iteration) — **unchanged and sharpened.** §6 resolves
  chapter 3 §12.2's open pointer to *yes* with no carve-out.
- **#15 Q24 / chapter 1 §7.7** — **reversed, twice.** A second range operator (§8), and *not
  grammar* corrected to *the operator is grammar, the value is a type* (§9). This is the reversal
  that makes this ADR owed under ADR-0044 §6.
- **ADR-0021 §1 / chapter 1 §13.6–§13.7, §13.12** — **recount**, per §13.
- **ADR-0017 / chapter 2 §7.2, §7.4** — **unchanged.** §12 records that they are what defers
  `retain`.
- **[#107](https://github.com/ludo-lang/ludo/issues/107)** — filed, not resolved here: chapter 3
  §12.2's `for x in !xs` contradicts chapter 1 §7.10's postfix mark. Independent of this ADR and
  on the map in its own right.

## 15. Costs and the three lenses

**The cost, named:** four names on `List` (`remove`, `swap_remove`, `pop`, `truncate`), one on
`Map`, one grammar operator, two prelude types, and a budget overrule — for a question that
looked like it would cost one method. The spill is real and §8–§10 are where it went.

**Simplicity.** Mixed, and this is the lens that loses most. A Lua user recognises `remove` and
`table.remove`'s shifting semantics, and will not recognise `swap_remove`, `>..`, or a
`truncate` that keeps its buffer. What is preserved is that every one of them is a *name* rather
than a rule to remember: nothing here changes the meaning of code that does not use it.

**Robustness.** Two silent-bug classes deleted outright — the ascending swap-remove skip (§13) and
the iterator invalidation §6 keeps banned — and one deliberately left as a documented consequence
(§5's held index), stated rather than hidden.

**Agent-friendliness — the decisive lens twice.** It is what makes `swap_remove` a separate name
(§1: the contract is legible without reading the rest of the program) and what keeps `truncate`'s
contested name (§11: corpus recall beats description). Both are cases where the agent lens
outranked the beginner lens, which the map's Notes predicted would happen and asked to have named.
