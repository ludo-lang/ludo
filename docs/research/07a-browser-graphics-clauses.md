# Browser semantics of the mandated graphics facade

Research for [#56](https://github.com/adamico/ludo/issues/56) — the graphics half.
Checks the two frozen graphics clauses ADR-0004 explicitly parked ("the
pixel-array upload guarantee and the AA tolerance both need checking against
browser semantics before they freeze") against the WebGPU, WGSL and WebGL2
specifications.

The obligation being checked is **negative**: the spec does not mandate a web
target, but nothing in the mandated facade may be *unimplementable* in a browser.

## Verdicts

| Clause | Source | Verdict |
| --- | --- | --- |
| 1. Pixel-array upload cost proportional to mutation | ADR-0004, ADR-0009 | **Divergent** — the browser mechanisms are all present and normative; the *facade* cannot supply the mutation signal they need |
| 2. Coverage tolerance on `smooth` targets | ADR-0005 | **Divergent, softly** — the right *shape*, but no floor is derivable: the MSAA resolve is entirely unspecified |
| 2b. `crisp` is bit-exact across the whole backend matrix | ADR-0005 | **Divergent** — three named holes. Not a clause the ticket listed; the more serious of the two findings |
| 2c. The byte-exact blit | ADR-0005, ADR-0010 | **Supplied**, with one condition on how a backend implements it |

The headline: both named clauses need edits, and the AA investigation ran into a
*third* clause — `crisp` bit-exactness — that is stated more strongly than any GPU
backend, web or native, can deliver.

---

## Clause 1 — upload cost proportional to mutation

### What the spec says today

ADR-0004:

> **Pixel arrays are first-class, with a performance guarantee in the contract:**
> upload cost is proportional to *mutation*, not to *use*.

ADR-0009 attaches that guarantee to a specific shape:

> `create_pixel_image` and `get_pixels` are: a computed `Image` and a mutable
> `[]u32` view into it, with ADR-0004's upload-cost-proportional-to-mutation
> guarantee attaching to the view.

### The browser supplies every mechanism, normatively

`GPUQueue.writeTexture` is specified as a triple loop bounded strictly by
`copySize`, writing texel blocks from `dstBlockOriginX/Y`
([WebGPU §19.2](https://www.w3.org/TR/webgpu/#dom-gpuqueue-writetexture)). **A
partial-region write is normatively specified as touching only that region** — no
texel outside `origin + copySize` is written, and no whole-texture floor appears
in validation.

The load-bearing detail is alignment. `GPUTexelCopyBufferInfo` validation
requires `bytesPerRow` to be a multiple of 256
([§11.2.2](https://www.w3.org/TR/webgpu/#abstract-opdef-validating-gputexelcopybufferinfo)),
but that step applies to `copyBufferToTexture`/`copyTextureToBuffer` only.
`writeTexture` carries an explicit normative note to the contrary:

> "Note: unlike `GPUCommandEncoder.copyBufferToTexture()`, there is no alignment
> requirement on either `dataLayout.bytesPerRow` or `dataLayout.offset`."

Combined with the texel-block offset formula
`blockOffset = (x × blockBytes) + (y × bytesPerRow) + offset`, this means **a
narrow sub-rectangle of a wide CPU array uploads with no repacking**: set
`offset = y₀ × stride + x₀ × bpp`, `bytesPerRow = stride`, `size = {w, h}`,
`origin = {x₀, y₀}`. This was the stated design intent, not an accident —
"with writeTexture, you don't need to require the bytesPerRow alignment that
copyBufferToTexture has […] It's a linear staging allocator"
([gpuweb WG minutes, 2020-05-18](https://github.com/gpuweb/gpuweb/wiki/GPU-Web-2020-05-18)).

WebGL2 supplies the same thing through `UNPACK_ROW_LENGTH` / `UNPACK_SKIP_PIXELS`
/ `UNPACK_SKIP_ROWS`: "Subrect selection is possible using UNPACK_ params"
([WebGL 2.0 §5](https://registry.khronos.org/webgl/specs/latest/2.0/)). These are
absent from WebGL1, which inherits only ES 2.0's pixel-store set — **WebGL1
cannot satisfy this guarantee at all** and is not a viable backend for it.

No primary-source evidence was found for driver-level whole-texture reupload on
`texSubImage2D`. Treat that as folklore.

### But no spec bounds *cost* — and the facade cannot supply the mutation signal

Two separate gaps, and the second is the fatal one.

**First**, no backend spec makes cost claims. A sweep of the WebGPU editor's
draft for performance language turns up only `powerPreference`, a colour-space
elision `should`, a `viewFormats` note, and §2.2.2/§2.2.3 on machine-specific
differences — which explicitly treat such timing as intractable and out of scope.
The one note attached to `writeTexture` is a `should` about the *JS-side*
ArrayBuffer snapshot ("Implementations should optimize by copying only the
necessary bytes"), not the GPU-side transfer. So the guarantee is not derivable
from any backend specification; it can only be an obligation the ludo spec places
*on* backends.

**Second, and decisive**: `get_pixels` hands back a plain mutable `[]u32` view,
and the program writes into it with ordinary stores. **Nothing in the facade tells
the implementation which texels were written.** At the point of use the
implementation has three options:

1. **Diff against a shadow copy** — cost proportional to the array's *size*, not
   the mutation. The guarantee is inverted, and the shadow copy doubles memory.
2. **Upload the whole array** — cost proportional to size. The guarantee is false.
3. **Let the view alias memory the GPU reads directly**, so no upload happens and
   the cost genuinely is the mutation.

Option 3 is the only one that makes the guarantee true, and **a browser forecloses
it absolutely**. WebGPU has no texture mapping at all: `mapAsync` exists only on
`GPUBuffer`, needs `MAP_READ`/`MAP_WRITE` declared at creation, is asynchronous,
and "while the `GPUBuffer` is mapped it cannot be used in any GPU commands"
([MDN, `GPUBuffer.mapAsync`](https://developer.mozilla.org/en-US/docs/Web/API/GPUBuffer/mapAsync)).
A persistently-mapped, GPU-readable, CPU-writable texture does not exist in the
web platform.

Note the scope: **options 1 and 2 are equally forced on a native backend.** Vulkan
and Metal can persistently map a *buffer*, and a linear-tiled image on some
drivers, but the facade's `Image` is a sampled texture drawn by `draw_sprite`, and
a persistently-mapped optimally-tiled texture is not a thing there either. The
browser check surfaced a defect in the clause itself rather than a web-specific
divergence.

### Narrowest edit that closes it

Two changes, both small.

**A — give the implementation the mutation signal the guarantee presupposes.**
The smallest form that adds no call:

> The mutable view is obtained for a region — `get_pixels(image, region)` — and
> the implementation may upload exactly that region when the view is released.

One added parameter on an existing call, reusing `Rect`, which ADR-0009 already
has; the whole-image case is the default argument. The alternatives are worse: an
explicit `commit_pixels(image, region)` adds a call to a facade whose size #24
charges for and admits the forget-to-commit bug; runtime dirty-rect tracking needs
write barriers on a `[]T` view, which #15's boundary type does not have and must
not grow.

**B — restate the guarantee as work issued, not cost incurred**, and name the
backend floor:

> The facade issues upload work proportional to mutation, not to use: a written
> sub-rectangle causes at most one upload of that sub-rectangle; an unwritten
> array causes no upload; repeated draws without writes cause no upload. Backends
> must provide sub-rectangle upload from a strided host array without repacking
> (WebGPU `writeTexture` with unaligned `bytesPerRow`; WebGL2
> `UNPACK_ROW_LENGTH`/`UNPACK_SKIP_PIXELS`/`UNPACK_SKIP_ROWS`). This is an
> obligation on the backend, not a property derivable from any backend
> specification.

Weakening the clause to a non-guarantee is not available: ADR-0004's own rationale
forbids it — "a pixel array whose performance is unguaranteed is a trap rather
than a primitive."

### Bonus finding: forbid `copyExternalImageToTexture`

Not asked, but it falls out and is spec-justified. `copyExternalImageToTexture` is
not a byte copy — it is a per-pixel colour-managed conversion ("This operation
performs color encoding into the destination encoding"), and for a 2d-canvas
source WebGPU §3.11 pins alpha as **"Always premultiplied"**, not optional. §3.11
also defines the out-of-gamut hazard: "when such values are output to a visible
canvas, the result is undefined." It is the only WebGPU upload path whose output
bytes are not a function of the input bytes alone. ADR-0005's existing rule that
"the web backend must decode images itself and upload raw pixels" is therefore
better justified than it states.

---

## Clause 2 — the coverage tolerance on `smooth` targets

### What the spec says today

ADR-0005:

> The tolerance is on **coverage, not colour**: an implementation's computed
> coverage must fall within a stated bound of the true analytic coverage. […] It
> applies only to `smooth` targets.

### The shape is right

Coverage is the correct quantity, and ADR-0005 reasoned its way to the one
tolerance a browser could meet. WebGPU is explicit that this is what varies:

> "There are some machine-specific rasterization/precision artifacts […] This
> applies to rasterization coverage and patterns, interpolation precision of the
> varyings between shader stages"
> — [WebGPU §2.2.2](https://www.w3.org/TR/webgpu/) (non-normative)

More is pinned than expected. **Standard sample positions are normative** — §23.2.5
gives the count-1 position as (0.5, 0.5) and the count-4 pattern explicitly, and
states "Implementations **must** use the standard sample pattern for the given
`multisample.count` when performing rasterization." That is stronger than Vulkan,
where standard locations are an optional feature. Sample counts are restricted to
1 or 4 ([§10.3.3](https://www.w3.org/TR/webgpu/#multisample-state)).

### But no bound is derivable — two cracks

**The MSAA resolve is entirely unspecified.** §17.1.1, in its entirety:

> "If `colorAttachment.resolveTarget` is not null: Resolve the multiple samples of
> every texel of `colorSubregion` to a single sample and copy to
> `colorAttachment.resolveTarget`."

No weighting, no filter, no box-average requirement, no statement about
sRGB-linearity. A `smooth` target's boundary pixels are bounded by nothing the
spec provides.

**And the spec contradicts itself on sample positions.** Against §23.2.5's `must`,
§23.2.5.4 under `multisample: enabled` says the opposite: "Each pixel is
associated with `descriptor.multisample.count` locations, which are
**implementation-defined**." A facade leaning on standard positions should take
§23.2.5.4 as the pessimistic reading, or file the inconsistency upstream.

### Narrowest edit

Keep the coverage framing — it is right — but stop implying the bound is derived:

> Delete any numeric tolerance that claims specification derivation. State the
> bound as an empirical property of tested backends, measured and published, not
> as a number the backend specifications imply.

This is #19's MUST-MEASURE posture, which the map has already adopted for reload
latency and for the grammar budget (#24), applied a third time. That consistency
is the argument for it.

**Conformance-suite note** (out of scope here, but it will bite): coverage is not
directly observable through a browser API. The suite reads back rendered pixels
and must *infer* coverage from colour, so tests must draw with known,
maximally-separated colours for the inversion to be well-conditioned.

---

## Clause 2b — `crisp` is bit-exact across the whole backend matrix

Not a clause the ticket listed. It is what the AA investigation ran into, and it is
stated more strongly than the tolerance clause it is paired with.

ADR-0005:

> the conformance rule [is] a sentence about one named thing — *a `crisp` target
> is bit-exact across the whole backend matrix*

ADR-0004 states the same as the conformance predicate: "Interior and exterior
pixels must match exactly; boundary pixels may differ within a stated tolerance
**only** for targets that opted into AA."

### Verdict: divergent, in three citable places

`crisp` covers the whole facade, not just sprites. On a `crisp` target the
mandated primitives still include `fill_rect`, `fill_ellipse` and `stroke_line` at
**arbitrary non-integer logical coordinates**, under an **arbitrary target
transform** (ADR-0005 mandates offset, scale and rotation). Three holes:

**1. There is no top-left fill rule.** WebGPU deliberately declines D3D's, and
leaves the tie undefined — §23.2.5.4:

> "If a pixel center is on the edge of the polygon, whether or not it's included
> is **not defined**."
> "Note: this becomes a subject of precision for the rasterizer."

**2. WGSL specifies no rounding mode, and permits reassociation.** The accuracy
table (§15.7.4.1) is better than feared for the operations a textured quad uses —
`+`, `-`, `*` and `-x` are all **correctly rounded**, comparisons give the correct
result, only `/` carries 2.5 ULP. But "correctly rounded" is weaker than IEEE-754
here (§15.7.4):

> "the smallest value in T greater than x, or the largest value in T less than x.
> That is, the result may be rounded up or down: **WGSL does not specify a
> rounding mode.**"

So two conforming implementations may differ by 1 ULP on any `+`/`-`/`*` whose
exact result is not representable in f32. And §15.7.5: "An implementation **may
reassociate** operations", unconditionally — so a 4×4 matrix-vector transform is
not bit-reproducible in general, since `dot()`'s accuracy is inherited from an
unparenthesised sum that may be freely reordered.

**3. The float→unorm store is implementation-defined.** §23.2.7 Output Merging
ends with "Set the value of attachment at fragment.destination to color" and gives
no conversion rule from `vec4<f32>` to e.g. `rgba8unorm`. The one place the spec
comments on it (§3.12) says: "**For non-integer types, the exact choice of value
is implementation-defined.**" A shader emitting exactly `1.0/255.0` is not
guaranteed to store `1`, and the facade cannot close this through pipeline
configuration.

Any rotated or scaled sprite, or any fractional position, reaches all three.

### Narrowest edit

Scope the exactness claim to the geometry class that supports it, rather than
widening the tolerance:

> On a `crisp` target, interior and exterior pixels are exact everywhere. Boundary
> pixels are exact for axis-aligned geometry whose edges lie on integer
> framebuffer coordinates, because no pixel centre lies on such an edge. For
> geometry with edges passing through a pixel centre, coverage is
> implementation-defined. Bit-exactness across backends additionally requires that
> all transform arithmetic be exactly representable in f32; identity and integer
> translation satisfy this, arbitrary scale and rotation do not.

This keeps the conformance rule a sentence about one named thing, keeps the
ordinary pixel-art program fully testable, and stops the spec promising an
exactness no backend can deliver for a rotated ellipse.

---

## Clause 2c — the byte-exact blit

ADR-0005 / ADR-0010:

> a sprite drawn at integer logical coordinates, unrotated, under an identity or
> integer-translation transform, on a `crisp` target, is a byte-for-byte blit.

### Verdict: supplied — with one condition

This one holds, and the reasoning is worth recording because it is exactly why the
*general* `crisp` claim does not. With `sampleCount: 1`, `nearest` min/mag
filtering (both are the WebGPU defaults,
[§7.1.1](https://www.w3.org/TR/webgpu/#GPUSamplerDescriptor)), no blending, an
integer-aligned quad and an identity or integer-translation transform:

- every arithmetic input is exactly representable in f32, so the unspecified
  rounding mode and permitted reassociation are both **inert**;
- the quad's edges lie on integer framebuffer coordinates while pixel centres sit
  at `fract(C) = (0.5, 0.5)` (§23.2.5.4), so **no pixel centre ever lies on an
  edge** and §23.2.5.4's undefined tie is never reached;
- at count 1 the sample position is normatively (0.5, 0.5), so there is no
  position ambiguity;
- nearest sampling at exact texel centres is unambiguous, so the unspecified
  texel-boundary tie is never reached either. (WebGPU defines `nearest` only as
  "Return the value of the texel nearest to the texture coordinates" and points at
  Vulkan's texel filtering in a **non-normative** note — so Vulkan's
  `i = floor(u)` convention is *not* imported, and the exact-half-texel tie is
  genuinely unspecified. The blit does not hit it.)

The one residual gap is §23.2.7's implementation-defined float→unorm store — and it
vanishes if the blit is implemented as `copyTextureToTexture` rather than as a
draw, which is arguably what "byte-for-byte blit" should mean anyway.

### Suggested edit

Make that implementation condition explicit rather than leaving it to be
rediscovered:

> Backends must implement this case as a texture-to-texture copy, not as a
> rasterised quad; a rasterised quad is permitted only where the backend
> additionally pins float-to-normalised store rounding, which WebGPU §23.2.7
> leaves implementation-defined.

---

## Sources

- [WebGPU — W3C](https://www.w3.org/TR/webgpu/) — §§2.2.2, 3.11, 3.12, 7.1.1, 10.3.3, 11.2.1, 11.2.2, 17.1.1, 19.2, 23.2.5, 23.2.5.4, 23.2.7
- [WebGPU editor's draft](https://gpuweb.github.io/gpuweb/)
- [WebGPU Shading Language — W3C](https://www.w3.org/TR/WGSL/) — §§8.2, 15.7.4, 15.7.4.1, 15.7.5
- [WebGL 2.0 Specification — Khronos](https://registry.khronos.org/webgl/specs/latest/2.0/)
- [gpuweb WG minutes, 2020-05-18](https://github.com/gpuweb/gpuweb/wiki/GPU-Web-2020-05-18)
- [MDN — `GPUQueue.writeTexture()`](https://developer.mozilla.org/en-US/docs/Web/API/GPUQueue/writeTexture) (agrees with the spec; no divergence)
- [MDN — `GPUCommandEncoder.copyBufferToTexture()`](https://developer.mozilla.org/en-US/docs/Web/API/GPUCommandEncoder/copyBufferToTexture)
- [MDN — `GPUBuffer.mapAsync()`](https://developer.mozilla.org/en-US/docs/Web/API/GPUBuffer/mapAsync)
