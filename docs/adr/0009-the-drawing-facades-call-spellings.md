---
status: accepted
---

# The drawing facade's call spellings: `$.graphics`, verb-first, a descriptor per call

ADR-0005 fixed the drawing facade's *semantics* and explicitly left open "its call
names and signatures under the reserved `$.` root". ADR-0007 then fixed the
**naming rules** once for both facades and applied them to audio. This ADR applies
them to drawing.

The six rules are settled input and are not reopened: one module per facade area
under `$.`; verb-first for actions and bare noun for values; `snake_case`; no
abbreviations at all; a descriptor struct as the single argument past two
parameters; no overloads. Every ADR-0005 semantic is likewise inherited unchanged
— Y-down top-left, straight alpha and sRGB, the single `style:` token, two
normalized anchors, the byte-exact blit, kerning in `measure`, the closed blend
enum, and coverage-not-colour tolerance.

## The module is `$.graphics`, not `$.draw`

`$.audio` is an **area noun**. `$.draw` is a verb, and a verb module collides with
rule 2 the moment its calls are also verb-first: `$.draw.draw_text` stutters, and
avoiding the stutter means abandoning verb-first for the whole surface.
`$.graphics` is the same part of speech as `$.audio`, so rule 1 and rule 2 hold
together with no deviation to record.

**Colour constructors stay at the root.** ADR-0004 named `$.rgb8` and `$.rgbf` at
the `$.` root, which looks like a rule 1 violation and is not, under a distinction
this ADR states so it stops being an exception: **root names are constructors for
blessed types; modules are facade areas.** `$.rgb8` is the same category as
`$.vec3(1, 2, 0)`, which #15 already put at the root, and `Color` is used by code
that never draws — configuration, data tables, interpolation.

**Input is not in this module and is not in this ADR.** ADR-0004 mandated "input
query" in the same breath as the drawing calls, but input shares nothing with
drawing except a backend, and — unlike drawing — has no settled semantics ADR to
apply names to. Spelling it here would decide its semantics under a naming ADR's
cover. It is its own area, its own module, and its own ticket.

## Verb-first, and the three verbs

The set is verb-first. This is rule 2, but it is also what #4's evidence
independently recommends: the corpus a model has actually seen is overwhelmingly
verb-first for 2D drawing — Canvas 2D's `fillRect`/`strokeRect`/`fillText`,
raylib's `DrawRectangle`, SDL's `SDL_RenderFillRect`, LÖVE's `love.graphics`
module name. A noun-first set (`rect_fill`, `text_measure`) was considered and
rejected: it buys grep locality for shape families and loses first-try
correctness, and #4 says that is the wrong trade.

There are exactly three verbs, with non-overlapping meanings:

- **`fill_`** — the shape's area, in one solid colour.
- **`stroke_`** — a path with thickness. Every stroke descriptor carries
  `thickness`.
- **`draw_`** — a thing that carries its own appearance, and so is neither filled
  nor stroked.

**The rule choosing between them: a shape gets `fill_`/`stroke_` only if it admits
both; otherwise `draw_`.** This is what puts `draw_line` and `draw_sprite` in the
set rather than `stroke_line` and a sprite pair. A line has no interior, so
`fill_line` would never exist and the pair is meaningless for it — which means the
`draw_line` corpus prior (raylib, LÖVE) comes free instead of being paid against.

**`fill_text`, not `draw_text`.** Text is the case where the rule bites in the
other direction: glyphs have an interior, outlined text is a real thing, and
Canvas has `fillText` **and** `strokeText` with no `drawText`. Criterion 4 freezes
a mandated name, so spending `draw_text` today makes a future `stroke_text` pair
incoherently with it forever. `fill_text` reserves the pair at zero cost.

`stroke` is a verb in its own right, not an API convention: its graphics sense
descends from PostScript, where `fill` and `stroke` are imperative operators on a
built path. `stroke_rect` parses as "stroke this rect".

## The mandated set

Nine drawing calls, matching ADR-0004's count, and six further names.

```
$.graphics.fill_rect(target: Target, desc: RectDesc)
$.graphics.stroke_rect(target: Target, desc: RectStrokeDesc)
$.graphics.fill_ellipse(target: Target, desc: EllipseDesc)
$.graphics.stroke_ellipse(target: Target, desc: EllipseStrokeDesc)
$.graphics.fill_triangle(target: Target, desc: TriangleDesc)
$.graphics.stroke_triangle(target: Target, desc: TriangleStrokeDesc)
$.graphics.draw_line(target: Target, desc: LineDesc)
$.graphics.draw_sprite(target: Target, desc: SpriteDesc)
$.graphics.fill_text(target: Target, desc: TextDesc)

$.graphics.measure_text(desc: TextDesc) -> TextMetrics
$.graphics.slice_image(image: Image, region: Rect) -> Image
$.graphics.create_pixel_image(size: Vec2) -> Image
$.graphics.get_pixels(image: Image) -> []u32

$.graphics.default_font -> Font
$.graphics.checkerboard -> Image
```

## Stroke for every shape, and what a `_border` is

ADR-0004 mandated "filled rect and border" — one outline, for rect only. **This
ADR widens that to `stroke_` for every closed shape**, and records why the
original narrowness did not survive scrutiny.

The reasons offered for keeping ellipse and triangle outlines out were real but
solvable, and one reason offered was not a reason at all:

- **"An ADR mandated it"** is authority, not an argument. Dropped.
- **"Four lines double-blend at the corners"** — true, and not a defect. An
  immediate-mode facade with straight alpha *defines* two overlapping draws as
  compositing twice; a backend that merged them would be non-conforming. The user
  asked for four lines and got four lines. A primitive is not mandated because
  the hand-rolled version is mildly annoying in a way the author knew about in
  advance.
- **Triangle outline needs a join policy** — miter, bevel or round are visibly
  different at sharp angles, and a miter's length grows without bound as the
  angle narrows. Real, and the joins/caps class ADR-0004 deleted.
- **An ellipse has no closed-form constant-width outline** — exact perpendicular
  distance to an ellipse requires solving a quartic per pixel. Also real.

Both survivors dissolve under a single definition applied to the whole family:

> **A stroke is the shape minus its inset shape.** `stroke_rect` is the rect
> grown by `thickness/2` minus the rect shrunk by `thickness/2`; `stroke_ellipse`
> and `stroke_triangle` are the same sentence. Where the inner shape degenerates
> — a negative radius, an inverted inset — the result is the filled shape.

That is a *definition*, not a policy choice: the triangle's corner behaviour is
the miter, stated rather than selected, and there is no miter limit to pick
because a sliver triangle's stroke swallows the triangle instead of growing
spikes. Every case is analytically statable, pixel-exact, and rendered as nested
SDFs with no tessellation.

**The cost is stated out loud:** a stroked ellipse's band is **not** constant
width. It is thicker near the ends of the major axis and thinner at the flat
parts, and the discrepancy grows with eccentricity. This is the honest
consequence of taking the cheap exact definition over the quartic, and the spec
says so rather than leaving implementations to diverge.

## Descriptors, and `Vec2` over scalars

Counting the target, every drawing call exceeds rule 5's two-parameter threshold,
so every drawing call takes a descriptor struct. One descriptor type per call, not
a shared one: a single struct whose meaningful fields depend on which call
received it is rule 6's overload wearing a type, and an agent cannot tell from the
declaration which fields are live.

Descriptor fields are **math types, not scalars** — `position: Vec2`, not `x, y`.
The argument that decided it is the call site: real game code stores position as a
vector, so scalar fields make every draw call in every program splat
`e.position.x, e.position.y`, every frame, forever — a tax on the common case to
serve literal coordinates, which are mostly a beginner's first hour and debug
drawing. Two arguments offered for scalars did not survive: `Vec2{x, y}` is a
two-field struct a beginner reads once, not high-level mathematics; and field
defaults are irrelevant for position and size, since nobody draws a rect at a
default position. The honest remaining cost is #4's: `$.vec2` is a name an agent
must know exists and get the arity right. #15's `$.vec3(1, 2, 0)` precedent and
#33's shader-visible math types outweigh it, and the payoff is that one value
crosses from gameplay to draw call to shader without changing shape.

```
RectDesc            { position: Vec2, size: Vec2 }
RectStrokeDesc      { position: Vec2, size: Vec2, thickness: f32 }
EllipseDesc         { position: Vec2, size: Vec2 }
EllipseStrokeDesc   { position: Vec2, size: Vec2, thickness: f32 }
TriangleDesc        { point_a: Vec2, point_b: Vec2, point_c: Vec2 }
TriangleStrokeDesc  { point_a: Vec2, point_b: Vec2, point_c: Vec2, thickness: f32 }
LineDesc            { start: Vec2, end: Vec2, thickness: f32 }
SpriteDesc          { image: Image, position: Vec2, size: Vec2,
                      anchor: Vec2, rotation_anchor: Vec2, angle: Radians,
                      flip_horizontal: bool, flip_vertical: bool }
TextDesc            { text: string, font: Font, font_size: f32,
                      position: Vec2, anchor: Vec2 }
```

Every descriptor also carries `color: Color` and `blend: Blend`, defaulting to
`alpha`. **`blend` is per-call, not on the target.** ADR-0005 put `style:` on the
target because mixing aesthetics within one frame is the failure mode; that
argument does not transfer, because mixing blend modes within one frame is
ordinary and correct — additive particles over alpha sprites. Putting either on
the target as set-and-draw state was rejected outright: that is ambient state,
which ADR-0005 already refused for the origin.

Stroke descriptors repeat their fill descriptor's fields rather than nesting one
inside the other (`RectStrokeDesc { shape: RectDesc, thickness }`). At three or
four fields, nesting buys nothing, and a flat literal is what an agent writes
correctly on the first try.

Field names carry two deliberate choices. **`thickness`, not `width`** — `width`
is already the x component of a `size`, and a name meaning two things in one
descriptor set is the ambiguity that is invisible at a call site. **`font_size`,
not `size`** — `size` is a `Vec2` extent everywhere else, and the font's is a
scalar.

## The ellipse is inscribed in a bounding box

`EllipseDesc` is `{ position, size }` — field-for-field identical to `RectDesc`,
top-left anchored, with the ellipse inscribed in that box. Centre-plus-semi-axes
(`{ center, radius }`) was the initial recommendation and was wrong.

The case for `{ center, radius }` was that a circle's radius is usually a gameplay
number — collision radius, blast radius — and centre-anchoring takes it
unconverted. But that assumes entities are centre-based, and they are not: an
entity's bounding box is top-left anchored, so a centre-anchored circle forces
`x + width/2, y + height/2` at the call site instead. **A centre-anchored shape
among top-left-anchored ones is the trap, not the convenience.** Under the
inscribed-box form, drawing a circle over an entity's bounding box is a
field-for-field copy — `{position = e.position, size = e.size}` — with no
arithmetic anywhere.

Foci and eccentricity were considered and rejected: right for orbital mechanics,
wrong for drawing. It degenerates for the circle (coincident foci, undefined
orientation), it is numerically fragile near zero eccentricity, it needs the
semi-major axis anyway, and since shapes are axis-aligned the foci can only lie on
an axis, making the encoding redundant.

One trap this closes by construction: **`width`/`height` of an ellipse mean the
full axis lengths**, never the semi-axes. A field named `width` holding a
semi-axis would be undetectable at a call site.

## Shapes are axis-aligned; sprites rotate

ADR-0005 put rotation on the target, frame-wide, and gave sprites a per-call
rotation pivot. Those two decisions were made separately and left an asymmetry
nobody chose: **sprites have a per-call `angle` and shapes do not.**

This ADR keeps the asymmetry and states it. A rotated sprite is the single most
common thing in a 2D game; a rotated rect is rare enough that two
`fill_triangle`s cover it, and `draw_line` covers arbitrary angles directly.
Giving every shape an `angle` and a pivot would duplicate a transform the target
already owns, and would require a byte-exact-blit equivalent clause for every
primitive.

This is also why **`draw_line` does not collapse into `fill_rect`**: without a
per-call angle, `fill_rect` is axis-aligned in logical space, and a thick line is
a capsule with round ends where a rect's are square. Two different pictures and
one capability that only `draw_line` has.

## The target arrives at the frame entry

`Target` — the bare noun, no abbreviation. Not `Canvas`, which collides with the
*logical canvas* that is a field inside it; not `Screen`, which the reserved
offscreen shape would immediately falsify.

A program does not construct or fetch the screen target: **the runner passes it
into the per-frame entry point.** A fetch (`$.graphics.screen()`) is ambient state
wearing a function, with nothing preventing two calls mid-frame. #26's frame entry
is already the place frame-wide things arrive, and the `style:` token and the
logical canvas are declared once at that entry rather than per frame. The reserved
offscreen path later adds a constructor without disturbing this.

UFCS does the rest. Every call is declared `$.graphics.<verb>(target: Target,
desc: …)`, and #16's free-functions-plus-UFCS makes `target.fill_rect({...})` fall
out at the call site with no second name and no overload. This is one of the few
places the beginner and agent lenses do not conflict: the agent writes the
qualified form it can guess, the human writes the short one, and they are the same
function.

## Images: sliced, not sub-rectangled

`Image` is the type. Not `Texture`, which is the renderer tier's vocabulary
leaking upward past the line ADR-0004 drew; not `Sprite`, which is the drawn
instance rather than the pixels, and is already the call's name.

**This ADR names the type and does not spell its loader.** How an `Image` is
obtained is asset IO — path resolution, missing-file behaviour, synchronous or
not — three decisions this ticket has no basis for. One carve-out: `Image` is the
**same type the pixel array produces**, because otherwise ADR-0004's guarantee
that "a pixel array is an ordinary sprite forever" has nowhere to live. That is
what `create_pixel_image` and `get_pixels` are: a computed `Image` and a mutable
`[]u32` view into it, with ADR-0004's upload-cost-proportional-to-mutation
guarantee attaching to the view. There is no pixel-array *draw* call — a pixel
array is drawn with `draw_sprite`, which is the whole point, and a second path to
the screen would have nowhere to put anchors, flips and sub-images.

Sprite sheets are handled by **`slice_image(image, region) -> Image`**, which
returns a view into another image, rather than by a `source: Rect` field on
`SpriteDesc`. The sheet concept moves out of the hot path and into a value named
once at startup: animation code holds `frames: [4]Image`, a tileset is sliced into
its tiles, and the call site reads `draw_sprite(target, {image = frames[i], ...})`
with nothing sheet-shaped in it. It is #15's view-into-owned-storage shape, so the
lifetime rule is familiar rather than new: **a slice must not outlive its parent.**

Flipping is `flip_horizontal` and `flip_vertical` as booleans. Negative `size`
components meaning flip was rejected as rule 6 in its purest form — a field whose
*sign* changes the operation — with a silent interaction with anchors on top.

## `measure_text` is pure

`measure_text(desc: TextDesc) -> TextMetrics` takes **no target**. The initial
recommendation was that it must, so it could not silently disagree with what is
drawn; the argument was wrong, and it was wrong in a way worth recording, because
it proves too much: `smooth` changes edge sampling for every primitive without
changing a rect's *size*, and it does not change a glyph's advance either.
Advances and bounding boxes come from the font's metrics and the requested size,
and ADR-0005 already pins the font by name, version and content hash precisely so
those are exact. `measure_text` reports logical units, so the logical canvas does
not enter either.

The consequence is a real gain: `measure_text` is a **pure function**, so it is
testable under **core** conformance, headless, rather than needing full
conformance with a backend. The map's completion test is measured against core.

`measure_text` and `fill_text` take the **same descriptor type**, so what is
measured is literally what is drawn.

`TextMetrics { advance: f32, bounds: Rect }` — two quantities, not one, because
returning a bare rect loses the advance width, which is the thing `measure` exists
for. `Font` is named here on the same carve-out as `Image`, with
`$.graphics.default_font` reaching ADR-0005's name-version-and-hash-identified
font.

## Case conventions, stated rather than implied

ADR-0007's rules fixed `snake_case` for functions. The rest was convention by
example, and is now explicit: **types are `PascalCase`** (`SpriteDesc`,
`TextMetrics`), and **enum members are `snake_case`** — they are values, and rule
2 says bare noun for values. `SCREAMING_CASE` is a C macro habit with no referent
in a language that has no macros.

`Style { crisp, smooth }` and `Blend { alpha, additive, multiply }` live in
`$.graphics`, since rule 1's one-module-per-area covers types as well as
functions.

## The checkerboard is a value

ADR-0007 kept the `nil`-pixel-array checkerboard for drawing and ruled it should be
a mandated *named* debug pattern. It is named here as
**`$.graphics.checkerboard`, an `Image`** — not an error-only behaviour and not a
tenth drawing call.

The failure it serves is silent: a sprite whose image failed to load draws
nothing, and *nothing* is also what a correctly-drawn transparent sprite, an
off-screen sprite and a zero-alpha sprite draw. Four causes, one observation. A
high-contrast, non-natural pattern collapses that to one, at the moment it
happens, with no tooling — #18's error-as-pause posture applied to a fault too
cheap to pause for.

Making it a **value** rather than a behaviour makes the affordance reachable: "art
missing" and "art not made yet" become one code path, a user can test their
loading path by substituting it deliberately, and the placeholder they would
otherwise hand-roll is conformance-specified. It costs one root name and no
functions.

ADR-0007's asymmetry test is why this exists for drawing and has no audio
equivalent: **a wrong image is visible; wrong silence is not.**

## What this ADR deliberately does not do

**`fill_sprite` is not mandated here**, though the analysis is recorded because
the name is now reserved coherently by the `fill_` definition. Defining a sprite's
"area" as its alpha silhouette makes `fill_sprite` cheap and exact — `rgb =
color.rgb`, `alpha = sprite_alpha * color.a`, no threshold, one shader constant,
byte-exact on a `crisp` target — and it buys the **hit flash**, which is in
approximately every action game and is currently unspellable in the facade without
a shader. Rule 6 prefers it to the alternative spelling, a `color_mode: multiply |
replace` field, which is an optional argument that changes meaning.

It is excluded on **scope**: this is a naming ADR, and mandating a new primitive
here expands the conformance surface under cover of spelling things. Criterion 4
makes the asymmetry decisive — adding it later is a pure addition that breaks
nothing, while shipping it wrongly specified is frozen forever. It has its own
ticket.

**`stroke_sprite` is renderer-tier and this is recorded so it is not
re-litigated.** Outlining an alpha silhouette needs a threshold to decide where
the edge is (what is the boundary at alpha 0.5?), then a distance field to offset
it — an O(thickness²) kernel or a jump-flood pass — and `thickness` becomes
ambiguous between logical and texel space once the sprite is scaled. That is the
policy class ADR-0004 deleted from the facade. RTS-style unit outlines are a
documented drop-down case.

**`circle` is not a name.** A circle is an ellipse with equal extents, and a
second name for it would be an alias, which #5 forbids — one spelling per entity.
The set already names shapes by their general form and reaches the special case by
equal fields: there is no `square` beside `fill_rect`, so there is no `circle`
beside `fill_ellipse`. #4's argument that a model reaches for `circle` is real but
defanged by criterion 5: a missing `$.graphics.circle` is a compile error at a
named source location, and #22's oracle answers it. The failure #4 measures that
actually hurts is a wrong guess that *compiles*.

## Consequences

- **#24 companion count delta: one module, 13 functions, 2 values, 16 types**
  under `$.graphics` — against audio's one module, ten functions, five types.
  Types are where it grew, and the cause is traceable: full `fill_`/`stroke_`
  symmetry costs three functions and three descriptor types, and rule 5 forces a
  descriptor per call rather than positional arguments. The count exists to make
  exactly this visible, so it is stated rather than buried.
- **A second delta is still owed** for `$.input`, whose surface is its own ticket.
- **ADR-0004's mandated set is widened**: ellipse and triangle gain stroke
  variants, on the inset-shape definition above.
- **A stroked ellipse's band is not constant width**, and the spec says so.
- **`measure_text` is core-conformance testable**, being pure.
- **Slices must not outlive their parent image**, which is #15's view lifetime
  rule reaching the facade.
- **Sprites rotate per call; shapes do not** — an ADR-0005 asymmetry now stated
  deliberately rather than inherited by accident.
