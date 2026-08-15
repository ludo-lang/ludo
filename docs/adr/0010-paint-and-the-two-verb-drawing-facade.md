---
status: accepted
---

> **Amended by [ADR-0015](0015-assets-are-declared-not-loaded.md):** the checkerboard among
> the six further names is demoted to an explicitly-reached value, and renamed.
>
> **Amended by [ADR-0022](0022-the-spec-promises-only-what-is-derivable.md):** the
> `stretch`-paint blit restatement stands and gains an implementation condition, but
> **`fill_sprite`'s byte-exactness with a colour paint — the hit flash — is withdrawn**.
>
> **Amended by [ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md):**
> §3 makes the logical canvas a coordinate mapping, so the blit clause's *1:1 device extent*
> precondition self-disables at any fit scale other than 1. The clause's fate is
> [#69](https://github.com/adamico/ludo/issues/69), not ADR-0030.
>
> **Amended by [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md):**
> the sixth further name is **`debug_image`**, not `checkerboard`.
>
> **Amended by [ADR-0034](0034-the-byte-exact-blit-is-withdrawn-and-the-texel-rule-replaces-it.md):**
> the `stretch`-paint blit restatement is **deleted in full**, so the ADR-0022 and ADR-0030
> stamps above are discharged — the clause has no fate left to decide. Its citation of
> ADR-0005's texel-centre and tie-break convention goes with it: that convention was never
> written. `fill_sprite` and `stroke_sprite` are otherwise untouched.
>
> **Amended by [ADR-0043](0043-a-string-is-a-literal-and-constructed-text-is-bytes.md):**
> `TextDesc`'s `text` field is `[]u8`, not `string` — the same descriptor and the same
> change as ADR-0009's. ADR-0043 §11 declares this amendment and no stamp was applied;
> added retroactively while transcribing spec chapter 6, per #72.

> **Absorbed by [spec ch6 §4.3-§4.7](../spec/06-stdlib.md):** the chapter is normative; this ADR
> keeps the argument.

# `Paint` and the two-verb drawing facade: `fill_` and `stroke_`, and `draw_` deleted

ADR-0009 fixed three verbs — `fill_` (the shape's area in one solid colour),
`stroke_` (a path with thickness), `draw_` (a thing carrying its own appearance) —
under a rule choosing between them: **a shape gets `fill_`/`stroke_` only if it
admits both; otherwise `draw_`.** That rule was sound given its inputs. This ADR
changes one of the inputs — `fill_` no longer takes a `Color`, it takes a
**`Paint`** — and the third verb dissolves: `draw_sprite` becomes
`fill_sprite`/`stroke_sprite`, `draw_line` becomes `stroke_line`, and the
adjudication rule has nothing left to adjudicate.

Everything else ADR-0009 fixed is settled input and is not reopened: `$.graphics`
as the module, verb-first, `snake_case`, no abbreviations, a descriptor struct per
call, no overloads, `Vec2` over scalars, the inscribed-box ellipse, per-call
`blend`, the target at the frame entry, `slice_image` over a `source: Rect`,
`measure_text`'s purity, and the case conventions. Every ADR-0005 semantic is
likewise inherited unchanged except the byte-exact blit clause, which is restated
below in paint vocabulary and **strengthened**.

## What this ADR is not

The ticket that opened this (#45) proposed reaching two verbs by collapsing
`draw_sprite` into `fill_rect` with an image paint plus the anchors, angle and
flips `SpriteDesc` already carries. **That collapse is rejected outright**, and the
rejection is recorded so it is not re-proposed:

- It forces `RectDesc` to carry `anchor`, `rotation_anchor`, `angle`,
  `flip_horizontal` and `flip_vertical` as fields that are dead for every other
  paint — rule 6's overload wearing a struct, where an agent cannot tell from the
  declaration which fields are live.
- It deletes `draw_sprite`'s successor name in the one place the corpus is
  unanimous (`drawImage`, `DrawTexture`, `draw`), buying an internal symmetry a
  model never sees. #4's evidence is about first-try correctness.

The sprite keeps its own call and its own descriptor. It reaches the two-verb set
the same way every other shape did: **by admitting both a fill and a stroke.**

## `Paint`, and why the UV question is a field rather than a leak

```
Paint     = Color | Texture
Texture   { image: Image, mapping: Mapping, offset: Vec2 }
Mapping   { stretch, tile }
```

A sum of two variants, not three or four. #11 is static-only with no `dyn`, so
heterogeneity is a sum type; there is no interface and no `distinct` wrapper.

**"Pattern" is not a third variant.** A pattern is not a different kind of pixels —
it is the same pixels under a different mapping, so it is `Mapping.tile`, and the
type says so. Collapsing it into `Mapping` is what keeps `Paint` at two variants
while covering all three things a fill can be filled with.

**`Mapping` exists because ADR-0004 admits a primitive on a *statable observable*.**
An image paint needs UVs, and an unstated UV convention fails that test. The
alternative to stating it is a convention living in prose, which is the renderer
tier's vocabulary leaking upward — the thing ADR-0005 refused for blend factors.
Making the mapping a **field of the paint** is what keeps it out of the renderer
tier: the facade never exposes a texture matrix, a sampler, an address mode or a
UV set. It exposes two named behaviours.

Both are stated against the shape's **bounding box**, Y-down, in logical units:

- **`stretch`** — the image's full extent maps onto the bounding box, corner to
  corner. A `fill_rect` with a stretch paint therefore draws exactly what
  ADR-0009's `draw_sprite` drew at that position and size.
- **`tile`** — the image is placed at native size with its top-left at the
  bounding box's top-left, repeated in both axes, clipped to the shape.

`offset: Vec2` (default zero) shifts the mapping within the bounding box. It is
what makes a scrolling background spellable, and it is the whole of the escape
hatch: there is no `scale` field, because scaling a tile is `slice_image` on a
differently-sized source, or it is `stretch`.

**The tile origin is the shape's, not the target's.** Target-origin tiling makes
adjacent shapes share one continuous pattern, which is occasionally wanted and
always surprising: moving a rect would slide its own texture underneath it. Shape-
local origin keeps a shape's appearance independent of where it sits, and `offset`
buys the continuous case back for anyone who wants it.

## `Paint` replaces `Color` in every descriptor

ADR-0009 gave every descriptor `color: Color`. That field becomes `paint: Paint`
on **all nine calls** — fills, strokes and text alike.

Restricting `Paint` to `fill_` only, or to shapes only, was considered and
rejected. Any restriction is a rule an agent must learn and a compile error where
the obvious thing was written, and none of them is free to state. A stroke's band
is a shape; `Mapping` is defined against a bounding box; so `stroke_rect` with a
texture paint needs no machinery that `fill_rect` did not already need. Textured
glyphs fall out of `fill_text` for nothing.

**The cost is stated out loud:** `Color` no longer appears in any descriptor, so
every `color = $.rgb8(...)` call site in every prior example becomes
`paint = $.rgb8(...)`. That is a tax on the common case — the same objection that
sank the `fill_rect` collapse — and it is paid here because this time it buys a
**capability** (textured and tiled fills, and the sprite pair below) rather than an
internal symmetry. `Color` survives as a `Paint` variant and as `$.rgb8`/`$.rgbf`'s
return type, so no name is deleted and nothing at the root moves.

An implicit `Color`-to-`Paint` conversion was considered and rejected: implicit
conversion is a mechanism this language does not otherwise have, and adding one
for call-site brevity is a language feature bought by a facade.

## The sprite gains the pair

```
$.graphics.fill_sprite(target: Target, desc: SpriteDesc)
$.graphics.stroke_sprite(target: Target, desc: SpriteStrokeDesc)
```

**`fill_sprite` fills the region where `image`'s alpha is non-zero, with `paint`,
modulating by that alpha.** One sentence, three pictures:

- `paint` left at its default — the descriptor's **own image**, `stretch`, zero
  offset — draws the ordinary sprite, byte-for-byte identical to ADR-0009's
  `draw_sprite`.
- `paint = $.rgb8(255, 255, 255)` is the **hit flash**: `rgb = color.rgb`,
  `alpha = sprite_alpha * color.a`, no threshold, one shader constant, byte-exact
  on a `crisp` target. ADR-0009 recorded this as cheap and exact and parked it on
  scope; the scope objection was ADR-0009's own remit as a naming ADR and does not
  transfer here.
- `paint` naming a **different** image is a masked fill — a sprite-shaped window
  onto another texture, which is how patterned and tinted power-up states get made.

That third case is **legal and defined, not forbidden**. Forbidding it would be a
per-call-type restriction on `Paint`, which is the carve-out the previous section
refused; and the rule that makes the ordinary case work — silhouette from `image`,
colour from `paint` — makes it work for free.

`SpriteDesc` therefore keeps **both** `image` and `paint`. `image` is the
silhouette; `paint` is what fills it. Dropping `image` and letting `paint` be the
only source was considered and is self-refuting: a colour paint would then have no
silhouette to fill.

`fill_sprite` also settles the alternative spelling ADR-0009 named: a
`color_mode: multiply | replace` field is rule 6 in the open — an optional argument
that changes meaning — and the paint model reaches the same two pictures as two
values of one field whose meaning never changes.

### `stroke_sprite` is mandated, and its two ambiguities are answered

ADR-0009 ruled `stroke_sprite` renderer-tier and asked that it not be
re-litigated. It is re-litigated here and the ruling is reversed — but only by
**answering** what ADR-0009 raised, not by inheriting symmetry from the naming:

- **The edge threshold is fixed at `alpha >= 0.5`**, not exposed as a field. A
  per-call threshold is a knob whose correct value depends on how the artist
  authored the alpha, which is not a decision a call site can make; exposing it
  would put an authoring detail in the hot path and make two sprites disagree for
  reasons invisible at the call site.
- **`thickness` is in logical units**, like every other stroke. This is the
  decisive one: an outlined sprite beside an outlined box at the same `thickness`
  must match, or the word is broken. Texel-space thickness is cheaper and was
  rejected for exactly that reason — a scaled sprite's outline would change width
  while a rect's did not.

**The cost is stated out loud, in ADR-0009's own style:** this is a distance-field
offset of a thresholded silhouette. It is the most expensive primitive in the
facade — an O(thickness²) kernel or a jump-flood pass — and it is the one call
whose cost is not proportional to the area it covers. Artwork authored with
premultiplied or feathered edges will outline slightly fat, because a threshold on
a soft edge is a choice and 0.5 is the one this spec makes. RTS-style unit
outlines, which ADR-0009 documented as a drop-down case, are now in the facade.

The reason this is admissible where ADR-0009 said it was not: ADR-0004 deletes
**policy classes** from the facade — joins, caps, miter limits, blend factors —
where the spec would have to *select* among visibly different correct answers with
no principle. A fixed threshold and logical-unit thickness are not a selection
menu; they are two sentences, both observable, both testable under coverage-not-
colour tolerance. The expense is real and is a performance fact, not a semantic
one.

## `stroke_` generalises to open paths, and `draw_line` becomes `stroke_line`

ADR-0009 defined a stroke as **the shape minus its inset shape**, which only parses
for closed shapes, and put `draw_line` in the set on the grounds that a line has no
interior so the pair is meaningless for it.

With the sprite taking the pair, `line` is the only call keeping the third verb
alive — and a verb that exists for exactly one call is worse than the rename. So
the definition widens:

> **A stroke is a path with thickness.** For a closed shape this is the shape grown
> by `thickness/2` minus the shape shrunk by `thickness/2` — ADR-0009's sentence,
> demoted from *definition* to *consequence*. For an open path the ends are
> **round caps**.

Round caps are not a new policy selection: ADR-0009 already stated that a thick
line is a capsule with round ends, when it explained why `draw_line` does not
collapse into `fill_rect`. This ADR names that as the open-path half of the stroke
definition instead of as a property of one call.

**`fill_line` still does not exist**, and that is not an exception to anything: the
verb rule is gone, and a line simply has no area. The set is not "every shape has
both"; it is "every call is a fill or a stroke."

**No polyline is admitted here.** The moment a path has an interior vertex it needs
a **join** policy — miter, bevel, round are visibly different and a miter grows
without bound — which is ADR-0004's deleted class. Caps for a two-point path are
statable; joins are a selection menu. A polyline is not fog for this ADR to
graduate; it is out of scope for the facade on ADR-0004's own rule.

## The mandated set

Ten drawing calls — one more than ADR-0004 and ADR-0009 — and the same six further
names (`measure_text`, `slice_image`, `create_pixel_image`, `get_pixels`,
`default_font`, `checkerboard`), unchanged.

```
$.graphics.fill_rect(target: Target, desc: RectDesc)
$.graphics.stroke_rect(target: Target, desc: RectStrokeDesc)
$.graphics.fill_ellipse(target: Target, desc: EllipseDesc)
$.graphics.stroke_ellipse(target: Target, desc: EllipseStrokeDesc)
$.graphics.fill_triangle(target: Target, desc: TriangleDesc)
$.graphics.stroke_triangle(target: Target, desc: TriangleStrokeDesc)
$.graphics.stroke_line(target: Target, desc: LineDesc)
$.graphics.fill_sprite(target: Target, desc: SpriteDesc)
$.graphics.stroke_sprite(target: Target, desc: SpriteStrokeDesc)
$.graphics.fill_text(target: Target, desc: TextDesc)
```

The delta against ADR-0009 is `−draw_line`, `−draw_sprite`, `+stroke_line`,
`+fill_sprite`, `+stroke_sprite` — one net addition, since `stroke_sprite` has no
predecessor. `fill_text` is untouched.

Descriptors, with `paint: Paint` and `blend: Blend` (default `alpha`) on every one:

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
SpriteStrokeDesc    { image: Image, position: Vec2, size: Vec2,
                      anchor: Vec2, rotation_anchor: Vec2, angle: Radians,
                      flip_horizontal: bool, flip_vertical: bool,
                      thickness: f32 }
TextDesc            { text: []u8, font: Font, font_size: f32,
                      position: Vec2, anchor: Vec2 }
```

`SpriteStrokeDesc` is a **flat repeat** of `SpriteDesc` plus `thickness`, like
every other stroke pair. Nesting (`{ sprite: SpriteDesc, thickness }`) was
rejected on ADR-0009's own reason — a flat literal is what an agent writes
correctly on the first try, and nesting makes the call site ask which field lives
at which depth. A single `SpriteDesc` with `thickness` defaulting to zero meaning
"no stroke" is rule 6 outright: a field whose *value* selects the operation.

**Recorded honestly:** at nine repeated fields this is the first pair where the
flat rule strains, and it is the place to look first if a future ADR revisits
descriptor composition. It is not revisited here, because inventing a composition
mechanism inside a facade ADR is the mistake ADR-0009 avoided when it declined to
mandate `fill_sprite`.

## The byte-exact blit, restated and strengthened

> **Deleted by [ADR-0034](0034-the-byte-exact-blit-is-withdrawn-and-the-texel-rule-replaces-it.md).**

This section restated ADR-0005's byte-exact blit against the paint rather than the
sprite — *a `stretch` texture paint whose source extent equals its destination
extent in device pixels, at integer logical coordinates, unrotated, under an
identity or integer-translation transform, on a `crisp` target, is a byte-for-byte
blit* — and called the restatement stronger and free.

The restatement was sound as a *generalisation*; what it generalised was not true.
ADR-0034 §1 withdraws the clause on five grounds, of which the one this section
should have caught is that **it states no blend condition**, while ADR-0009 gives
every descriptor `blend: Blend` defaulting to `alpha` — so the sentence promises a
byte-for-byte blit for a semi-transparent texture over a non-black background.
ADR-0022 §4 had already withdrawn this section's colour-paint claim (the hit flash)
for riding the rasterised path, and missed that alpha blending disqualifies the
untinted case identically.

The section's closing citation of ADR-0005's *texel-centre and tie-break convention*
is deleted as well: ADR-0034 §4 found that convention was never written in the
first place — this section preserved a rule that did not exist.

## How it fares on the three lenses

- **Simplicity.** Better than ADR-0009 on the verb set — two verbs, no adjudication
  rule, no exception list to memorise — and worse on the fill argument, because
  every call site now writes `paint =` where it wrote `color =`, and a beginner
  meets a sum type at the first `fill_rect`. The mitigation is that `$.rgb8(...)`
  is still what you pass, so the sum type is invisible until you want the second
  variant. Named, not resolved: this is the trade.
- **Robustness.** `Mapping` turns an unstated UV convention into a field, which is
  ADR-0004's admission test satisfied rather than deferred. `stroke_sprite`'s
  fixed threshold and logical thickness remove the two places implementations would
  have diverged silently.
- **Agent-friendliness.** Net positive, with one loss stated: `draw_sprite` is the
  corpus-guessable name and it is deleted, so a model reaching for it gets a
  compile error at a named source location — criterion 5's case, and #22's oracle
  answers it. Against that, the surface an agent must learn has **two** verbs with
  no shape-by-shape rule, and the hit flash — approximately universal in action
  games — becomes spellable without a shader.

## Consequences

- **#24 companion count delta**: `$.graphics` goes to **one module, 14 functions,
  2 values, 20 types** — from ADR-0009's 13 functions and 16 types. Functions +1
  (`stroke_sprite`); types +4 (`Paint`, `Texture`, `Mapping`, `SpriteStrokeDesc`).
  This is the first decision on the map to **delete a verb while raising the type
  count**, which is the budget working as intended: the concept count went down,
  the declared surface went up, and both are visible.
- **ADR-0009's three-verb set is amended to two**, and its adjudication rule is
  deleted rather than narrowed.
- **ADR-0009's stroke definition is demoted** from definition to consequence for
  closed shapes; round caps are the open-path half.
- **ADR-0005's byte-exact blit is restated in paint vocabulary and widened** to any
  1:1 `stretch` paint.
- **`stroke_sprite` is reversed out of renderer tier** into the mandated facade,
  with a fixed 0.5 alpha threshold and logical-unit thickness, and its cost stated.
- **`fill_sprite` is mandated**, closing the item ADR-0009 parked on scope, and
  with it the hit flash.
- **A polyline stays out of the facade** on the join policy, stated so it is not
  re-proposed as a natural extension of `stroke_line`.
- **`Color` is no longer a descriptor field type anywhere**, while remaining a
  `Paint` variant and the root constructors' return type.
