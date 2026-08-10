# Context

Glossary for the ludo language-design effort. Terms only — no decisions, no
implementation detail. Decisions live in `docs/adr/` and in GitHub issues.

## Platform layer

The lowest tier of the graphics/input/audio stack: window creation, input event
delivery, audio device, GPU device and swapchain. The tier SDL and GLFW occupy
for other projects.

Not the [renderer](#renderer) and not the [engine](#engine). When a discussion
says "low level", it usually means this tier — say so explicitly, because the
three tiers have different owners and different answers.

## Renderer

The general-purpose draw surface sitting on the platform layer: the thing a user
issues draw calls against, without knowing which GPU API is underneath.

Distinguished from the platform layer because shipping a renderer is what makes a
third-party [engine](#engine) possible — the Flutter/Flame relationship recorded
in issue #12.

## Drawing facade

The small set of immediate-mode drawing, sound and input calls the spec mandates
in the reserved `$.` root, sitting on the [renderer](#renderer). *Facade* is the
operative word: it is a fixed, frozen surface over a tier that is free to evolve
beneath it.

Not the renderer — issue #28 mandates the facade and delegates the renderer, so
using either word for the other loses the distinction the decision turns on.

## Draw target

What drawing calls are issued against. Carries the properties that must hold for
a whole frame rather than a single call — the [style](#style), the 2D transform,
and the [logical canvas](#logical-canvas) — and is passed, never ambient. Spelled
`Target`, and delivered by the runner to the per-frame entry point rather than
constructed or fetched (ADR-0009).

## Fill / stroke / draw

The drawing facade's three verbs, with non-overlapping meanings (ADR-0009).
*Fill* is a shape's area in one solid colour; *stroke* is a path with thickness,
defined as the shape minus its inset shape; *draw* is a thing carrying its own
appearance, and so neither filled nor stroked. A shape gets a fill/stroke pair
only if it admits both — which is why lines and sprites are drawn, and text is
filled.

## Descriptor

The single struct argument a facade call takes past two parameters, using
named-field-only literals with field defaults. One type per call, never shared
between calls: a struct whose meaningful fields depend on which call received it
is an overload wearing a type. `VoiceDesc` for audio, `SpriteDesc` and its
siblings for drawing.

## Style

The one aesthetic token a [draw target](#draw-target) carries: `crisp` (no
anti-aliasing, nearest sampling) or `smooth` (anti-aliasing, linear sampling).
One token rather than separate anti-aliasing and sampling settings, so a frame
cannot mix the two aesthetics incoherently.

## Logical canvas

The coordinate space a program declares and draws in, letterboxed and scaled to
whatever the real window is. Distinct from the window's actual pixels; the space
in which integer coordinates are meaningful.

## Core conformance / full conformance

The two levels a ludo implementation can satisfy. *Core* is the compiler, the
language and the non-visual standard library, and is testable headless. *Full*
is core plus the [drawing facade](#drawing-facade) over at least one
[backend](#backend). The destination's completion test is measured against core.

## Voice

One sounding unit in the [drawing facade](#drawing-facade)'s audio half: a
waveform or a clip, with its envelopes and its filter parameters. Carries fixed,
`O(1)`, spec-sized state and allocates nothing, which is the line separating what
the facade mandates from what the [engine](#engine) tier owns (ADR-0006).

Not a channel and not a track — a voice routes to nothing.

## Play cursor

The count of sample frames the audio device has actually consumed, reported
monotonically by the [platform layer](#platform-layer). Distinct from how much
has been *pushed*: the difference between the two is the latency, so synchronising
against the push position fires events early. Music sync is measured against the
cursor.

## Engine

Frame loop, component model, sprites, animation, collision, camera. The tier
LÖVE2D, DragonRuby and Flame occupy.

Issue #12 settled that ludo does not bundle one: an engine is an ordinary library
you link. Avoid using "engine" as a loose synonym for the whole stack.

## Backend

One platform's implementation of the platform layer — the Win32/D3D12/WASAPI set,
or the Cocoa/Metal/CoreAudio set. A single platform-layer API has many backends.

## Shader ludo

Shaders written in ludo itself — a strict subset of the language, covering the
vertex and fragment stages only, bounded by what the shared shader IR can
express. The authoring path for someone who does not already know a shader
language.

*Strict subset* is the operative phrase (ADR-0008): shader ludo differs from
ordinary ludo in what is **permitted**, never in what anything **means**. A
function that stays inside the subset and carries no stage marker is callable
from both CPU and shader code.

Not a dialect in the sense of variant semantics — saying "dialect" loosely
reopens the *two modes of one language* line (issue #16) that the subset
requirement exists to keep closed.

## Extern shader declaration

The declaration in ludo source that names a `.wgsl` file and its entry point,
reaching a shader written in a foreign language. Modelled on the C FFI's
`extern "SDL3" fn` (issue #29): the file is named in ludo source, there is no
build step and no configuration anywhere else, and the compiler reads the file's
declaration surface when it reads the declaration.

**Not an asset, and the word is retired** (ADR-0008). "Asset" implied a
processing stage between a file and your program, which is the asset pipeline
issue #12 ruled out of scope. What the word was protecting still holds: WGSL is
**not** a second ludo syntax and not a language dialect, and describing it as
"a syntax ludo also accepts" reopens the grammar budget (issue #24) and issue
#16's *two modes of one language* line.

## Runner

The dev-mode process launched by the bare `ludo` command. It owns the program's
process, drives the frame entry, holds `persist` state across a rebuild, and
holds a faulted program paused instead of exiting. `ludo build` drops it.

## Experience contract

The set of testable properties any conforming implementation must deliver —
reload latency with state preserved, error-as-pause, single-command run, no
unchecked escape path, machine-readable diagnostics. Issue #19 owns its wording.
