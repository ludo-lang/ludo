---
status: accepted
---

# The platform layer is delegated to a third-party library, reversing ADR-0001

[ADR-0001](0001-own-the-platform-layer.md) decided that we bind the OS and GPU
APIs directly — Win32, Cocoa, X11/Wayland, Vulkan, Metal, D3D12, WASAPI,
CoreAudio, ALSA/PipeWire — and write the platform layer ourselves in ludo on top
of those externs, rather than binding SDL or GLFW. **That is reversed.** ludo
delegates the platform layer to an existing third-party library. Which library is
open and is decided by [#132](https://github.com/ludo-lang/ludo/issues/132).

## 1. The goal ADR-0001 was measured against was never stated in it

The project's goal is a **programming language for game developers**, and it does
not include owning OS interaction. ADR-0001 never claimed otherwise — it argued
from the experience contract, not from ambition — but its consequences amount to
an eighteen-backend platform library, and nothing in the corpus ever weighed that
against the goal it serves. This ADR performs that weighing.

`CONTEXT.md` makes the distinction the argument turns on. An **engine** — frame
loop, component model, sprites, animation, collision, camera — is ruled out by
[#12](https://github.com/ludo-lang/ludo/issues/12) and stays ruled out. A
**platform layer** is the triple of window/input, renderer and audio device.
SDL3 and raylib sit at the second tier. *Not building an SDL* is therefore a
statement about the platform layer and not about the engine, and the two are not
in tension.

## 2. The spec does not require ownership, and never did

This is the finding that makes the reversal cheap, and it was already on record
in three places before this ADR was written.

- **Chapter 6 §10.1**: *"The following eight rules constrain the platform-layer
  API, **which is not specified here**."* Eight constraints, no API.
- **[ADR-0004](0004-a-mandated-drawing-facade-over-a-delegated-renderer.md)** is
  titled *a mandated drawing facade over a **delegated** renderer*. The facade is
  mandated; the tier under it is delegated.
- **[ADR-0055](0055-absorption-has-a-complement-and-status-md-is-deleted.md)'s
  absorption audit** found ADR-0001 and ADR-0002 **unabsorbable** — they decide an
  unmandated surface no chapter transcribes.
  `docs/spec/coverage/08-conformance.md` records it in as many words.

ADR-0001 says it itself: *"building the platform layer does not mandate it in the
spec, and mandating it would require every conforming implementation to ship a
renderer across four backends."* Ownership was always orthogonal to the spec.

**Consequently this reversal changes no normative clause.** A conforming
implementation satisfies chapter 6 by making the mandated facade behave as
specified and honouring the eight rules. How it reaches the screen is its own
business, and always was.

## 3. Why ADR-0001's own argument does not hold

ADR-0001's grounds are [#19](https://github.com/ludo-lang/ludo/issues/19): state-
preserving reload and error-as-pause each need two halves, the second half is a
platform-layer responsibility, and *"if that layer is a third-party C library we
do not control, the contract is unenforceable."*

**The frame loop is not the library's.** Binding SDL3, the runner still decides
when to step, when to stop stepping, when to re-present the last image and when to
keep pumping events. The half ADR-0001 says is lost is not lost. What a
third-party layer actually costs is the ability to fix a bug *inside* it — real,
and a different claim from unenforceability.

**The conformance matrix survives delegation, and shrinks.** ADR-0001 correctly
identifies the true cost as *"every backend must satisfy the experience contract
identically, on real hardware, in CI — that is the actual cost, not the code."*
Delegating does not remove that obligation. It hands the per-platform half to a
library that has already discharged it on more platforms than this project will
ever test, and leaves us the part that is ours: that the runner behaves
identically on top.

**The rejected alternatives argue for this one.** ADR-0001 rejects *bind nothing
above libc* as **the Beef failure mode** — *"a decade of full-time work with
nothing shipped."* Eighteen hand-written backends is that failure mode one rung
up, and ADR-0001 lists it under Consequences as a cost accepted rather than under
Considered options as a cost weighed.

## 4. What does not change

Recorded so the reversal is not read wider than it is.

- **The mandated facade.** ADR-0004's drawing surface, ADR-0007's audio surface,
  ADR-0009 and ADR-0010's spellings — untouched. They are chapter 6's, they are
  normative, and delegation is invisible to them.
- **The eight rules.** Chapter 6 §10.2–§10.9 constrain the platform-layer API
  whoever writes it. R1's *nothing may block* now becomes a **selection
  criterion** for the library rather than a rule we impose on ourselves.
- **The delegated-renderer split** (ADR-0004) and the admission test
  (ADR-0037): a renderer component is admissible iff it satisfies the normative
  surface at its claimed rung. There is still no admissible set.
- **`#12`'s no-bundled-engine ruling.** Delegating the platform layer does not
  bundle an engine; the tiers are distinct and the higher one stays out.
- **The C ABI premise.** #12's binding-first decision is what makes delegation
  possible at all. It is confirmed, not weakened.
- **The web as a shaping constraint.** ADR-0006's rules were written so the
  deferred web backend stays reachable. They survive; whether the chosen library
  reaches the browser is #132's question.

## 5. What this deletes

- **The eighteen-backend matrix**, and with it ADR-0001's *"four subsystems across
  five platform families… and Linux is two windowing systems, not one."*
- **Runtime dynamic loading as a platform-binding strategy** (`dlopen`/
  `LoadLibrary` plus symbol resolution). Its purpose was making
  `ludo build --target X` work with no platform SDK installed; a delegated layer
  changes that calculus, and it is #132's to re-decide rather than this ADR's to
  keep.
- **`objc_msgSend` through ordinary externs, and the Objective-C shim.**
  ADR-0001's macOS carve-out exists only because we were writing Cocoa and Metal
  calls ourselves.
- **The platform layer written in ludo**, and with it a bootstrap ordering
  problem: ADR-0001 states *"the platform layer is written in ludo, so it cannot
  exist before the compiler does"*, which put a foreign-function layer inside the
  throwaway interpreter as a prerequisite for the first pixel.
  [#49](https://github.com/ludo-lang/ludo/issues/49) scoped none of that.
- **Dogfooding as ADR-0001 framed it** — the platform layer as *"the largest
  honest test of #8's explicit allocators and #11's monomorphisation."* Named as
  the secondary benefit there, and genuinely lost. A keeper-era test needs another
  subject.

## 6. ADR-0002 loses its implementation premise

[ADR-0002](0002-webgpu-model-as-renderer-api.md) adopts the **WebGPU model** as
the renderer API and says *"we implement it ourselves over D3D12, Metal, Vulkan
and (deferred) WebGPU proper."* The second half falls with ADR-0001.

The first half is **not decided here**. Whether the renderer API still adopts the
WebGPU model depends on what the chosen library exposes — SDL3's GPU API is close
to it, raylib's is not — so it moves to #132 rather than being settled by a
reversal aimed at something else. ADR-0037 already demoted ADR-0002's four API
names to *"non-normative evidence of tractability, not an admissible set"*, so
what remains at risk is the model adoption and not a roster.

## 7. Amendments this ADR makes

- **[ADR-0001](0001-own-the-platform-layer.md)** — **reversed in full.** The
  platform layer is not ours; it is delegated. §4 above lists what survives the
  reversal, none of it from this ADR's own reasoning.
- **[ADR-0002](0002-webgpu-model-as-renderer-api.md)** — the implement-it-ourselves
  premise is withdrawn. Whether the
  WebGPU *model* survives as the renderer API is deferred to #132.

Nothing else moves. The mandated facade ADRs, ADR-0006's eight rules and
ADR-0037's admission test are what this ADR applies, not what it changes.

## 8. What this owes

**#132 — which library.** SDL3 and raylib sit at different tiers, and the
precedents cut both ways: DragonRuby ships a Ruby game runtime on SDL, and Usagi
runs Lua over raylib through Rust bindings. Both are this project's exact shape —
a language over a C platform library — and neither settles it. The eight rules of
chapter 6 §10 are the test, R1's *nothing may block* first among them.

Until #132 lands, "third-party library" is what is decided and the library is not.
