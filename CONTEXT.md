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
a whole frame rather than a single call — anti-aliasing among them — and is
passed, never ambient.

## Core conformance / full conformance

The two levels a ludo implementation can satisfy. *Core* is the compiler, the
language and the non-visual standard library, and is testable headless. *Full*
is core plus the [drawing facade](#drawing-facade) over at least one
[backend](#backend). The destination's completion test is measured against core.

## Engine

Frame loop, component model, sprites, animation, collision, camera. The tier
LÖVE2D, DragonRuby and Flame occupy.

Issue #12 settled that ludo does not bundle one: an engine is an ordinary library
you link. Avoid using "engine" as a loose synonym for the whole stack.

## Backend

One platform's implementation of the platform layer — the Win32/D3D12/WASAPI set,
or the Cocoa/Metal/CoreAudio set. A single platform-layer API has many backends.

## Shader ludo

Shaders written in ludo itself — a restricted dialect, bounded by what the
shared shader IR can express. The authoring path for someone who does not
already know a shader language.

## Shader asset

A shader supplied as a file in a foreign format (`.wgsl`) and ingested by the
toolchain. An *asset*, in the sense that a `.png` is an asset — **not** a second
ludo syntax and not a language dialect. Keep this distinction: describing WGSL
as "a syntax ludo also accepts" reopens the grammar budget (issue #24) and the
*two modes of one language* line (issue #16), both of which the asset framing
leaves untouched.

## Runner

The dev-mode process launched by the bare `ludo` command. It owns the program's
process, drives the frame entry, holds `persist` state across a rebuild, and
holds a faulted program paused instead of exiting. `ludo build` drops it.

## Experience contract

The set of testable properties any conforming implementation must deliver —
reload latency with state preserved, error-as-pause, single-command run, no
unchecked escape path, machine-readable diagnostics. Issue #19 owns its wording.
