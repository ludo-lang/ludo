# Context

Glossary for the ludo language-design effort. Terms only: what each word means
and what it is deliberately not. No implementation detail, and no decision that
is not already settled elsewhere — where an entry states a fixed property, it
names the ADR that fixed it, and that ADR is the authority. Decisions live in
`docs/adr/` and in GitHub issues.

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

## Facade

A mandated area of the reserved `$.` root: one module, a fixed set of calls, a
surface frozen by criterion 4. *Facade* is the operative word — it is a stable
front over a tier that is free to evolve beneath it.

There are five, each its own area with its own module and its own ADR:
`$.graphics` (ADR-0009, ADR-0010), `$.audio` (ADR-0007), `$.input` (ADR-0011),
`$.storage` (ADR-0026) and `$.video` (ADR-0038). They share ADR-0007's six
naming rules and nothing else; input in particular shares only a backend with
drawing.

`$.video` is the odd one: it holds no operations, only the two **player
preferences** the runner owns and the program may read and set — fullscreen
(ADR-0025) and the [render scale](#render-scale). It is named after the settings
menu players already know, and that familiarity is a stated cost — `$.video` is
**not** a precedent for `resolution()`, `size()` or `dpi()`, which stay refused
on ADR-0028 §6's grounds.

## Drawing facade

The `$.graphics` facade specifically, sitting on the [renderer](#renderer): ten
immediate-mode drawing calls plus text measurement, image slicing, pixel images,
decoding, the canvas declaration and the coordinate conversions.

Not the renderer — issue #28 mandates the facade and delegates the renderer, so
using either word for the other loses the distinction the decision turns on. Not
the whole mandated surface either: sound and input are their own facades, and
using "drawing facade" for all of them re-collapses the split ADR-0009 made.

## Draw target

What drawing calls are issued against. Carries the properties that must hold for
a whole frame rather than a single call — the [style](#style) and the 2D
transform — and is passed, never ambient. The [logical canvas](#logical-canvas)
is **not** among them: it is a language constant, not a property of a target
(ADR-0032). Spelled `Target`, and delivered by the runner to the per-frame entry
point rather than constructed or fetched (ADR-0009, ADR-0013).

## Fill / stroke

The drawing facade's **two** verbs, with non-overlapping meanings (ADR-0010).
*Fill* is a shape's area, painted with a [paint](#paint); *stroke* is a path with
thickness — for a closed shape, the shape grown by `thickness/2` minus the shape
shrunk by `thickness/2`, and for an open path, round caps.

**Every call is a fill or a stroke**, and the set is not "every shape has both":
`fill_line` does not exist because a line has no area. A third verb `draw_` was
spelled by ADR-0009, along with a rule adjudicating between the three; ADR-0010
deleted both when `fill_` began taking a paint, so `draw_line` is `stroke_line`
and `draw_sprite` is the `fill_sprite`/`stroke_sprite` pair. Using *draw* as a
verb of the facade names a call that does not exist.

## Paint

What a fill or stroke is painted with: a sum of a `Color` and a `Texture`, the
latter carrying an image, a `Mapping` (`stretch` or `tile`) and an offset
(ADR-0010). Every descriptor carries one, so `Color` is no longer a field type
anywhere — it survives as a paint variant and as `$.rgb8`/`$.rgbf`'s return type.

*Pattern* is not a third variant: a pattern is the same pixels under a different
mapping, which is what `Mapping.tile` says.

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
cannot mix the two aesthetics incoherently. **`crisp` is nearest sampling at
every scale**, including fractional ones — its meaning never depends on the
window size, which the program cannot see (ADR-0031).

## Logical canvas

The coordinate space every program draws in, letterboxed and scaled to whatever
the real window is. **Fixed at 1280×720 for every ludo program** — a language
constant, not a declaration, so a library can lay out against the screen without
being told its size (ADR-0032). Available as `$.graphics.canvas_size`. Distinct
from the window's actual pixels; the space in which integer coordinates are
meaningful. A non-16:9 game centres its play area inside the grid and owns the
rest. The real window size is not exposed to the program. It is a **coordinate mapping**, not an intermediate
framebuffer: the backend folds it into its own transform and rasterises at native
device resolution (ADR-0030).

## Fit

The mapping from the [logical canvas](#logical-canvas) to the window: a uniform
scale `k` plus a centring translation, and nothing else — **aspect ratio is
preserved and no implementation may stretch to fill**, because a similarity
transform is what keeps the world the same shape on every machine. `k` is
`min(w/W, h/H)`, with **no `style` branch and no integer-scaling rule** — that
was deleted as a fidelity guarantee costing a third of the screen at 1080p. There
is no program-facing knob; the fit is spec-fixed (ADR-0030, ADR-0031). The
[render scale](#render-scale) is not part of the fit and does not change `k`.

## Render scale

A player preference `s ∈ [0.25, 1.0]`, quantised to sixteenths, that rasterises
the frame at `s · k` and resamples it up to `k` for presentation — the spec's
**only performance escape hatch**, for the player or the program (ADR-0038).

Down only: `s > 1` is supersampling, a fidelity feature costing throughput, and
loses to the map's standing ordering. At `s = 1`, the default, an implementation
**MUST NOT** require an intermediate render target; below 1 it **MAY** use one.
The resample is **linear on both style tokens** — nearest at a non-integer ratio
shimmers in motion — which is a distinct step from how draws sample textures,
where [style](#style) still governs.

Not the [fit](#fit) and not a resolution: it changes how many device pixels are
shaded, never the world a program draws, never the coordinate mapping, and never
where a pointer reports (ADR-0030 §4). Runner-owned and surviving relaunch, with
who set it not recorded.

## Letterbox bars

The device pixels outside the fitted canvas rect. **Opaque black, never
program-reachable** — rendering is clipped to the canvas rect — and the region a
pointer reports a position outside the canvas for (ADR-0011, ADR-0030).

## Core conformance / full conformance / shader conformance

The three levels a ludo implementation can satisfy, **totally ordered by
containment**: core ⊂ full ⊂ shader. *Core* is the compiler, the language and the
non-visual standard library, and is testable headless — `measure_text`'s purity
and the storage slot's round-trip both land here. *Full* is core plus the
[facades](#facade) that need a device — graphics, audio, input — over at least
one [backend](#backend), and it is where the clauses that presuppose a window
live: click-free reload, drain-then-silence, the runner's fullscreen affordance,
and P13's fitted canvas, the first #19 property core cannot run. *Shader* is full
plus ADR-0008's two authoring paths — [shader ludo](#shader-ludo) and the
[extern shader declaration](#extern-shader-declaration). The destination's
completion test is measured against core.

A rung is a **partition of #19's existing properties**, not a set of extra ones
(ADR-0037). The split between full and shader is available because the mandated
facade is analytically specifiable and satisfiable by a CPU rasterizer, while the
shader paths are the only place a programmable pipeline is load-bearing. A
program's required rung is **readable from its source**: `extern shader` is an
`extern`, and a shader declaration built against a lower-rung implementation is a
compile error, never a runtime fault.

## Voice

One sounding unit in the `$.audio` [facade](#facade): a waveform or a clip, with
its envelopes and its filter parameters. Carries fixed,
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

A backend is a **triple**: window/input, renderer, audio device. Say **renderer
component** for the graphics third — D3D12, Metal, Vulkan, WebGPU, a WebGL2
context, a CPU rasterizer. ADR-0022 §1 and ADR-0002 both use "backend" for the
component alone, and issue #74 found that half of its ambiguity was this
collision and nothing else; the glossary definition is the one that holds.

**There is no admissible set of either** (ADR-0037). A renderer component is
admissible iff it can satisfy the spec's normative surface at the
[rung](#core-conformance--full-conformance--shader-conformance) its
implementation claims — so WebGL1's exclusion is a consequence of failing
ADR-0022 §1's strided upload, not membership in anything, and WebGL2 is neither
admitted nor refused. A **software rasterizer is admissible** at the full rung
and is never mandated.

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

## Quiescence

The state a program must be in for the [runner](#runner) to swap anything
underneath it: **no ludo code on any stack, no `extern` call in flight, and the
only live ludo state is `persist` plus declared assets plus resolved storage
contents** (ADR-0024, ADR-0026).

A **state predicate, not a location**. It holds between two invocations of the
frame entry, after top level and before the first frame, and — as a theorem
rather than a special case — forever in a faulted program, which is what lets a
reload rescue one. Three mechanisms cite it: the dylib swap, the asset byte-swap
behind a handle, and the backend re-point.

Ludo code may be entered **only from the frame task** — by task, never by thread,
because a host with one thread still has many places it can call in from
(ADR-0041 §7). The rescue is a theorem the host can still defeat: a discarded
browser tab is described by no specification and fires no event, so no
implementation can flush around it (ADR-0041 §8).

## Experience contract

The set of testable properties any conforming implementation must deliver —
reload latency with state preserved, error-as-pause, single-command run, no
unchecked escape path, machine-readable diagnostics. Issue #19 owns its wording.

## Diagnostic stream

The machine-readable output of the toolchain, and the agent's half of the
read-diagnostic → edit loop. One shape serves both producers: the compiler, when
a program does not compile, and the [runner](#runner), when a program faults. A
message carries a format version, a `compile`/`fault` producer discriminant, a
severity from a closed set of three, a spec-owned stable code, a primary source
location as file plus byte offset plus length, the named entity, and human text
that carries no obligation. A `fault` message adds #18 §8's contents. The one
mandatory encoding is JSON, one object per line; the human rendering is not part
of it, and the stream carries messages and nothing else. A **suggestion** is a
structured field — a source range and replacement text — and is omitted rather
than guessed (ADR-0018).

## Wrapper

A user-written function over a mandated facade call, giving it a short,
project-local name (`pixel`, `footstep`, `jump_pressed`). The intended
counterpart to the facades' deliberate verbosity: the agent writes the
qualified unabbreviated call, the human writes the wrapper over it (ADR-0012).
Wrappers live in user namespace forever — never blessed into the reserved `$.`
root, and not counted by #24's companion count. Third-party wrapper packages are
permitted and unconstrained; this project publishes none.

## Forwarding function

A function whose body is a single call expression, with no captures. A
structural predicate, not a judgement about optimisation quality. A conforming
implementation must not emit a call to one — the guarantee that makes the
wrapper idiom free (ADR-0012). The **one** bounded constraint the spec places on
how an implementation compiles: ADR-0022 §4's copy-not-quad blit condition was
briefly a second, and ADR-0034 deleted it with the clause it served.

## Host language

The language a ludo compiler is itself written in. Distinguished from the
languages ludo *targets* and from the languages it *binds* — a compiler's host,
its codegen output and its `extern` surface are three separate questions that
discussions routinely collapse into one (ADR-0020).

## Bootstrap compiler

A ludo compiler written in a [host language](#host-language) rather than in
ludo. Named for its role in the build chain, not its quality or its lifespan: a
bootstrap compiler may be the production toolchain for years.

## Self-hosting

The state in which ludo's compiler is written in ludo, so that building from
source runs the bootstrap compiler to produce a compiler that then compiles
itself. A property of the toolchain only — it says nothing about the
[platform layer](#platform-layer), which is written in ludo from the start.

## Drawing entry

The entry that draws, and the only place a `Target` exists. It **is** the frame
entry, and there is no other — ADR-0035 accepts the [render ceiling](#render-ceiling),
so simulation and rendering are never split and no second entry exists. Its signature
is governed by one rule — **the entry's parameters are exactly the values only
the runner can supply** — which today means two: `screen: !Target`, fresh each
frame with its transform reset to identity, and `scratch: !Scratch`, the
per-frame arena the runner resets at the frame boundary (ADR-0042). An offscreen target is
constructed by the program and never joins the list. Top-level code has no
target and therefore cannot draw.

## Frame

**One frame entry call, one simulated step, and one presented image — the same
thing under three descriptions.** The collapse is definitional, not incidental:
ADR-0035 fixes the render rate to the simulation rate, so the word has exactly
one referent everywhere in the spec and never needs disambiguating from context.
(*Sample frame* in audio is a different unit and keeps its own entry — see
[play cursor](#play-cursor).)

A frame that overruns the 60Hz step still yields exactly one entry call and
exactly one late image: the simulation runs late and the framerate drops, and no
catch-up call is made (#19 P7 with ADR-0035 §3).

## Render ceiling

The permanent equality of render rate and simulation rate at 60Hz (ADR-0035).
Chosen rather than inherited: an immediate-mode [drawing facade](#drawing-facade)
forecloses interpolation — which would need the renderer to read two states, and
so a second entry and per-game double-buffering — and the map's ordering of
**frame stability above rendering fidelity** resolves that trade against
smoothness. The accepted cost is uneven cadence on a fixed-refresh high-refresh
panel, or windowed where VRR does not engage.

The [runner](#runner) presents **exactly one image per returned frame entry**,
never twice and never skipped; *how* it paces — vsync, sleep, free-run — is
unspecified, because how you wait is environment and how many images the player
sees per simulated step is not.

## Module

A file. The unit of compilation and the unit `use` and paths resolve into. A
directory is a namespace node over modules. There is no separate package
concept — a [backend](#backend), a [library](#library), and the second file of
your own program are the same kind of thing (ADR-0014).

## `#explicit`

The one attribute that marks a **module** rather than a declaration: one line at
the top of a file, opting that file into the enforced floor of progressive
disclosure. It **forbids a spelling that omits a name**, and nothing else — a
closed list of four (a binding without its type, an aggregate literal without its
type name, a call omitting a defaulted argument, a UFCS call). Descriptor field
defaults, `+=`, the implicit tail return and `?T` are permitted, because none
omits a name. It changes zero semantics; a violation is a hard error, never a
warning. It binds only the file carrying it, so no check needs a whole-program
view, and a layer choice never crosses a [library](#library) boundary in any
direction — a library can neither force, forbid, nor see its consumers' choice
(ADR-0029).

## Library

A directory that claims a root name, with one `library <name>` line at its root.
That line is the whole difference between a library and a plain directory in
your program. The name belongs to the library — no aliasing, no re-export, no
rename — and two libraries claiming one name is a hard error with no escape
hatch. Within one program a root name denotes exactly one library, which is what
keeps nominal type identity intact across a dependency boundary; the cost is no
in-program version pluralism. A library carries no version, hash or origin, so a
vendored edited copy is a fork and *is* that library for that program
(ADR-0014). Distinct from a [wrapper](#wrapper) package, which is one thing a
library may contain.

## Reference discipline

The rule that a program reaches its own files **by path** and a library **by
name**, and that a path reference may never cross a library boundary — you
cannot path into a library, and a library cannot path out. It makes provenance
legible at every reference site rather than in a directory listing, and it is
the whole of what the language decides about code ownership: editing a
library's files is legal and takes effect, and no language mechanism marks a
directory as third-party (ADR-0014).

## Allocator

The type a function receives when it is allowed to allocate. A `struct` whose
fields are non-capturing function pointers — a value, not a language concept, so
it can be exchanged while the program runs without the boxed existential #11
forbids. Three operations: allocate, grow, reset. There is **no per-allocation
free** in the safe layer; release is bulk, which is what makes double-free
structurally absent rather than merely discouraged. Exhaustion is an ordinary
fallible return, never a fault — a fixed-capacity pool reaching its cap is a
design condition, not a defect. Storable: a container captures one at
construction and keeps it. Distinct from [scratch](#scratch), which is the same
shape and may not be stored (ADR-0042).

## Scratch

The per-frame arena, and the one allocator a program never stores. **Transient
and non-escaping** — parameter position only, never in a struct field, never
returned, never outliving the call — which is the rule `[]T` views already carry
and the reason a container built on it cannot survive to the next frame. The
[runner](#runner) supplies it to the [drawing entry](#drawing-entry) and resets it
at the frame boundary, so nothing in the program owns its lifetime and no
forgotten reset can leak. Its restriction is on positions, not lifetimes: no
regions and no borrow checking (ADR-0042).

## Prelude

The core types every program has without asking, and the answer to where
[`Allocator`](#allocator) and [`Scratch`](#scratch) live. Deliberately **not** a
sixth [facade](#facade): a facade is a stable front over a tier free to evolve
beneath it, and there is no tier beneath memory, no backend to delegate to, and
core conformance is headless — so these must exist where no backend does
(ADR-0042). [`format`](#text-buffer) and [`TextBuf`](#text-buffer) live here on
the same reasoning: text formatting delegates to no tier and must work headless
(ADR-0043).

## String

A view into the [runner](#runner)'s constant blob, and **only** that. Immutable
UTF-8, not an indexable aggregate — no `s[i]`, and iteration goes through
`chars()` (#15). A `string` may be stored, `persist`ed and used as a map key
**because** its referent outlives every frame, reload and image swap, so the
permission is derived rather than granted and no lifetime machinery appears
(ADR-0043).

**Nothing constructs one.** A `string` widens implicitly to `[]u8` at parameter
position and there is no conversion back, because the type's meaning *is* the
memory it points at. Constructed and player-entered text is therefore `[]u8`, and
a text-entry program never names this type at all. The consequence, stated rather
than discovered: a map cannot be keyed by text a player typed. Distinct from a
[text buffer](#text-buffer), which is where constructed text lives.

## Text buffer

`TextBuf[N]` — a fixed-capacity `struct { bytes: [N]u8, len: usize }` and the
home for constructed text. A plain value with no pointer, so it lives in
`persist` and survives reload like any array. `append` truncates silently and
returns nothing, so a player holding a key down at capacity sees nothing happen
rather than an error, and no must-use fires at the call site.

Beneath it is `format(dst: ![]u8, "...", ...)`, which writes into a caller-owned
buffer and returns what it wrote. The format string is a **literal the compiler
checks**, so a wrong hole count or argument type is a compile error — not a macro
(ADR-0021 forbids those) but a compiler-known signature, the exception shape #15
used for `Eq`/`Hash`/`Clone`. Holes are `{}` only, over a closed list of
primitives; the math types are excluded because `Vec2` has no single obvious text
form. Overflow truncates at a scalar boundary, so the destination is always valid
UTF-8 (ADR-0043).

## Fog

A named area of the language nobody has decided yet — enumerated in issue #1's
*Not yet specified* section, and therefore **finite and countable**. Fog shrinks
monotonically: closing a ticket removes an entry and adds none.

Process vocabulary rather than language vocabulary, and kept here because it was
being used for two different things (ADR-0044 §2). Distinct from a [hole](#hole),
which is what people usually mean when they say the fog is growing.

## Hole

A contradiction or a gap surfaced by writing something concrete — a spec chapter,
a piece of research, a ludo program. **Generated rather than enumerated, and
unbounded by construction**: any corpus large enough to cite itself has more of
them than anyone has looked for.

A hole is not evidence about [fog](#fog). Most holes found so far were
transcription defects rather than undecided design — three clauses cited by
multiple ADRs and never authored (ADR-0044 §3). A hole is therefore repaired in
the spec text directly, and earns an ADR only where it reverses a decision
(ADR-0044 §6).
