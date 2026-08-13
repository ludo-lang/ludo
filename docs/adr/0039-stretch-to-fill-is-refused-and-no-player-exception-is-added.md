---
status: accepted
---

# Stretch-to-fill is refused, and no player exception is added

[ADR-0031](0031-integer-scaling-loses-to-the-priority-ordering.md) §4 recorded
stretch-to-fill *as a player preference* as open, on the ground that it hangs on
[ADR-0025](0025-fullscreen-is-a-player-preference-the-program-can-read-and-set.md)'s
preference machinery and nothing presses on it.
[ADR-0038](0038-the-escape-hatch-is-a-player-set-render-scale.md) built that
machinery out to a second instance and fixed its root, which discharged the
dependency. This ADR answers the question the discharge exposed, and the answer
is no.

## 1. What is refused

Both mechanisms, and no surface ships for either:

- **Anisotropic stretch** — scale X and Y independently so the canvas fills the
  window. The candidate ADR-0031 §4 named.
- **Zoom-and-crop** — `k = max(w/W, h/H)`, uniform, with the overflow falling
  outside the window. A candidate no prior document named, raised while deciding
  this and refused in §4.

The fit remains ADR-0031 §2's single formula, `k = min(w/W, h/H)`, with the bars
opaque black and never program-reachable ([ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md)
§6).

## 2. ADR-0030 §9 rejected this without arguing it

The record needs correcting before the decision, because the corpus currently
says two things.

ADR-0030 §9 states the fit *"was proposed with a `fit:` field on `set_canvas`
alongside `style:`, **and with a player preference in ADR-0025's mould**, and
both were rejected."* One hour later ADR-0031 §4 — citing §9 in the same
paragraph, for the `fit:` field — records the player version as open fog.

§9's conclusion stands. Its **reasons do not reach the player case**, and both
of them say so on their face: a `fit:` field *"re-admits the aspect-varying
design mistake #60 rejected, one level down"*, and it *"would also make
ADR-0011's bars conditional on a **program's** choice, putting an input clause
at the mercy of a graphics field."* Neither sentence is about a player. A
rejection whose every stated ground addresses a different proposal is unargued
as to this one, and ADR-0031 §4 — written by the same corpus, aware of §9 —
read it that way.

So this is not an overturn. It supplies the argument §9 lacked, and reaches the
same conclusion.

## 3. The three tests it passes, and the one it fails

ADR-0031 §4 lists three tests the player version passes. All three hold, and one
of them was verified here rather than inherited:

- **It shows no more world**, so [#60](https://github.com/adamico/ludo/issues/60)'s
  hardware-as-a-gameplay-advantage argument does not reach it.
- **It is symmetric across players.**
- **Pointer input maps back through the same transform**, so aiming still works.
  #76 asked for this to be checked against
  [ADR-0011](0011-the-input-facade-a-virtual-controller-over-ordinal-buttons.md)
  rather than assumed. It is true: position is reported in logical-canvas space,
  `to_logical` inverts whatever transform presented the frame, and under a
  stretch there are no bars, so ADR-0011's *pointer in the bars reports a
  position outside the canvas rect* goes vacuous rather than wrong. **Confirmed.**

That last confirmation establishes only that the feature is not incoherent,
which is the weakest thing a test can establish.

**The test it fails is the one ADR-0031 §4 supplied one paragraph earlier and
did not apply to the player case.** §4 re-grounded the ban on stretching, moving
it off fidelity and onto stability:

> A uniform scale plus a translation is a **similarity transform** — distances,
> angles and circles survive it, so the program's world is the same *shape* on
> every machine. Non-uniform scale is not, and makes the visual output a function
> of the display. That is the **simulation-stability** half of the ordering, not
> the fidelity half.

A player-set anisotropic scale reintroduces exactly the variance that clause
exists to exclude. It does not matter that a human asked for it; the invariant
is about whether the world is the same shape everywhere, and under the knob it
is not.

**Consent does not distinguish it**, and this is the load-bearing paragraph.
Consent settles *who bears a cost*, not *whether an invariant holds*. Both
existing player preferences are consent-gated and neither touches the invariant:
fullscreen changes the frame around the image, and render scale changes how many
device pixels shade it — ADR-0038 §11 goes out of its way to show it changes
nothing else a program or a player can detect. **Stretch would be the first
player preference that alters the geometry of program content itself.** That is
the category line, and it does not run where the consent line runs.

## 4. Zoom-and-crop, considered and refused

Raised because it is genuinely stronger and no prior document had named it: it
keeps the uniform scale, so §3's grounds do not reach it at all. Shapes stay
correct; circles stay circles. The cost moves from *how things look* to *what is
visible*: content near the canvas edge falls outside the window, so a player sees
**less** world, not more — which dodges #60 from the other side.

It is refused on **ADR-0030 §6's own argument pointed the other way**. §6
rejected program-drawable bars because *"the amount of it varies per player, so
anything drawn there is either decorative-and-ignorable or a second
resolution-dependent playfield — and a spec cannot tell which one a program
drew."* Crop is that sentence mirrored: the amount removed varies per player,
and a spec cannot tell whether what vanished was a decorative border or the
health bar.

Two further costs, either of which would be sufficient:

- It makes **the visible playfield depend on window aspect**, which is precisely
  what §6's clip clause was written to prevent. §6 named that *"#60's hardware as
  a gameplay advantage wearing a third outfit, arriving this time not as a query
  but as a side effect of a missing sentence."* This is a fourth outfit, arriving
  from the player's side.
- Under [ADR-0032](0032-the-canvas-is-fixed-at-1280x720.md)'s fixed 1280×720
  canvas an author has **one authoring rect and no safe-area concept**. Shipping
  crop would oblige the corpus to mint a title-safe region and teach it, which is
  real new surface for a feature nobody asked for.

## 5. Nobody is asking, and that is a finding rather than a ground

The arithmetic, recorded because it is the first time it has been written down
and it is more lopsided than the fog note implied. Canvas `1280×720`, fullscreen:

| display | `k` | bars |
| --- | --- | --- |
| 1920×1080 / 2560×1440 / 3840×2160 (16:9) | 1.5 / 2 / 3 | **none** |
| 1920×1200 (16:10) | 1.5 | 60px top and bottom — 10% of height |
| 3456×2234 (16" laptop) | 2.7 | 145px top and bottom — 13% of height |
| 3440×1440 (21:9) | 2.0 | 440px each side — 26% of width |
| 5120×1440 (32:9) | 2.0 | 1280px each side — 50% of width |

**On every 16:9 display the knob does nothing at all**, because identical aspects
letterbox to nothing — ADR-0031 §1's finding, applied to the feature rather than
to integer scaling. It is real only on 16:10 panels, where it is worth a tenth of
the height, and on ultrawide, where the population most affected is also the one
most likely to hold the view that bars are the correct behaviour.

**A discharged dependency is not pressure.** ADR-0038 removed the reason this
could not be decided; it supplied no reason it should ship. The fog note's
*nothing presses on it* was accurate when written and is still accurate, and this
ADR records it as the finding it is — **not** as a ground. The grounds are §3 and
§4, which do not depend on demand and would refuse the feature in a world where
players were asking.

## 6. The refusal is normative, not an absence

ADR-0031 §2 already carries the operative sentence, inherited verbatim from
ADR-0030 §4:

> aspect ratio is preserved, and **no conforming implementation may scale the
> axes independently, shear, or stretch to fill**

It has no player exception. **This ADR adds none**, and that is the whole of its
normative content.

Saying so is not redundant, because
[ADR-0036](0036-the-window-opens-maximal-and-belongs-to-the-player.md)
established runner-owned, program-invisible state as legitimate — the window size
is exactly that. Absent this sentence, a runner could ship a stretch toggle in
its own affordance and argue the ban binds only the program-facing path, since
every document that states it does so while discussing a program-facing field.
It does not. The clause binds the **presentation**, whoever asked for it.

The ban was already operative in test form: P13 reads back the presented
framebuffer and checks the rect's aspect against the declared aspect, so a runner
presenting a stretched frame fails full conformance today. §6 closes the loophole
in prose as well, so a reviewer does not have to derive it from a harness.

## 7. No reopening trigger, and no head start

A named trigger — *revisit if players ask* — invites the next author to argue the
trigger fired rather than argue the case, and §5 is explicit that demand was never
the ground. So there is none.

Following ADR-0038 §10's precedent of paying in writing rather than absorbing:

> **This ADR is not a survey of the option space.** A future proposal for any
> non-similarity presentation transform — anisotropic, cropping, or otherwise —
> argues from scratch against §3, with no head start from this ADR's having
> considered two of them.

## 8. Conformance: no new property

**P13 already tests this**, in ADR-0031 §5's restatement: the canvas image
occupies *"a centred rect whose aspect equals the declared aspect, at
`k = min(w/W, h/H)`, with every pixel outside it opaque black and no program
content among them."* A stretched presentation fails the aspect clause and a
cropped one fails the `k` clause. Full-conformance only, unchanged.

Adding a property for *the runner offers no stretch affordance* was considered
and rejected: it is an assertion about a user interface, not about program-visible
behaviour, and [#19](https://github.com/adamico/ludo/issues/19)'s properties are
test-shaped or they do not ship.

## 9. Against issue #24: zero delta

No keyword, no operator, no facade call, no stdlib root name. **This is the
second time the fit has bought a zero by refusing a knob** — ADR-0030 §9 is the
first, and it says so in the same words.

Worth naming what was declined: had it shipped, the pair would have been
`$.video.stretch()` / `$.video.set_stretch()`, +2 functions on the root ADR-0038
§10 minted an ADR-0028 refusal clause to guard.

## 10. The three lenses

- **Simplicity.** A win. One fit, no branch, and nothing new for a player or a
  developer to learn. It would also have been the **first player preference whose
  effect a developer must anticipate in their art** — fullscreen and render scale
  are both invisible to authoring, and a stretch is not.
- **Robustness.** The similarity-transform invariant survives intact, and it is
  load-bearing twice over: ADR-0011's input mapping and ADR-0030 §6's clip rule
  both stand on it.
- **Agent-friendliness.** A win, and specifically #4's naming finding: a feature
  that does not exist cannot be reached for. Had it shipped, `$.video.stretch()`
  would sit beside the render scale in the root ADR-0038 §10 already identifies as
  an attractive nuisance, and the guard clause there would have been weakened by
  the first thing that walked through it.

**No lens conflict.** All three point the same way, which is uncommon enough in
this corpus to be worth stating.

## Amendments this ADR makes

- **ADR-0030** — §9's rejection of the ADR-0025-mould player preference is
  **confirmed, and the argument it lacked is supplied** (§2, §3). Its stated
  grounds address the `fit:` field only. §9 is otherwise unchanged, and its
  `fit:` holding is untouched.
- **ADR-0031** — §4's *"Recorded as open, and deliberately not decided here"* is
  **closed**: decided, and refused. §4's three passing tests are confirmed (§3),
  and its own similarity-transform grounding is what refuses it.
