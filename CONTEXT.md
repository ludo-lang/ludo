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

## Engine

Frame loop, component model, sprites, animation, collision, camera. The tier
LÖVE2D, DragonRuby and Flame occupy.

Issue #12 settled that ludo does not bundle one: an engine is an ordinary library
you link. Avoid using "engine" as a loose synonym for the whole stack.

## Backend

One platform's implementation of the platform layer — the Win32/D3D12/WASAPI set,
or the Cocoa/Metal/CoreAudio set. A single platform-layer API has many backends.

## Runner

The dev-mode process launched by the bare `ludo` command. It owns the program's
process, drives the frame entry, holds `persist` state across a rebuild, and
holds a faulted program paused instead of exiting. `ludo build` drops it.

## Experience contract

The set of testable properties any conforming implementation must deliver —
reload latency with state preserved, error-as-pause, single-command run, no
unchecked escape path, machine-readable diagnostics. Issue #19 owns its wording.
