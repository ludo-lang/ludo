---
status: accepted
---

> **Absorbed by [spec ch4](../spec/04-modules.md):** the chapter is normative; this ADR
> keeps the argument — a backend as a build input, claiming no root name and keyed in no mapping, is transcribed there.

# A backend is a build input, not a library, and it claims no name

[#112](https://github.com/ludo-lang/ludo/issues/112), found by a reader's question against spec
chapter 4 ([#88](https://github.com/ludo-lang/ludo/issues/88)) after it shipped. The corpus made a
backend a library claiming a root name and never said which name:

- [ADR-0014](0014-a-library-is-a-directory-that-claims-a-name.md) §9 — *"a backend is a library
  claiming a root name"*, and target selection is which of the mutually exclusive libraries claiming
  that name is in the build.
- Chapter 4 §10.2 transcribes that faithfully and inherits the hole.
- [ADR-0019](0019-claimants-conform-alone-and-extern-is-the-only-door-below-the-facade.md) §1 speaks
  of a claimant of a **spec-defined root**, presupposing an artifact that defines one. None does.

The name was never chosen because there was nothing to choose it for. **This ADR removes the
question rather than answering it.**

## 1. No source names the backend, and the spec already says so

Chapter 8 §3.2 states it outright — *"the source never names the backend"* (ADR-0006 R3) — because
target selection is a build-time module set and `extern` is the only door below `$.` (ADR-0019 §2).
No game source writes `use "<backend>"`. The reference program does not, and no line of it can.

So the root name was a key in chapter 4 §7's mapping that **no `use` ever reads**. It is a linking
identity wearing a naming identity's clothes.

## 2. Exclusivity was never the root name's work

The one thing §10.2's claim bought was chapter 4 §6.1 — one root name, one library, so a desktop and
a web backend can never coexist. But chapter 4 §7.4 already requires a mapping carrying two entries
for one root name to be **rejected, loudly, at the point it is supplied**, and the toolchain, not
the program's author, supplies the backend's entry. The exclusivity holds without the claim.

ADR-0014 §9 reached for the root-name mechanism because it was the mechanism at hand. It was built
for names authors type, and it was applied to a name nobody types.

## 3. A backend is one triple, selected as a unit

Chapter 4 §10.1 pins the term: a backend is **one platform's implementation of the platform layer**,
a triple of window and input, renderer, and audio device. It is supplied and selected **whole**. The
alternative — a key per component, so a build could pair a Cocoa window layer with a CPU rasteriser
— is rejected. It buys a mixing matrix nobody asked for, multiplies the conformance surface by
three, and contradicts §10.8's runtime seam, which re-points **one** struct of non-capturing
function pointers at a quiescent frame boundary, not three.

A CPU rasteriser that must be selectable is therefore a **distinct backend**, not a component swap.

## 4. The backend leaves the mapping entirely

Leaving the backend as a key in chapter 4 §7's flat program-wide table keeps precisely the defect
the root-name framing caused. If the toolchain picks `platform` and any dependency also claims
`platform`, §7.4 fails the build over a declaration the program's author neither owns nor can rename
— §6.2 is a hard error with **no escape hatch**, and the author never typed the name.

**The backend is not a key in the mapping.** It is a separate build input the compiler is handed
alongside the mapping. The collision cannot arise by construction, no name is reserved, and the
`libs/` default (chapter 4 §8) cannot supply a backend, because §8.2 keys on a subdirectory's
`library` claim and a backend makes none.

The alternatives were both worse. Leaving the backend in the table and calling the collision the
toolchain's problem leaves a live footgun the spec has chosen not to mention. Reserving the name
pays a second reserved root — beside `$`, the only one — plus a #24 delta and this ADR's weight, for
a name no program contains.

## 5. `library` keeps meaning *claims a name*

With the claim gone, a backend no longer meets ADR-0014's definition of a library — *a directory
that claims a name*. Rather than weaken the term, the term keeps its meaning and the backend loses
the label.

**A backend is a build input**, defined by the interface it satisfies and by being supplied by the
toolchain. What it shares with a library is that it is a directory of ludo source, which is
packaging, not identity. This matters beyond tidiness: chapter 4 §6.4 rests chapter 2's nominal
identity rule on *one root name is one library*, and a `library` that sometimes claims no name would
put a hole in that argument.

## 6. The obligation never needed a spec-defined root

Chapter 6 §10.1 says the platform-layer API **is not specified here** — deliberately, not by
omission. There is therefore no spec-side interface declaration for ADR-0019 §1's *spec-defined
root* to point at, and there never was.

The obligation stands on chapter 8 §3.4's three artifacts, which already carry it in rank order: the
nominal interface is compiler-checked and catches shape alone; ADR-0006 R1–R8 are
reviewer-checkable; §5's properties plus the normative text of chapters 5 and 6 are **the gate**. A
claimant conforms iff it passes them. *Spec-defined root* was loose wording, and it is closed as
such rather than left standing as a presupposition.

This files no new hole. The API being unspecified is chapter 6 §10.1's choice.

## 7. What this reverses

ADR-0014 §9's first clause, and only it. **A backend is no longer a library claiming a root name,
and target selection is no longer which claimant of that name is in the build.** What §9 decided
that survives untouched: a backend satisfies a declared nominal interface, so a divergent signature
is a type error (chapter 4 §10.4, amending ADR-0006 R4); mutually exclusive backends are never in
one program; and the compile-time seam does not replace the runtime seam.

This is a reversal, which is why it is an ADR and not a spec-text repair
([ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §6). The
*name* was never decided — that much is #112's finding — but the **framing** was, and it propagated
into two chapters.

**Stamped:**

- **ADR-0014 §9's root-name clause is reversed**; the rest of §9 stands. ADR-0014's Consequences
  claim that the companion count is *otherwise unaffected* is **confirmed, not excepted** — no root
  name is reserved here.
- **ADR-0019 §1's *spec-defined root* is closed as loose wording.** The conformance obligation is
  unchanged and now rests where chapter 8 §3.4 already put it.
- **ADR-0006 R3 is confirmed.** Target selection at build, never inside a function body, is exactly
  what this ADR keeps; it removes only the claim about *how* the build expresses the choice.

## 8. #24 delta

**Zero.** No root name is reserved, no name is added to the language, no spelling is authored, and
the grammar is untouched — chapter 1 §13.9.1 gains no row. `$` remains the only reserved root.

## 9. The three lenses

- **Simplicity** — a Lua user never encounters a backend at all; removing a name they would have had
  to avoid is strictly less to know.
- **Robustness** — it deletes a failure class rather than diagnosing one: the unfixable collision
  between a third-party library and the toolchain's backend key is now unreachable.
- **Agent-friendliness** — an agent writing ludo can never collide with the backend, and cannot be
  tempted to `use` it. The compiler-as-oracle key set (§7.9) stays exactly the names a program may
  write.

## 10. What this is not

This does not make backends interchangeable or blessed. Chapter 4 §10.7 stands: mutually exclusive
claimants are **legal and unblessed** for third parties, and nothing here implies the language
supports swapping implementations. It also does not specify the platform-layer API, name the
interface, or say how a toolchain names or locates the backend it supplies — chapter 4 §7.10's
posture, that the spec fixes no file format, CLI flag or registry, extends to this input unchanged.
