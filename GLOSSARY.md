# ludo

Glossary for the ludo language-design effort: what each word means and what it is
deliberately not. Where an entry states a fixed property it names the ADR that fixed it, and
that ADR (or the spec chapter that absorbed it) is the authority.

## The stack

**Platform layer**:
The lowest tier of the graphics/input/audio stack: window, input event delivery, audio device,
GPU device and swapchain — the tier SDL and GLFW occupy.
_Avoid_: low level (say which tier)

**Renderer**:
The general-purpose draw surface on the platform layer, issued draw calls without knowing which
GPU API is underneath; shipping one is what makes a third-party engine possible (issue #12).
_Avoid_: drawing facade, platform layer

**Engine**:
Frame loop, component model, sprites, animation, collision, camera — the tier LÖVE2D, DragonRuby
and Flame occupy, and an ordinary library you link rather than something ludo bundles (issue #12).
_Avoid_: the whole stack

**Backend**:
One platform's implementation of the platform layer, a triple of window/input, renderer and
audio device; admissible iff it satisfies the spec at the conformance rung it claims (ADR-0037).
_Avoid_: backend for the graphics third alone (say renderer component)

**Renderer component**:
The graphics third of a backend — D3D12, Metal, Vulkan, WebGPU, a WebGL2 context, a CPU
rasterizer.
_Avoid_: backend

## Facades and drawing

**Facade**:
A mandated area of the reserved `$.` root — one module with a fixed, frozen set of calls that
is a stable front over a tier free to evolve beneath it. There are five: `$.graphics`,
`$.audio`, `$.input`, `$.storage` and `$.video` (ADR-0007, ADR-0009, ADR-0011, ADR-0026, ADR-0038).

**Player preference**:
A setting the runner owns and the program may read and set; `$.video` holds the two that exist,
fullscreen and the render scale (ADR-0025, ADR-0038).
_Avoid_: resolution, size, dpi

**Drawing facade**:
The `$.graphics` facade specifically, sitting on the renderer (ADR-0009).
_Avoid_: renderer, the whole mandated surface

**Draw target**:
What drawing calls are issued against, spelled `Target`: it carries the frame-wide style and 2D
transform, is passed rather than ambient, and is delivered by the runner (ADR-0009, ADR-0013).

**Fill / stroke**:
The drawing facade's only two verbs: *fill* paints a shape's area with a paint, *stroke* paints a
path with thickness (ADR-0010).
_Avoid_: draw (as a verb of the facade)

**Paint**:
What a fill or stroke is painted with: either a `Color` or a `Texture` (an image, a `Mapping` of
`stretch` or `tile`, and an offset) (ADR-0010).
_Avoid_: pattern

**Descriptor**:
The single struct argument a facade call takes past two parameters, one type per call and never
shared between calls — `VoiceDesc`, `SpriteDesc`.

**Style**:
The one aesthetic token a draw target carries: `crisp` (no anti-aliasing, nearest sampling at
every scale) or `smooth` (anti-aliasing, linear sampling) (ADR-0031).
_Avoid_: anti-aliasing setting, sampling mode

## Canvas and presentation

**Logical canvas**:
The coordinate space every program draws in, fixed at 1280×720 as a language constant and mapped
onto the real window; a coordinate mapping, not an intermediate framebuffer (ADR-0030, ADR-0032).
_Avoid_: window size, screen resolution

**Fit**:
The mapping from the logical canvas to the window: a uniform scale `k = min(w/W, h/H)` plus a
centring translation, aspect-preserving and spec-fixed (ADR-0030, ADR-0031).
_Avoid_: stretch, integer scaling

**Render scale**:
A player preference `s ∈ [0.25, 1.0]` that rasterises the frame at `s · k` and resamples it up —
the spec's only performance escape hatch (ADR-0038).
_Avoid_: resolution, supersampling

**Letterbox bars**:
The device pixels outside the fitted canvas rect: opaque black and never program-reachable
(ADR-0011, ADR-0030).

## Conformance

**Core / full / shader conformance**:
The three levels an implementation can satisfy, ordered core ⊂ full ⊂ shader: core is headless
compiler, language and non-visual stdlib; full adds the device facades over a backend; shader adds
ADR-0008's two authoring paths. Each rung partitions #19's existing properties (ADR-0037).
_Avoid_: rung as a set of extra features

**Experience contract**:
The testable properties every conforming implementation must deliver — reload with state
preserved, error-as-pause, single-command run, no unchecked escape path, machine-readable
diagnostics; issue #19 owns its wording.

## Audio

**Voice**:
One sounding unit in `$.audio` — a waveform or clip with its envelopes and filter — carrying
fixed, spec-sized state and allocating nothing (ADR-0006).
_Avoid_: channel, track

**Play cursor**:
The count of sample frames the audio device has actually consumed, as opposed to how many were
pushed; music sync is measured against it.
_Avoid_: push position

## Shaders

**Shader ludo**:
Shaders written in a strict subset of ludo — vertex and fragment stages only — that differs from
ordinary ludo in what is permitted, never in what anything means (ADR-0008).
_Avoid_: dialect

**Extern shader declaration**:
The declaration in ludo source that names a `.wgsl` file and its entry point, modelled on the C
FFI's `extern` (issue #29, ADR-0008).
_Avoid_: shader asset, a second ludo syntax

## The runner and the frame

**Runner**:
The dev-mode process launched by the bare `ludo` command: it owns the program's process, drives
the frame entry, holds `persist` state across a rebuild, and pauses a faulted program.

**Quiescence**:
The state predicate under which the runner may swap anything beneath a program: no ludo code on
any stack, no `extern` call in flight, and only `persist`, declared assets and resolved storage
live (ADR-0024, ADR-0026, ADR-0041).
_Avoid_: a location in the frame

**Frame**:
One frame entry call, one simulated step and one presented image — the same thing under three
descriptions (ADR-0035).
_Avoid_: sample frame (the audio unit)

**Drawing entry**:
The frame entry, the only place a `Target` exists; its parameters are exactly the values only the
runner can supply (ADR-0035, ADR-0042).
_Avoid_: render entry, a second entry

**Render ceiling**:
The permanent equality of render rate and simulation rate at 60Hz, with exactly one image
presented per returned frame entry (ADR-0035).
_Avoid_: interpolation

**Diagnostic stream**:
The toolchain's machine-readable output — one JSON object per line, shared by compile errors and
runtime faults — and the agent's half of the read-diagnostic → edit loop (ADR-0018).

**Suggestion**:
A diagnostic's structured fix — a source range and replacement text — omitted rather than guessed
(ADR-0018).

## Toolchain

**Host language**:
The language a ludo compiler is itself written in, distinct from what ludo targets and what it
binds through `extern` (ADR-0020).

**Bootstrap compiler**:
A ludo compiler written in a host language rather than in ludo, named for its role in the build
chain rather than its quality or lifespan.

**Self-hosting**:
The state in which ludo's compiler is written in ludo and compiles itself; a property of the
toolchain only.

**Wrapper**:
A user-written function giving a mandated facade call a short, project-local name; it lives in
user namespace forever and is never blessed into `$.` (ADR-0012).

**Forwarding function**:
A function whose body is a single call expression with no captures, which a conforming
implementation must not emit a call to (ADR-0012).

## Code organisation

**Module**:
A file — the unit of compilation and of `use`; a directory is a namespace node over modules
(ADR-0014).
_Avoid_: package

**Library**:
A directory that claims a root name with one `library <name>` line; the name belongs to it, and
within one program a root name denotes exactly one library (ADR-0014).
_Avoid_: package, version, alias

**Reference discipline**:
A program reaches its own files by path and a library by name, and no path reference crosses a
library boundary (ADR-0014).

**`#explicit`**:
The one attribute that marks a module rather than a declaration, forbidding any spelling that
omits a name and changing no semantics (ADR-0029).

## Memory

**Allocator**:
The type a function receives when it may allocate: a storable struct of non-capturing function
pointers offering allocate, grow and reset, with no per-allocation free (ADR-0042).

**Scratch**:
The per-frame arena the runner supplies to the drawing entry and resets at the frame boundary —
the one allocator that is never stored (ADR-0042).

**Place**:
A named storage location — a binding — and the thing the `!` mutation mark attaches to;
mutability is a property of the place, never of the type (#11, ADR-0052).

**Raw pointer**:
An address, spelled `^T`, and the escape hatch's own type: one non-nullable type with no
arithmetic or indexing, whose operations require `unsafe` (ADR-0053).
_Avoid_: pointer, for the function pointer (ADR-0006) or the input cursor (ADR-0011)

**View**:
A window into memory something else owns, spelled `[]T`, which may be passed and bound to a local
but never stored, persisted, or kept past the call (ADR-0047, ADR-0052).
_Avoid_: slice, reference, borrow

**Origin**:
The place a view derives from — the container the view's expression names, never a part of one;
a mark or move on it kills every view derived from it (ADR-0052).

**Pool**:
A handle-indexed store of many values of one type, either AoS (elements whole) or columnar (split
into columns), with identical semantics (#8, #25).

**Column**:
One field of a columnar pool's element type, stored contiguously and named by field access on the
pool; its origin is the pool (#25, ADR-0052).

**Prelude**:
The core types every program has without asking — `Allocator`, `Scratch`, `format`, `TextBuf`
(ADR-0042, ADR-0043).
_Avoid_: a sixth facade

## Text

**String**:
A view into the runner's constant blob and only that: immutable UTF-8 that nothing constructs
(#15, ADR-0043).
_Avoid_: string for constructed or player-entered text (that is `[]u8`)

**Text buffer**:
`TextBuf[N]`, a fixed-capacity byte array with a length — the home for constructed text, written
by the compiler-checked `format` (ADR-0043).

## Process

**Fog**:
A named area of the language nobody has decided yet, enumerated in issue #1 and therefore finite
and monotonically shrinking (ADR-0044 §2).
_Avoid_: hole

**Hole**:
A contradiction or gap surfaced by writing something concrete, generated rather than enumerated
and unbounded by construction (ADR-0044 §3, §6).
_Avoid_: fog

**Induced hole**:
A hole opened by writing the repair of an earlier one rather than by reading the corpus — the
only class that can diverge.
