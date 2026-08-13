---
status: accepted
---

> **Amended by [ADR-0009](0009-the-drawing-facades-call-spellings.md):** the naming rules
> gain two explicit case conventions (`PascalCase` types, `snake_case` enum members) and a
> stated distinction between root-level type constructors and per-area modules.
>
> **Amended by [ADR-0015](0015-assets-are-declared-not-loaded.md):** the mandated
> checkerboard-behaviour sentence is **removed** — the state it fixed up is unrepresentable,
> and what survives is `$.graphics.debug_image`, a value reached explicitly.
>
> **Amended by [ADR-0016](0016-the-blessed-math-set-concrete-types-scalar-quantities-no-simd-mandate.md):**
> `$.audio.cursor()` returns `SampleFrames`, not `int`, and conversion takes the device rate
> as an argument.
>
> **Amended by [ADR-0022](0022-the-spec-promises-only-what-is-derivable.md):** *reload* is
> defined as the in-process swap of game code, without which this ADR's play-cursor clause
> is vacuous under one reading of the word.
>
> **Amended by [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md):**
> "a clip adds no root name" is **superseded** — `Clip` is a root type, and the type count is
> six rather than five.

# The audio facade's surface: voices that play, a clip as the procedural path, and a push that is not it

ADR-0006 fixed the *set* and the *line* for audio — the mandated facade gains a
voice, a filter as a voice parameter, a raw sample push and a play cursor, and
the line between mandated and engine tier is `O(1)` state, no allocation by the
facade, no routing between voices. It deliberately did not fix names, signatures
or conformance wording. This is the surface, the same job ADR-0005 did for
drawing.

Every choice here is frozen by criterion 4 once mandated, so each is recorded
with its reason rather than only its outcome.

## Voices play; they do not render

A mandated voice **plays**: the facade owns the active voice set, sums it, and
fills the ring itself. It does not hand the caller a generator to render into a
buffer.

The alternative — `render(v!, out: ![]f32)`, the caller sums and pushes — is
more literally consistent with *no routing between voices*, and it is rejected
anyway, because it destroys ADR-0006's own headline justification. The whole
point of the altitude correction was that the beginner's jump sound goes from
*find a `.wav`* to one call; a facade whose voices must be summed by hand
requires the user to write a mixer before hearing anything, which is the jukebox
problem with extra steps.

**The sum into one device is not routing.** ADR-0006 forbids *a route between
voices* — voice A's output becoming voice B's input, which is what a mix bus and
a send are. Every voice reaching one DAC is unavoidable arithmetic, not a
topology the caller configures. Recorded explicitly so nobody later reads the
routing clause as forbidding output.

The render shape is not lost: it falls out of voices plus the raw push for
anyone who wants it, and that audience is named below.

## There is no voice cap

ADR-0006's clause is that **a voice** carries fixed, `O(1)`, spec-sized state and
allocates nothing. That bounds each voice, not how many exist. N voices is
N × `O(1)` — a size the program decides at runtime, exactly as N entities in a
pool is.

A first draft of this decision turned the per-voice clause into a global one,
invented a mandated minimum capacity, added a `voice_capacity()` call to report
it, and had `play` return absence at exhaustion. All of that is deleted. It also
made ADR-0006's *no voice cap and no stealing policy belongs in the spec — that
is engine policy* into something to be worked around rather than something
satisfied.

Consequences:

- `$.audio.play(desc) -> Voice` returns a handle, not `?Voice`. Exhaustion is
  not a state.
- `$.audio.get(v) -> ?Voice` stays, because a finished one-shot frees its slot
  and the handle goes *gone* — the generational-handle pattern of #8/#9 doing
  its ordinary job, not a capacity signal.
- No stealing policy, because there is nothing to steal from.
- The voice set is runner-owned and grows with the runner's allocator, the same
  side of the line as ADR-0006 R8's runner-owned ring. The **facade** still
  allocates nothing: no mandated call takes a size, and no DSP parameter implies
  a buffer.
- **The real limit is stated where it lives: frame budget.** ADR-0006 R5 already
  records that audio DSP spends the frame and that overrun is #19's P7. A
  thousand voices runs late; it does not fail. Unbounded growth from a program
  that never stops a voice is that program's bug, under #8's leaks-detected-not-
  prevented.

## Mono voices, stereo out, constant-power pan

A mandated voice is **mono** and carries a continuous `pan` parameter. Output is
fixed at two channels, f32 interleaved at the device rate (ADR-0006 R6).

Pan law is **constant power, −3 dB at centre**, stated as a formula so it is a
conformance target rather than a taste:

```
theta = (pan + 1) * PI / 4        # pan in [-1, 1]
gain_left  = cos(theta)
gain_right = sin(theta)
```

A **stereo clip** is the one exception: it plays to both channels, and `pan`
acts as a balance (attenuating the opposite channel by the same gains) rather
than as a position.

## The naming rules, and the call set they generate

The literal spelling of the drawing calls was left open by ADR-0005 and is still
open. What is genuinely shared between the two surfaces is the **rule set**, not
the names, so the rules are fixed here and the drawing surface applies them
mechanically in its own ticket.

**Rules:**

1. One module per facade area under the reserved `$.` root: `$.audio.*`.
2. Verb-first for actions, bare noun for values: `play`, `stop`, `push`,
   `cursor`, `sample_rate`.
3. `snake_case`.
4. **No abbreviations at all** — `frequency`, `amplitude`, `resonance`, not
   `freq`, `amp`, `res`. #4 puts wrong-or-non-existent names at 43.7% of agent
   compile errors, and a model's default guess is the full word.
5. **A descriptor struct as the single argument** for anything with more than
   two parameters, using #15's named-field-only literal with field defaults.
6. **No overloads** (criterion 3). An operation with a variant gets a second
   name, not an optional argument that changes meaning.

Rule 6 is what makes `stop` two calls rather than one: `$.audio.stop(v)` and
`$.audio.stop_all()`. A single `stop()` whose behaviour depends on a missing
argument is an overload in all but name, and `stop_all()` is greppable in a way
that a call site with one fewer argument is not.

**The mandated set — ten functions, grouped by audience:**

```
# Voice path — the developer surface
$.audio.play(desc: VoiceDesc) -> Voice
$.audio.stop(v: Voice)
$.audio.stop_all()
$.audio.get(v: Voice) -> ?Voice
$.audio.set(v: Voice, patch: VoicePatch)

# Device access for a library with its own mixer — the escape hatch
$.audio.push(samples: []f32)
$.audio.free_space() -> int

# Device facts — serving both
$.audio.cursor() -> int
$.audio.sample_rate() -> int
$.audio.underruns() -> int
```

Five types: `Voice`, `VoiceDesc`, `VoicePatch`, `Waveform`, `FilterType`. A clip
is the caller's own `[]f32` plus a rate and a channel count, so it adds no root
name.

**#24 companion count delta: one new module, ten functions, five types.** They
sit under `$.audio` rather than at the root, which is the shape the count exists
to make visible and the precedent the drawing surface will follow.

### Re-parameterising a live voice

`$.audio.set(v, patch)`, one call, where `VoicePatch` has `?T` fields and an
absent field means unchanged. Legal under #15: struct literals are named-field
only *with field defaults allowed*, so `{frequency = 440.0}` is a complete
literal.

Five per-parameter setters were rejected: five names for one operation is the
grammar budget's pressure leaking into the stdlib companion count, and the patch
struct makes *which parameters are continuous* a **type**, readable in one
declaration, rather than a list in prose an agent has to remember.

## Ramping

**Linear over exactly one frame** — the fixed 1/60 s step of #26 — for
`frequency`, `amplitude`, `pan`, `cutoff` and `resonance`.

One frame is the one interval that needs no new number in the spec, and it makes
the ramp target exactly *the value set last frame*, which is checkable. Discrete
parameters — waveform, filter type, loop flag, direction — switch at the next
frame boundary with no ramp.

Two clauses that would otherwise be a silent per-backend divergence:
**amplitude ramps linearly in amplitude, not in dB**, and **frequency ramps
linearly in Hz, not in cents**.

## `stop` is the interrupt, not the note-off

An instant cut mid-waveform is a discontinuity — the exact artefact ADR-0006's
ramping mandate exists to remove.

- `stop` and `stop_all` **ramp amplitude to zero over one frame**, then free the
  slot. Same one-frame linear ramp as every other continuous parameter, so it is
  not a new concept.
- The **handle goes *gone* immediately**; the slot frees a frame later. A caller
  must not have to wait a frame to learn that the voice it stopped is stopped.
- A voice **with a release segment** in its amplitude envelope runs its release
  instead. `stop` is the interrupt; the release is the note-off.

## Envelopes

**Amplitude: ADSR** — attack time, decay time, sustain level, release time. A
sustaining voice's note-off is `stop` (above).

**Pitch: a three-number decay-to-target** — start offset, time, linear curve —
not a second ADSR. The jump sound's rising pitch and the laser's falling one are
both exactly this, and a *sustaining* pitch offset is a vibrato request, which
ADR-0006 already routes to a per-frame `sin` from the frame entry. No routing
matrix, no second control language.

## Clips

**A clip is a caller-owned decoded buffer** — a `[]f32` plus a channel count and
a source sample rate — never a path. The facade does no loading and no decoding:
that is ADR-0006 R7's poll-shaped IO plus the engine tier's decoder, and putting
a path in `play` would smuggle blocking IO into the mandated surface, breaking
R1.

- **One loop region** (start, end) in sample frames. One is `O(1)` and covers
  the sustaining-instrument case.
- Direction forward or reverse, per ADR-0006.
- **Resampling to the device rate is the facade's job**, and lands in the
  toleranced bucket below, not the exact one.

### The clip is the procedural path — a correction to ADR-0006

ADR-0006 justifies the raw sample push by saying it lets a third-party audio
library run on conforming ground, *the service the pixel array already does for
a third-party rasterizer*. **That parallel is wrong and is recorded as wrong.**

A pixel array is **content**. A developer composes something arbitrary — a
plasma, a fire effect, a procedural texture — and hands it to the *ordinary*
drawing path, where it picks up the transform and the target style like any
other primitive. It is expressive and first-class, not a bypass.

Audio's structural equivalent of that is the **caller-authored clip**: fill a
`[]f32` yourself, play it through a voice, and get the envelope, pitch, loop,
filter and ramp for free. Same shape exactly — arbitrary caller-composed content
entering through the normal call.

The raw push is a different animal: it **bypasses the voice system to reach the
device**. Its drawing analogue would be writing to the presentation surface with
your own present timing, which ADR-0004 does *not* mandate.

So the push survives on a **narrower and truer justification**: a third-party
library that owns **its own mixer** — an engine-tier synth, tracker or effects
chain — needs to reach the device without going through voices. That is a real
audience, and it is precisely the tier ADR-0006 put in its own repo, blessed but
unmandated.

This is what makes ADR-0006's own admission land properly. It says the push was
*wrongly offered first as the ergonomic answer — it is not one*. The ergonomic
answer existed all along, and it is the clip. **Nobody should be sent to `push`
to synthesise a sound.**

## The escape hatch: `push`, `free_space`, and no returned count

`free_space()` is ADR-0006 R6's fourth clause — *the caller polls free space and
fills it* — and it describes the **ring**, not the voices. A program that only
plays voices never calls it; the facade fills the ring itself.

It is **external, and useless to an ordinary developer, deliberately**. The
audience is a library author pushing samples, who needs to know how many frames
to generate *before* generating them, or the work is done and discarded.

**`push` returns nothing**, and pushing more than `free_space()` frames is a
`$.assert` bug (see below). Three alternatives were checked and each breaks
something already settled:

- `push` blocks until there is room — violates R1, nothing blocks.
- `push` silently drops the excess — the unhearable silent failure the error
  section forbids.
- `push` returns frames-accepted instead of a poll — makes the poll optional, so
  the common path becomes generate-then-discard, and the pusher still needs a
  pre-generation number to size its work.

Two calls is the floor. `cursor()`, by contrast, **is** a developer call:
frame-precise music sync is R6's stated justification, and a rhythm game written
entirely with `play` and `set` still reads it.

## The play cursor

**Monotonic, in sample frames, never resets, for the process lifetime.**

- **Underrun**: the cursor **advances through the silence**. It counts frames
  the device consumed, silence included, because it is a *clock*; a stalling
  clock makes music sync worse rather than better.
- **Device change**: the cursor **continues**, it does not reset to zero. The
  sample rate may change, so `sample_rate()` must be re-read; the spec states
  the cursor is in frames *at the current device rate*, with a discontinuity in
  wall-clock-per-frame permitted exactly at a device change.
- **Reload**: **untouched**. ADR-0006 R8's runner-owned ring makes this free,
  and it is what makes click-free reload audible in the cursor too.

## Underruns are counted, not raised

Two clauses, kept apart because they are different events at different tiers.

**Ring underrun** (the caller pushed too little, or a late frame): the device
**outputs silence for the missing frames**, and `$.audio.underruns() -> int`
returns a monotonic count since process start. A number, not an error return —
it is diagnostic, not a failure of the call that observes it, and #19's
measurement posture applies: observable, not prohibited.

**File-stream underrun** stays ADR-0006 R7's clause, one tier up, unmandated.

## The error affordance: silence is not a checkerboard

ADR-0005 parked one item here: DragonRuby renders a `nil` pixel array as a
**checkerboard** rather than failing.

**Keep it for drawing, and spec it as a mandated named debug pattern.** It works
for pixels precisely because a wrong image is *visible* — the developer sees the
checkerboard and fixes it in the same second.

**Audio has no equivalent, and inventing one would be worse than nothing.** A
debug tone is a hostile thing to ship into a player's ears, and silence — audio's
natural degenerate output — is indistinguishable from success. So the affordance
moves into the type system and the fault machinery instead:

- **Absent clip**: `play` cannot receive one — a clip is a `[]f32` the caller
  already holds, and an empty slice plays as a zero-length one-shot that
  finishes immediately.
- **NaN, infinite or negative frequency; negative amplitude; NaN pan**: a
  `$.assert` bug per #10 — always on, faults, attributable. Unlike a wrong
  colour these are *unhearable* and would otherwise ship.
- **Amplitude above 1.0 is legal and clamped at the output**, not a bug.
  Summing voices legitimately exceeds unity, and faulting on it would make a
  loud moment a crash.

## Conformance: what is exact, and the tolerance for what is not

ADR-0006 put the tolerance on the same footing as ADR-0005's coverage-not-colour
move: sample-exact where the computation genuinely is exact, a stated per-sample
amplitude tolerance elsewhere.

**Sample-exact:**

- **square, saw and triangle** — sign-and-phase arithmetic, no transcendental;
- **clip playback at unity rate** — a copy;
- **linear envelope segments**;
- **noise**, given the spec-mandated PRNG below. Without a specified generator,
  noise is not a conformance target at all.

**Toleranced, at ±1/32768 (one 16-bit LSB) per sample of amplitude at unity
gain:** sine, any filtered signal (a biquad is recursive and accumulates
floating-point difference), and **resampled clip playback**.

### The mandated noise generator

Noise is exact only if every backend generates the same sequence. The spec fixes
**xorshift32**:

```
x ^= x << 13
x ^= x >> 17
x ^= x << 5
sample = (x as i32) / 2147483648.0    # in [-1, 1)
```

Each voice seeds its own generator from a value in `VoiceDesc`, defaulting to a
spec-stated constant, so two noise voices with the same seed are identical and a
caller who wants decorrelated noise says so.

### The filter's coefficient formulas

**The RBJ Audio-EQ-Cookbook biquad, transcribed into the spec**, for all three
enum members (lowpass, highpass, bandpass), with **resonance expressed as `Q`**.

Chosen because it is the formula family every backend author already knows — the
names transfer, which is #4's naming evidence applied to an implementation
contract — it covers the whole closed enum with one shared derivation, and `Q` is
a stated number, where a "resonance in 0..1" means something different in every
implementation that has ever shipped one.

Fixed alongside the formulas, because each is otherwise a silent divergence:

- **Direct form I.**
- **Coefficients computed in f64**, at the ramp rate (once per frame), and
  applied per-sample.
- Coefficients follow the cookbook's `alpha = sin(w0) / (2 * Q)` form, with
  `w0 = 2 * PI * cutoff / sample_rate`.
- Cutoff is clamped to the open interval `(0, sample_rate / 2)`; a cutoff at or
  past Nyquist is a `$.assert` bug, not a clamp, since it is an unhearable
  mistake.

## Three lenses

**Simplicity — good, with one admitted cost.** The developer surface is five
calls, and the beginner's jump sound is one `play` with a descriptor literal. The
cost is that the procedural path (author a `[]f32`, play it as a clip) requires
knowing what a sample is — but that cost is the same one the pixel array
charges, and it buys the same thing.

**Robustness — strong.** Two silent-failure classes are closed before a backend
exists: an unhearable bad parameter faults rather than shipping, and a push that
overruns the ring faults rather than truncating. The cursor's behaviour is fixed
across all three of its hard cases, and noise is made conformable by naming the
generator rather than by exempting it.

**Agent-friendliness — strong.** No abbreviations, no overloads, one module, one
patch type that *is* the list of continuous parameters, and a filter specified by
a formula family a model has already read a thousand times. The one thing an
agent must not get wrong — sending a developer to `push` for procedural audio —
is now a written sentence rather than an inference from a bad analogy.
