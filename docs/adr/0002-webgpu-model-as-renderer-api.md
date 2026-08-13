---
status: accepted
---

> **Amended by [ADR-0037](0037-a-backend-clears-a-floor-and-shaders-are-the-third-rung.md):** the four API
> names here are **non-normative evidence of tractability, not an admissible set** — issue
> #74 found the corpus had no statement of the admissible set and none is added, because a
> roster is the wording shape ADR-0019 §1 outlaws. The **ceiling stated below is scoped to
> the shader conformance rung**; the mandated facade is analytically specifiable and needs
> no GPU, so a facade-rung claimant is not bound by it. WebGL2 is consequently neither
> admitted nor refused, and "backend" here means the **renderer component**, not
> `CONTEXT.md`'s platform triple.

# The renderer API adopts the WebGPU model, implemented over native backends

ADR-0001 committed us to one platform-layer API with many backends, pinned to
the intersection of what every backend can do. The renderer API adopts the
**WebGPU model** — its resource, pipeline, bind-group and command-encoder shape
— and we implement it ourselves over D3D12, Metal, Vulkan and (deferred) WebGPU
proper.

WebGPU *is* that intersection, designed by the same vendors who ship the three
native APIs and already validated by shipping browsers. Adopting it converts our
hardest design problem — inventing a portable GPU abstraction — into an adoption
decision, and it makes the deferred web backend nearly free rather than a second
renderer.

**On desktop there is no browser and no WebGPU runtime.** We adopt the model;
our backends *are* the implementation, translating it into `MTLDevice`,
`ID3D12Device` and `vkCreateGraphicsPipelines` calls. Dawn and wgpu-native are
evidence the translation is tractable, not dependencies. The payoff is inverted
from what the name suggests: desktop does not get WebGPU — rather, the deferred
web backend becomes a thin pass-through instead of a second renderer.

**Shaders are covered by ADR-0003**, not here.

## Considered options

- **Vulkan everywhere, MoltenVK on macOS.** Rejected twice over: MoltenVK is a
  C++ dependency, which is the SDL-shaped dependency ADR-0001 declined by another
  name; and Vulkan does not exist on the web, so it fails the web-as-shaping-
  constraint rule outright.
- **Native per platform with an abstraction of our own invention.** Same four
  backends, but we would be designing the portable layer from scratch that
  WebGPU's working group already spent years designing.
- **A GLES3 baseline.** Cheapest and near-universal, but deprecated on macOS at
  4.1 with no compute shaders. A 2012 ceiling on a language shipping in 2027 or
  later.
- **Binding Dawn or wgpu-native.** This is ADR-0001's rejected "bind a framework"
  position wearing WebGPU's name, and it would hand the reload/pause half of #19
  back to a third party. We adopt the *model*; we do not link the implementation.

## Consequences

- The API ceiling is WebGPU's ceiling. Features the three native APIs expose but
  WebGPU does not (bindless, mesh shaders, raytracing) are unreachable through
  the portable surface, and reaching them means an escape hatch we have not
  designed.
- WebGPU is a versioned moving standard we do not control. Adopting the model
  means tracking it, and its stability obligations meet #19's.
