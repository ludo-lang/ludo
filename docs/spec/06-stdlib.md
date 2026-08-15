# Chapter 6 — The standard library

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where this chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under chapter 7's envelope (ADR-0018).

**0.3** This chapter owns **every name under `$.`** ([#99](https://github.com/ludo-lang/ludo/issues/99)):
the five facade modules `$.graphics`, `$.audio`, `$.input`, `$.video` and
`$.storage`; the blessed math set and the shipped quantity types; the naming
rules and case conventions the whole reserved root obeys; the
forwarding-function guarantee; the shader-ludo dialect and the `extern shader`
declaration; and the platform-layer shapes that reach back into the mandated
contract.

**0.4** This chapter does not own the **spelling** of any form named here
(chapter 1, with [`grammar.ebnf`](grammar.ebnf) the authority on syntax); the
**typing** of the operators §3.6 admits (chapter 2 §16.1); `#align(n)` **as a
mechanism** (chapter 3 §8.3) — §3.8 states only which blessed types carry which
alignment; **`$.` as a module root**, its two privileges and its resolution path
(chapter 4 §9); the **entry file, the frame entry, `persist`, reload, the fixed
canvas and the fit** (chapter 5); the **diagnostic envelope** (chapter 7); or
**conformance rungs, claimant obligations and the #24 count** (chapter 8). Where
a clause reaches one of them, it names the routing.

**0.5** Every clause cites its source. A clause with no citation is a defect.

**0.6 Two clauses of ADR-0016 are transcribed elsewhere and cited, not restated
here** ([#99](https://github.com/ludo-lang/ludo/issues/99)). ADR-0016 §3's
operator rule is a typing rule and lives at **chapter 2 §16.1**; §6's SIMD and
float-evaluation halves are conformance obligations and live in **chapter 8**,
with the language-level float rule at **chapter 2 §2.5**. §3.6 and §3.9 below
name them and point.

**0.7 The reserved root is `$.`, and this chapter is its whole contents.**
Chapter 4 §9 establishes that `$.` is an ordinary module set privileged in
exactly two ways — the root name is reserved, and it is in scope with no `use`.
Nothing in this chapter re-privileges it. (ADR-0014 §4; ch4 §9.1–§9.2.)

---

## 1. How everything under `$.` is named

**1.1** The following six rules bind **every** name under `$.`, in this chapter
and in any future addition to the root. (ADR-0007, *the naming rules*.)

**1.1.1** **One module per facade area** under the reserved `$.` root.
(ADR-0007 rule 1.)

**1.1.2** **Verb-first for actions, bare noun for values**: `play`, `stop`,
`push`, `cursor`, `sample_rate`. (ADR-0007 rule 2.)

**1.1.3** **`snake_case`** for functions. (ADR-0007 rule 3.)

**1.1.4** **No abbreviations at all** — `frequency`, `amplitude`, `resonance`,
never `freq`, `amp`, `res`. (ADR-0007 rule 4; #4.)

**1.1.5** **A descriptor struct is the single argument** for any call with more
than two parameters, written as a named-field literal with field defaults
(ch3 §5.6). (ADR-0007 rule 5.)

**1.1.6** **No overloads.** An operation with a variant gets a second name,
never an optional argument that changes meaning. (ADR-0007 rule 6; #5
criterion 3.)

**1.2** Rule §1.1.6 is what makes `stop` two calls: `$.audio.stop(v)` and
`$.audio.stop_all()`. (ADR-0007, *the naming rules*.)

**1.3** **Types are `PascalCase`** (`SpriteDesc`, `TextMetrics`). **Enum members
are `snake_case`** — they are values, so rule §1.1.2's bare-noun half governs
them. `SCREAMING_CASE` does not appear anywhere under `$.`. (ADR-0009, *case
conventions*.)

**1.4 Root names are constructors for blessed types; modules are facade areas.**
`$.rgb8`, `$.rgbf` and `$.vec3` sit at the root and are not violations of
§1.1.1, because a `Color` or a `Vec3` is used by code that never draws.
(ADR-0009, *the module is `$.graphics`*; ADR-0004; #15.)

**1.5** A facade area is named by an **area noun**, never a verb: the module is
`$.graphics`, not `$.draw`, because a verb module stutters against §1.1.2's
verb-first calls. (ADR-0009, *the module is `$.graphics`*.)

**1.6** Enum types belong to the module of their area rather than to the root:
`Style` and `Blend` live in `$.graphics`, because §1.1.1's one-module-per-area
covers types as well as functions. (ADR-0009, *case conventions*.)

**1.7** UFCS (ch2) applies to every `$.` free function whose first parameter is
the value being operated on, so `target.fill_rect({...})` and
`save1.write(bytes)` are the same functions as their qualified spellings. There
is no second name and no overload. The spec teaches the qualified spelling.
(ADR-0009, *the target arrives at the frame entry*; ADR-0026 §4.)

---

## 2. The forwarding-function guarantee

**2.1** The mandated facades are verbose by construction (§1.1.4, §1.1.5,
§1.1.6), and **the user is expected to wrap them**. That intent is rationale and
binds no implementation. (ADR-0012 §1.)

**2.2** A **forwarding function** is a function whose body is a single call
expression, with no captures. This is a structural predicate, decidable by
inspection. (ADR-0012 §2.)

**2.3** A conforming implementation **MUST NOT emit a call to a forwarding
function**. (ADR-0012 §2.)

**2.4** §2.3 is stated structurally — *no call to a forwarding function appears
in the emitted program* — and is checked by inspecting the emitted program, never
by timing. (ADR-0012 §3.)

**2.5** §2.3 is a property of **functions**, not of facades, and applies
identically to `$.graphics`, `$.audio`, `$.input`, `$.video` and `$.storage`.
(ADR-0012 §8.)

**2.6** §2.3 is **the spec's one constraint on how an implementation compiles**.
No other clause of this specification prescribes a compilation strategy.
(ADR-0012 §2; restored by ADR-0034 §5 after ADR-0022 §4's withdrawal.)

**2.7** There is **no inline marker**. (ADR-0012 §2.)

**2.8** A wrapper is **never blessed into the reserved root**. An implementation
MUST NOT add a second spelling for an entity `$.` already names. Where a wrapper
turns out to be universal, the repair is to change the facade call and record
the delta against #24, not to add an alias. (ADR-0012 §4; #5.)

**2.9** This project publishes **no wrapper library**, not even blessed by
reference, and **forbids no one else's**. A third-party wrapper package is an
opt-in dependency and is not shared vocabulary; #24's companion count is
unaffected by wrappers of either kind, because the count measures the reserved
root. (ADR-0012 §5.)

---

## 3. The blessed math set

**3.1** The blessed math types are **concrete `f32` types**. There is no element
type parameter: no `Vec2[T]`, no `Vec2[Radians]`. (ADR-0016 §1.)

**3.2** The set ships at **2, 3 and 4 components; matrices square; float and
integer vectors**:

```
$.vec2  $.vec3  $.vec4        f32 components
$.mat2  $.mat3  $.mat4        square, f32
                              integer vectors at the same three widths
```

(ADR-0016 §2.)

**3.3** **`mat2` ships** even though `mat3` subsumes it, because *2/3/4 across
the board* is one rule with no exception. (ADR-0016 §2.)

**3.4** **Non-square matrices and quaternions are not in the set.** (ADR-0016
§2.)

**3.5** Each math type has a **positional constructor** — `$.vec3(1, 2, 0)` —
because struct literals are named-field only (ch3 §5.6). (ADR-0016 §7; #15.)

**3.6 The arithmetic operators on the blessed math types are chapter 2's rule.**
`a + b`, `a - b`, `v * 2.0` and `m * v` are admitted on the blessed types and on
no user type; `+` and `-` are elementwise on matching types; `*` is
vector-scalar, matrix-scalar, matrix-vector and matrix-matrix; **there is no `/`
between two vectors and no elementwise `*` between two vectors**, both of which
are spelled as named functions. The rule is transcribed at **chapter 2 §16.1**
and is not restated here. (ADR-0016 §3; #99.)

**3.7 The shipped quantity types are `Radians`, `Seconds` and `SampleFrames`.**
`Color` is a `distinct` type from #28 and is unchanged. Each shipped quantity
appears in a `$.` return type; a quantity the stdlib would define and never use
does not ship. **`PxPerSec`, `PxPerTick` and `Ticks` are not in the set.**
(ADR-0016 §4.)

**3.8 Alignment is specified on the blessed types**, by the type-level
`#align(n)` mechanism chapter 3 §8.3 owns. `vec4` is **16-byte aligned by
declaration**, not by an implementation's good luck. An implementation MUST NOT
give a blessed math type an alignment other than the one its declaration states.
(ADR-0016 §6 clause 1; #25.)

**3.9 The remaining two halves of ADR-0016 §6 are chapter 8's.** A conforming
implementation is **not required to lower operations on the blessed math types
to SIMD instructions**; and **float evaluation is exactly as written** — no
reassociation, no implicit FMA contraction, no fast-math, in any mode. The first
is a conformance non-obligation and the second a determinism obligation; both are
transcribed in chapter 8, with the language-level statement of the float rule at
chapter 2 §2.5. The batch shape they rely on is `[]T` views (ch3 §6.1) and needs
no clause here. (ADR-0016 §6 clauses 2–3; #99.)

**3.10** The standard library ships **no batch functions** — no
`$.math.add_all`-shaped surface. The batch shape is ordinary functions over
`[]T`. (ADR-0016 §7.)

**3.11** The `distinct numeric` unit discipline **stops at scalars**: a `vec2`
carries no unit, so a velocity is unprotected. This is a stated forfeit, not an
omission. (ADR-0016 §1.)

---

## 4. `$.graphics`

### 4.1 Coordinates, colour and alpha

**4.1.1** The coordinate system is **top-left origin, Y-down**, and is **not
configurable**. There is no origin mode. (ADR-0005, *coordinate system*.)

**4.1.2** Centre-origin is reached by the target's transform (§4.2.3) —
mathematically `translate(w/2, h/2)`. An implementation MUST NOT provide a global
origin mode of any kind. (ADR-0005, *coordinate system*.)

**4.1.3** A transform does not buy Y-up: a negative Y scale flips a sprite's
texture coordinates with the quad. Handedness is frozen by §4.1.1. (ADR-0005,
*coordinate system*.)

**4.1.4** Colour is **straight (non-premultiplied) alpha, in sRGB**. (ADR-0005,
*colour and alpha*.)

**4.1.5** A web backend MUST configure an **opaque presentation surface**, so the
page never composites the program's alpha. (ADR-0005, *colour and alpha*.)

**4.1.6** A web backend MUST **decode images itself and upload raw bytes**, and
MUST NOT route image upload through the browser's image pipeline. (ADR-0005,
*colour and alpha*.)

**4.1.7** **`copyExternalImageToTexture` is forbidden.** It is the only WebGPU
upload path whose output bytes are not a function of its input bytes alone.
(ADR-0022 §5.)

### 4.2 The draw target

**4.2.1** `Target` is an **opaque value, passed and never ambient**. The type is
named `Target` — not `Canvas`, not `Screen`. (ADR-0005, *the draw target*;
ADR-0009, *the target arrives at the frame entry*.)

**4.2.2** A `Target` carries a **`style:` token** with the closed enum
`Style { crisp, smooth }`. `crisp` is **no anti-aliasing, nearest sampling**;
`smooth` is **anti-aliasing, linear sampling**. `crisp` is the default. It is
**one token, not two independent fields**. (ADR-0005, *the draw target*.)

**4.2.3** `crisp` means nearest sampling at **every scale**, not only integer
ones. Its meaning MUST NOT depend on a window size the program cannot observe.
(ADR-0031, amending ADR-0005.)

**4.2.4** A `Target` carries a **2D transform** — offset, scale, rotation —
applied to every call made against it. (ADR-0005, *the draw target*.)

**4.2.5** **Offscreen targets are a reserved shape.** The type is specified so
that offscreen targets can be added without a signature change; this
specification mandates no constructor for one. (ADR-0005, *the draw target*.)

**4.2.6 A program does not construct or fetch the screen target.** The runner
delivers it at the frame entry, and there is no `$.graphics.screen()`. The entry
parameter, its spelling and its per-frame freshness are **chapter 5's**
(ADR-0013). (ADR-0009, *the target arrives at the frame entry*; ADR-0005 as
amended by ADR-0013.)

**4.2.7 The logical canvas is not a field of the target and is not
caller-declared.** ADR-0005's caller-declared logical canvas is **overturned**:
the canvas is a language constant, and the fit is a coordinate mapping rather
than an intermediate framebuffer. Both are **chapter 5's** (ADR-0030, ADR-0032).
This chapter records only that no `$.graphics` call takes or returns a canvas
size. (ADR-0005 as amended by ADR-0030 and ADR-0032.)

**4.2.8** `$.graphics` additionally carries the two coordinate-mapping calls the
fit and the pointer rule require — mapping window coordinates to canvas
coordinates, and inverting a `Target`'s transform. **`to_world` lives in
`$.graphics`, not in `$.input`**, because inverting a `Target`'s transform is a
`Target` operation and giving it two owners would give one computation two
spellings. Their semantics are **chapter 5's** (ADR-0013 §8, ADR-0030 §4).
(ADR-0011, *the mandated set*; ADR-0033 §9.)

### 4.3 `Paint`

**4.3.1**

```
Paint     = Color | Texture
Texture   { image: Image, mapping: Mapping, offset: Vec2 }
Mapping   { stretch, tile }
```

`Paint` is a **sum of exactly two variants**. (ADR-0010, *`Paint`*.)

**4.3.2** A **pattern is not a third variant**: it is the same pixels under a
different mapping, and is spelled `Mapping.tile`. (ADR-0010, *`Paint`*.)

**4.3.3** Both mappings are stated against the shape's **bounding box**, Y-down,
in logical units:

- **`stretch`** — the image's full extent maps onto the bounding box, corner to
  corner.
- **`tile`** — the image is placed at native size with its top-left at the
  bounding box's top-left, repeated in both axes, clipped to the shape.

(ADR-0010, *`Paint`*.)

**4.3.4** **The tile origin is the shape's, not the target's.** (ADR-0010,
*`Paint`*.)

**4.3.5** `offset: Vec2` (default zero) shifts the mapping within the bounding
box. There is **no `scale` field** on `Texture`. (ADR-0010, *`Paint`*.)

**4.3.6** The facade exposes **no texture matrix, no sampler object, no address
mode and no UV set**. The mapping is a field of the paint and nothing more.
(ADR-0010, *`Paint`*.)

**4.3.7** **Every descriptor carries `paint: Paint`** — fills, strokes and text
alike. `Color` is **not a descriptor field type anywhere**, while remaining a
`Paint` variant and the return type of `$.rgb8` and `$.rgbf`. (ADR-0010,
*`Paint` replaces `Color`*.)

**4.3.8** There is **no implicit `Color`-to-`Paint` conversion**. (ADR-0010,
*`Paint` replaces `Color`*.)

### 4.4 The mandated call set

**4.4.1** Ten drawing calls:

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

(ADR-0010, *the mandated set*.)

**4.4.2** Six further names:

```
$.graphics.measure_text(desc: TextDesc) -> TextMetrics
$.graphics.slice_image(image: Image, region: Rect) -> Image
$.graphics.create_pixel_image(size: Vec2) -> Image
$.graphics.get_pixels(image: !Image) -> []u32

$.graphics.default_font -> Font
$.graphics.debug_image  -> Image
```

(ADR-0009, *the mandated set*; ADR-0010, *the mandated set*; ADR-0047 §5 for
`get_pixels`'s mark; ADR-0033 for `debug_image`'s name.)

**4.4.3 There are exactly two verbs, `fill_` and `stroke_`.** `draw_` does not
exist, and there is no rule adjudicating between verbs — every call is a fill or
a stroke. (ADR-0010, *`stroke_` generalises*; amending ADR-0009's three verbs.)

**4.4.4** **`fill_line` does not exist**, because a line has no area. This is not
an exception to §4.4.3: the set is *every call is a fill or a stroke*, not *every
shape has both*. (ADR-0010, *`stroke_` generalises*.)

**4.4.5** **`circle` is not a name.** A circle is an ellipse with equal extents.
(ADR-0009, *what this ADR deliberately does not do*; #5.)

**4.4.6** **There is no point primitive.** A pixel is `fill_rect` with
`size = $.vec2(1, 1)`; a dot is `fill_ellipse`. (ADR-0009, *what this ADR
deliberately does not do*; ADR-0004.)

**4.4.7** **No polyline is in the facade.** A path with an interior vertex needs
a join policy, which is a deleted policy class. (ADR-0010, *`stroke_`
generalises*; ADR-0004.)

**4.4.8** **Blend is a closed enum of three modes**, `Blend { alpha, additive,
multiply }`, with no `none` and no `opaque` member. Composable blend factors are
not in the facade. (ADR-0005, *blend spelling*; ADR-0004; ADR-0034 §1 records
the closed set's consequence.)

### 4.5 Descriptors

**4.5.1** Every drawing call takes a **descriptor struct**, one type per call.
A shared descriptor whose live fields depend on the receiving call is forbidden
by §1.1.6. (ADR-0009, *descriptors*.)

**4.5.2** Descriptor fields are **math types, not scalars**: `position: Vec2`,
never `x, y`. (ADR-0009, *descriptors*.)

**4.5.3**

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

Every descriptor additionally carries **`paint: Paint`** (§4.3.7) and
**`blend: Blend`, defaulting to `alpha`**. (ADR-0010, *the mandated set*;
ADR-0043 §11 for `TextDesc.text`.)

**4.5.4** **`blend` is per-call, not on the target**, because mixing blend modes
within one frame is ordinary and correct. Neither `blend` nor `style` may be
set-and-draw ambient state. (ADR-0009, *descriptors*.)

**4.5.5** **Stroke descriptors repeat their fill descriptor's fields flat**
rather than nesting one inside the other. This holds for `SpriteStrokeDesc`'s
nine repeated fields as well. (ADR-0009, *descriptors*; ADR-0010, *the mandated
set*.)

**4.5.6** A single descriptor whose `thickness` value selects between fill and
stroke is forbidden by §1.1.6. (ADR-0010, *the mandated set*.)

**4.5.7** Two field names are fixed against collisions: **`thickness`, not
`width`** (a `width` is the x component of a `size`), and **`font_size`, not
`size`** (a `size` is a `Vec2` extent everywhere else). (ADR-0009,
*descriptors*.)

### 4.6 Shapes and strokes

**4.6.1 A stroke is a path with thickness.** For a **closed** shape it is the
shape grown by `thickness / 2` minus the shape shrunk by `thickness / 2`. For an
**open** path the ends are **round caps**. (ADR-0010, *`stroke_` generalises*;
demoting ADR-0009's shape-minus-inset-shape definition to a consequence.)

**4.6.2** Where the inner shape degenerates — a negative radius, an inverted
inset — the result is the filled shape. (ADR-0009, *stroke for every shape*.)

**4.6.3** A triangle's corner behaviour under §4.6.1 is the **miter, stated
rather than selected**, and there is **no miter limit**. (ADR-0009, *stroke for
every shape*.)

**4.6.4** **A stroked ellipse's band is not constant width.** It is thicker near
the ends of the major axis and thinner at the flat parts, and the discrepancy
grows with eccentricity. (ADR-0009, *stroke for every shape*.)

**4.6.5** Two overlapping draws composite twice. An implementation that merged
them is non-conforming. (ADR-0009, *stroke for every shape*.)

**4.6.6 An ellipse is inscribed in its bounding box.** `EllipseDesc` is
field-for-field identical to `RectDesc`, top-left anchored. An ellipse's `width`
and `height` are the **full axis lengths**, never the semi-axes. (ADR-0009, *the
ellipse is inscribed*.)

**4.6.7 Shapes are axis-aligned; sprites rotate.** No shape descriptor carries an
`angle` or a rotation pivot; `SpriteDesc` and `SpriteStrokeDesc` do. (ADR-0009,
*shapes are axis-aligned*.)

**4.6.8** `stroke_line` does not collapse into `fill_rect`: without a per-call
angle a rect is axis-aligned in logical space, and a thick line is a capsule with
round ends (§4.6.1). (ADR-0009, *shapes are axis-aligned*.)

### 4.7 Sprites

**4.7.1** A sprite carries **two normalized anchors** — a position `anchor` and a
`rotation_anchor` — independent, both defaulting to centre, both in normalized
rather than pixel units. (ADR-0005, *sprites*.)

**4.7.2** Flipping is the two booleans `flip_horizontal` and `flip_vertical`. A
negative `size` component MUST NOT mean a flip. (ADR-0009, *images*.)

**4.7.3 `fill_sprite` fills the region where `image`'s alpha is non-zero, with
`paint`, modulating by that alpha.** With `paint` left at its default — the
descriptor's own image, `stretch`, zero offset — this draws the ordinary sprite.
(ADR-0010, *the sprite gains the pair*.)

**4.7.4** A **colour** paint on `fill_sprite` yields `rgb = color.rgb` and
`alpha = sprite_alpha * color.a`, with no threshold. This case carries **no
byte-exactness guarantee**; §4.11 is the whole of what is promised. (ADR-0010,
*the sprite gains the pair*; the exactness claim withdrawn by ADR-0022 §4 and
ADR-0034 §1.)

**4.7.5** A paint naming a **different** image is a masked fill and is **legal
and defined**, not forbidden. (ADR-0010, *the sprite gains the pair*.)

**4.7.6** `SpriteDesc` keeps **both** `image` and `paint`: `image` is the
silhouette, `paint` is what fills it. (ADR-0010, *the sprite gains the pair*.)

**4.7.7** There is **no `color_mode` field** on any descriptor. (ADR-0010, *the
sprite gains the pair*; §1.1.6.)

**4.7.8 `stroke_sprite`'s edge threshold is fixed at `alpha >= 0.5`** and is not
exposed as a field. (ADR-0010, *`stroke_sprite` is mandated*.)

**4.7.9** `stroke_sprite`'s `thickness` is in **logical units**, like every other
stroke, so an outlined sprite and an outlined box at the same `thickness` match.
(ADR-0010, *`stroke_sprite` is mandated*.)

**4.7.10** Artwork authored with premultiplied or feathered edges outlines
slightly fat under §4.7.8. This is a stated consequence. (ADR-0010,
*`stroke_sprite` is mandated*.)

### 4.8 Text

**4.8.1** `measure_text(desc: TextDesc) -> TextMetrics` takes **no target** and
is a **pure function**. (ADR-0009, *`measure_text` is pure*.)

**4.8.2** `measure_text` and `fill_text` take the **same descriptor type**, so
what is measured is what is drawn. (ADR-0009, *`measure_text` is pure*.)

**4.8.3** `TextMetrics { advance: f32, bounds: Rect }` — two quantities.
`measure_text` reports **logical units**. (ADR-0009, *`measure_text` is pure*;
ADR-0005, *text*.)

**4.8.4** Measurement **accounts for kerning**. (ADR-0005, *text*.)

**4.8.5** **Per-glyph positions are not exposed.** A caller needing cursors or
selection measures substrings. (ADR-0005, *text*.)

**4.8.6** The default font is identified by **name, version and content hash**,
and is reached as `$.graphics.default_font`. A bare name is not sufficient
identification. (ADR-0005, *text*; ADR-0009, *`measure_text` is pure*.)

**4.8.7** `TextDesc.text` is **`[]u8`**, not `string`: text drawn by a program is
not necessarily a literal, and `string` is a view into the runner's constant blob
(ch2 §17.1). (ADR-0043 §11.)

**4.8.8** There is **no `stroke_text` in the mandated set**, and the name is
reserved coherently by §4.6.1. `fill_text` is the spelling, never `draw_text`.
(ADR-0009, *verb-first*.)

### 4.9 Images and pixel arrays

**4.9.1** The type is **`Image`** — not `Texture` as a content type, not
`Sprite`. (ADR-0009, *images*.)

**4.9.2** **This specification does not spell an image loader.** How an `Image`
is obtained is asset IO, decided elsewhere (ADR-0015 and chapter 5). (ADR-0009,
*images*.)

**4.9.3** `create_pixel_image(size: Vec2) -> Image` produces a **computed
`Image`**, and `get_pixels(image: !Image) -> []u32` returns a **mutable `[]u32`
view into it**. It is the **same `Image` type** the rest of the facade draws, so
a pixel array is an ordinary sprite. (ADR-0009, *images*; ADR-0004; ADR-0047 §5
for the mark.)

**4.9.4** There is **no pixel-array draw call**. A pixel array is drawn with
`fill_sprite`. (ADR-0009, *images*.)

**4.9.5 Taking the view is the upload signal.** A frame in which the program does
not call `get_pixels` for an image causes **no upload** of that image. A frame in
which it does causes **at most one**. (ADR-0022 §1, restating ADR-0004's
upload-cost guarantee.)

**4.9.6** `get_pixels` takes **no region parameter**, and MUST NOT be given one.
(ADR-0022 §1; §1.1.6.)

**4.9.7** Backends MUST be able to upload from a **strided host array without
repacking**. (ADR-0022 §1.) An implementation whose renderer cannot satisfy this
cannot serve a conforming implementation above the core rung; the rung wording is
chapter 8's. (ADR-0037, restating ADR-0022 §1's final sentence.)

**4.9.8** `slice_image(image: Image, region: Rect) -> Image` returns **a view
into another image**. Sprite sheets are handled this way; there is **no `source:
Rect` field** on any sprite descriptor. (ADR-0009, *images*.)

**4.9.9** **A slice MUST NOT outlive its parent image.** (ADR-0009, *images*;
#15's view lifetime rule, ch3 §6.)

**4.9.10** `$.graphics.debug_image` is an **`Image` value**, mandated and reached
explicitly. It is not an error-only behaviour, not a fallback an implementation
substitutes, and not a drawing call. (ADR-0009, *the checkerboard is a value*, as
amended by ADR-0015 and renamed by ADR-0033.)

**4.9.11** There is **no screen readback**: `get_pixels` takes an `Image` and
never the screen. (ADR-0038 §11; ADR-0036 §7.)

### 4.10 Reserved and refused surface

**4.10.1** Resolution-dependent asset substitution — a `@150`-style sprite
variant selected by the runner — is **not in the facade**. (ADR-0005,
*consequences*; #12.)

**4.10.2** The following are **refused by name** and do not return as natural
extensions: composable blend factors (§4.4.8), a point primitive (§4.4.6), a
`circle` alias (§4.4.5), a polyline (§4.4.7), a `color_mode` field (§4.7.7), a
`source: Rect` sprite field (§4.9.8), a `region` parameter on `get_pixels`
(§4.9.6), and a global origin mode (§4.1.2). (ADR-0005; ADR-0009; ADR-0010;
ADR-0022 §1.)

### 4.11 What the spec promises about exactness

**4.11.1 The spec promises only what is derivable** from the specifications a
backend is built on. Where a property cannot be derived, this specification says
plainly that it is not guaranteed rather than weakening it into a promise that
sounds kept. (ADR-0022 §0.)

**4.11.2** There is **no claim that a `crisp` target is bit-exact**. That
sentence is deleted, not caveated. `crisp` is what §4.2.2 says it is: no
anti-aliasing, nearest sampling. (ADR-0022 §2.)

**4.11.3 Geometry coverage.** On a `crisp` target, **interior and exterior pixels
are exact everywhere**. **Boundary pixels are exact for axis-aligned geometry
whose edges lie on integer framebuffer coordinates**, because no pixel centre
lies on such an edge. For geometry with an edge passing through a pixel centre,
coverage is **implementation-defined**. Bit-exactness across backends
additionally requires that all transform arithmetic be exactly representable in
`f32`; identity and integer translation satisfy this, arbitrary scale and
rotation do not. (ADR-0022 §2.)

**4.11.4 Texel selection.** Under `crisp`'s nearest sampling, where a sample
point lies **strictly inside** a texel, that texel is selected. Where a sample
point lies **on a texel boundary**, selection is **implementation-defined**.
(ADR-0034 §3, added to ADR-0022 §2.)

**4.11.5** §4.11.4 promises **which texel, never what byte**. (ADR-0034 §3.)

**4.11.6 `smooth` coverage.** On a `smooth` target, **fully-covered pixels are
exact and fully-uncovered pixels are untouched**. The value of a
**partially-covered boundary pixel is implementation-defined**. There is **no
numeric coverage tolerance**, and none may be invented. (ADR-0022 §3.)

**4.11.7 There is no byte-exact blit clause**, and no implementation condition
requiring any draw to be compiled as a texture-to-texture copy. Both are
withdrawn. §4.11.3 and §4.11.4 are the whole of what this specification promises
about graphics exactness, on both the coverage and the sampling axis. (ADR-0034
§1; ADR-0022 §4, deleted.)

**4.11.8 There is no texel-centre or tie-break convention.** The fractional and
rotated cases are covered by §4.11.3 and §4.11.4 — interior determined, boundary
implementation-defined — and a conforming implementation MUST NOT be held to a
convention this specification does not state. (ADR-0034 §4.)

**4.11.9** An implementation MUST NOT satisfy any exactness clause by publishing
a measured figure. (ADR-0022 §0.)

---

## 5. `$.audio`

### 5.1 The model

**5.1.1 A mandated voice plays; it does not render.** The facade owns the active
voice set, sums it, and fills the ring itself. It MUST NOT hand the caller a
generator to render into a buffer. (ADR-0007, *voices play*.)

**5.1.2** **The sum into one device is not routing.** ADR-0006's ban on a route
between voices forbids voice A's output becoming voice B's input; every voice
reaching one output is not that. (ADR-0007, *voices play*.)

**5.1.3 There is no voice cap.** The `O(1)` clause bounds each voice, not how
many exist. An implementation MUST NOT mandate a maximum voice count, MUST NOT
implement a stealing policy, and MUST NOT report exhaustion. (ADR-0007, *there is
no voice cap*; correcting ADR-0006.)

**5.1.4** The voice set is **runner-owned** and grows with the runner's
allocator. The **facade** still allocates nothing: no mandated call takes a size
and no DSP parameter implies a buffer. (ADR-0007, *there is no voice cap*;
ADR-0006, *the line*.)

**5.1.5** The real limit on voice count is **frame budget**, not a cap: a
thousand voices runs late, it does not fail. Unbounded growth from a program that
never stops a voice is that program's bug. (ADR-0007, *there is no voice cap*;
#8; #19 P7.)

**5.1.6 A mandated voice is mono and carries a continuous `pan` parameter.**
Output is fixed at **two channels, f32 interleaved, at the device rate**.
(ADR-0007, *mono voices*; ADR-0006 R6.)

**5.1.7 Pan law is constant power, −3 dB at centre:**

```
theta      = (pan + 1) * PI / 4        # pan in [-1, 1]
gain_left  = cos(theta)
gain_right = sin(theta)
```

(ADR-0007, *mono voices*.)

**5.1.8** A **stereo clip** plays to both channels, and `pan` acts as a
**balance** — attenuating the opposite channel by the same gains — rather than as
a position. (ADR-0007, *mono voices*.)

### 5.2 The mandated set

**5.2.1**

```
# Voice path
$.audio.play(desc: VoiceDesc) -> Voice
$.audio.stop(v: Voice)
$.audio.stop_all()
$.audio.get(v: Voice) -> ?Voice
$.audio.set(v: Voice, patch: VoicePatch)

# Device access for a library with its own mixer
$.audio.push(samples: []f32)
$.audio.free_space() -> int

# Device facts
$.audio.cursor() -> SampleFrames
$.audio.sample_rate() -> int
$.audio.underruns() -> int

# Conversion
$.audio.seconds(frames: SampleFrames, rate: int) -> Seconds
```

(ADR-0007, *the naming rules*; `cursor`'s return type and `seconds` from
ADR-0016 §5.)

**5.2.2** The types are `Voice`, `VoiceDesc`, `VoicePatch`, `Waveform`,
`FilterType` and **`Clip`**. (ADR-0007, *the naming rules*, as corrected by
ADR-0033: `Clip` is a root type and the count is six, not five.)

**5.2.3** `play` returns a **`Voice` handle, not `?Voice`**, because exhaustion is
not a state (§5.1.3). (ADR-0007, *there is no voice cap*.)

**5.2.4** `get(v) -> ?Voice` reports absence because a finished one-shot frees its
slot and the handle goes gone. This is the ordinary generational-handle behaviour
(ch3 §10.2), never a capacity signal. (ADR-0007, *there is no voice cap*.)

**5.2.5** `set(v, patch)` is **one call**. `VoicePatch` has `?T` fields, and an
absent field means **unchanged**. Per-parameter setters MUST NOT be added.
(ADR-0007, *re-parameterising a live voice*.)

**5.2.6 `$.audio.cursor()` returns `SampleFrames`, not `int`.** (ADR-0016 §5,
superseding ADR-0007's signature.)

**5.2.7 Conversion from frames to seconds takes the rate as an argument and never
as a constant**: `$.audio.seconds(frames, rate)`. (ADR-0016 §5.)

### 5.3 Ramping and stopping

**5.3.1 Continuous parameters ramp linearly over exactly one frame** — the fixed
1/60 s step — for `frequency`, `amplitude`, `pan`, `cutoff` and `resonance`.
(ADR-0007, *ramping*; ADR-0006, *two clauses the addition drags in*; #26.)

**5.3.2** **Amplitude ramps linearly in amplitude, not in dB**, and **frequency
ramps linearly in Hz, not in cents**. (ADR-0007, *ramping*.)

**5.3.3** **Discrete parameters do not ramp** — waveform, filter type, loop flag
and direction switch at the next frame boundary. (ADR-0007, *ramping*.)

**5.3.4** Per-frame modulation from ordinary program code **is** the modulation
mechanism. There is no routing matrix and no second control language; an LFO is a
`sin` computed in the frame entry. (ADR-0006, *two clauses the addition drags
in*; excluded from the facade by the routing clause, §5.6.2.)

**5.3.5 `stop` and `stop_all` ramp amplitude to zero over one frame**, then free
the slot. (ADR-0007, *`stop` is the interrupt*.)

**5.3.6** The **handle goes gone immediately**; the slot frees a frame later.
(ADR-0007, *`stop` is the interrupt*.)

**5.3.7** A voice **with a release segment** runs its release instead of §5.3.5's
ramp. `stop` is the interrupt; the release is the note-off. (ADR-0007, *`stop` is
the interrupt*.)

### 5.4 Envelopes, waveforms and clips

**5.4.1** The **amplitude envelope is ADSR** — attack time, decay time, sustain
level, release time. (ADR-0007, *envelopes*.)

**5.4.2** The **pitch envelope is a three-number decay-to-target** — start
offset, time, linear curve — and is **not** a second ADSR. (ADR-0007,
*envelopes*.)

**5.4.3** The **waveform enum is closed**: `sine`, `square`, `triangle`, `saw`,
`noise`. A voice carries frequency, amplitude, an amplitude envelope, a pitch
envelope, and one-shot or loop. (ADR-0006, *the mandated addition*.)

**5.4.4 A clip is a caller-owned decoded buffer** — a `[]f32` plus a channel
count and a source sample rate — **never a path**. The facade does **no loading
and no decoding**. (ADR-0007, *clips*; ADR-0006 R7.)

**5.4.5** A clip has **one loop region** (start, end) in sample frames.
(ADR-0007, *clips*.)

**5.4.6** Clip playback direction is **forward or reverse**. (ADR-0007, *clips*;
ADR-0006.)

**5.4.7** **Resampling a clip to the device rate is the facade's job**, and falls
in the toleranced bucket (§5.8.2). (ADR-0007, *clips*.)

**5.4.8 The caller-authored clip is the procedural path.** A program that wants
to synthesise a sound fills a `[]f32` and plays it through a voice, gaining the
envelope, pitch, loop, filter and ramp. **`push` is not the procedural path** and
a program MUST NOT be directed to it for that purpose. (ADR-0007, *the clip is
the procedural path*, correcting ADR-0006's stated justification.)

### 5.5 The escape hatch

**5.5.1** `push(samples: []f32)` **bypasses the voice system to reach the
device**. Its audience is a library that owns its own mixer. (ADR-0007, *the clip
is the procedural path*; *the escape hatch*.)

**5.5.2** `free_space()` describes **the ring, not the voices**. A program that
only plays voices never calls it, and the facade fills the ring itself.
(ADR-0007, *the escape hatch*; ADR-0006 R6.)

**5.5.3** **`push` returns nothing.** It MUST NOT block, MUST NOT silently drop
excess, and MUST NOT return a frames-accepted count. (ADR-0007, *the escape
hatch*.)

**5.5.4** Pushing more than `free_space()` frames is a **`$.assert` bug**, not a
truncation. (ADR-0007, *the escape hatch*.)

**5.5.5** **Push is a copy**, never a handoff of a mapped region. There is no
`map`, `lock_buffer` or `get_write_pointer` anywhere in the audio path.
(ADR-0006 R6.)

**5.5.6** **The device reports its sample rate; the caller never demands one.**
The push format is fixed at **f32 interleaved**, and backends convert where the
platform wants another format. (ADR-0006 R6.)

### 5.6 The mandated/engine-tier line

**5.6.1** A mandated voice carries **fixed, `O(1)`, spec-sized state and
allocates nothing**, and applies to **one voice with no routing to another**.
(ADR-0006, *the line*.)

**5.6.2** Anything needing a caller-sized buffer, or a route between voices, is
**engine tier and unmandated**. Delay and reverb are out on the memory clause; a
mix bus is out on the routing clause; an LFO is out on the routing clause alone.
Oscillator, envelope, filter and clip playback are in. (ADR-0006, *the line*.)

**5.6.3** **Caller-supplied memory is fine.** A clip and a pushed buffer are the
caller's. It is **allocation by the facade** that is forbidden, not the presence
of a large buffer. (ADR-0006, *the line*.)

**5.6.4** A synthesizer, an effects chain, MML, a mixer and a streamer are the
**engine tier**: unmandated, and conformance MUST NOT depend on them. (ADR-0006,
*the audio engine is engine tier*.)

### 5.7 The cursor, underruns and faults

**5.7.1** The play cursor is **monotonic, in sample frames, never resets, for the
process lifetime**. (ADR-0007, *the play cursor*; ADR-0006 R6.)

**5.7.2** On **underrun** the cursor **advances through the silence**: it counts
frames the device consumed, silence included. (ADR-0007, *the play cursor*.)

**5.7.3** On a **device change** the cursor **continues** and MUST NOT reset. The
cursor is in frames **at the current device rate**, and a discontinuity in
wall-clock-per-frame is permitted exactly at a device change. `sample_rate()`
must be re-read. (ADR-0007, *the play cursor*.)

**5.7.4** **Reload leaves the cursor untouched.** *Reload* is the in-process swap
of game code, not a restart of the host process or document. (ADR-0007, *the play
cursor*; the definition from ADR-0022 §5.)

**5.7.5 Ring underrun outputs silence for the missing frames**, and
`$.audio.underruns() -> int` returns a **monotonic count since process start**.
It is a number, never an error return. (ADR-0007, *underruns are counted*.)

**5.7.6** **File-stream underrun** is one tier up and unmandated: an underrunning
stream outputs silence and reports, and MUST NOT stall the frame. (ADR-0006 R7.)

**5.7.7** `play` **cannot receive an absent clip**: a clip is a `[]f32` the caller
holds, and an empty slice plays as a zero-length one-shot that finishes
immediately. (ADR-0007, *the error affordance*.)

**5.7.8** **NaN, infinite or negative frequency; negative amplitude; NaN pan** are
each a **`$.assert` bug** — always on, faults, attributable. (ADR-0007, *the error
affordance*; #10.)

**5.7.9** **Amplitude above 1.0 is legal and is clamped at the output**, never a
bug. (ADR-0007, *the error affordance*.)

**5.7.10** **There is no mandated audio debug tone.** The drawing facade's
visible-failure affordance (§4.9.10) has **no audio equivalent**, because silence
is indistinguishable from success and a debug tone is hostile to ship. (ADR-0007,
*the error affordance*.)

### 5.8 What is exact and what is toleranced

**5.8.1 Sample-exact:** **square, saw and triangle** waveforms; **clip playback
at unity rate**; **linear envelope segments**; and **noise**, given §5.9's
generator. (ADR-0007, *conformance*; ADR-0006.)

**5.8.2 Toleranced at ±1/32768** — one 16-bit LSB — **per sample of amplitude at
unity gain:** **sine**, **any filtered signal**, and **resampled clip playback**.
(ADR-0007, *conformance*.)

### 5.9 The mandated noise generator

**5.9.1** Noise is **xorshift32**:

```
x ^= x << 13
x ^= x >> 17
x ^= x << 5
sample = (x as i32) / 2147483648.0    # in [-1, 1)
```

(ADR-0007, *the mandated noise generator*.)

**5.9.2** Each voice **seeds its own generator** from a value in `VoiceDesc`,
defaulting to a spec-stated constant. Two noise voices with the same seed are
identical. (ADR-0007, *the mandated noise generator*.)

### 5.10 The filter

**5.10.1** The **filter-type enum is closed**: `lowpass`, `highpass`, `bandpass`,
with **cutoff** and **resonance** as voice parameters. (ADR-0006, *the mandated
addition*.)

**5.10.2** The filter is the **RBJ Audio-EQ-Cookbook biquad**, for all three
members, with **resonance expressed as `Q`**. (ADR-0007, *the filter's
coefficient formulas*.)

**5.10.3** The realisation is **direct form I**. (ADR-0007, *the filter's
coefficient formulas*.)

**5.10.4** **Coefficients are computed in `f64`**, at the ramp rate (once per
frame), and applied per-sample. (ADR-0007, *the filter's coefficient formulas*.)

**5.10.5** Coefficients follow the cookbook's `alpha = sin(w0) / (2 * Q)` form,
with `w0 = 2 * PI * cutoff / sample_rate`. (ADR-0007, *the filter's coefficient
formulas*.)

**5.10.6** **Cutoff is clamped to the open interval `(0, sample_rate / 2)`**, and
a cutoff at or past Nyquist is a **`$.assert` bug, not a clamp**. (ADR-0007,
*the filter's coefficient formulas*.)

---

## 6. `$.input`

### 6.1 The model

**6.1.1** The primary surface is a **virtual controller** which the
implementation maps onto real hardware. The raw keyboard, mouse and pointer
queries are the **escape hatch**. (ADR-0011, *the mandated surface*.)

**6.1.2** **A game written against the virtual controller never names a key.**
(ADR-0011, *the mandated surface*.)

**6.1.3** **A rebindable action layer is not in the facade.** (ADR-0011, *the
mandated surface*.)

**6.1.4** The default binding table (§6.7) is **normative, not advisory**.
(ADR-0011, *the normative default binding table*.)

### 6.2 Buttons are ordinals

**6.2.1**

```
Button = one | two | three | four | five | six
       | seven | eight | nine | ten | eleven | twelve
```

Hardware nouns MUST NOT appear as member names. (ADR-0011, *buttons are
ordinals*.)

**6.2.2** A control is in the set **if and only if every conforming platform's
pad has it**. (ADR-0011, *buttons are ordinals*.)

**6.2.3** The ordinals are fixed normatively by prominence:

| Ordinal | Pad control |
|---|---|
| `one` … `four` | Face buttons, in south, east, west, north order |
| `five`, `six` | Left and right shoulders |
| `seven`, `eight` | Left and right triggers |
| `nine`, `ten` | Start, select |
| `eleven`, `twelve` | Left and right stick clicks |

(ADR-0011, *buttons are ordinals*.)

**6.2.4** **`one` is always the physically-south button.** No platform-specific
confirm/cancel remap is applied, on any target. (ADR-0011, *what this deletes*.)

**6.2.5** **Integer button indices and bitmasks are not in the facade.**
(ADR-0011, *buttons are ordinals*.)

**6.2.6** **Analog trigger values are not exposed.** Triggers are ordinals
`seven` and `eight`, and are booleans. (ADR-0011, *analog triggers*.)

### 6.3 Direction

**6.3.1**

```
$.input.direction(side: Side = .left, player: int = 0) -> Vec2
Side = left | right
```

Direction is a `Vec2`, not four buttons. (ADR-0011, *direction is a `Vec2`*.)

**6.3.2** **Keys synthesise magnitude 1.0**, and a keyboard diagonal is **unit
length**. (ADR-0011, *direction is a `Vec2`*.)

**6.3.3** **A stick's true magnitude passes through, clamped to 1.0.**
(ADR-0011, *direction is a `Vec2`*.)

**6.3.4 The dead zone is fixed by this specification and is already applied**:
the returned vector is zero inside the zone and **rescaled so it reaches 1.0 at
the rim**. It is not a parameter, and an implementation MUST NOT expose one.
(ADR-0011, *direction is a `Vec2`*; *why the dead zone is not a knob*.)

**6.3.5** `direction(.left)` **unions** the d-pad, the left stick and the
keyboard's bound direction keys. (ADR-0011, *direction is a `Vec2`*.)

**6.3.6** `direction(.right)` **falls back to the face buttons** where no right
stick exists. (ADR-0011, *direction is a `Vec2`*.)

**6.3.7** No scalar axis, percentage or angle query ships beside `direction`.
(ADR-0011, *one number, not three*.)

### 6.4 Edges, players and the pointer

**6.4.1** `_down` (level), `_pressed` and `_released` (edge, exactly this frame)
are the triple, for buttons, keys and pointer buttons alike. (ADR-0011, *edge
queries*.)

**6.4.2** **The edge state is the runner's**, computed against the previous
frame's snapshot. A program MUST NOT need a hand-rolled previous-frame latch.
(ADR-0011, *edge queries*; *conformance*.)

**6.4.3** **Auto-repeat is not mandated**, and MUST NOT be added as a flag on
`pressed`. (ADR-0011, *edge queries*.)

**6.4.4** Players are **slots, not devices**, with a **fixed maximum of four**.
Every controller query takes `player: int = 0`. There is no `Controller` handle.
(ADR-0011, *players are slots*.)

**6.4.5** **An absent device is present and idle** — no buttons down, zero
direction — never an error and never a missing symbol. (ADR-0011, *players are
slots*.)

**6.4.6** **Unplugging a pad leaves its slot present and idle**; higher-numbered
players MUST NOT renumber downward. (ADR-0011, *players are slots*.)

**6.4.7** **The keyboard is player 0** and is never auto-assigned elsewhere. Two
players on one keyboard is not covered by the default table. (ADR-0011, *players
are slots*.)

**6.4.8** `connected(player)` means **"has produced input"**, not "is physically
attached". (ADR-0011, *conformance*.)

**6.4.9 Pointer position is reported in logical-canvas space** — the fit removed,
the `Target`'s camera transform **not** applied. (ADR-0011, *the pointer*;
ADR-0030 §4.)

**6.4.10** **A pointer in the letterbox bars reports a position outside the
canvas rect.** It MUST NOT be clamped. `pointer_over()` is what asks the
question. (ADR-0011, *the pointer*, with ADR-0030 supplying the precondition that
bars exist.)

**6.4.11** **Pointer lock is a request that can fail**, never a setter.
(ADR-0011, *the pointer*.)

**6.4.12** `scroll()` is a `Vec2` delta, and horizontal scroll exists.
(ADR-0011, *the pointer*.)

### 6.5 The raw layer

**6.5.1 Keys are identified by what they print. Keys that print nothing are
identified by position.** This is one rule. (ADR-0011, *the raw layer*.)

**6.5.2** `Key` is a **closed enum**, and a key it does not name is **dropped**
— invisible to the program. (ADR-0011, *the raw layer*.)

**6.5.3** The international keys are **named in**. F13–F24, media and vendor keys
are **out**. (ADR-0011, *the raw layer*.)

**6.5.4** There is **no `unknown(u32)` key variant**. (ADR-0011, *the raw
layer*.)

**6.5.5** **Modifiers are ordinary `Key` members, with left and right
distinguished.** There is **no modifier bitset**. (ADR-0011, *the raw layer*.)

**6.5.6** `$.input.text()` returns **the text composed this frame**, as
**`[]u8`** — not `string`, because composed text is not a literal — and the view
is **runner-owned memory valid until the frame entry returns**. (ADR-0011,
*`text()` is not an escape hatch*, as stamped by ADR-0043 §11.)

**6.5.7** **There is no general event queue.** Polling is the whole input model,
and `text()` is the one sequence, scoped to one frame. (ADR-0011, *`text()` is
not an escape hatch*.)

**6.5.8** **`key_label` is refused by name** and MUST NOT be added: it is
unimplementable in a conforming browser. The mandated idiom for a rebind screen
is to read `key_pressed` for the binding and `text()` for the label **in the same
frame**. (ADR-0011, *`key_label` is rejected by name*; ADR-0006's negative web
obligation.)

**6.5.9** **`mapping_for(action)` is not mandated.** A game renders its own glyphs
from `last_source()` plus a `Button` value. (ADR-0011, *`key_label` is rejected by
name*.)

**6.5.10** **A controller name is not exposed.** (ADR-0011, *the mandated set*.)

### 6.6 The mandated set

**6.6.1**

```
# Virtual controller
$.input.direction(side: Side = .left, player: int = 0) -> Vec2
$.input.button_down(b: Button, player: int = 0) -> bool
$.input.button_pressed(b: Button, player: int = 0) -> bool
$.input.button_released(b: Button, player: int = 0) -> bool
$.input.connected(player: int = 0) -> bool
$.input.last_source() -> Source

# Pointer
$.input.pointer_position() -> Vec2
$.input.pointer_motion() -> Vec2
$.input.pointer_over() -> bool
$.input.pointer_down(b: PointerButton) -> bool
$.input.pointer_pressed(b: PointerButton) -> bool
$.input.pointer_released(b: PointerButton) -> bool
$.input.scroll() -> Vec2
$.input.set_cursor_visible(visible: bool)
$.input.cursor_visible() -> bool
$.input.request_pointer_lock()
$.input.release_pointer_lock()
$.input.pointer_locked() -> bool

# Raw keyboard, plus text
$.input.key_down(k: Key) -> bool
$.input.key_pressed(k: Key) -> bool
$.input.key_released(k: Key) -> bool
$.input.text() -> []u8
```

Five types: `Key`, `Button`, `Side`, `PointerButton`, `Source`.

```
Side           = left | right
PointerButton  = left | middle | right
Source         = keyboard | mouse | gamepad
```

(ADR-0011, *the mandated set*; `text()`'s return type from ADR-0043 §11.)

**6.6.2** **No `$.input` call takes a descriptor struct**: every call takes two
parameters or fewer, so §1.1.5 does not fire. (ADR-0011, *the mandated set*.)

**6.6.3** `last_source()` is what a game reads to switch glyph art the instant the
player switches devices. (ADR-0011, *the mandated set*.)

**6.6.4** **`to_world` is not in `$.input`** — see §4.2.8. (ADR-0011, *the
mandated set*.)

### 6.7 The normative default binding table

**6.7.1** The table names keyboard keys by **physical position** (US legend),
while the `Key` enum is layout-mapped (§6.5.1). This is deliberate: a binding is a
fact about where a control sits, and the physical addressing never surfaces as a
`Key` value.

| Control | Pad | Keyboard (player 0) |
|---|---|---|
| `direction(.left)` | D-pad + left stick | `WASD` + arrow keys |
| `direction(.right)` | Right stick | Face-button keys, per §6.3.6 |
| `one` … `four` | South, east, west, north | `Z`, `X`, `C`, `V` |
| `five`, `six` | Left, right shoulder | `Q`, `E` |
| `seven`, `eight` | Left, right trigger | `1`, `3` |
| `nine`, `ten` | Start, select | `Enter`, `Tab` |
| `eleven`, `twelve` | Left, right stick click | `Left Shift`, `Right Shift` |

(ADR-0011, *the normative default binding table*.)

**6.7.2** **Players 1–3 have no keyboard bindings.** (ADR-0011, *the normative
default binding table*.)

### 6.8 How input reaches the frame

**6.8.1** Input is read by **ambient calls**, not delivered as a value at the
frame entry. (ADR-0011, *how input reaches the frame*.)

**6.8.2 Input is latched once per frame, before the frame entry runs.** Every
query within one frame observes the same snapshot, so two calls cannot disagree.
There is no ambient *state*, only an ambient *constant*. (ADR-0011, *how input
reaches the frame*; *conformance*.)

**6.8.3** §6.8.1 does not extend to `Target`, which is delivered at the entry
(§4.2.6). (ADR-0011, *how input reaches the frame*; the conflict it raised
resolved by ADR-0013.)

---

## 7. `$.video`

**7.1**

```
$.video.render_scale()             -> f32
$.video.set_render_scale(s: f32)
$.video.fullscreen()               -> bool
$.video.set_fullscreen(on: bool)
```

Four functions, no types, no values. (ADR-0038 §10, §13.)

**7.2 Render scale is the spec's only performance escape hatch**, for the player
or the program. (ADR-0038, opening.)

**7.3** The arithmetic, given chapter 5's fit factor `k`:

```
render scale  s ∈ [0.25, 1.0], quantised to sixteenths
rasterise at  s · k
present at    k
```

At `s = 1` nothing whatsoever changes. Below 1, the frame is rasterised into an
offscreen target at `s · k` and resampled up to `k` for presentation. (ADR-0038
§1.)

**7.4** **The world a program draws is identical at every scale.** `s` changes
only the number of device pixels shaded. (ADR-0038 §1.)

**7.5** **`s > 1` is not legal.** The ceiling is `1.0`. (ADR-0038 §2.)

**7.6** The hatch **covers all rendering**, at every conformance rung, and is not
confined to the shader rung. (ADR-0038 §3.)

**7.7** An implementation **MUST NOT require an intermediate render target at
`s = 1`**, and **MAY** use one only below it. Where it does, it allocates at most
one, never larger than the window. (ADR-0038 §4.)

**7.8** **The runner MUST provide an affordance** for setting the render scale,
and **the program may set it** through its own menu. (ADR-0038 §5; ADR-0025 §3.)

**7.9 The scale is not the program's to cache.** The runner may change it without
telling the program; a program reads it when it needs it. The same discipline
binds the fullscreen pair. (ADR-0038 §5, §10; ADR-0025 §3.)

**7.10** **Runner-adaptive scaling is refused.** A conforming runner MUST NOT
lower the render scale on its own initiative. Adaptive behaviour is expressible in
program code and belongs there. (ADR-0038 §5.)

**7.11** The argument is `f32` and is **quantised to the nearest sixteenth in
`[0.25, 1.0]` — thirteen legal values**. The getter returns the **quantised
value**, never the argument. (ADR-0038 §8.)

**7.12** **Out-of-range input is clamped silently**, never faulted. (ADR-0038
§8.)

**7.13** **No new quantity type is minted** for the scale: it is a unitless ratio.
(ADR-0038 §8; ADR-0016 §4.)

**7.14** The upscale resample from `s · k` to `k` is **linear, fixed, on both
style tokens**. (ADR-0038 §7.)

**7.15** §7.14 **does not touch content sampling**: `crisp` stays nearest and
`smooth` stays linear for how draws sample their textures, at whatever rate the
frame is rasterised (§4.11.4). The linear rule governs one whole-image resample at
the end. (ADR-0038 §7.)

**7.16** The render scale is **runner-owned state that survives relaunch**. This
specification names no file, no format and no location. (ADR-0038 §9; ADR-0025
§6.)

**7.17** **Who set the scale is not recorded**, and a program's adaptive
controller therefore persists whatever it converged to. (ADR-0038 §9.)

**7.18** **A program cannot observe the offscreen target, its size, or the
resample.** The scale is visible through its own getter and nowhere else (§4.9.11).
(ADR-0038 §11.)

**7.19** **The render scale has no effect on input mapping.** `s` is applied after
the coordinate transform, so aiming is unaffected at every scale. (ADR-0038 §11;
§6.4.9.)

**7.20 `$.video` is not a precedent.** `$.video.resolution()`, `$.video.size()`
and `$.video.dpi()` remain refused: they return hardware magnitudes and no
mandated facility needs them. The existence of the root does not admit them, and a
future proposal to add one runs the full admission test with no head start.
(ADR-0038 §10; ADR-0028 §6.)

**7.21** Neither pair belongs in `$.graphics`: every `$.graphics` call takes a
`Target` or an `Image`, and these take neither. (ADR-0038 §10.)

**7.22** The roots `$.screen`, `$.view`, `$.settings`, `$.prefs` and `$.window`
are **refused by name**, as are bare root-level spellings of these four calls.
(ADR-0038 §10.)

---

## 8. `$.storage`

**8.1** A storage slot is declared in the same form as an asset, minus the source
string:

```ludo
extern storage save1
extern storage save2
```

`storage` occupies the kind-noun slot, and the **core grammar gains nothing**
(ch1). (ADR-0026 §3.)

**8.2** **Slot names are fixed at compile time.** There is **no runtime key
construction, no path, no directory and no string lookup**. (ADR-0026 §3.)

**8.3** A program declares as many slots as it wants. (ADR-0026 §3.)

**8.4**

```
$.storage.write(save1, bytes) -> !
$.storage.read(save1)         -> ?[]u8
```

Two functions under one root name. (ADR-0026 §4.)

**8.5** **The payload is `[]u8`, and this specification blesses no serializer.**
Serialization is a library's job. (ADR-0026 §4; ADR-0014.)

**8.6** `?[]u8` carries first-launch: **a slot never written reads absent**, and
the check is the bind. There is **no `exists` predicate**. (ADR-0026 §4; ch2 §9.)

**8.7 The read resolves before top level runs.** There is no loading state and
nothing to poll. (ADR-0026 §5.)

**8.8 The write is submit-and-forget.** It never blocks and returns no
completion. The runner flushes it — at the latest, at the last moment the host
guarantees it can run code. (ADR-0026 §5, as restated by ADR-0041.)

**8.9** An implementation **MUST accept a write of at least 1 MiB per slot**. A
larger write **MAY** be refused, **MUST** be reported as an error value, and
**MUST NOT** be silently truncated. (ADR-0026 §6.)

**8.10** **Oversize is knowable at the call** and is returned as an error value at
the call site. (ADR-0026 §7; #10.)

**8.11** **IO failure — disk full, quota denied, storage evicted — happens after
the call returns, and the program never learns of it.** The runner reports it on
chapter 7's diagnostic stream. There is **no status channel and no
did-my-write-land query**. (ADR-0026 §7.)

**8.12 Slots resolve once per process and are never re-resolved.** (ADR-0026 §8.)

**8.13** Read-after-write within a process returns what was written. (ADR-0026
§8.)

**8.14** A reload **preserves resolved contents** exactly as it preserves
`persist`. The runner's only reload obligation is to **flush pending writes before
the swap**. (ADR-0026 §8; #17.)

**8.15** **Live state for the quiescence predicate is `persist`, plus declared
assets, plus resolved storage contents.** The predicate itself is chapter 5's.
(ADR-0026 §8, amending ADR-0024.)

**8.16** **A fault does not discard a submitted write**, including one submitted
in the faulting frame. (ADR-0026 §8; #18.)

**8.17 No settings concept ships.** There is no `$.settings`. Volume and key
bindings are program-defined and belong in a declared slot; fullscreen and render
scale are runner-owned because the runner implements them (§7). (ADR-0026 §9;
ADR-0038 §10.)

**8.18** The 1 MiB round-trip — write, relaunch, read back identical — is a
**core, headless** conformance test. The rung wording is chapter 8's. (ADR-0026
§6.)

---

## 9. Shader ludo and the `extern shader` declaration

**9.1 Shaders are opt-in.** A conforming implementation may support none, and a
program may ship without touching one. (ADR-0008, opening.)

**9.2** There are **two authoring paths**: shader ludo, and a `.wgsl` file named
by an `extern shader` declaration. Together they define the **shader conformance
rung**; the rung itself is chapter 8's. (ADR-0008 §1, §8; ADR-0037.)

**9.3** An `extern shader` declaration or shader-ludo source, built against an
implementation that does not claim the shader rung, is a **compile error naming
the declaration** under chapter 7's envelope — never a runtime fault. (ADR-0037,
amending ADR-0008.)

**9.4** The **shader IR and its emitters are not specified here** and are not
mandated. What is specified is the dialect's rules and the checker obligation.
(ADR-0008 §1.)

**9.5** The dialect **may not exceed what the shared IR expresses**, which is
fixed at the intersection the renderer targets. (ADR-0008 §1; ADR-0002.)

**9.6 Shader ludo is a strict subset of ludo.** Everything it accepts means what
it means on the CPU. (ADR-0008 §2.)

**9.7** **Removals are free**: no allocators, no pools, no handles, no `List` or
`Map`, no `defer`, no `unsafe`, no FFI, no recursion. (ADR-0008 §2.)

**9.8 Additions are excluded rather than granted new syntax.** Anything that
cannot be expressed with existing language machinery is **excluded from the
dialect**. (ADR-0008 §2.)

**9.9** Consequently: texture and sampler handles are **`distinct` opaque types in
the `$.` root**; stage builtins and sampling are **ordinary free functions** in a
shader-only stdlib module; **address spaces ride on the binding declaration**, not
on the type; and **uniformity is a checker rule with a named diagnostic**, never a
type-system feature. (ADR-0008 §2.)

**9.10 Shader ludo covers the vertex and fragment stages. Compute shaders are
authored as `.wgsl` only.** (ADR-0008 §3.)

**9.11** Within shader ludo, indexing into a fixed-size array is **provably in
range**, so the out-of-bounds guarantee is **vacuous rather than violated**.
Dynamic indexing into a variable-length storage buffer is what §9.10 deletes.
(ADR-0008 §3.)

**9.12** Texture **sampling is not indexing**: clamp-versus-repeat is a **declared
parameter of the sampler**, so the behaviour is in the signature. (ADR-0008 §3.)

**9.13 Integer overflow is the one enumerated semantic difference** between CPU
ludo and shader ludo: GPU integer arithmetic wraps, and the difference is written
here rather than discovered. An implementation MUST NOT emit in-shader overflow
checks with a readback to report them. (ADR-0008 §3.)

**9.14** A shader function is a **marked declaration, never a magic name**. The
markers are the **`#vertex` and `#fragment` attributes**, reusing the attribute
mechanism; **no new keyword is spent**. (ADR-0008 §4.)

**9.15** The marker is what puts the checker into shader mode: a function is
checked as CPU ludo or as shader ludo, never ambiguously. (ADR-0008 §4.)

**9.16 An unmarked function that stays inside the subset is callable from both CPU
and shader code.** (ADR-0008 §5.)

**9.17** **Calling a `#vertex` or `#fragment` function from CPU code is a compile
error.** (ADR-0008 §5.)

**9.18** The blessed math types (§3) stay **library types in the `$.` root**, and
the shader emitter **recognises them by name**, mapping them to GPU primitives.
They are not built-in types. (ADR-0008 §6.)

**9.19** §9.18 is what fixes the blessed set at what the GPU can express — the
veto §3.2 satisfies. (ADR-0008 §6; ADR-0016 §2.)

**9.20 Sampling is two calls:**

- **`sample(tex, uv)`** — automatic detail level, legal **only in straight-line
  fragment code**.
- **`sample_level(tex, uv, level)`** — caller-stated level, **legal anywhere**.

The restriction on the first is a **checker rule with a named diagnostic**.
(ADR-0008 §7.)

**9.21 A `.wgsl` file is named by an `extern shader` declaration in ludo source**,
in the same shape as an `extern` C declaration. The compiler reads the file when it
reads the declaration. There is **no build configuration and no intermediate
compilation step**. (ADR-0008 §8.)

**9.22** A `.wgsl` file is **not an asset** and does not pass through an asset
pipeline. (ADR-0008 §8; #12.)

**9.23** An `extern shader` declaration **carries no `unsafe`**, because §9.24
checks the shader side. (ADR-0008 §8; contrast #29.)

**9.24 The interface check compares three things and nothing more:**

1. the **parameter block's layout** — field order, types, sizes;
2. the **resource list** — how many textures and buffers, in which slots, of which
   kind;
3. the **vertex input list**.

The toolchain parses the **declaration surface only**, never expression semantics
and never codegen. (ADR-0008 §9; ADR-0003.)

**9.25** Item 1 is decidable because every struct's byte layout is computable from
its declaration text alone (ch3 §8.2). (ADR-0008 §9; #25.)

**9.26** **A WGSL type ludo cannot name is a compile error naming the type and the
field** — never accepted-and-skipped. (ADR-0008 §9.)

**9.27** **The mismatch is reported at the `extern shader` declaration in ludo
source**, with the `.wgsl` file and its declaration named as the counterparty.
(ADR-0008 §9.)

**9.28** **A failed shader recompile retains the last-good pipeline, reports an
error value, and does not fault.** This is a full-conformance obligation; the rung
wording is chapter 8's. (ADR-0008 §10.)

---

## 10. The platform layer

**10.1** The following eight rules constrain the **platform-layer API**, which is
not specified here. They appear in this chapter because each either binds the
mandated surface above it or is observable through it. (ADR-0006, *scope*.)

**10.2 R1 — Nothing in the platform-layer API may block.** Every wait is a poll or
a host-driven callback. A blocking read is not permitted in loading either.
(ADR-0006 R1.)

**10.3** R1's asset case is resolved by the entry file's top level running to
completion **before any frame** (chapter 5). In-frame streaming is the deferred
case, not the default. (ADR-0006 R1; #26.)

**10.4 R2 — The backend seam is one struct of non-capturing function pointers.** A
loaded backend and a linked backend MUST be the same interface. (ADR-0006 R2.) The
language consequence — function pointers exist as a type, with no capture and no
closure — is chapter 2 §7. (ADR-0017.)

**10.5 R3 — There is no conditional compilation; a backend is a module.** Target
selection happens **at build**, by which modules the build includes, never inside a
function body. (ADR-0006 R3.) The module-set consequence is chapter 4 §10.

**10.6 R4 — The unavailable-backend error has the same signature on every
target.** A signature that differs by target is forbidden; a build whose path never
returns the error still declares it. A backend satisfies a declared nominal
interface, so a divergent signature is a **type error**, not reviewer discipline.
(ADR-0006 R4, as amended by ADR-0014.)

**10.7 R5 — Audio is pushed from the frame entry; nothing calls program code
back.** Mixing happens in the frame entry, on the main thread, in ordinary ludo
code, and the platform layer's audio API is push-only. **No ludo-visible
off-thread callback exists.** (ADR-0006 R5, confirmed and generalised by
ADR-0024.)

**10.8** Audio DSP spends frame budget: audio load and frame overrun are the same
budget. (ADR-0006 R5; #19 P7.)

**10.9 R6 — The audio API copies, reports, and never demands.** Its four clauses
are transcribed at §5.5.5 (copy, not a mapped region), §5.5.6 (the device reports
its rate; f32 interleaved), §5.7.1 (a monotonic play cursor in sample frames) and
§5.5.2 (the caller polls free space and fills it). (ADR-0006 R6.)

**10.10** Audio latency is a **ring depth the program chooses**, and is therefore a
visible number rather than a hidden backend property. (ADR-0006 R6.)

**10.11 R7 — File IO is request-and-poll, never a blocking read.** Compressed audio
is decoded by the implementation, not by the host's decoder, and an underrunning
stream outputs silence and reports (§5.7.6). (ADR-0006 R7.)

**10.12 R8 — Two runner guarantees.** **The ring buffer is runner-owned and
survives reload**, so **reload is click-free**. **After a fault the backend drains
what is queued, then outputs silence, never a loop.** Both are observable at the
**full** conformance rung, attaching to existing properties rather than creating
new ones; the rung and property wording are chapter 8's. (ADR-0006 R8; #19 P1,
P2.)

**10.13** R8 is the one place the unmandated audio tier reaches back into the
mandated contract, and it does so through the runner, which is mandated. Core
conformance is headless, so nothing here is imposed on an implementation that never
has a device. (ADR-0006 R8.)

---

## 11. Routed, dropped and authored

**11.1 Routed out of this chapter.** The following are named by this chapter's
sources and decided elsewhere: the frame entry and the delivery of `screen: !Target`
(chapter 5, ADR-0013); the fixed canvas and the fit (chapter 5, ADR-0030,
ADR-0032); asset declaration and resolution (chapter 5, ADR-0015); `#align(n)` as
a mechanism (chapter 3 §8.3); the operators on the blessed math types (chapter 2
§16.1); SIMD non-lowering and the float-evaluation rule (chapter 8, with chapter 2
§2.5); the conformance rungs, the property list and the #24 count (chapter 8);
function-pointer types (chapter 2 §7); and the target-selected module set (chapter
4 §10).

**11.2 Explicitly dropped.** This chapter transcribes no clause of its sources that
is **rationale about a rejected alternative** — the `fill_rect`-plus-image-paint
collapse (ADR-0010), the noun-first call set and the `{center, radius}` ellipse
(ADR-0009), the physical-key and layout-mapped key enums (ADR-0011), the
`render(v!, out:)` voice shape and the five per-parameter setters (ADR-0007), the
global voice cap and its `voice_capacity()` call (ADR-0007), the widened tolerance
and the published-measurement escapes (ADR-0022), the `opaque`/`none` `Blend`
variant (ADR-0034), the runner-adaptive scale (ADR-0038, whose *outcome* is §7.10),
the settings concept (ADR-0026, whose *outcome* is §8.17), the map-owns-the-IR and
map-owns-none forks (ADR-0008), and the comparative and cost-model statements of the
forwarding guarantee (ADR-0012, whose *outcome* is §2.4). Where a rejection produces
a rule a program can violate, that rule **is** transcribed — §4.10.2, §6.5.8,
§7.22, §2.7 and §2.8 are the cases.

**11.3 Authored here.** Nothing. Every clause above was located in a source. The
two spellings this chapter's sources leave to others — the loader that produces an
`Image` (§4.9.2) and the frame entry's parameter (§4.2.6) — are routed, not
authored.

**11.4 Marked gaps.** None. Where a source's clause was found to have been
withdrawn (§4.7.4, §4.11.7, §4.11.8), the withdrawal is transcribed as the rule,
which is what a reader needs.

**11.5** The source-by-source checklist discharging the closing test is
[`coverage/06-stdlib.md`](coverage/06-stdlib.md).
