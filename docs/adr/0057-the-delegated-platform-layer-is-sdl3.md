---
status: accepted
---

# The delegated platform layer is SDL3, and the browser renders through its 2D API

[ADR-0056](0056-the-platform-layer-is-delegated-to-a-third-party-library.md)
delegated the platform layer to a third-party library and deliberately left the
library open, owing the choice to
[#132](https://github.com/ludo-lang/ludo/issues/132). **The library is SDL3.**
Only SDL3: no second SDL major, and no other platform library beside it.

The evidence is
[`docs/research/09-which-library-the-delegated-platform-layer-uses.md`](../research/09-which-library-the-delegated-platform-layer-uses.md),
which scores SDL3, raylib and sokol against the eight rules of spec ch6 §10 rule
by rule. This ADR records the decision and the costs; the research holds the
citations and, importantly, the list of what could not be verified.

## 1. raylib fails the rules, and fails them in its own source

Not a matter of taste. Chapter 6 §10's rules are the test, and raylib's own code
answers three of them.

- **R1 — nothing in the platform-layer API may block.** `rcore.c`'s
  `EndDrawing()` calls `WaitTime()`, which calls `Sleep`/`nanosleep`/`usleep`;
  `LoadFileData()` is `fopen` plus `fread`. The escape hatch,
  `SUPPORT_CUSTOM_FRAME_CONTROL`, is **disabled by default** and its own
  documentation says *"use at your own risk"*. A rule satisfied only by a
  non-default build is not satisfied.
- **R5 — push-only audio, nothing calls back.** `raudio.c`'s own field comment
  describes callbacks *"for buffer filling on audio threads"*, with mixing on
  miniaudio's device thread under a mutex.
- **R6 — copies, reports, never demands.** No play cursor, and no free-space
  count in frames.

These follow from raylib sitting **one rung above the platform layer**, which is
the same fact that makes it collide with the mandated drawing facade and carry
engine-tier API that [#12](https://github.com/ludo-lang/ludo/issues/12) excludes.
It is the wrong tier, and the rule failures are the measurement of that.

## 2. SDL3 clears the rules through its supported API

`SDL_AsyncIO`, documented to never block, answers R7. Audio with a NULL callback
is push-only, answering R5. Swapchain acquisition is documented non-blocking. The
library sits **below** the mandated facade, so ADR-0004's facade and ADR-0006's
rules land on top of it without competing with anything it already offers. It is
**Zlib**, as this repository is.

R3, R4 and R8 are obligations on **our** code; no library supplies them. What a
library can do is put them out of reach, and SDL3 does not.

## 3. The browser: one program shape, two renderer components

Emscripten is a first-class SDL3 platform — `docs/README-emscripten.md` and
`docs/INTRO-emscripten.md` in `libsdl-org/SDL`. SDL3's entry-point document states
the browser *"absolutely requires"* the main callbacks **and that the same
callbacks work everywhere else**, so there is one program shape across native and
web. That is what R3 wants, supplied by the library rather than worked around.

**SDL_GPU does not reach the browser today.** Its WebGPU backend exists only as
pull requests against `libsdl-org/SDL`, and their status was checked rather than
assumed (2026-08-18): #12046 **closed unmerged**, #15722 **closed unmerged**,
#16020 **open and active that day**, with feature request #10768 open. Two
attempts closed without landing; a third is in progress.

So the web renders through **SDL's 2D render API on WebGL**, and desktop renders
through SDL_GPU. **These are two renderer components, not one model with two
implementations** — precisely what ADR-0002 adopted the WebGPU model to avoid. It
is accepted here because [ADR-0006](0006-forbidden-and-required-shapes-of-the-platform-layer-api.md)
makes the web a **shaping constraint with the backend deferred**: nothing ships on
it, and the constraint is on the API's shape, which SDL3 satisfies. **The cost is
time-dependent** — #16020 landing erases it — so this clause is to be re-checked,
not cited.

## 4. Falling back to SDL2 for the browser is rejected

Considered, because the browser gap is real and Emscripten has bundled an SDL2
port for years while SDL3's browser support is newer. Rejected, because it does
not do what it appears to do.

- **SDL2 has no GPU API.** `SDL_gpu.h` is in `include/SDL3`. Falling back to SDL2
  cannot restore a GPU path; it lands on a 2D renderer over WebGL — **the same
  tier SDL3 already provides in the browser**. The fallback buys nothing it does
  not already have.
- **It breaks R3.** *A backend is a module, not an `#if`.* Two libraries with two
  ABIs is that `#if` raised to the dependency level, and R3 is one of the rules
  SDL3 was chosen for satisfying.
- **It forfeits the one-program-shape property** of §3, which is a principal
  reason SDL3 won.

The genuine argument underneath it — SDL3's Emscripten maturity relative to SDL2's
— is an argument for **measuring** that maturity when the web backend stops being
deferred. It is not an argument for shipping two SDL majors.

## 5. What this costs, stated rather than discovered later

- **R6's play cursor is not supplied directly** by any candidate; it is derived.
  Whether the derivation is tight enough for ch6 §5.7.1's frame-precise sync
  **needs measurement, not reading**, and is unresolved.
- **A dependency we do not control** now sits under the experience contract.
  ADR-0001 called this unenforceability; ADR-0056 §3 established that it is not,
  because the runner keeps the frame loop. What remains true is narrower: a bug
  inside SDL3 is not ours to fix on our schedule.
- **The desktop/web renderer split** of §3, for as long as it lasts.

## 6. sokol is the alternative, not raylib

Recorded so a future reconsideration starts from the right place. sokol survives
every rule check and has the **only first-party WebGPU backend found in a released
tree** (`SOKOL_WGPU` beside `SOKOL_GLES3`). It was not chosen because its
callback-only entry point, smaller ecosystem, and the **unverified** maturity of
its WGPU and WebAudio paths make it a larger bet on less evidence — not because
anything disqualified it.

**If the desktop/web renderer split of §3 becomes decisive, sokol is what to
re-examine.** raylib is not, at any point: its failures are tier failures and no
change of circumstance repairs them.

## 7. Amendments this ADR makes

- **[ADR-0002](0002-webgpu-model-as-renderer-api.md)** — the renderer API adopts
  the WebGPU model **in part**. Resource, pipeline object and command
  encoder/pass survive as SDL_GPU's shape too. **The bind-group shape does not**:
  SDL_GPU binds per stage per slot and pushes uniforms. The **API ceiling changes
  owner** — it becomes SDL_GPU's ceiling, close to WebGPU's but set by a library
  we do not control and with no stated commitment to track WebGPU. And the payoff
  clause — *"the deferred web backend becomes a thin pass-through instead of a
  second renderer"* — is **withdrawn while SDL_GPU has no browser backend**.
- **[ADR-0056](0056-the-platform-layer-is-delegated-to-a-third-party-library.md)
  §8** — the owed library decision is discharged. Nothing else in ADR-0056 moves;
  its §4 list of what survives the ADR-0001 reversal is unchanged.

The mandated facade ADRs, ADR-0006's eight rules and ADR-0037's admission test are
what this ADR is measured against, not what it changes.
