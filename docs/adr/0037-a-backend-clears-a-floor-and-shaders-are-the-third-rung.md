---
status: accepted
---

# A backend clears a floor, not a roster; shaders are the third conformance rung

Issue #74 asked what the admissible backend set is, and whether WebGL2 is in it.
[ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md)
§10 raised it and deliberately left it: ADR-0002 adopts the WebGPU model and
names D3D12, Metal, Vulkan and a deferred WebGPU-proper web backend, while
[ADR-0022](0022-the-spec-promises-only-what-is-derivable.md) §1 writes a floor
that names WebGL2's unpack parameters and says **WebGL1 "is excluded as a
backend"** — which reads as admitting a fifth API the corpus never adopted.

The answer is that **the question had the wrong shape twice over**. There is no
admissible set, because a roster is not how this map states obligations; and
WebGL2 was never a candidate for the thing the word "backend" denotes. What the
ticket was actually circling — *may a target with a lower ceiling conform?* —
turns out to be a question about **shaders**, not about API families, and it is
answered by a third conformance rung.

## 0. What the corpus was calling two things by one name

`CONTEXT.md` defines a **backend** as *one platform's implementation of the
platform layer — the Win32/D3D12/WASAPI set, or the Cocoa/Metal/CoreAudio set*.
A backend is a **triple**: window/input, renderer, audio device. ADR-0022 §1 uses
"backend" to mean the **renderer component** alone, and so does its excluded-as-a-
backend sentence.

**`CONTEXT.md`'s definition holds; ADR-0022 §1's usage is the loose one.** WebGL2
is not a candidate backend and never was — it is a candidate **renderer
component of a hypothetical web backend**, alongside the Web Audio device and the
canvas's input surface. ADR-0002's four names are components in exactly the same
way.

This is not pedantry about a word. Issue #4 measures naming things that do not
exist as the largest agent failure class at 43.7%, and a term that means the
whole platform triple in the glossary and one third of it in a normative clause
is a machine for producing exactly that. **Half of #74's ambiguity is this
collision and nothing else.**

## 1. No admissible set ships. The floor is the normative surface

**The spec names no graphics API, ever, as a membership test.**

An enumerated roster — *D3D12, Metal, Vulkan and WebGPU are the backends* — is
the wording shape [ADR-0019](0019-claimants-conform-alone-and-extern-is-the-only-door-below-the-facade.md)
§1 outlaws. §1's rule is that **a conformance obligation is stated on one
claimant, never on a pair**, and never as a comparison no artifact can perform. A
roster is worse than a pair: it is a statement about a *population*, checkable by
nobody, and stale the day a fifth API ships.

A capability list authored as a floor was considered and is **rejected too**. It
is a roster wearing different clothes — it goes stale identically, and it invites
the failure where a claimant clears every bullet on the list and still fails a
normative property.

So the floor is not a new artifact:

> **A renderer component is admissible if and only if it can satisfy the spec's
> normative surface at the conformance rung its implementation claims.** The
> surface is ADR-0005, ADR-0007, ADR-0009, ADR-0010 and ADR-0011's normative
> text, made executable by issue #19's P1–P14. Nothing else is a membership
> test, and there is no list to keep current.

WebGL1's exclusion survives, restated as a consequence rather than a decree: it
cannot satisfy ADR-0022 §1's strided upload — no `UNPACK_ROW_LENGTH` — and
ADR-0022 §1 is part of the surface. **That is the whole of it.** The exclusion is
not membership in anything.

WebGL2 is therefore **neither admitted nor refused**, and asking which is a
category error the corpus is now free of. If someone builds a web backend whose
renderer component is WebGL2 and it satisfies the surface at the rung it claims,
it conforms. If it cannot, it does not. Nobody needs to have decided in advance.

## 2. Shader support is the third conformance rung

[ADR-0004](0004-a-mandated-drawing-facade-over-a-delegated-renderer.md) split
conformance into **core** (compiler, language, non-visual stdlib; headless) and
**full** (core plus the facades that need a device, over at least one backend).
This ADR adds a third:

> **Core ⊂ full ⊂ shader.** *Shader* conformance is full conformance plus
> ADR-0008's two shader authoring paths — shader ludo and the `extern` shader
> declaration. The rungs are **totally ordered by containment**: shaders need the
> facade, the facade needs core.

The reason this is available at all is that the two halves come apart cleanly:

- **The mandated facade is analytically specifiable.** ADR-0004's own argument
  for mandating circles was that *specifying a shape is not specifying how it is
  built*. Every mandated call in `$.graphics` is statable that way, and every one
  is satisfiable by a CPU rasterizer. The facade needs no GPU.
- **The shader paths are not.** ADR-0008 §3 records four locked guarantees with
  **no GPU implementation** — bounds checking, overflow checking, faults, `?T`'s
  tag word — which is why shader ludo is a *strict subset* rather than the
  language. The shader tier is where a device's programmable pipeline becomes
  load-bearing, and it is the only place it does.

An orthogonal capability flag — *"full conformance, shader-capable"* — was
considered and rejected. Containment here is real, and its only advantage would
be describing an implementation that ships shaders without the facade, which is
incoherent. Issue #4's lens punishes a two-axis matrix an agent has to hold in
its head where a rung would do.

**ADR-0002's ceiling is unchanged and now has a stated scope**: it binds
implementations claiming the **shader** rung. Its argument — that adopting
WebGPU's model converts inventing a portable GPU abstraction into an adoption
decision, and makes the deferred web backend *a thin pass-through instead of a
second renderer* — survives intact, because nothing below the shader rung asks
that abstraction to exist.

### Why this does not reopen criterion 4

The obvious objection is that a program now runs or does not run depending on the
target, which is issue #5's criterion 4 — dialect variance — arriving through the
back door of build configuration. It does not, and the reason is already in the
corpus:

**`extern shader` is an `extern`.** ADR-0019 §2's finding is that the only
divergence channel below `$.` is `extern`, and that `extern` is **visible in the
signature** — which is exactly what criterion 4 demands. A program that depends
on shaders says so in its source, in a declaration, at file scope. A reader,
human or agent, can determine a program's required rung by reading it.

Shader ludo carries the same visibility: a stage marker is a declaration, not an
inference.

## 3. A software renderer is admissible, and never mandated

Software rendering appears **once** in the entire corpus, and about something
else: ADR-0006 observes that *a user could write a software rasterizer against
conforming ground* — a **program** rasterizing into a pixel array via
`get_pixels`, cited as evidence that drawing had raw access where audio did not.
Whether an **implementation's** renderer component may be a CPU rasterizer was
never written anywhere. It was not excluded; it was unaddressed.

> **A claimant may ship a software rasterizer as its renderer component and be
> fully conformant.** It cannot claim the shader rung.

It follows from §1 with nothing added — a CPU rasterizer satisfies the facade's
normative surface, and satisfies ADR-0022 §1's strided upload trivially, since a
host array is already the thing it draws into. Recording it is worth an entry
anyway, because the silence read as exclusion to at least one reader of ADR-0002.

**Mandating one is refused.** ADR-0006 rejected the equivalent shape for audio in
as many words: *mandating them would make conforming mean shipped a DSP stack*,
which is ADR-0004's rejected full-cover argument with different nouns. Full
conformance already requires *at least one working backend*; software rendering
becomes a legal choice of one, not a second obligation on everybody.

### What this buys, and it is the ticket's central tension

Issue #74's strongest argument for admitting WebGL2 was that **WebGPU's browser
availability is not universal, and a web backend that cannot run anywhere is not
a web backend**. That argument is answered without admitting WebGL2 into
anything:

**A wasm software rasterizer writing into a canvas needs neither WebGPU nor
WebGL.** It is the maximally-available web renderer that exists, it clears the
floor, and at the facade rung it is fully conformant. The web stops being
hostage to WebGPU's rollout, and ADR-0004's **negative web obligation** —
*nothing in the facade may be unimplementable in a browser* — stops resting on
any vendor's shipping schedule. The obligation holds identically under both
readings the ticket asked about, and now holds for a stronger reason than it did.

The cost is stated plainly: at ADR-0032's fixed 1280×720 canvas and ADR-0035's
fixed 60Hz, a software renderer owes ~55 Mpx/s of fill. Ordinary for sprite
blitting; hopeless for a fullscreen fragment shader — which is the shader rung,
which it does not claim.

**This is free against the map's standing ordering.** *Performance and frame
stability above rendering fidelity* binds what the spec may guarantee; it does
not oblige every claimant to be fast. Admitting a slower renderer costs no
program anything, because no program is forced onto one. Conformance is a
correctness predicate, and the only throughput figure in the corpus is #19's
MUST-MEASURE reload latency, which is untouched.

## 4. A shader declaration against a facade-only target is a compile error

> **An `extern` shader declaration, or shader-ludo source, in a program built
> against an implementation that does not claim the shader rung, is a compile
> error naming the declaration.** It carries a spec-owned code in ADR-0018's
> diagnostic envelope.

A runtime fault was considered, by analogy with
[ADR-0015](0015-assets-are-declared-not-loaded.md)'s missing asset, and is wrong
here. ADR-0015's case is a fault because **file existence is genuinely
runner-time**; a rung is not. ADR-0006 R3 makes target selection a build-time
module set, so the implementation's rung is known when the compiler runs, and a
compile error is both available and cheaper.

A silent no-op is criterion-4 dialect variance outright and needs no discussion.

## 5. ADR-0022 §1's sentence is restated

The clause stands; only its last sentence changes, and only to stop implying a
set:

> Backends must upload from a strided host array without repacking (WebGPU
> `writeTexture` with unaligned `bytesPerRow`; WebGL2 `UNPACK_ROW_LENGTH` /
> `UNPACK_SKIP_PIXELS` / `UNPACK_SKIP_ROWS`). ~~WebGL1 cannot satisfy this and is
> excluded as a backend.~~ **A renderer component that cannot do this cannot
> satisfy this clause, and therefore cannot serve a conforming implementation at
> any rung above core. WebGL1 is the known instance.**

The API names in the parenthesis are **evidence that the floor is satisfiable**,
which is what they always were, and they are explicitly not a roster. ADR-0033
§10 read the sentence correctly and could not fix it, because the fix needed a
decision about the set and there was no set; §1 above supplies the absence the
restatement rests on.

## 6. The reference-implementation temptation, refused

A software rasterizer is **deterministic** where a GPU is not. It is therefore
tempting to make one the conformance suite's reference and re-promise the pixel
exactness ADR-0022 withdrew — the `crisp` bit-exactness headline and the `smooth`
coverage tolerance, falsified by issue #56 and deleted by ADR-0022 §2 and §3.

**Refused, and recorded so nobody retries it.** ADR-0022 §0 is that *the spec
promises only what is derivable from the specifications a backend is built on*. A
reference implementation is an **artifact**, not a specification. Deriving a
promise from our own rasterizer's behaviour is the same error as ADR-0022's
rejected *promise wearing a lab coat*, differing only in whether the invented
number comes from a measurement or from a codebase. ADR-0022 §2's texel rule —
**which texel, never what byte** — remains the corpus's whole statement of
graphics exactness.

The permitted half is worth naming, because it is genuinely useful and is not the
same thing: **the suite may run on a software renderer**, which makes the full
rung testable in CI on a machine with no GPU. Running a property against an
implementation is not deriving a promise from it.

## 7. Against issue #24, and against issue #19

- **Core grammar: unchanged. Type sublanguage: unchanged. Stdlib root names:
  unchanged.** No call, value or type is added or removed. **Zero delta.**
- **No new #19 property.** A conformance rung **partitions** the existing
  assertions rather than adding one: P1–P14 are unchanged in content, and the
  rung decides which of them a given claimant must pass. The shader paths had no
  property before this ADR and gain none from it.
- One diagnostic code is added under §4, in ADR-0018's existing envelope. Codes
  are spec-owned and are not counted against #24.

## The three lenses

- **Simplicity.** Invisible to a program that does not use shaders, which is the
  beginner's whole first year. A program that does use them fails to compile with
  a named declaration rather than misbehaving, which is the better half of a bad
  day.
- **Robustness.** The gain is on the record. A normative clause implying a
  membership test with no set is a clause an implementer can be confidently wrong
  about, and #56 is the precedent for what that costs — it falsified two frozen
  promises by reading the primary sources. The floor cannot go stale, because it
  is the surface, and the surface is the thing that was going to be checked
  anyway.
- **Agent-friendliness.** Strongest, and it is the reason this is an ADR and not
  an erratum. `backend` collapses to one referent (§0). An agent asked *is WebGL2
  supported* previously had two accepted ADRs implying opposite answers; it now
  has one question it can actually evaluate. And an agent can determine a
  program's required rung by reading the program, because the dependency is an
  `extern`.

**No lens conflict.**

## Amendments this ADR makes

- **ADR-0002** — its four API names are **non-normative evidence of
  tractability**, not an admissible set (§1); its ceiling is scoped to the
  **shader** rung (§2).
- **ADR-0004** — the conformance split gains a **third rung** (§2); a software
  renderer is an admissible choice of backend (§3); its negative web obligation
  holds for a stronger reason (§3).
- **ADR-0022** — §1's final sentence is **restated** (§5); the API names in it
  are evidence, never a roster.
- **ADR-0008** — its two authoring paths define the **shader** rung, and a
  declaration against a lower-rung implementation is a compile error (§4).
- **ADR-0033** — §10's second open item is **discharged**.
