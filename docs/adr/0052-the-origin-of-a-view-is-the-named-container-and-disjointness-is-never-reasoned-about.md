---
status: accepted
---

# The origin of a view is the named container, and disjointness is never reasoned about

[#103](https://github.com/ludo-lang/ludo/issues/103), found while transcribing spec chapter 3
([#87](https://github.com/ludo-lang/ludo/issues/87)). Two sources disagree about whether distinct
columns of one pool are distinct places, and both are load-bearing:

- [#25](https://github.com/ludo-lang/ludo/issues/25) §9 — *"Exclusivity is per place, and distinct
  columns are distinct places."* `rocks.pos!` beside `rocks.vel` is accepted; `rocks.pos!` beside
  `rocks.pos` is a compile error naming the column. Called *"the one place the design gets a
  borrow-checker result without a borrow checker"*.
- [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md) §3 —
  *"**There is no exclusivity rule.** Any number of views of one place may co-exist, readable or
  writable; only `!` or a move on the **origin** kills them."*

Spec chapter 3 §10.10 transcribed only what both agree on and recorded the disagreement, because
resolving it either way reverses a decision and
[ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §6
reserves a reversal for an ADR. This is that ADR.

**The disagreement is smaller than it looked, and it was already half-settled.** #25 §9 is stated
over an expression the landed grammar rejects, and the range dimension of the same question was
decided after ADR-0047 by [#101](https://github.com/ludo-lang/ludo/issues/101). What is left is
one rule that covers both dimensions, and it is §2.

## 1. #25 §9 is written in a spelling the grammar does not have

`rocks.pos!` is not a legal expression. Chapter 1 §7.10 fixes where the mark sits, and it does not
leave the field case open:

> **`!` marks mutation at the place, and it rides the root binding.** The mark sits immediately
> after the root binding and the suffix chain follows it, **whatever the suffixes are — field,
> call, index or slice.**

Chapter 1 §7.10a states the negative for the slice suffix — `name[a..<b]!` parses as a `Suffix*`
chain but is an error, *"the mutation mark belongs on the root"* — and a field is a suffix by the
same clause. Chapter 3 §6.12 repeats it. All three trace to #101, which is **later than both #25
and ADR-0047**.

**So the mandated spelling of a writable column is `rocks!.pos`**, and #25 §9's two exhibits are
ungrammatical as written. This is stated first because it changes what the rest of this ADR has to
do: the question *"is a `!`-use of `rocks.pos` a `!`-use of `rocks`?"* has no referent, since there
is no such use to classify. The only writable use available is a mark on `rocks`.

**This ADR does not decide the marker grammar.** #101 did. ADR-0047 §5's closing note — that
whether a writable sub-view is `name![a..<b]` or `name[a..<b]!` *"remains #15's marker grammar"* —
is stale, and §6 stamps it.

## 2. The origin of a derived view is the named container

**A view derives from the container its expression names, and no disjointness of any kind is
reasoned about.** A mark on that container kills every view derived from it, whatever part of it
each view covers.

Two dimensions, one rule. The range dimension is #101's, already in chapter 3 §7.6:

```ludo
a := xs[k..<n]
xs![0..<k]                               -- kills a, disjoint or not
```

The field dimension is this ADR's, and it is the same sentence:

```ludo
p := rocks.pos
rocks!.vel                               -- kills p, different column or not
integrate(rocks!.vel, dt)                -- likewise, by §7.3
```

**#25 §9 is reversed.** Distinct columns are **not** distinct places. Its permission (`rocks!.pos`
beside `rocks.vel` in one call) survives, but it survives as an instance of §7.4 — nothing is bound,
so nothing can die — and not as a disjointness grant. Its prohibition is withdrawn: it was expressed
in a spelling §1 rejects, and the shape it meant to forbid is already unwritable.

**Why the container and not the column.** The alternative — the column is the origin — was available
and is defensible: column names are literal field names of `T`, fixed at compile time, so the check
is a comparison of two identifiers and costs nothing. It was rejected on three grounds.

**It contradicts §7.6 in the other dimension.** #101 refused disjointness reasoning for ranges and
booked the cost openly. Admitting it for fields would mean the language reasons about disjointness
when the parts are named by identifier and refuses to when they are named by index — two granularity
rules where §7 has one, and no principle that explains which applies.

**The grammar has no place to express it.** Per §1 the mark rides `rocks`. A rule under which a mark
on `rocks` kills only *some* views of `rocks` makes the text say one thing and the rule do another,
which is the defect §7.8 exists to prevent.

**It reopens a closed list.** ADR-0047 §6 recorded *"no shared/exclusive distinction"* as a closed
list precisely so §7 is not read as a borrow checker arriving by instalments. #25 §9's own framing —
a borrow-checker result without a borrow checker — is the thing that list refuses.

## 3. Exclusivity leaves the memory model

**The words *exclusive* and *lend* are withdrawn from the view rules.** One mechanism remains: a view
derives from an origin, and a mark or move on the origin kills it. There is no second rule that keeps
views apart, and chapter 3 §7.4 already says so.

Retaining the vocabulary as description was rejected. A reader who meets *exclusive lend* looks for
the rule behind it and finds §7.4 denying that one exists. **#25 §9's claim to be "the one place the
design gets a borrow-checker result without a borrow checker" is withdrawn with it**, which is what
ADR-0047 §6 already declined for §7 as a whole.

The word survives in exactly one place, unrelated to views: chapter 3 §12.2's loop lend, which is
*"exclusive for its duration"* and is what deletes iterator invalidation. That is a statement about a
`for` loop's subject, not about views of a place, and it is untouched.

## 4. `pool.each()` does not exist and its clause is deleted

Chapter 3 §10.11 reads, in full: *"`pool.each()` lends the whole pool. (#25 §9.)"* It is deleted.

**This is a separate defect from §2's reversal, and it is not caused by it.** The clause names a call
the language does not have. `each` occurs exactly twice in the corpus — §10.11 and its own
`coverage/` row — and in no other chapter, no other ADR, and not in the reference program. Chapter 3
§12.1 forbids it outright:

> **There is one iteration construct**: `for x in xs` and `for k, v in m` over every aggregate.

A method that takes a body and calls it per element is a second iteration construct. The adapter
shape §12.4 permits is a different thing — `xs.pairs()` returns a value satisfying `Iter[T]` and the
`for` still iterates — and an `each()` of *that* kind is `for x in pool` with more words. A callback
form is unsupported below the grammar too: the language has non-capturing function pointers only
(ADR-0042 §2; #11 forbids the boxed existential), so a per-element callback could not see the loop's
surroundings.

Replacing the clause with a cross-reference to §7 was rejected: it would preserve a pointer to a call
that does not exist.

## 5. Named cost: a column cannot be bound across a write to its pool

**The ordinary two-system frame does not compile.**

```ludo
p := rocks.pos
integrate(rocks!.vel, dt)                -- kills p
$.print(p[0])                            -- error
```

This is stated as a cost, not discovered later. Columnar SoA is the blessed shape for exactly this
access pattern (#8 §7; ADR-0016 §2 calls a column *"a `[]T` view"*), and §2 makes the shape that
motivates it unwritable in bound form.

**The unbound spelling is unaffected**, and it is what the reference program already writes — at
[line 285](../spec/reference/reference.ludo) in the range dimension, and at line 346 in the field
dimension:

```ludo
integrate(pos![0..<live], vel[0..<live], dt)          -- line 285
integrate_live(field!.pos, field.vel, field.live, 1.0 / 60.0)   -- line 346
```

**Line 346 is the canonical batch op, and it is already spelled with the mark on the root.** `field`
is a hand-rolled SoA struct (line 245: *"two columns, one length"*), so its origin is `field` and
both arguments derive from it. Nothing is bound, so §7.4 governs and nothing dies. The reference
program therefore compiles unchanged under §2, and it never needed #25 §9's grant. The re-take is the other fix, and it needs no
annotation:

```ludo
integrate(rocks!.vel, dt)
p := rocks.pos
```

**The lost capability is filed, not conceded silently.** Chapter 3 §7.7 already booked the range half
as [#102](https://github.com/ludo-lang/ludo/issues/102); §2 makes the two halves one rule, so #102 is
widened to cover both — *binding disjoint parts of one origin*, whether the parts are ranges or
columns.

## 6. What this ADR stamps and what it leaves

**Stamped:**

- **#25 §9 is reversed in full** — superseded by §2 and §3 of this ADR. #25 is otherwise unaffected
  and remains authoritative for the rest of the layout chapter.
- **ADR-0047 §5's closing note is stale.** Its open item — whether a writable sub-view is
  `name![a..<b]` or `name[a..<b]!` — was closed by #101 and is landed at chapter 1 §7.10a and
  chapter 3 §6.12. ADR-0047 is otherwise untouched, and §3 of it is upheld and generalised here.

**Left open, and not this ADR's:**

- **Whether `for x in !rocks` is legal on a columnar pool.** Chapter 3 §12.2 gives a writable view
  per element; §10.8 rejects the synthesised in-place `!Entity` lend and says true in-place mutation
  exists only through columns. Those meet, and the spec does not say what happens. It follows from
  §10.8 and §12.2 alone — neither a source this ADR touches — so it is filed separately rather than
  ruled on in an ADR about origins, where a later reader would not look for it.

## 7. What this is not

§2 does not extend the check. It restates §7.1 with the origin named precisely, and every clause of
ADR-0047 §6's closed list still holds: no lifetimes, no regions, no shared/exclusive distinction, and
the check never crosses a function boundary. **The rule got shorter, not longer** — one sentence now
covers what §7.6 and #25 §9 covered as two, and the dimension a view narrows along stopped being a
thing the language has an opinion about.
