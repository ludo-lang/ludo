---
status: accepted
---

> **Amended by [ADR-0007](0007-the-audio-facade-surface.md):** the naming rules this ADR
> left open are now fixed for both facades, and the parked `nil`-pixel-array checkerboard
> is kept and specced as a mandated *named* debug pattern. Drawing's call spellings are
> issue #42.
>
> **Amended by [ADR-0009](0009-the-drawing-facades-call-spellings.md):** the call spellings
> this ADR left open are fixed under `$.graphics`, and the target is delivered at the frame
> entry rather than fetched.
>
> **Amended by [ADR-0010](0010-paint-and-the-two-verb-drawing-facade.md):** the byte-exact
> blit clause is restated against the paint rather than the sprite, and widened to any 1:1
> `stretch` texture paint at integer coordinates on a `crisp` target.

# The drawing facade's surface: top-left Y-down, straight alpha, and a target that carries the frame

ADR-0004 fixed the *line* — the spec mandates a thin drawing facade and delegates
the tiers beneath it — and deferred the *surface* to issue #40. This is the
surface. Every choice here is frozen by criterion 4 once mandated, so each is
recorded with its reason rather than only its outcome.

The primary evidence was DragonRuby's own API documentation read locally, from a
copy in the push-n-pull tree: `docs/api/grid.md`, `outputs.md`,
`pixel_arrays.md`, `layout.md`. That is a materially better source than #27
could reach — `docs.dragonruby.org` returned 403 and 404 to that research, whose
DragonRuby claims came through search summaries — and reading it reversed two of
this decision's own starting positions.

## Coordinate system: top-left origin, Y-down, fixed

**Handedness is Y-down and is not configurable.** The load-bearing reason is
**prior conformance, not backend convenience**. Top-left with Y-down is the
overwhelming convention: OS mouse coordinates, image file row order, the
framebuffer spaces of WebGPU, D3D12 and Metal, HTML and CSS. It is what a model
predicts by default and what a human already knows. DragonRuby's bottom-left has
to be re-learned and re-documented at the start of every agent session — a
standing tax that is invisible in a feature comparison and constant in practice.
That is #4's naming evidence applied to a coordinate system.

The secondary argument is real but minor: Y-down means no flip anywhere between
the facade and the WebGPU-model renderer of ADR-0002.

**A transform does not buy Y-up.** This ADR's own first draft claimed it did, and
that was wrong. A `y_scale = -1` transform flips the quad's vertices, and a
sprite's texture coordinates ride on those vertices, so the image flips with
them; reaching Y-up through a transform therefore requires flipping every
texture coordinate too. That is the same systematic flip, relocated. Handedness
must be frozen in the spec.

A detail from inside the Y-up peer points the same way: DragonRuby documents
pixel-array indexing as `(height - y) * width + x` *"for a bottom-left
coordinate system"*. Its pixel memory is row-0-is-top, and its API convention
fights its own storage.

**Origin *location* is fixed top-left, with centre-origin reached by the target
transform.** DragonRuby offers a global mutable mode — `Grid.origin_bottom_left!`
and `Grid.origin_center!`, readable as `Grid.origin_name`, with no top-left
option at all. Rejected: an origin is a **shared vocabulary** between a caller
and every library it uses, unlike anti-aliasing, which is a rendering property
that composes. Under a mode, a library drawing at `y = 10` means different things
to different callers and must ask or guess — a dialect wearing a field name, and
#4's evidence says nothing at the call site would tell an agent which convention
is live. Centre-origin is mathematically `translate(w/2, h/2)`, so the transform
below supplies the capability with no mode and no global state.

## Colour and alpha: straight alpha, sRGB

Deliberately the *less correct* option. Linear-space blending makes a 50%-alpha
sprite composite differently from what the artist saw in their editor; for a
pixel-art-facing facade that is the tool ecosystem disagreeing with the runtime,
not a problem education fixes. Premultiplied alpha is better engineering —
correct filtering and mipmaps, alpha and additive in one blend state — but it
forces every `.png` through a conversion on load and makes the pixel array's
`[]u32` stop meaning what the user wrote into it, breaking the guarantee
ADR-0004 mandated. Both rejected options stay reachable at the renderer tier
through the non-sealed facade.

**ADR-0004's negative web obligation earned its keep here**, catching a silent
desktop/web divergence in a backend that does not exist yet. The browser holds an
opinion about premultiplication in two places, and both are closed now rather
than deferred:

- **Canvas compositing.** The web backend must configure an **opaque presentation
  surface**, so the page never composites our alpha and the browser's
  premultiplication behaviour is unreachable. The cost is transparent
  canvas-over-HTML, which nothing in the facade exposes.
- **Texture upload.** The web backend must **decode images itself and upload raw
  bytes**, never routing through the browser's image pipeline, which may return
  premultiplied pixels depending on flags and image metadata. Otherwise sprite
  blending diverges between desktop and web *silently*. No dependency question
  arises: asset IO and the font rasterizer are already ours under ADR-0001.

## The draw target

An opaque value, passed and never ambient, carrying everything that must hold for
a whole frame.

- **`style:`** — `crisp` (no anti-aliasing, nearest sampling) or `smooth`
  (anti-aliasing, linear sampling). **One token, not two fields.** Both landed on
  the target because mixing aesthetics inside one frame is the failure mode; two
  independent fields would re-admit that mix one level down, with two of the four
  combinations incoherent. `crisp` is the default. It also makes the conformance
  rule a sentence about one named thing — *a `crisp` target is bit-exact across
  the whole backend matrix* — instead of a predicate over a field combination.
  DragonRuby's per-sprite `scale_quality_enum` is the road not taken.
- **A 2D transform** — offset, scale, rotation — applied to every call against the
  target. Mandated, because the origin decision *depends* on it: without a
  transform, a fixed top-left origin has no escape valve. Camera, zoom and
  screen-shake come free, and the nine signatures are unchanged.
- **A caller-declared logical canvas.** The program states its logical size and
  the facade letterboxes and scales to the real window. DragonRuby's Grid fixes
  720p and 16:9 for everyone, which is untenable for a language not exclusively
  for 16:9 games; real pixels with no logical canvas means a beginner's first
  game breaks on a different monitor, a bad first-run experience for a project
  whose artifact is the felt loop. The logical space is also where the integer
  blit guarantee lives.
- **Offscreen-target shape reserved.** The type is specified so offscreen targets
  can arrive without a signature change. This is what serves the deliberate
  mixed-aesthetic case — a `smooth` HUD over a `crisp` game — that a single
  `style:` token otherwise forecloses.

## Sprites

**Two normalized anchors, not one:** a position anchor and a rotation pivot,
independent, both defaulting to centre. A sprite positioned by its feet
(`anchor_y = 0`) that rotates about its centre of mass is ordinary, and with a
single anchor the caller redoes that arithmetic every frame. DragonRuby ships
both — `anchor_x`/`anchor_y` and `angle_anchor_x`/`angle_anchor_y` — which is
evidence that one was not enough in practice. Normalized rather than pixel units,
so an anchor survives a change of art size.

**The byte-exact blit is normative in its own right:** a sprite drawn at integer
logical coordinates, unrotated, under an identity or integer-translation
transform, on a `crisp` target, is a byte-for-byte blit. This is the property
pixel-art work actually depends on and the one people file bugs about, so it is
a sentence in the spec rather than an emergent consequence of a rounding rule.
Fractional and rotated cases get the explicit texel-centre and tie-break
convention.

## Text

`measure` returns advance width and bounding box, and **accounts for kerning** —
otherwise it disagrees with what is drawn, which is the whole point of mandating
it. Per-glyph positions are excluded: that is the beginning of a text-layout API
and the facade is not one. A caller needing cursors or selection measures
substrings, and reaches for a real text library beyond that.

The default font is identified by **name, version and content hash**. A bare name
reopens exactly the divergence that naming it was meant to close, because font
metrics move between versions and `measure` exactness is the guarantee at stake.

## Blend spelling

A **closed enum** of ADR-0004's three modes. DragonRuby exposes composable blend
factors — `Numeric.compose_blendmode(BLENDFACTOR_ZERO, ...)`, with a holepunch
example — which is evidence the need is real. But in a nine-call facade that is
the renderer's vocabulary leaking upward, and it would make the conformance
surface the entire blend-factor cross product instead of three modes.
Composition stays at the renderer tier, and this is a concrete case the
documented drop-down path must serve.

## Conformance tolerance

The tolerance is on **coverage, not colour**: an implementation's computed
coverage must fall within a stated bound of the true analytic coverage. A fixed
per-channel colour tolerance means something entirely different on a
high-contrast edge than on a low-contrast one, so it is either too loose or too
tight depending on the colours; and "floating-point evaluation error" is an
excuse rather than a bound, which leaves the conformance suite unwritable.
Coverage is the quantity the spec already states analytically, and a coverage
bound is assertable across the backend matrix without knowing which colours were
used. It applies only to `smooth` targets.

## Consequences

- **`crisp` is the bit-exact case and the default**, so the ordinary program is
  fully conformance-testable across every backend; the tolerance clause is
  something a user opts into.
- **The literal spelling is still open.** This ADR fixes the facade's semantics,
  not its call names and signatures under the reserved `$.` root. Naming belongs
  with the module-system and stdlib-naming questions, not with drawing.
- **Excluded: DragonRuby's `@125`/`@150`/`@200` sprite-variant auto-selection.**
  Resolution-dependent asset substitution is an asset pipeline, ruled out by #12.
- **Worth stealing, recorded for wherever facade error behaviour is written:**
  DragonRuby renders a pixel array whose pixels are `nil` as a **checkerboard**
  rather than failing. An error affordance that shows the mistake on screen fits
  #18's error-as-pause posture.
