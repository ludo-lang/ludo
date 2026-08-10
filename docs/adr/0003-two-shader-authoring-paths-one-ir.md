---
status: accepted
---

> **Amended by [ADR-0008](0008-the-shader-ludo-dialect-and-the-extern-shader-declaration.md):**
> the dialect this ADR left open is specified — a strict subset covering vertex and
> fragment only — and the *asset* framing is replaced by an `extern` shader declaration,
> because "asset" collided with issue #12's out-of-scope asset pipeline.

# Two shader authoring paths, one shader IR; WGSL ingested as an asset

ADR-0002 adopted the WebGPU model for the renderer API but said nothing about
shaders, which is a hole: the model carries WGSL, while D3D12 wants DXIL, Metal
wants MSL and Vulkan wants SPIR-V.

Shaders may be authored two ways. **Shader ludo** — a restricted dialect of the
language itself — is the path for someone who does not already know a shader
language. A **`.wgsl` file** is the path for someone who does; the compiler picks
it up directly. Both lower into **one shader IR** feeding one set of
SPIR-V/DXIL/MSL emitters.

The two paths differ in *authoring only*, never in guarantees or in capability.
That is the project's layering thesis applied to shaders, rather than a beginner
tier and an expert tier with different safety properties.

## The asset framing is load-bearing

A `.wgsl` file is an **asset**, in the sense that a `.png` is an asset — a format
the toolchain ingests. It is not a second ludo syntax and not a dialect. Held
that way, it costs nothing against #24's grammar budget and does not reopen
#16's *two modes of one language, not two languages*. Described the other way —
"ludo also accepts WGSL syntax" — it breaks both. See `CONTEXT.md`.

## Considered options

- **WGSL with a compiler of our own, no shader ludo.** Rejected: it costs the
  same three emitters as shader ludo *plus* a foreign frontend, where shader ludo
  reuses a frontend we must build regardless.
- **Shader ludo only.** Rejected on this ticket's originating point: a shader
  author already fluent in HLSL should not have to learn a new language to use
  the one they know how to write.
- **SPIR-V as a second ingestible asset format.** Tempting — it would reach HLSL
  and GLSL through existing offline compilers, and its reflection surface is
  better specified than WGSL text. Rejected because SPIR-V is a superset of what
  WebGPU permits, so accepting it means owning a validator for everything WebGPU
  forbids; and a `.spv` blob is unreadable, undiffable and undebuggable, which
  lands badly against both #4's failure modes and the reload loop.
- **Accepting HLSL or GLSL directly**, or invoking an external compiler (DXC,
  glslang) from the toolchain. Rejected: it breaks #19's single-command run and
  puts a C++ toolchain in the box, which is ADR-0001's rejected position.

## Consequences

- **A shader asset is interface-checked, not trusted.** The toolchain parses the
  WGSL *declaration surface* — entry points, bindings, types — and validates it
  against the ludo call sites' bind group layouts, vertex attributes and uniform
  struct layouts, rejecting mismatches at compile time. No expression semantics,
  no codegen; reflection only. An unchecked shader path would be precisely the
  hole #19 names as *no unchecked escape path*, in the place where silent
  mismatches are hardest to debug.
- **Shader ludo's ceiling is the shared IR's ceiling**, which is WebGPU's
  intersection — the same ceiling ADR-0002 already accepted CPU-side.
- **Shader ludo is a restricted dialect**, because GPU semantics are not CPU
  semantics. This is the real cost of this decision and it puts genuine pressure
  on #16's *two modes of one language* line. The dialect's exact restriction is
  not settled here.
- **Shaders fall inside the reload contract.** A saved shader edit is reflected
  under #19's stated bound, with `persist` state preserved — pipeline recreation
  touches no persistent state. A failed shader recompile is therefore a *runtime*
  event: it is an error value per #10 with a defined fallback (retain the
  last-good pipeline), and must **not** route through #18's error-as-pause.
- **Getting from HLSL or GLSL to WGSL is explicitly not our concern.** Transpilers
  exist, they run out-of-band, and their fidelity is not a property this project
  owns or documents. Recorded as a stated non-goal in the spec's front-matter,
  alongside #12's "why not an engine?", so it is not re-litigated.
