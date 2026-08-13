---
status: accepted
---

> **Amended by [ADR-0031](0031-integer-scaling-loses-to-the-priority-ordering.md):** §5's
> integer scaling on `crisp` targets is **deleted** under the map's *performance and frame
> stability above rendering fidelity* ordering — it cost a third of the screen at 1080p — and
> §4's formula loses its `style` branch, its floor and its too-small-window case. §3, §6, §7
> and §9 stand; P13 is restated.
>
> **Amended by [ADR-0032](0032-the-canvas-is-fixed-at-1280x720.md):** unchanged in
> substance — the canvas is fixed at 1280×720, so `W` and `H` in the fit formula are now
> constants rather than the program's declaration.
>
> **Amended by [ADR-0034](0034-the-byte-exact-blit-is-withdrawn-and-the-texel-rule-replaces-it.md):**
> §7's deferral is **discharged** — the byte-exact blit and ADR-0022 §4's copy-not-quad
> condition are **withdrawn**, on §7's three grounds plus two more. §3's coordinate-mapping
> finding turned out to be the strongest of them: composed with ADR-0031's real `k` and
> ADR-0032's fixed canvas, the clause was reachable only at a window of exactly 1280×720.
>
> **Amended by [ADR-0038](0038-the-escape-hatch-is-a-player-set-render-scale.md):** §3's
> rejection of the intermediate framebuffer is **scoped, not overturned**. It holds for a
> target that is mandatory, permanent and sized to the *canvas* — the thing that would cap
> text and `smooth` content at canvas density for everyone. It does not reach an
> **optional, player-invoked** target sized to `s · k`, a fraction of *device* resolution.
> At `s = 1` an implementation MUST NOT require one, so §3's chosen path is still the
> default path.
>
> **Amended by [ADR-0039](0039-stretch-to-fill-is-refused-and-no-player-exception-is-added.md):**
> §9's rejection of the ADR-0025-mould **player preference** is confirmed and supplied with
> the argument it lacked — every ground §9 states addresses the `fit:` field, not a player,
> which is why ADR-0031 §4 read it as still open. The `fit:` holding is untouched.

# The fit is a uniform transform, and the bars are not the program's

Issue #67 asked whether the letterbox guarantee ships as a normative clause. It
does. But the ticket's framing understated the defect and the grilling changed
the mechanism, so what lands here is larger than a missing sentence.

[#65](https://github.com/adamico/ludo/issues/65) / ADR-0028 §8 found the gap
while establishing that the admission test is *not* what keeps a game's world
hardware-independent — the logical canvas is. The word **aspect** appears in no
ADR in the corpus. The only sentence carrying the guarantee is an ADR-0005
*rationale bullet*: "The program states its logical size and the facade
letterboxes and scales to the real window."
[ADR-0022](0022-the-spec-promises-only-what-is-derivable.md) re-cut the graphics
conformance clauses and did not touch it. By that ADR's own lesson — **a
property nobody wrote as a clause is a property no implementation owes** — a
conforming implementation may today stretch the logical canvas to fill the
window.

## 1. The gap is worse than unwritten: an operative clause already depends on it

ADR-0011 ships this, normatively, in the input facade:

> **A pointer in the letterbox bars reports a position outside the canvas rect.**
> Clamping would make the canvas edge unhittable but not obviously so.

That sentence presupposes bars. Against a conforming stretch-to-fill backend it
is not merely unenforced — it is **unreachable**, describing a region that never
exists. So the corpus does not have one rationale bullet and a silence; it has an
input clause whose precondition graphics never states. This is recorded because
it changes the severity: the fit is not a nicety the spec forgot to promise, it
is load-bearing for a clause already shipped, and for ADR-0028 §8 and
[#60](https://github.com/adamico/ludo/issues/60)'s entire *nowhere to put the
answer* argument.

## 2. "Integer scale" was never a clause — #60 and ADR-0025 cite a belief

The ticket asked whether integer scaling is likewise only rationale. It is worse
than rationale: it is **absent**. Across the whole corpus the phrase occurs
exactly twice, both in prose asserting that something *else* mandates it —
ADR-0025 §2 ("under ADR-0005's letterbox + integer scale it changes nothing the
program can observe") and #60 ("ADR-0005 mandates letterbox + integer scale, so a
program computing `floor(display / canvas)` re-derives what the runner just
did"). ADR-0005 mandates no such thing.

What ADR-0005 and ADR-0010 actually condition the byte-exact blit on is integer
**coordinates** and an identity-or-integer-**translation** transform. Neither is
a statement about presentation scale. Two decisions have now leaned on a clause
that was never written, which is the same failure mode as §1 and the reason both
are fixed here rather than separately.

## 3. The canvas is a coordinate mapping, not an intermediate framebuffer

The grilling's root question, because it decides what "scale" names, whether
integer scale is even expressible, and what the bars are. Two mechanisms were
compatible with everything written:

- **An intermediate target** — the frame renders into a texture of exactly the
  logical size, which the runner then presents into the window. One fit step, one
  filter, at the end.
- **A coordinate mapping** — the logical space is a transform the backend folds
  into its own matrices and rasterises directly at device resolution. No
  intermediate surface.

**The mapping is chosen.** The intermediate target was the tempting answer,
because it is the only reading under which ADR-0010's blit clause — conditioned
on *source extent equals destination extent in device pixels* — is achievable in
a scaled window at all. It is rejected because it forces every backend to
render at logical resolution and upscale, which caps text and any `smooth`
content at the canvas's pixel density permanently, and makes the memory cost of a
large canvas unavoidable. The mapping renders everything at native device
resolution.

**The accepted consequence is stated rather than hidden:** under the mapping,
ADR-0010's blit clause self-disables whenever the scale factor is not 1. It is
not falsified — it is **vacuous** in the ordinary case (a 320×180 canvas on a
1440p display), surviving only in a window that happens to be exactly canvas
sized. That consequence is what sent the blit clause to
[#69](https://github.com/adamico/ludo/issues/69) (§7), where it turned out to be
in worse trouble for reasons that have nothing to do with this ADR.

## 4. The clause, stated as arithmetic

ADR-0022's lesson argues for explicit over idiomatic: *letterbox* is a word with
folk meaning, not a definition, and naming the noun is what produced the gap in
§1. So the guarantee is written as the arithmetic and *then* named.

> **The logical-to-device mapping is a uniform scale followed by a
> translation, and nothing else.** Given a logical canvas of `(W, H)` and a
> window of `(w, h)` in device pixels, let `s = min(w/W, h/H)`. The scale factor
> `k` is `s` on a `smooth` target and `max(1, floor(s))` on a `crisp` target
> (§5). The canvas image occupies a rect of `(kW, kH)` device pixels, centred:
> `origin = (round((w - kW)/2), round((h - kH)/2))`. The same `k` applies on both
> axes — **aspect ratio is preserved, and no conforming implementation may
> scale the axes independently, shear, or stretch to fill.** The device pixels
> outside that rect are the **bars**.

Three things this buys that the noun did not. Aspect preservation is a sentence
of its own, so it can be cited and failed. The bars are **defined as a region**
rather than as the leftover of a blit — necessary under §3's mapping, where
there is no present step to leave anything over — which is what makes ADR-0011's
pointer clause derivable instead of assumed. And ADR-0011's logical-space pointer
reporting becomes the stated inverse of a stated transform, so the two facades
agree by construction.

Under fullscreen (ADR-0025) the window is the display and the clause is
unchanged; that is the whole of the interaction.

## 5. Integer scale on `crisp`, real scale on `smooth`

Two candidate rules were rejected. **Real `k` everywhere** is simplest and
matches what a general 2D engine does. **Integer `k` everywhere** taxes games
that never wanted it: a photographic or vector-art game gets fat bars for a
property it cannot perceive.

The split ships because `crisp` already exists as the token that says *this is a
pixel-art program*, and ADR-0005 chose one token over two fields precisely so
conformance rules could be sentences about one named thing. This is the second
such sentence.

**The justification is a fact about sampling, not a spec promise.** At
non-integer `k` under nearest filtering, some source pixels occupy three device
pixels and their neighbours four — **uneven pixel size**, the most visible and
most complained-about pixel-art artifact. This matters to record: during grilling
this rule was first justified by keeping ADR-0010's blit clause non-vacuous, and
when that clause collapsed (§7) the rule survived unchanged on this independent
ground. It does not depend on #69's outcome in either direction.

**Costs, stated.** A 320×180 `crisp` canvas at 2560×1440 gets `k = 8` and clean
bars. At 1366×768 it gets `k = 4` rather than `4.27`, so 86 device pixels of bar
it could have avoided. And `floor(s)` is `0` whenever the window is smaller than
the canvas, which is why the clause floors `k` at 1 and the overflow is clipped
by §6 — a too-small window shows the centre of the canvas rather than nothing.

## 6. Content is clipped to the canvas rect; the bars are never program-reachable

Free under an intermediate target, where a draw outside the logical size falls
off the texture. **Under §3's mapping it must be said**: the transform maps
logical `x = -50` into the bar region as happily as anywhere else, so without a
clip clause program content leaks into the bars.

> Rendering is clipped to the canvas rect. No drawing call can affect a device
> pixel outside it.

Beyond keeping ADR-0011 coherent, the clip is what stops the visible playfield
from depending on window aspect — which is #60's *hardware as a gameplay
advantage* wearing a third outfit, arriving this time not as a query but as a
side effect of a missing sentence.

**The bars are opaque black**, spec-fixed. It is one word, it is what every
console and emulator does, and *implementation-defined* here means the same
program looks different across backends for no gain — the divergence ADR-0022
spent its length refusing to promise away.

**Recorded as rejected: program-drawable bars.** An overscan surface is what a
game whose art continues past the frame actually wants, and it is the strongest
of the three options on aesthetics. It loses because the amount of it varies per
player, so anything drawn there is either decorative-and-ignorable or a second
resolution-dependent playfield — and a spec cannot tell which one a program
drew.

## 7. What this ADR does not decide: the byte-exact blit

Raised during grilling as *"ADR-0010's byte-exactness hinders more than it
regulates"*, checked, and **substantiated on three independent grounds**:

1. **It is unsatisfiable in the facade's own vocabulary.** ADR-0022 §4 derives
   the guarantee from a condition list including *no blending*. ADR-0009 gives
   every descriptor a `blend: Blend` **defaulting to `alpha`**, over a closed
   enum `{ alpha, additive, multiply }` with **no `none` or `opaque` mode**. Every
   draw the facade can express is blended, so every draw rides the rasterised
   path and inherits WebGPU §23.2.7's implementation-defined store rounding —
   which is the exact reasoning ADR-0022 §4 used to withdraw the tinted hit
   flash. It withdrew the tint case for shader multiply and missed that alpha
   blending disqualifies the untinted case identically.
2. **Where it is not vacuous it mandates an implementation with a cliff.**
   *"Backends must implement this case as a texture-to-texture copy, not a
   rasterised quad"* — a copy is a command outside a render pass, so a pixel-art
   game drawing 2000 sprites at integer coordinates breaks its batch 2000 times,
   in the exact case the facade exists to make fast. Noted alongside:
   ADR-0012 records the forwarding-function guarantee as *"the spec's one
   deliberate constraint on how an implementation compiles"*, and this is a
   second one, so that claim is already false.
3. **Its protective content is duplicated, derivably.** ADR-0022 §2 already
   promises that on a `crisp` target interior and exterior pixels are exact
   everywhere, and boundary pixels are exact for axis-aligned geometry whose
   edges lie on integer framebuffer coordinates. A sprite quad at integer
   coordinates is that geometry. The remainder is the texture-sampling half,
   which nearest filtering at exact texel centres supplies.

**This is not decided here.** The fork — withdraw outright, or add an
`opaque`/`none` blend mode to make it satisfiable and thereby walk into (2)'s
batching cliff — is a genuine two-option grilling that amends ADR-0010 and
ADR-0022 rather than the fit, and burying a facade-wide withdrawal inside a
letterbox ADR is how §1's defect happened in the first place. It is
[#69](https://github.com/adamico/ludo/issues/69). ADR-0022 §4's copy-not-quad
condition goes with it.

Nothing in §4–§6 depends on that outcome.

## 8. Conformance: an ADR clause and #19 P13, full-conformance only

The corpus has both patterns — ADR-0022 re-cut clauses without adding a property;
#59 added P12. This warrants a property: the invariant is load-bearing for two
prior decisions (§1), and it is testable by one framebuffer readback.

> **P13 — the canvas is fitted, not stretched.** *Precondition:* a program
> declaring a logical canvas of a known aspect. *Action:* run it in windows of
> several aspects, including one narrower and one wider than the canvas, and read
> back the presented framebuffer. *Observable:* the canvas image occupies a
> centred rect whose aspect equals the declared aspect, at the `k` of §4/§5, with
> every pixel outside it opaque black and no program content among them.

**This is the first #19 property core conformance cannot run**, and the asymmetry
is stated rather than smoothed over: under ADR-0004 the fit is a facade property,
so it is **full only**, and a headless core implementation is conformant without
it. That is the intended reading of ADR-0004's split, not an exception to it.

## 9. Against issue #24: zero delta

No keyword, no operator, no facade call, no stdlib root name. §4–§6 constrain
what existing surface means.

The zero is a **result of a decision**, not an accident: the fit was proposed with
a `fit:` field on `set_canvas` alongside `style:`, and with a player preference in
ADR-0025's mould, and **both were rejected**. One fit, spec-fixed, no knob.
#60's holding is that a program has nowhere to put a resolution answer, and
ADR-0013 §5 already made canvas and style immutable for the process's life for
this class of reason; a `fit:` field re-admits the aspect-varying design mistake
#60 rejected, one level down, where it is harder to see. A stretch-to-fill option
would also make ADR-0011's bars conditional on a program's choice, putting an
input clause at the mercy of a graphics field.

## How it fares on the three lenses

- **Simplicity.** Neutral to positive, and nothing new to learn: no call, no
  field, no mode. What a beginner gets is the promise they already assumed —
  their game looks the same on every monitor — now actually owed. The cost is
  that §5's rule is a thing to know once (`crisp` means integer scale, which
  means bars) at the moment they first wonder why the bars are thicker than they
  expected.
- **Robustness.** The clear win. A property two prior ADRs already leaned on
  becomes checkable, an operative ADR-0011 clause stops resting on a
  precondition nothing states, and §6's clip closes a content-leak nobody had
  noticed. Net stated guarantees rise; unlike ADR-0022 this re-cut adds rather
  than withdraws, because everything added is derivable from arithmetic the spec
  now fixes.
- **Agent-friendliness.** Strongly positive, on #4's naming finding read
  sideways: the surface an agent could reach for here — `fit:`, `$.display.size`,
  a scale query — **does not exist**, so the failure mode is a compile error at a
  name, not a program that quietly behaves differently per player. §4's
  arithmetic is also the kind of clause an agent can check its own output
  against, which a noun like *letterbox* is not.

## Amendments this ADR makes

- **ADR-0005** — the letterbox rationale bullet is promoted to the normative
  clause of §4; the canvas is fixed as a coordinate mapping, not an intermediate
  framebuffer (§3); the clip and the bars are specified (§6).
- **ADR-0011** — unchanged in text; its pointer-in-bars clause gains the
  precondition it was written against (§1, §4).
- **ADR-0013** — §9's *the real window size is not exposed* is unchanged and
  strengthened: §4 makes the mapping derivable without it.
- **ADR-0025** — §2's *"under ADR-0005's letterbox + integer scale"* cited a
  clause that did not exist; the citation is now true, with integer scale holding
  on `crisp` only (§2, §5).
- **ADR-0010 / ADR-0022** — **not amended here.** The byte-exact blit and the
  copy-not-quad condition go to [#69](https://github.com/adamico/ludo/issues/69)
  (§7).
