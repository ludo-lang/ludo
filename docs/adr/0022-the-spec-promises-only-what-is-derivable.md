---
status: accepted
---

# The spec promises only what is derivable, and says so where it cannot

Issue #58 inherited four falsified graphics clauses from [issue
#56](https://github.com/adamico/ludo/issues/56), whose findings are in
[`docs/research/07-browser-semantics-of-the-mandated-facade.md`](../research/07-browser-semantics-of-the-mandated-facade.md).
The research read the normative WebGPU and WGSL text and found that four
conformance promises this map had already frozen are **not true** — not merely
hard to implement, but false against a conforming implementation. It had no
standing to change the conformance surface. This ADR does.

## 0. The rule this ADR applies

Every clause below is settled by one rule, adopted here and stated once:

> **The spec promises only what is derivable from the specifications a backend is
> built on. Where a property cannot be derived, the spec says plainly that it is
> not guaranteed, rather than weakening it into a promise that sounds kept.**

The rule matters because two tempting escapes were on the table and both are
refused by it:

- **Widening a tolerance** until the falsifying case fits. The number is then
  invented rather than derived, and a conformance suite testing an invented
  number tests nothing.
- **Restating a promise as a measured-and-published figure** — #19's
  MUST-MEASURE posture, applied to reload latency and to #24's grammar budget.
  It was proposed here as a third application and is **declined**. Reload
  latency is a performance target where the honest answer really is a number
  from a machine; pixel exactness is a correctness property, and a published
  measurement of it is a promise wearing a lab coat. An implementation could
  resolve to visible garbage and conform by publishing a large figure.

What this costs is stated up front: the spec now promises **less** about drawing
than it did yesterday. What it buys is that everything it still promises is
true.

## 1. The pixel array: the guarantee is restated, and the signature does not change

ADR-0004 guarantees that upload cost is proportional to mutation. #56 found it
false on every backend, because ADR-0009's `get_pixels(image) -> []u32` hands
back a bare mutable view and no implementation can know which texels were
written.

#56 proposed adding a `region` parameter. **That is rejected**, on two grounds.

First, it misreads the primitive. `create_pixel_image` plus `get_pixels` is
DragonRuby's pixel array: an image the **program authors**, pixel by pixel, in
code. It is not a readback of a loaded asset — ADR-0009 says so, and the carve-out
exists only so ADR-0004's "a pixel array is an ordinary sprite forever" has
somewhere to live. For a program-authored image the common case is writing most
or all of it in the frame you draw it; a required rectangle taxes every use to
serve the rare one. Second, the proposed form was an optional argument defaulting
to the whole image, which ADR-0007 rule 6 — inherited verbatim by ADR-0009 —
forbids outright.

The correct reading supplies the mutation signal for free:

> **Taking the view is the signal.** A frame in which the program does not call
> `get_pixels` for an image causes no upload of that image. A frame in which it
> does causes at most one.

This is derivable, keepable, and proportional to mutation at frame granularity:
not writing means not asking. #8's transient non-escaping access already forbids
a view outliving its scope, so "held across frames" is not a case that arises and
needs no clause.

**`get_pixels(image) -> []u32` is unchanged.** Clause 1 therefore ceases to be a
signature change, and the criterion-4 note #58 anticipated is moot.

One obligation on backends survives, demoted from load-bearing to a floor on how
the whole-image upload is done:

> Backends must upload from a strided host array without repacking (WebGPU
> `writeTexture` with unaligned `bytesPerRow`; WebGL2 `UNPACK_ROW_LENGTH` /
> `UNPACK_SKIP_PIXELS` / `UNPACK_SKIP_ROWS`). **WebGL1 cannot satisfy this and is
> excluded as a backend.**

This is a requirement on an implementer, not a property a program observes, so
§0's rule permits it.

## 2. `crisp` is not bit-exact, and the headline claim is deleted

ADR-0005 states the conformance rule as a sentence about the style token — *"a
`crisp` target is bit-exact across the whole backend matrix"*. #56 falsified it
in three citable places: WebGPU declines a top-left fill rule and leaves
pixel-centre-on-edge undefined; WGSL specifies no rounding mode and permits free
reassociation; float-to-unorm store is implementation-defined.

**That sentence is deleted, not caveated.** Leaving a false headline with a
footnote beneath it is the shape an agent reads wrong, and the true statement is
about a **geometry class**, not about a style token. `crisp` reverts to being
what it always was operationally: no anti-aliasing, nearest sampling.

The replacement:

> On a `crisp` target, interior and exterior pixels are exact everywhere.
> Boundary pixels are exact for axis-aligned geometry whose edges lie on integer
> framebuffer coordinates, because no pixel centre lies on such an edge. For
> geometry with an edge passing through a pixel centre, coverage is
> **implementation-defined**. Bit-exactness across backends additionally requires
> that all transform arithmetic be exactly representable in `f32`; identity and
> integer translation satisfy this, arbitrary scale and rotation do not.

The ordinary pixel-art program stays fully conformance-testable. The spec stops
promising an exactness no backend delivers for a rotated ellipse.

## 3. The `smooth` coverage tolerance is deleted with no replacement number

ADR-0005 bounds an implementation's computed coverage against the true analytic
coverage. The coverage framing is right and is kept. **The bound is not
derivable and never was:** the MSAA resolve is entirely unspecified, and WebGPU
contradicts itself on whether sample positions are standard or
implementation-defined. Any number here is invented.

So the number goes, and nothing numeric replaces it — no widened tolerance, no
published measurement. In its place stands the part that **is** derivable, which
is the same interior/exterior split §2 uses:

> On a `smooth` target, fully-covered pixels are exact and fully-uncovered pixels
> are untouched. The value of a partially-covered boundary pixel is
> **implementation-defined**.

`smooth` conformance therefore keeps real pass/fail content — an implementation
that resolves to garbage fails on the interior — while the spec stops asserting a
boundary bound it cannot support.

**Non-normative note for whoever writes the conformance suite**, recorded because
it will otherwise be rediscovered painfully: coverage is not directly observable
through a browser API. The suite reads back rendered pixels and must *infer*
coverage from colour, so tests must draw with known, maximally separated colours
for that inversion to be well conditioned.

## 4. The byte-exact blit holds, with its condition stated — and the tint's does not

This is the one clause that survives, and #56's reasoning for why is exactly the
reasoning for why §2's general claim does not. At `sampleCount: 1` with `nearest`
filtering (both WebGPU defaults), no blending, an integer-aligned quad and an
identity or integer-translation transform: every arithmetic input is exactly
representable in `f32`, so the unspecified rounding mode and permitted
reassociation are inert; the quad's edges lie on integer framebuffer coordinates
while pixel centres sit at `fract(C) = (0.5, 0.5)`, so no pixel centre lies on an
edge and the undefined tie is never reached; at count 1 the sample position is
normatively `(0.5, 0.5)`; and nearest sampling at exact texel centres never
reaches the unspecified texel-boundary tie.

One residual gap remains — WebGPU §23.2.7's implementation-defined float-to-unorm
store — and it vanishes only if the blit is a copy rather than a draw. So the
condition is stated rather than left to be rediscovered:

> Backends must implement this case as a **texture-to-texture copy**, not as a
> rasterised quad. A rasterised quad is permitted only where the backend
> additionally pins float-to-normalised store rounding, which WebGPU §23.2.7
> leaves implementation-defined.

ADR-0010's restatement in paint vocabulary — a `stretch` texture paint at 1:1
device extent, integer coordinates, unrotated, `crisp` — is unaffected and
remains the spelling of this guarantee.

**What is withdrawn:** ADR-0010 also claims byte-exactness for `fill_sprite` with
a **colour** paint — the hit flash. A tinted sprite multiplies in a shader and
therefore cannot be a texture-to-texture copy; it rides the rasterised path and
inherits §23.2.7. **That claim is withdrawn.** The hit flash still works on every
backend and remains a mandated call; it is no longer promised byte-identical
across them. Neither ADR-0005 nor ADR-0010 currently distinguishes the copyable
case from the drawn one, and this is the distinction.

## 5. Two riders

**`copyExternalImageToTexture` is forbidden.** It is not a byte copy — it
performs colour encoding into the destination encoding, and for a 2d-canvas
source WebGPU §3.11 pins alpha as *"always premultiplied"* rather than optional,
with out-of-gamut values explicitly undefined on a visible canvas. It is the only
WebGPU upload path whose output bytes are not a function of its input bytes
alone. ADR-0005 already requires a web backend to decode images itself and upload
raw pixels; this names the specific mechanism that rule excludes, and justifies
it better than ADR-0005 does.

**Reload is defined in ADR-0007** as *"the in-process swap of game code; not a
restart of the host process or document."* Without it, ADR-0007's audio
play-cursor clause is vacuous under one reading of the word.

## 6. Against issue #24: the count is unchanged

Nothing here adds a keyword, an operator, a facade call or a stdlib root name.
§1 rejected the only proposed signature change. Recorded as a data point: **a
conformance re-cut can be free against the grammar budget**, because the budget
counts surface and this ADR changed only what the surface promises.

## Amendments this ADR makes

- **ADR-0004** — the upload guarantee is restated at frame-and-view granularity
  (§1); the backend floor is added and WebGL1 excluded.
- **ADR-0005** — the `crisp` bit-exactness sentence is **deleted** and replaced
  (§2); the coverage tolerance is **deleted** with no numeric replacement (§3);
  the blit's implementation condition is added (§4); `copyExternalImageToTexture`
  is named as forbidden (§5).
- **ADR-0007** — reload is defined (§5).
- **ADR-0010** — the `stretch`-paint blit restatement stands; the **colour-paint
  hit flash's byte-exactness is withdrawn** (§4).
- **ADR-0009** — unchanged. `get_pixels` keeps its published signature.

## How it fares on the three lenses

- **Simplicity.** Nothing new to learn, and one fewer thing: no rectangle
  parameter on the pixel array, no tolerance number to look up. The promises a
  beginner meets are now the ones that hold.
- **Robustness.** Net loss in stated guarantees, net gain in true ones. The
  interior/exterior rule is checkable on both style tokens, which is more
  conformance content than a tolerance nobody could derive.
- **Agent-friendliness.** The direct win: an agent that reads *"a `crisp` target
  is bit-exact"* and writes a test asserting it produces a test that fails on
  conforming implementations. Deleting the sentence deletes that failure mode.
  Every remaining claim names its condition in the same sentence, which is
  criterion R1 applied to prose.

**Where the lenses conflict:** robustness against itself. Fewer promises reads as
a weaker spec on a feature comparison, and is a stronger one in practice. The
trade is named rather than hidden: this map would rather ship a small true
conformance surface than a large aspirational one.
