---
status: accepted
---

> **Amended by [ADR-0006](0006-forbidden-and-required-shapes-of-the-platform-layer-api.md):**
> the mandated audio surface was at the wrong altitude — one `sound` call is replaced by a
> voice, a filter as a voice parameter, a raw sample push and a play cursor.
>
> **Amended by [ADR-0007](0007-the-audio-facade-surface.md):** the audio surface is spelled,
> the facade's naming rules are fixed for both halves, and the `nil` pixel array's
> checkerboard is kept as a mandated *named* debug pattern with no audio equivalent.
>
> **Amended by [ADR-0009](0009-the-drawing-facades-call-spellings.md):** the outline half of
> the mandated set is widened — ellipse and triangle gain stroke variants beside rect's —
> and input is separated from the drawing facade into its own area.
>
> **Amended by [ADR-0010](0010-paint-and-the-two-verb-drawing-facade.md):** the mandated set
> gains a tenth call — the sprite takes a `fill_`/`stroke_` pair, reversing `stroke_sprite`
> out of the renderer tier with a fixed alpha threshold and logical-unit thickness — and
> fills take a `Paint`, whose `Mapping` field states the UV convention the admission test
> requires.

# The spec mandates a thin drawing facade; the renderer and platform layer are delegated

ADR-0001 committed us to writing the platform layer ourselves and recorded that
it *"does not decide #28 — ownership and spec-coverage stay orthogonal"*. This is
the spec-coverage half. Issue #28 asked whether the standard library covers the
screen or stops at the destination's line and delegates it.

Neither. The spec mandates a **drawing facade** — roughly nine immediate-mode
calls in the reserved `$.` root — and delegates everything beneath it. The
platform layer and the WebGPU-model renderer stay ours, blessed by reference in
their own repo, on their own clock, **unmandated**. The engine tier stays out
per #12.

The destination therefore holds. It does not widen to a mandated platform
surface.

The question could only be answered once `CONTEXT.md` split **platform layer**,
**renderer** and **engine**: "covering the screen" was collapsing three tiers
with different owners into one word, and the answer differs per tier.

## Why not cover

The case for covering is real, and the facade is what preserves it. Names in the
reserved root are names the oracle knows, which is the direct mitigation for
#4's largest measured failure mode — 43.7% of agent compile errors are wrong or
non-existent names. And a language with **no ecosystem yet** cannot delegate the
way LÖVE delegated UI: #27 found that delegation is only as good as what you
delegate to, and on day one there is nothing.

What killed full cover is *where* its cost falls: the renderer. ADR-0002 already
records WebGPU as *"a versioned moving standard we do not control"*. Mandating
its pipeline and bind-group model in a spec governed by criterion 4 — a
behavioural change must force a signature change — and by #19's stability
obligations means freezing someone else's moving target while giving up the
escape valve #27 watched LÖVE use at every major version: colour range, matrix
majorness, shader language, hardware floor. The 0–255 → 0–1 colour change is the
warning, because it is silently wrong rather than loud.

A facade absorbs that churn. `$.draw.circle` does not move when WebGPU revises
its binding model.

## Why not delegate

It leaves #16's felt loop undelivered and makes ludo the only entry in #27's
comparison set where a fresh install cannot draw.

## The mandated set

Sprite (nearest sampling), filled rect and border, triangle, line, circle and
ellipse, pixel array, text with `measure`, sound playback, input query. Each was
admitted on having a statable observable, not on being useful.

**Specifying a shape is not specifying how it is built.** This is the reasoning
that admitted circles and thick lines after an earlier pass excluded them.
Stated analytically — a circle covers exactly those pixels whose centre
satisfies `|p − c| ≤ r`; a thick line is a capsule, `dist(p, segment) ≤ w/2` —
they are pixel-exact, backend-independent, and mention no tessellation.
Implementations render them as an SDF on a quad, which is cheaper and more
accurate than a triangle fan. Segment counts, joins, caps and miter limits, the
policies that made these look unmandatable, cease to exist. A circle is *more*
exactly specifiable than a rotated sprite.

Triangle is the general escape valve: any shape not mandated is a triangle list,
so the library tier is unblocked without reaching past the facade. Polygon and
concave tessellation stay in the library tier, being a genuine algorithm rather
than a distance function.

**There is no points primitive.** A pixel is a 1×1 rect. `love_PointSize` is one
of the things LÖVE 12 broke and point size has no portable meaning across
backends; deleting the primitive deletes the failure class.

## Considered options

- **Mandate the platform layer and the renderer** (issue #28's option ii). This
  is what LÖVE and DragonRuby both do. Rejected above: it freezes WebGPU's model
  under criterion 4, and it makes a four-backend renderer a precondition of the
  word "conforming".
- **Mandate nothing above the destination's line** (option i). Rejected above.
- **Mandate the platform layer, delegate the renderer.** The worst
  mandate-to-benefit ratio available: eighteen backends of obligation for names
  almost nobody types directly, and it still leaves *how do I draw a sprite*
  unanswered, which is the whole of #16.
- **Vendor-and-bind**, LÖVE's `love.physics`/Box2D pattern — ship a third-party
  engine and present it as native. Not applicable here for the same reason
  ADR-0001 declined SDL: it puts the #19 half outside our control.

## Consequences

- **Conformance splits in two.** *Core* is the compiler, the language and the
  non-visual stdlib, and is headless-testable. *Full* is core plus the facade
  plus at least one working backend — one platform, not four. **The map's
  completion test is measured against core**, which is the mechanism that lets
  the facade be mandated while *an implementer could start a compiler without
  asking further design questions* stays literally true.
- **Colour is a `distinct` type** with `$.rgb8` and `$.rgbf` constructors and no
  exposed representation. #11's machinery makes the LÖVE colour flip
  *unrepresentable* rather than merely forbidden: there is no bare number left
  to reinterpret.
- **Blend modes are alpha, additive and multiply.** The line is fixed-function
  blend state — all three are blend-equation parameters on every backend WebGPU
  targets, so all three are pixel-exact and free. Screen, overlay, subtract and
  the rest need a shader or a framebuffer read, and are renderer-tier.
- **Draw order is an explicit `layer:`**, stable-sorted, call order breaking
  ties, default 0. DragonRuby's fixed per-primitive bucket order was rejected:
  it is an invisible permanent constant, and #4's evidence punishes exactly that
  — nothing at a call site tells an agent that labels always beat sprites.
  `layer:` is criterion 2 applied to rendering.
- **Anti-aliasing is a property of the draw target, passed and never ambient.**
  AA is a scene-level aesthetic: smooth circles beside hard-edged everything
  else in one frame is worse than either choice made consistently, and
  per-primitive defaults would produce that mix in beginner code that passes no
  arguments at all. A target property gives one aesthetic per frame by
  construction, keeps #8's no-ambient-state rule intact, and generalises to
  render-to-texture, where a differing style is deliberate. A blanket AA-off
  mandate was considered and rejected: it buys exactness by selling smoothness
  whose absence is a lived nuisance in DragonRuby.
- **The conformance predicate is bit-exact with one carve-out.** Interior and
  exterior pixels must match exactly; boundary pixels may differ within a stated
  tolerance **only** for targets that opted into AA. A crisp-target program is
  bit-exact across the whole backend matrix.
- **Pixel arrays are first-class, with a performance guarantee in the
  contract:** upload cost is proportional to *mutation*, not to *use*. Baked
  once, a pixel array is an ordinary sprite forever. This is one of the few
  places contract language reaches into the draw surface, and it is there
  because a pixel array whose performance is unguaranteed is a trap rather than
  a primitive. It costs no new machinery — a pixel array is #15's `[]T` view at
  the boundary.
- **The default font is named in the spec** and shipped by every implementation,
  with `measure` exact for it. Identity becomes a conformance property: a ludo
  program looks like a ludo program on any implementation. Custom `.ttf` keeps a
  metrics-only contract, since glyph rasterization cannot be specified
  pixel-exactly. **This obliges a font rasterizer written in ludo** — an
  `stb_truetype`-shaped design, ADR-0001 having declined FreeType-class C
  dependencies — and it is the single largest item behind this facade.
- **Text is the one stated exception to pixel-exactness**, narrowed by the named
  default font to custom fonts only.
- **The facade is not sealed.** The renderer beneath stays reachable as an
  ordinary blessed-by-reference library, with the drop-down documented. #27's
  clearest lesson is that LÖVE's escape hatch is folklore while DragonRuby's is
  designed, and the designed one is cheaper. Sealing at nine calls guarantees
  the first serious custom-pipeline user forks or leaves.
- **The web is a negative obligation, not a mandated target.** Nothing in the
  facade may be unimplementable in a browser; no conforming implementation is
  required to have a web backend. This writes down the discipline already
  operating — #12's runner-driven entry and ADR-0002's model choice were both
  decided on browser grounds — without making an unbuilt backend a precondition
  of conformance. It bites immediately: the pixel-array upload guarantee and the
  AA tolerance both need checking against browser semantics before they freeze.
- **This ADR fixes the line, not the surface.** Exact signatures, coordinate
  origin and Y-direction, anchor semantics, sampling rules, premultiplied versus
  straight alpha, and the colour working space go to issue #40. The last two
  especially: both are colour-flip-class choices that criterion 4 freezes
  forever, and neither may be settled by accident.
