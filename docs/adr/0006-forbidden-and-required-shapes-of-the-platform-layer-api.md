---
status: accepted
---

> **Amended by [ADR-0007](0007-the-audio-facade-surface.md):** the raw sample push's
> justification is corrected — the caller-authored clip, not the push, is the pixel
> array's analogue and the procedural path. ADR-0007 also records that the `O(1)`
> voice clause bounds each voice, not how many exist: there is no voice cap.
>
> **Amended by [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md):**
> "drawing needs eight verbs" is a wrong count — nine at the time of writing, ten after
> ADR-0010 — and the argument it decorates is untouched.

# The forbidden and required shapes of the platform-layer API

ADR-0001 made the web a *shaping constraint* on the platform-layer API from day
one while deferring the web backend itself. That deferral only survives if the
constraint is written as rules a reviewer can apply, rather than as a promise to
be careful. This ADR is those rules.

The ADR is titled for the rules and not for the web, because **two independent
forcing arguments** produced them and a web-titled document would hide the
second:

- **The browser.** No C ABI (imports are JS functions), no `dlopen`, no threads
  by default, and the host drives the callback loop — issue #12 already overruled
  the user-written loop on that ground.
- **The platform layer is written in ludo** (ADR-0001), and issue #17's reload
  swaps a dylib at a quiescent frame boundary. So **no ludo code may run off the
  frame thread**, on any platform. This is what actually forces the audio design
  below; the browser merely agrees.

Each rule names its origin, so that when the web backend is finally written
nobody relaxes a rule that turns out to have been #17's all along.

## Scope

This ADR does **not** design the platform-layer API. That API lives in its own
repository (ADR-0001) and this map's destination is a language spec. What lands
here is the rule set, plus the consequences that fall on the **language** and on
the mandated **drawing facade**. Where a rule forces something on the language,
it is routed rather than decided.

## The rules

### R1 — Nothing in the platform-layer API may block

Every wait is a poll or a host-driven callback. The browser forbids blocking the
host callback, and there is no thread to hide a block on.

The expensive case is **asset loading**, because the mandated facade draws
sprites and a beginner's first line loads one. It is resolved by #26: the entry
file's top level runs to completion **before any frame**, so a load issued there
is the runner's problem and the browser can satisfy it with the tab's own
asynchronous machinery before it ever calls the frame entry. In-frame streaming
is the explicitly deferred case, not the default one — and see R7.

Rejected: allowing a blocking read in loading only. That is the silent
desktop/web divergence ADR-0005 spent a section closing.

### R2 — The backend seam is one struct of non-capturing function pointers

A loaded backend and a linked backend must be the same interface, or the deferred
web backend is the rewrite this ADR exists to prevent. A `dlopen`'d backend
cannot be static dispatch — a resolved symbol *is* a function value — while a web
backend is linked and could be. Two interfaces would be the rewrite.

One interface: the seam is a plain struct of non-capturing function pointers, and
how it is filled (symbol resolution, or linked-in functions) is the backend's
private business.

**Language consequence, routed:** function pointers must exist as a type — the
same shape #29 already permits for callbacks out to C, no capture, no closure.
This is a hard requirement on the *function values* fog, and it lets that fog be
answered bottom-up from a real client.

### R3 — No conditional compilation; a backend is a module

Eighteen backends means per-platform code, and the web branch shares no code with
any native one. `#if web` is a semantic mode switch — criterion 4's textbook
violation, and #19's P8 makes mode-independence a tested property.

Target selection happens at **build**, by which modules the build includes, never
inside a function body. R1-locality holds (nothing outside the signature radius
changes a body's meaning) and #22's oracle stays honest (one file, one meaning).
"The web has no C ABI" becomes *the web backend is a different module*, which is
already true.

**Language consequence, routed:** the module system (still fog) inherits a
**target-selected module set**, and the criterion-4 question moves there.

### R4 — The unavailable-backend error has the same signature on every target

ADR-0001 made an unavailable backend an error value in the return type (#10),
never #18's error-as-pause. On the web there is nothing to load, so
unavailability is a build fact, and it is tempting to elide the error from the
web signature. That is a signature that differs by target — criterion 4, dead.

The signature is identical everywhere; the web build's path simply never returns
the error. Dead code in a web build is the price of one program shape. In
practice the variant earns its keep on the web anyway, because the browser has a
real *no audio device yet* condition (R6).

### R5 — Audio is pushed from the frame entry; nothing calls ludo back

Forced by the second argument above, not chosen. A backend-owned mixer runs on an
audio thread, and the platform layer is written in ludo, so that is ludo code
running off the frame thread — exactly what #17's quiescent-boundary dylib swap
forbids and what #29 declined.

Therefore mixing happens **in the frame entry, on the main thread, in ordinary
ludo code**, and the platform layer's audio API is push-only. Two properties fall
out that a backend-owned mixer would have destroyed: the mixer is
reload-swappable and fault-attributable like any other game code, and the play
cursor of R6 is reachable rather than hidden behind a thread.

**This closes #29's honest gap.** No ludo-visible off-thread callback exists, so
the concurrency fog is no longer a prerequisite for audio. #29's recorded
structural conflict — multi-threaded quiescence — stands for everything else, but
audio no longer waits on it.

*Recorded consequence, not a rule:* audio DSP spends frame budget, so audio load
and #19's P7 frame overrun are the same budget. The API already lets an engine
observe its own starvation (R6 plus free-space). No voice cap, no stealing
policy, and no voice model belongs in the spec — that is engine policy.

### R6 — The audio API copies, reports, and never demands

Four clauses, each of which a naive API would get wrong and only discover when
the web backend was written:

- **Push is a copy, never a handoff of a mapped region.** The browser's
  low-latency path is an AudioWorklet over a `SharedArrayBuffer`, which requires
  cross-origin isolation (COOP/COEP headers) the game's author may not control;
  without it the same design still works by copy. So no `map`, `lock_buffer` or
  `get_write_pointer` in the API. Cost: one memcpy per frame. It also composes
  with #8, where a lend that escapes the frame does not exist.
- **The device reports its sample rate; the caller never demands one.** A browser
  `AudioContext` runs at the device's rate and will not be told otherwise. The
  push format is fixed at f32 interleaved — the web-native one — and backends
  convert where the OS wants i16. *Named cost, one tier up:* the mixer must
  resample assets, because a 44.1 kHz clip on a 48 kHz device is the common case.
- **The device reports a monotonic play cursor in sample frames.** The ring runs
  ahead of what is audible, so synchronising against the push position fires
  every event early by the ring depth. Frame-precise music sync is unbuildable at
  any tier without the cursor. Every backend can supply one, the browser included.
- **The caller polls free space and fills it.** #19's P7 lets a frame run late,
  and a late frame that pushed exactly one frame of audio would underrun. Poll
  plus copy-push, no blocking, no callback — consistent with R1. *Consequence:*
  audio latency is a ring depth the program chooses, so it is a visible number
  rather than a hidden backend property. The facade picks one; a rhythm game
  lowers it and accepts the underrun risk knowingly.

### R7 — File IO is request-and-poll, never a blocking read

R1's preload answer does not cover **streaming**: a four-minute track cannot be
preloaded, and it is in-frame IO by definition. The browser has no synchronous
read. So the platform API asks for a chunk and polls for arrival.

Two bills, both already ours under ADR-0001 and ADR-0005: **we decode compressed
audio ourselves** — the browser's decoder is asynchronous-only and rate-converts
behind your back, the same argument that made ADR-0005 decode images itself — and
**an underrunning stream outputs silence and reports, never stalling the frame**.

### R8 — Two runner guarantees, reaching into the mandated contract

- **The ring buffer is runner-owned and survives reload**, the same shape as
  #29's runner-owned callback trampoline and for the same reason, so **reload is
  click-free**. Without this, every save clicks — the headline feature made
  audibly annoying.
- **After a #18 fault the backend drains what is queued, then outputs silence,
  never a loop.** A looping buffer under a dead simulation is the classic hung
  game death drone; silence is the honest signal that the simulation stopped.

Both attach to #19 at the **full** conformance level as observable clauses of
properties that already exist — click-free reload to **P1**, drain-then-silence
to **P2** — rather than becoming new properties. Core conformance is headless, so
nothing is imposed on an implementation that never has a device. This is the one
place the unmandated audio tier reaches back into the mandated contract, and it
does so through the runner, which is mandated.

## The audio engine is engine tier, and out of scope

A synthesizer, a mod-slot effects chain, MML, a mixer and a streamer are the
**engine** tier — the same treatment the renderer gets under ADR-0001 and
ADR-0004: ours, own repository, blessed by reference, **unmandated**. Mandating
them would make *conforming* mean *shipped a DSP stack*, which is ADR-0004's
rejected full-cover argument with different nouns.

The destination does not widen. #28's core/full conformance split already has the
slot.

## The facade's audio surface was at the wrong altitude

The mandated **drawing facade** was found asymmetric, and the asymmetry is real:

- drawing mandates primitives *and* raw framebuffer access (pixel arrays,
  first-class, with a guaranteed upload cost);
- audio mandated a jukebox — one `sound` call playing a file — and no raw access.

So a user could write a software rasterizer against conforming ground but not a
synthesizer, and *square wave, rising pitch, fast attack, long decay* — an
ordinary veteran's jump sound — was unreachable without leaving the spec.

**The call count was never the problem.** Drawing needs eight verbs because
rasterization is hard and lives on hardware you cannot otherwise reach; audio
composition is addition and the DAC accelerates nothing. The **altitude** was the
problem.

Three arguments settle it, and all three are ADR-0004's and ADR-0005's own:

- **Specifiability.** ADR-0004 mandated circles because *specifying a shape is
  not specifying how it is built*. A square wave at frequency `f` is analytically
  statable in the same way, and sample-exact — a cleaner conformance target than
  a rotated sprite, which needed a texel-centre tie-break rule.
- **Cost.** ADR-0004 already mandates **text**, forcing a font rasterizer and
  naming the default font by content hash. An oscillator set with envelopes is
  dramatically less code. Cost cannot be why audio got one call; the honest
  explanation is that #27's research was graphics-shaped and #28 inherited the
  blind spot.
- **Parameters are not effects.** Colour is not an effect applied to a rect; it
  is a parameter of the rect. A filter cutoff is a parameter of the voice, and
  timbre is to a sound what colour is to a shape.

### The mandated addition

- **A voice**: a closed **waveform enum** (sine, square, triangle, saw, noise),
  frequency, amplitude, an **amplitude envelope** and a **pitch envelope**,
  one-shot or loop; forward or reverse for clips.
- **A filter as a voice parameter**: a closed **filter-type enum** (lowpass,
  highpass, bandpass) with **cutoff** and **resonance**, specified by stated
  coefficient formulas so every backend computes the same filter. With an
  envelope this reaches the filter sweep, which is most of what makes a
  synthesized sound a game sound rather than a beep.
- **Raw sample push and the play cursor**, on a narrow and honest justification:
  they let a *third-party* audio library run on conforming ground, the service
  the pixel array already does for a third-party rasterizer. Not the ergonomic
  answer — nobody writes a jump sound by filling `samples[i]` — and it was wrong
  to offer it as one.

### The line

> A mandated voice carries **fixed, `O(1)`, spec-sized state and allocates
> nothing**, and applies to **one voice with no routing to another**. Anything
> needing a caller-sized buffer, or a route between voices, is engine tier.

This is **#8 applied to sound**, not an audio-taste line, and it is checkable by
a reviewer rather than arguable. It sorts every case: oscillator, envelope,
filter and clip playback are in (`O(1)`, no routing); delay and reverb are out on
the memory clause, since their buffer is `sample_rate × delay_time` and the
caller sizes it; a mix bus is out on the routing clause. **Caller-supplied memory
is fine** — a clip and a pushed buffer are the caller's, passed in — so it is
*allocation by the facade* that is forbidden, not the presence of a large buffer.

An earlier draft drew the line at *effects that operate across voices or across
time*. That was wrong and is recorded as wrong: an envelope operates across time,
and a biquad keeps history, so the rule excluded the things it was meant to admit.

**LFO is excluded by the routing clause alone** — its state is `O(1)` — and it
costs nothing, because of ramping below.

### Two clauses the addition drags in

- **Conformance tolerance.** Bit-exact audio across the backend matrix is not
  deliverable: a recursive filter accumulates floating-point difference and a
  sine depends on the platform's transcendental functions. Same problem ADR-0005
  met and solved by putting the tolerance **on coverage, not colour**. So:
  **sample-exact where the computation genuinely is exact** (square and saw are
  sign-and-phase arithmetic; clip playback is a copy) **and a stated per-sample
  amplitude tolerance for anything transcendental or recursive**. Same structure
  as `crisp` versus `smooth` — an exact default and a bounded case you opt into.
- **Parameter ramping is mandated.** Voice parameters are set from the frame
  entry, so they change in 60 Hz steps, and stepping a cutoff or an amplitude is
  audible as clicking. Ramping — continuous parameters interpolate to their new
  value across the frame — is one sentence of spec, removes a class of artefact
  that would otherwise read as an implementation bug, and makes **per-frame
  modulation from ordinary game code the modulation mechanism**. Vibrato is a
  `sin` in the frame entry: no routing matrix, no second control language. The
  same instinct as #16's free-functions decision — the general mechanism you
  already have beats a special one.

These names live under the reserved `$.` root, so #24 makes the addition visible
through the **stdlib root-name companion count**, not the grammar budget.

## Consequences

- **#29's honest gap is closed** (R5); the concurrency fog keeps its other
  obligations but no longer gates audio.
- **The function-values fog gains a hard requirement** (R2) from a real client.
- **The module-system fog gains a hard requirement** (R3): a target-selected
  module set.
- **#19 gains two observable clauses at full conformance** (R8), on P1 and P2.
- **The audio engine tier is out of scope** for this map, by the renderer's
  precedent.
- **ADR-0004's mandated facade is amended** and ADR-0005's surface work has a
  sequel: the voice, the filter, the sample push and the play cursor need call
  names, signatures, sample-exactness clauses, cursor units and monotonicity, and
  underrun behaviour. That is #40-shaped work and it gets **its own ticket**.
  Burying an amendment to a closed, mandated facade inside a platform-API ADR is
  how a spec quietly drifts, which is what #24's recorded-overrule rule exists to
  prevent.
- **Nothing here is built.** As with ADR-0001, these are shaping constraints on
  an API that must be designed before it is written.
