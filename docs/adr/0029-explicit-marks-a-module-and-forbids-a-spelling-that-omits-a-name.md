---
status: accepted
---

# `#explicit` marks a module, and forbids a spelling that omits a name

Issue #66 asked whether `#explicit` has any module- or library-level surface, or
whether it is strictly per-file and per-declaration.

[#6](https://github.com/adamico/ludo/issues/6) settled layering as **progressive
disclosure over one fixed-semantics language** and shipped exactly one construct
for it: `#explicit`, described there as "a per-module assertion that rejects
inference, sugar, and defaulted args", and as "a surface-only lint that changes
zero semantics". [ADR-0014](0014-a-library-is-a-directory-that-claims-a-name.md)
then made a library a directory that claims a name, and
[ADR-0027](0027-the-compiler-is-handed-a-name-to-location-mapping-and-never-searches.md)
fixed how that name resolves. The interaction between the two was left as fog.

**The ticket's own framing did not survive the investigation, and neither did the
first answer this session proposed.** Two claims were checked and both were
false. `#explicit` is *not* absent from the spec — ADR-0008 §4 cites it as one of
the two attributes that "already established" the attribute mechanism, and it is
half the precedent by which `#vertex` and `#fragment` spend zero new keywords. And
`#align` was not dropped: ADR-0024 §6 declined **field-level** `#align`, while
type-level `#align(n)` still ships and ADR-0016 depends on it. Deleting
`#explicit` was proposed on those two errors and is **withdrawn**.

What survived the check is a different and worse problem: `#explicit`'s
restriction set has never been written down, and read literally it **bans the
drawing facade**.

## 1. `#explicit` is kept, and progressive disclosure is not what it delivers

The deletion case rested on the claim that `#explicit` could be replaced by a
vendor lint plus [ADR-0023](0023-the-spec-never-mandates-a-warning.md) §3's
permitted external gate. That trade is refused.

The reason is precision about what `#explicit` is *for*. Progressive disclosure is
delivered by #6's three mechanisms — local inference, in-signature defaults, and
sugar with R1-local desugaring. `#explicit` is none of them. It is the **enforced
floor**: the assertion that one module sits at the bottom of the axis. So
deleting it would not erase progressive disclosure, as feared — it would erase
the ability to *enforce* the bottom of it.

That is still enough to keep it, on two grounds. `#explicit` is the only thing #6
shipped that a programmer can see; without it, progressive disclosure becomes an
invisible property of the language rather than a feature of it. And ADR-0008 §4
already spends it as precedent, so deleting it would retroactively weaken the
shader markers' justification.

## 2. `#explicit` marks a module, and there is no declaration-level form

**One line at the top of a file. No per-declaration form, no per-directory form,
no compiler flag.**

A **module** is a file (ADR-0014), and `use` is already file-scoped and
non-transitive, so the layer choice lands on a boundary that already exists
rather than inventing one.

A declaration-level form was rejected. It would let one file mix both spellings,
so a reader could not learn the rules from the file's header — which is the exact
non-local read the attribute exists to prevent.

A directory- or library-level form was rejected under §3.

A compiler flag was rejected outright by ADR-0023 §3: no flag may change the set
of programs that compile. A `--explicit` switch is that forbidden thing exactly.

**The named cost:** `#explicit` is the one attribute in the language that marks a
module rather than a declaration. `#align(n)` marks a type; `#vertex` and
`#fragment` mark functions. This asymmetry is real, is recorded here, and is
accepted because the alternative — a declaration-level form — costs the property
that makes the marker worth having.

## 3. A layer choice never crosses a library boundary

**A library cannot force `#explicit` on its consumers, cannot forbid it, and
cannot see it.** All three directions, permanently.

A library that could impose a spelling discipline on its consumers would be #5
criterion 4's dialect variance arriving through the dependency graph instead of
through a flag. **Reference discipline** is file-scoped and non-transitive, and
this is that rule applied to layering.

There is therefore no `#explicit`-requiring signature, no query, and no
`library`-line form of the marker. A library author who wants explicit source
marks **their own** modules, which is a statement about their source and not
about anyone else's.

## 4. The restriction set: `#explicit` forbids a spelling that omits a name

#6 left the restriction set as a phrase — "no inference, no sugar, no defaulted
args" — and the phrase has silently grown with every short spelling the map has
shipped since: UFCS, `+=`, the implicit tail return, `?T`, the target-typed
`{...}` literal, and descriptors with field defaults. No ticket since #6 has paid
that tax.

Read literally the phrase is also **unusable**. A **descriptor** is defined as a
named-field-only literal *with field defaults* (`CONTEXT.md`), and the mandated
spelling is `$.graphics.set_canvas({...})`, which is target-typed. "No sugar, no
defaulted args" bans the drawing facade, and every game program calls the drawing
facade.

So the phrase is replaced by a rule with a reason, plus a closed list.

> **`#explicit` forbids a spelling that omits a name. It forbids nothing else.**

The rule is derived, not chosen by taste. Issue #4 measured that agents fail at
**naming things that exist** (43.7%) and at non-local reasoning — not at
algorithms. A spelling that omits a name is precisely a spelling that forces a
reader, human or agent, to leave the call site to recover an identifier. That is
the failure `#explicit` is worth enforcing against, and it is the only one.

### The closed list

Four spellings are rejected in a module marked `#explicit`:

1. **A binding without its type.** `x = ...` must be written `x: T = ...`.
2. **An aggregate literal without its type name.** `{...}` must be written
   `SpriteDesc{...}`. The type is the omitted name.
3. **A call that omits a defaulted argument.** The argument is written at the
   call site.
4. **A UFCS call.** `a.f(b)` must be written as the qualified call. UFCS resolves
   to the module declaring the receiver's type (#11), and that module's name is
   the omitted name.

This list is **closed**. A future ticket that ships a new short spelling adds to
it only by amending this ADR, and only if that spelling omits a name.

### What is explicitly permitted

Recorded so it is not re-litigated:

- **Descriptor field defaults.** An omitted field is permitted, and this is the
  entry that separates the rule from #6's phrase. A descriptor is
  *named-field-only* and owned by exactly one call (`CONTEXT.md`), and entry 2
  now forces its type name to appear — so the reader has one declaration to look
  at and every field they do see is named. A positional argument has no name at
  the call site at all, which is why entry 3 goes the other way. The distinction
  is **named versus positional**, not defaults versus no defaults.
- **`+=`**, the **implicit tail return**, and **`?T`**. None omits a name. `?T`
  in particular *is* the name of the absence concept (#9), so forcing the
  underlying sum type would be an anti-goal.

`#explicit` therefore stays usable with the facade: a facade call takes two
parameters and one descriptor, and under this list it is written with the
descriptor's type named and its defaulted fields left alone.

## 5. No check needs a whole-program view

**`#explicit` binds the spelling in the file that carries it, and nothing else.**
A mixed program — some modules marked, most not — is definitionally fine, and no
diagnostic looks outside the file being checked.

Three edges were tested and all three resolve the same way, at the **call site**
rather than the declaration:

- A **generic** declared in a plain module, instantiated in an `#explicit`
  module: the instantiation spells its type arguments.
- An **in-signature default** declared in a plain module, called from an
  `#explicit` module: the call spells the argument, even though the default lives
  in the callee's signature.
- **UFCS**, where `a.f(b)` resolves into a module with a different layer: the
  receiver's module is irrelevant; this file writes the qualified call.

Two consequences follow, and both are the point. The checker stays **per-file**,
which is what #22's incremental-and-local guarantee requires. And a library that
changes its own layer **can never break a consumer's build** — the same property
§3 secures from the other direction.

## 6. A violation is an error, and the word "lint" leaves the spec

Four entries, four **spec-assigned codes** under ADR-0018 §5, severity `error`.

#6 called `#explicit` "a surface-only lint". The *claim* inside that phrase — it
changes zero semantics — survives intact and is why the marker clears #5
criterion 4. The **word** does not survive. ADR-0023 §1 fixed that a spec-owned
diagnostic is an error or it does not exist, and a lint is the middle position
that ADR found had no occupant.

ADR-0023 is satisfied rather than strained here, because the marker is **in the
source**. Every conforming implementation rejects the identical set of programs;
whether a program compiles depends on the program, never on a flag or a vendor.
There is no way to silence a violation — ADR-0023 §4 already bans in-source
silencing, and nothing in this ADR adds one.

## 7. Corrections this ADR makes

- **ADR-0024 §6** says "`#align(n)` remains the one shipped attribute". That is a
  drafting error, and it is corrected to say **type-level** attribute. The ADR's
  actual decision — field-level `#align` is declined permanently — is untouched.
- **`CONTEXT.md`** has never contained `#explicit`. It gains an entry.

## 8. The grammar delta is zero

**No new keywords, no new operators, no new grammar production** against
[#24](https://github.com/adamico/ludo/issues/24).

The attribute mechanism exists (ADR-0008 §4). The attribute `#explicit` itself
exists and is already counted, from #6. Everything this ADR adds is checker
behaviour and four diagnostic codes, and #24's unit is keywords plus operators.

The stdlib root-name companion count is likewise unchanged.

## The three lenses

- **Simplicity.** A beginner never types `#explicit` and never meets its
  diagnostics; it is opt-in and off by default, which is what makes it compatible
  with a language whose default posture is maximal inference. The cost is honest:
  a beginner reading someone else's marked module sees a spelling they were not
  taught to write. The closed list of four keeps that surprise finite and
  learnable, where the old phrase kept it open-ended.
- **Robustness.** Positive but modest, and it should not be oversold. `#explicit`
  catches no class of bug — semantics are identical either way, which is the
  whole basis of §6's criterion 4 argument. What it buys is a floor that a later
  edit cannot quietly sink below.
- **Agent-friendliness.** This is where it earns its keep, and §4's rule is
  derived directly from #4's measured finding. Every entry on the list deletes an
  occasion for the 43.7% naming failure. An agent told to emit into an
  `#explicit` module has the compiler as its oracle rather than a convention to
  recall, and #5 criterion 7 forbids "by convention" answers.

**Where the lenses conflict:** at the marker itself. `#explicit` creates two
legal spellings of one program, and #5 criterion 4 is uneasy about that — an
agent must read the file's header before it knows what it may write. The conflict
is named rather than resolved, and it is bounded by §2 (one line, top of file, so
the header read is R1-local) and by §5 (nothing outside the file matters). It is
accepted because the alternative was deleting the only visible construct #6
produced.
