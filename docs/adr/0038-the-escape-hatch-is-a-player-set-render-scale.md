---
status: accepted
---

# The one performance escape hatch is a player-set render scale

Issue #75 was split out of [#71](https://github.com/adamico/ludo/issues/71) /
[ADR-0036](0036-the-window-opens-maximal-and-belongs-to-the-player.md) §8. #71
killed the video menu's **resolution row** — under a canvas fixed at 1280×720
that is a coordinate mapping rather than a framebuffer
([ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md)
§3, [ADR-0032](0032-the-canvas-is-fixed-at-1280x720.md)), the row has nothing to
select — but recorded that **the row's motive survives its death**.

The motive is fill rate. A fullscreen fragment shader at 4K shades four times the
pixels it does at 1080p with no change to the game's content, and it bites
exactly where the player has no window to drag.

This ADR ships the hatch. It is the **only** performance escape hatch in the
spec, for the player or the program, which is why it exists at all: the map's
standing ordering puts **performance and frame stability above rendering
fidelity**, and an ordering with no mechanism anywhere is a preference rather
than a rule.

## 1. What ships, stated as arithmetic

[ADR-0031](0031-integer-scaling-loses-to-the-priority-ordering.md) collapsed the
fit to one unconditional factor:

```
k = min(w / W, h / H)          W, H = 1280, 720
```

Content rasterises at device resolution with `k` folded into the backend's
matrices. This ADR adds one factor in front of it:

```
render scale  s ∈ [0.25, 1.0], quantised to sixteenths
rasterise at  s · k
present at    k
```

At `s = 1` — the default, and the only state most players will ever see —
`s · k = k` and **nothing whatsoever changes**. Below 1, the frame is rasterised
into an offscreen target at `s · k` and resampled up to `k` for presentation.

`W`, `H` and the fit are untouched. The world a program draws is identical at
every scale; only the number of device pixels shaded to show it changes.

## 2. Down only, and `s > 1` is refused by the ordering rather than by argument

`s > 1` is supersampling: rasterise at more device pixels than the display has
and resample down, paying fill rate to buy edge quality. It is a **fidelity**
feature whose cost is **throughput**, which is the exact trade the standing
ordering was written to decide. It loses, and it loses by rule.

Recorded because the symmetry is tempting and a future reader will wonder why
the range is not `[0.25, 4.0]`: nothing about the mechanism forbids it. The
ordering does.

The ceiling is therefore `1.0`, and it is a ceiling on the **preference**, not
on quality. A player who wants a sharper image already has one: the canvas is a
coordinate mapping, so at `s = 1` the game is rasterised at the panel's native
resolution however large that is. There is no fidelity left on the table for
`s > 1` to collect except antialiasing, which is
[ADR-0005](0005-the-drawing-facade-surface.md)'s `style` token's business.

## 3. It is not confined to the shader rung

[ADR-0037](0037-a-backend-clears-a-floor-and-shaders-are-the-third-rung.md)
created `core ⊂ full ⊂ shader`, and #75's motivating case is a fullscreen
fragment shader — a shader-rung artifact. Scoping the hatch to that rung was
considered and rejected.

The shader case is the **loudest instance, not the class**. Alpha-blended
overdraw is a fill-rate problem with no shader anywhere near it: a particle
storm, or `smooth`-scaled sprites covering the screen several times over, shades
the same pixel repeatedly through
[ADR-0010](0010-paint-and-the-two-verb-drawing-facade.md)'s blend path and costs
device pixels the same way. That is reachable from the mandated facade alone, at
the **full** rung, by a beginner who has written no shader and does not know what
one is.

Scoping to the shader rung would make the spec's only performance hatch
unavailable to precisely the program shape this language targets. The hatch
covers all rendering.

## 4. ADR-0030 §3's intermediate framebuffer, paid for rather than absorbed

#75 named this as the debt any answer owes, and it is the only place this ADR
contradicts a section of the corpus.

ADR-0030 §3 chose the coordinate mapping over an intermediate target and was
explicit about why: an intermediate target *"forces every backend to render at
logical resolution and upscale, which caps text and any `smooth` content at the
canvas's pixel density permanently, and makes the memory cost of a large canvas
unavoidable."*

**Every word of that survives, and none of it reaches this ADR**, because §3
rejected an intermediate target that was **mandatory, permanent and sized to the
canvas**. What ships here is **optional, player-invoked and sized to `s · k`**:

- **It is not mandatory.** The normative clause is two-sided. An implementation
  **MUST NOT** require an intermediate target at `s = 1`, and **MAY** use one
  only below it. The default path is bit-for-bit the path ADR-0030 §3 chose.
- **It is not sized to the canvas.** At `s = 0.75` on a 1440p panel the target is
  2880×1620, far above the canvas's 1280×720. §3's permanent density cap is a cap
  at *canvas* resolution; a render-scale target is a fraction of *device*
  resolution, which is a different and strictly larger number in every case the
  hatch is used for.
- **The density loss is the purchase, not a side effect.** §3's objection is that
  a program author would be capped without asking. Here a **player** asked, for a
  reason they hold and the spec does not, and can undo it at any time.

The one real cost is stated plainly: **at `s < 1` a conforming implementation
allocates a second render target**, and on a memory-constrained platform that is
a real allocation the `s = 1` path does not make. It is bounded — at most one,
never larger than the window — and it exists only while the player has chosen to
pay it.

## 5. Two mutators, and runner-adaptive is refused

[ADR-0025](0025-fullscreen-is-a-player-preference-the-program-can-read-and-set.md)
§3's shape carries over exactly, for the same reasons:

- **The runner MUST provide an affordance.** The case is **weaker** than
  fullscreen's and is recorded as such: a player with no fullscreen affordance is
  *trapped in a window*; a player with no scale affordance is merely *running
  slow*. It carries anyway, because a beginner's game that ships no menu and
  chugs on a weak machine is the exact failure this hatch exists to prevent, and
  the cost is one row on a surface ADR-0025 already forced into existence.
- **The program may set it**, through its own menu, which is what players expect
  of a finished game.
- **The scale is not the program's to cache.** The runner can change it without
  telling the program. Read it when you need it — ADR-0025 §3's discipline,
  unchanged.

**Runner-adaptive is refused.** A runner that drops the scale under load with
nobody asking is the best match for the standing ordering read narrowly, and it
was the candidate most likely to win on that ground alone. It loses on two
counts. First, it is a **silent third mutator**: a developer cannot reproduce a
visual result the runner changed behind their back, and the diagnosis path ends
nowhere, which is
[ADR-0018](0018-one-diagnostic-stream-for-the-compiler-and-the-faulted-program.md)'s
concern applied to rendering. Second, and decisively, **it is not necessary**:
`$.time.now` is a mandated ambient clock
([ADR-0016](0016-the-blessed-math-set-concrete-types-scalar-quantities-no-simd-mandate.md)),
the step is fixed at 60Hz and exactly one image is presented per returned frame
entry
([ADR-0035](0035-the-render-ceiling-is-accepted-and-one-present-per-entry.md)),
so **a program can measure its own overrun and drop the scale itself**. Adaptive
behaviour is expressible in ludo, by the author, visibly. That is where the
surprise belongs.

This is the argument that decided ownership, and it was not available to #75 as
written: the ticket implied a program could not usefully evaluate a performance
knob. It can.

## 6. The admission test: limb one, limb two, and no further

[ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md) binds
whoever proposes a query into `$.` that reads platform or backend state. The
getter is such a proposal, and it is run here rather than asserted.

**Limb one — reformulate.** *How hard should this machine work* restates as a
**player preference**, structurally identical to fullscreen: a fact about what
the human at the keyboard wants, not about the machine they own.

**Limb two — the return guard.** The getter returns a **unitless ratio**. `0.5`
carries no size, no scale, no density, no count, no refresh rate — **zero bits
about the hardware**, which is the guard's own wording. A program at `s = 0.5`
cannot tell whether it is on a 720p laptop or a 4K monitor, and two players on
different hardware who both chose 0.5 read the same number.

**It never reaches limb three.** Strict necessity is not tested, exactly as it
was not tested for fullscreen, because a proposal that survives reformulation
with a clean return does not drop through.

**§5's invariance clause is satisfied by construction** — see §8. Every
conforming implementation returns the same value for the same player action, on
the same machine and on any other, because the legal value set is a spec-fixed
grid of thirteen numbers rather than anything a backend computes.

The corpus table in ADR-0028 §6 gains a row.

## 7. The upscale filter is linear, on both style tokens

The resample from `s · k` to `k` is a step the corpus has never had, so the spec
must say what filters it or two runners will look different.

**It is linear, fixed, for both `crisp` and `smooth`.**

ADR-0031 §3's rule — *a style token may not mean different things on different
monitors* — is satisfied by any fixed answer and therefore does not decide this.
Stability does. At `s < 1` the pixel grid is already gone: `s · k` is not an
integer multiple of anything, so **nearest at this step produces aliasing
patterns that shimmer as the camera moves** — error that is worst in motion,
which is the stability half of the standing ordering and the half that ADR-0031
already used to delete integer scaling. Linear blurs uniformly and statically.

The cost is named: **a `crisp` pixel-art game at `s = 0.5` looks soft, not
chunky.** A pixel-art author may dislike that, and it is not their call — it is
the player's machine, and the player chose it.

**This does not touch content sampling.** `crisp` stays nearest and `smooth`
stays linear for how draws sample their textures, at whatever rate the frame is
rasterised;
[ADR-0034](0034-the-byte-exact-blit-is-withdrawn-and-the-texel-rule-replaces-it.md)'s
*which texel, never what byte* governs that and is unchanged. The linear rule
here governs one whole-image resample at the end, which no style token was ever
written to describe.

## 8. The grid: continuous in shape, thirteen values in fact

The argument is `f32` and the spec quantises it to the nearest sixteenth in
`[0.25, 1.0]` — thirteen legal values. The getter returns the quantised value,
never the argument.

**Continuous in shape**, because a program-side adaptive controller (§5) is the
whole reason program ownership beat runner-only, and a stepped enum cripples it.
A menu wanting three rows presents three rows over a continuous setter; the
reverse is impossible. Enumerating a canonical step list in the spec would also
be a list nobody agrees on, re-argued forever.

**Quantised in fact**, for two reasons that a purely continuous `f32` fails:

- **Allocation churn.** An unbounded `s` means the offscreen target of §4 is
  resized to an arbitrary size whenever it changes. A program ramping the scale
  smoothly would reallocate every frame. A fixed grid bounds the set of target
  sizes to thirteen.
- **ADR-0028 §5.** A spec-fixed grid makes the value **implementation-invariant**
  in the one place this feature could have broken it. Without it, `set(0.3)`
  followed by `render_scale()` is whatever a backend rounded to, and two
  conforming implementations disagree — a #5 criterion 4 Tier 1 dialect-variance
  veto, arriving through the back door.

**The floor is 0.25**, so nothing renders to a postage stamp; **the ceiling is
1.0** by §2. **Out-of-range input is clamped silently**, never faulted:
ADR-0036 §4 already set the precedent that a stale window size is clamped rather
than reset, and this is a preference setter, not a program-logic error. Faulting
the process because a menu slider overshot by a float epsilon is the wrong
trade. The #4 objection — silent clamping hides mistakes from an agent — is real
and loses, because the getter reports the truth on the very next line and P15
makes it checkable.

**No new quantity type.** ADR-0016 blesses `Radians`, `Seconds` and
`SampleFrames`; a unitless ratio needs none, and minting one here would spend a
stdlib type to describe a number with no dimension.

## 9. It survives relaunch, and provenance is not tracked

The scale is **runner-owned state that survives relaunch**, on ADR-0025 §6's
terms: the spec names no file, no format and no location, only that a player who
set it finds it set next time.

**Who set it is not recorded.** ADR-0025 §3 makes both mutators legal without
distinguishing them, and splitting player-set from program-set would invent a
two-state model the corpus has nowhere else. The consequence is deliberate and
is a feature: **a program's adaptive controller persists whatever it converged
to**, so the next launch starts near the right number for that machine instead
of rediscovering it every time.

**It cannot go stale.** This is where it differs from
[#68](https://github.com/adamico/ludo/issues/68)'s display ordinal, which was
demoted to map fog partly because an unplugged monitor leaves an ordinal
pointing at nothing. Every value in `[0.25, 1.0]` is legal on every machine
forever, so there is no clamp-versus-reset question to answer and no hardware
change that can invalidate a stored scale.

## 10. `$.video`, and the guard the root name costs

The calls live under a new root:

```
$.video.render_scale()             -> f32
$.video.set_render_scale(s: f32)
$.video.fullscreen()               -> bool
$.video.set_fullscreen(on: bool)
```

Getters, not values: ADR-0025 §3's *not the program's to cache* binds both pairs.
Spelling follows [ADR-0009](0009-the-drawing-facades-call-spellings.md)'s
convention, with `set_` prefixing the mutator.

**This discharges [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md)
§9's pending item.** The fullscreen pair has had no module since ADR-0025, which
left it conditional and ADR-0033 deliberately left it uncounted. Render scale
forced the question, because putting two runner-owned player preferences in two
different roots is a split with no principle behind it.

**Neither call belongs in `$.graphics`.** Every `$.graphics` call takes a
`Target` or an `Image`; these take neither, because they are facts about how the
image reaches the player rather than operations that produce it.

### The roots that were rejected

- **`$.screen`** — dead on collision.
  [ADR-0013](0013-the-drawing-entry-takes-the-screen-target.md) §1 gives the
  frame entry `screen: !Target`, so `screen` already denotes the Target. #5
  criterion 4 Tier 1: one spelling per entity.
- **`$.view`** — dead on collision.
  [ADR-0015](0015-assets-are-declared-not-loaded.md) and #15 make `[]T` **views**
  the universal boundary.
- **`$.settings`** / **`$.prefs`** — dead by name.
  [ADR-0026](0026-a-program-writes-bytes-to-declared-storage-slots.md) §9 refuses
  a settings concept explicitly, on the ground that volume and key bindings are
  **program-defined** and belong in `$.storage`. A root spelled this way
  readmits them.
- **`$.window`** — ADR-0025's own suggestion, and **rejected**, which is recorded
  at length because two accepted ADRs point at it and a future reader will assume
  it was the plan. Under ADR-0030, ADR-0032 and ADR-0036 §7 a *window* is a noun
  the spec deliberately keeps out of the program's vocabulary: fixed canvas,
  unreachable bars, no program-visible geometry. Minting the root makes
  `$.window.size()` the most natural next reach an agent could make, and that is
  the query ADR-0028 §6 refuses.
- **Bare root names** — `$.fullscreen()`, `$.set_render_scale()` and so on, at
  the root beside `$.print`, `$.assert` and `$.time.now`. **Cheapest against
  #24** — zero module names — and rejected for having no domain: the root becomes
  the place things go when they belong nowhere, and ADR-0033 §9 already records
  that the root's contents are enumerated nowhere in the corpus.

### The guard `$.video` costs, and the lens conflict behind it

`$.video` is the word players and developers already use for this exact pair of
controls, and that familiarity is why it was chosen. It is also the word whose
**only two appearances in this corpus** — ADR-0028 §3 and ADR-0036 §8 — are the
phrase *"video settings menu"*, both times attached to the **resolution row that
was refused**.

So the root carries the same attractive nuisance that killed `$.window`, and it
is paid for in writing rather than absorbed:

> **`$.video` is not a precedent.** `$.video.resolution()`, `$.video.size()` and
> `$.video.dpi()` remain refused on ADR-0028 §6's stated grounds — they return
> hardware magnitudes and no mandated facility needs them. The existence of the
> root does not admit them, and a future proposal to add one runs the full
> admission test with no head start.

This is the **lens conflict** this ADR owes, named rather than silently resolved:
**simplicity picks `$.video`** — every player knows the menu it is named after —
and **agent-friendliness picks against it**, because #4's finding is that absence
is the only reliable defence against an agent reaching for a name from its
training data, and training data is full of `video.resolution`. Simplicity wins
here; agent-friendliness is repaid with the clause above, with P15, and with the
fact that the refused calls do not exist to be called.

## 11. What this does not make observable

Two consequences that could have been leaks and are not, recorded so nobody
re-derives them:

- **No readback.** `get_pixels` takes an `Image`, never the screen (ADR-0036 §7),
  so the corpus has no screen-readback path at all. A program cannot observe the
  offscreen target, its size, or the resample. The scale is visible through its
  own getter and nowhere else.
- **No effect on input mapping.** `to_logical` maps window coordinates to canvas
  coordinates through `k` (ADR-0030 §4). `s` is a rasterisation detail applied
  after the transform, so **aiming is unaffected at every scale** and a player at
  `s = 0.25` clicks exactly where a player at `s = 1` does.

Together these mean the render scale changes how sharp the frame is and changes
nothing else a program or a player can detect.

## 12. Conformance: P15, and the first property core can run

**One new property, P15**, testing the quantisation contract: `set_render_scale`
followed by `render_scale` returns the value on the grid, and out-of-range inputs
land on the stated bounds.

It is **core-conformance runnable**, which makes it the first property here that
is — P13 (the fit) and P14 (the window) are both full-conformance only, because
both need an external harness looking at a window. P15 needs neither a GPU nor a
screen: it is a pure API round-trip a headless implementation answers.

**There is deliberately no property for what the image looks like.** *"At
`s = 0.5` the frame is rasterised at half `k`"* is a framebuffer readback, and
ADR-0030 §8's own test rules it untestable — the same ground on which ADR-0035
declined a property for the presentation clause and
[ADR-0022](0022-the-spec-promises-only-what-is-derivable.md) declined to promise
what it could not derive. §1's arithmetic and §4's two-sided MUST NOT / MAY ship
as normative prose a backend reviewer reads, not as an assertion a harness runs.

## 13. Against issue #24

**Core grammar: zero.** No keyword, no operator. **Type sublanguage: zero.** No
new type, no new quantity (§8).

**Stdlib root-name companion count: +1 module name, +4 functions, 0 types,
0 values.** The spelled facade roots go from four to five — `$.graphics`,
`$.audio`, `$.input`, `$.storage`, `$.video`.

**#75's own new spend is +2 functions.** The module name was **pre-booked**:
ADR-0025's Budget section states the count *"gains one module name if these calls
live under a new root"*, and ADR-0033 §9 held the fullscreen pair out of the
table pending exactly this decision. This ADR converts two conditional entries
into actual ones and adds two of its own.

`$.video` — 4 functions, 0 types, 0 values.

## 14. The three lenses

- **Simplicity.** A Lua user recognises a video settings menu instantly, and the
  root is named after it. Two getters and two setters, no new type, no new
  concept. A program that never mentions `$.video` is unaffected: `s = 1` is the
  default and the fit is unchanged, so the hatch costs nothing to ignore.
- **Robustness.** The grid makes the value implementation-invariant where a raw
  `f32` would have licensed backend divergence (§8); clamping removes a fault
  path from a preference setter; §4's two-sided clause stops an implementation
  from quietly making everyone pay for a feature nobody switched on. Against
  that, §4 is honest that a second render target exists below `s = 1`.
- **Agent-friendliness.** The **weakest** of the three, and the conflict is named
  in §10 rather than hidden: `$.video` is the one name in this decision that an
  agent's training data will push past, toward `video.resolution`. The
  countermeasures are a written refusal clause, a getter that reports the grid
  truthfully, and P15. Below the root name the picture is clean — the effect is
  in the signature, one spelling per entity, no dialect variance, and an agent
  writing an adaptive controller has everything it needs locally (`$.time.now`,
  the setter, the getter) with no whole-program view required.

**One lens conflict, in §10, resolved in simplicity's favour on the ticket
owner's call.**

## Amendments this ADR makes

- **ADR-0030** — §3's rejection of the intermediate framebuffer is **scoped, not
  overturned**: it holds for a mandatory canvas-sized target and does not reach
  an optional, player-invoked target sized to `s · k` (§4).
- **ADR-0025** — its Budget section's **conditional module name is resolved**,
  and the fullscreen pair is **spelled** as `$.video.fullscreen` /
  `$.video.set_fullscreen` (§10). Its §3 two-mutator rule is extended to a second
  preference (§5), and its §6 persistence rule to the same (§9). The suggested
  `$.window` root is **rejected** (§10).
- **ADR-0033** — **§9's pending item is discharged.** The fullscreen pair enters
  the companion count, under `$.video` (§13).
- **ADR-0028** — its §6 corpus table gains a **row** for the render-scale getter,
  admitted at limbs one and two (§6). Its §6 refusals of display size and DPI are
  **reaffirmed against the new root**, which is not a precedent for them (§10).
- **ADR-0031** — §3's *a style token may not mean different things on different
  monitors* is **satisfied and does not decide** the upscale filter; stability
  does (§7).
- **ADR-0036** — its §8 hand-off is **discharged in full**.
