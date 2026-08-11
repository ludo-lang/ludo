# Browser semantics of the mandated audio facade clauses

Research for [#56](https://github.com/adamico/ludo/issues/56) — the audio half.
Checks the three clauses [#41](https://github.com/adamico/ludo/issues/41) froze
into [ADR-0007](../adr/0007-the-audio-facade-surface.md) against the Web Audio
API, under the negative obligation from
[#28](https://github.com/adamico/ludo/issues/28): the spec does not mandate a
web target, but nothing in the mandated facade may be *unimplementable* in a
browser.

## Sources and how they are weighted

Normative spec text outranks MDN prose throughout. Where they disagree it is
said so explicitly.

- **ED** — Web Audio API editor's draft, `webaudio.github.io/web-audio-api`,
  read from the specification source (`index.bs`, `WebAudio/web-audio-api@main`)
  so the `setSinkId()` algorithm could be quoted in full rather than through a
  truncated rendering.
- **TR 1.1** — [Web Audio API 1.1](https://www.w3.org/TR/webaudio-1.1/), the
  published snapshot. **It is not a subset of the ED**: TR 1.1 carries
  `AudioRenderCapacity` / `underrunRatio`, which no longer appears in the ED;
  the ED instead carries `AudioPlaybackStats`, which TR 1.1 does not have. Both
  are cited below and the split is called out where it matters.
- MDN for behavioural and support notes only.

The relevant ludo text is ADR-0007's *The play cursor* and *Underruns are
counted, not raised*, sitting on ADR-0006 **R5** (mixing happens in the frame
entry on the main thread; the platform audio API is push-only, nothing calls
ludo back) and **R8** (the ring buffer is runner-owned and survives reload).
R5 and R8 together are what make two of the three verdicts come out the way they
do: the ring is *ours*, not the device's.

---

## Clause 1 — The play cursor continues across a device change

> **Device change**: the cursor **continues**, it does not reset to zero. The
> sample rate may change, so `sample_rate()` must be re-read […] with a
> discontinuity in wall-clock-per-frame permitted exactly at a device change.
> — ADR-0007

### Verdict: **supplied**

The premise behind the ticket's flag — "a recreated `AudioContext` is a new
clock" — **does not hold under the current spec**. A programmatic device change
does not recreate the context.

**No recreation.** The ED's `setSinkId()` control-message algorithm operates on
the existing `AudioContext` throughout. In order, it: pauses the renderer after
the current render quantum, releases system resources, sets
`[[rendering thread state]]` to `"suspended"` and fires `statechange`, attempts
to *acquire system resources* for the new device, sets `[[sink ID]]`, resolves
the promise, fires `sinkchange`, then sets `[[rendering thread state]]` back to
`"running"` and fires `statechange` again. There is no step that constructs a
context, discards the graph, or resets any clock. `sinkchange` is defined as
dispatched "when changing the output device is completed", and explicitly "not
dispatched for the initial device selection in the construction of
`AudioContext`" — i.e. a sink change is by construction *not* a construction.

**No clock reset.** `currentTime` is "the time in seconds of the sample frame
immediately following the last sample-frame in the block of audio most recently
processed by the context's rendering graph", zero only "if the context's
rendering graph has not yet processed a block of audio", and in the `"running"`
state "monotonically increasing […] updated by the rendering thread in uniform
increments". `resume()` is defined as resuming "the progression of the
`AudioContext`'s `currentTime` **when it has been suspended**" — resuming, not
restarting. Nothing in the ED allows `currentTime` to decrease or return to
zero for a live context. Chrome's own developer documentation for `setSinkId()`
agrees behaviourally: "the audio clock accessible through
`audioContext.currentTime` will still advance to render the audio graph"
([Chrome for Developers](https://developer.chrome.com/blog/audiocontext-setsinkid)).
Chrome shipped `setSinkId()` in M110.

Two caveats, neither of which breaks the clause:

1. **The cursor stalls across the gap; it does not advance through it.** During
   the suspended interval the rendering thread is not processing, so
   `currentTime` does not advance. ADR-0007's *underrun* case says the cursor
   "advances through the silence" because the device consumed those frames; in
   a sink change no device consumes anything, so a stall is the honest reading
   and is covered by the "discontinuity in wall-clock-per-frame permitted
   exactly at a device change" already in the clause. Monotonic and non-resetting
   both hold.
2. **`sampleRate` never changes.** `BaseAudioContext.sampleRate` is the rate at
   which the context handles audio, fixed at construction (`AudioContextOptions.
   sampleRate`), and "all `AudioNode`s in the context run at this rate";
   `setSinkId()` does not touch it. A browser backend resamples to the new
   device internally. So the ADR's *the rate may change, re-read it* is a
   permission the browser never exercises — permissive, not divergent.

**Where the spec is silent, and it is the case that actually bites.** The ED
normatively specifies only the *solicited* change (`setSinkId()`). It says
nothing about an **unsolicited** one — the user unplugs headphones, or the
system default device changes underneath a context whose `[[sink ID]]` is the
empty string. There is no event, no algorithm, and no guarantee. Chromium has
carried device-change handling internally for a long time (e.g.
[crbug 153056](https://bugs.chromium.org/p/chromium/issues/detail?id=153056),
[crbug 557620](https://bugs.chromium.org/p/chromium/issues/detail?id=557620)),
and today follows the default device without asking the page to rebuild the
graph, but that is implementation behaviour, not a contract. Likewise, if
acquisition of the new device *fails*, `setSinkId()` rejects with
`InvalidAccessError` and leaves the context suspended; a page that recovers by
constructing a fresh `AudioContext` gets a clock that starts at zero.

**This does not make the clause unimplementable**, because of R5. The ludo
cursor is not `currentTime`. Under R5/R8 the ring is runner-owned and the mixer
runs in the frame entry, so a browser backend keeps its own monotonic frame
counter and, in the one case where it is forced to build a new context, folds
the old count into an offset. The clause costs a backend one integer, which is
the correct answer for a clause whose whole point is that the cursor is a game
clock and not a device clock.

**Narrowest edit: none required.** If clarity is wanted, one word: change "with
a discontinuity in wall-clock-per-frame permitted exactly at a device change" to
"…permitted exactly at a device change, including an interval during which the
cursor does not advance at all". That records the browser's suspend gap without
touching monotonicity, and it is a clarification of the existing permission
rather than a new one.

---

## Clause 2 — The play cursor is untouched by reload

> **Reload**: **untouched**. ADR-0006 R8's runner-owned ring makes this free.
> — ADR-0007

### Verdict: **supplied under the correct mapping; vacuous under the wrong one**

The clause's meaning depends entirely on what "reload" maps to in a browser, and
the two candidate mappings give opposite answers.

**Wrong mapping — document reload.** If ludo's reload were taken to mean the tab
reloading, the clause is unsatisfiable and also meaningless. Navigating away
destroys the document and with it every `AudioContext`; a new context's
`currentTime` is zero by definition, since "the value of zero corresponds to the
first sample-frame in the first block processed by the graph". Web Audio has no
persistence mechanism, no transferable context, and no way to hand a rendering
graph to a successor document. Back/forward cache does not help: it is not
reached by a reload, and a restored context is a *suspended* one, not a running
one.

**Right mapping — in-process code swap.** ludo's reload is
[#17](https://github.com/adamico/ludo/issues/17)'s quiescent-boundary dylib
swap: the *runner* persists and the game code is replaced. The browser analogue
is re-instantiating the game's WebAssembly module inside a document that stays
alive. Under R5 and R8 everything that carries the cursor is on the runner's
side of that line — the `AudioContext`, its `AudioWorklet`, and the ring buffer
the worklet drains. None of them is owned by the swapped module, so the swap
does not touch them, and the cursor is untouched **by construction rather than
by guarantee**. This is exactly R8's stated mechanism ("the ring buffer is
runner-owned and survives reload, […] so reload is click-free"), landing
unchanged in a browser.

So: no browser API is needed to supply this clause, and no browser API can
threaten it. The clause is only at risk from a *misreading* of what reload
means.

**Narrowest edit: one clause of definition, not of behaviour.** Amend
ADR-0007's reload bullet to read "**Reload** (the in-process swap of game code;
not a restart of the host process or document): **untouched**." That closes the
only failure mode — a web backend implementing "reload" as `location.reload()`
and then correctly reporting that the clause cannot be met.

---

## Clause 3 — Underruns are counted and reported

> **Ring underrun** (the caller pushed too little, or a late frame): the device
> **outputs silence for the missing frames**, and `$.audio.underruns() -> int`
> returns a monotonic count since process start. — ADR-0007

### Verdict: **supplied** — and, unusually, supplied twice over

**The decisive route needs no browser API at all.** ADR-0007 defines the counted
event as a **ring** underrun: the ludo-side mixer failed to supply frames into a
ring the runner owns. Under R5 the mixer runs in the frame entry and the ring is
ours; a browser backend's `AudioWorklet` `process()` callback drains that ring
itself, and therefore *knows*, at each call, whether the ring held a full render
quantum. Counting shortfalls is arithmetic on the backend's own data structure,
not an observation of the browser. Nothing about `underruns()` depends on the
platform exposing anything. This is the point of R5 that the ticket's framing
("is this unobservable in a browser?") did not anticipate.

**And the browser now exposes device-level underruns too**, which the backend
does not need but which makes the report strictly better where available. Here
the two specification documents diverge:

- **ED — `AudioPlaybackStats`.** This is the near-exact match for
  `underruns()`. It defines an *underrun frame* as "an audio frame played by the
  output device that was not provided by the `AudioContext`" ("typically
  silence" — the same behaviour ADR-0007 mandates), and an *underrun event* as
  "the playback of a continuous sequence of underrun frames". It exposes
  `underrunEvents` (an `unsigned long`, "the total number of underrun events
  that have occurred […] since its construction") and `underrunDuration`.
  Monotonic, cumulative from construction, a number rather than an error — the
  same shape as `underruns()`.

  Two gates the ED attaches, both privacy-motivated (the spec records a
  cross-site covert-channel risk): the stats update **only once per second**,
  and the update algorithm aborts entirely unless the document is fully active
  and visible, or microphone permission is granted. A backgrounded tab therefore
  freezes the count. Coarse resolution and a visibility gate — not absence.

- **TR 1.1 — `AudioRenderCapacity`.** The published snapshot has no
  `AudioPlaybackStats`; it has `AudioContext.renderCapacity`, started with
  `start(options)`, firing `update` events carrying `timestamp`, `averageLoad`,
  `peakLoad`, and `underrunRatio` — "a ratio between the number of buffer
  underruns (when a load value is greater than 1.0) and the total number of
  system-level audio callbacks over the given update interval", with load
  precision "limited to 1/100th". This is a *rate over a window*, not a
  cumulative count, and it is event-driven rather than pollable, so
  reconstructing a monotonic total from it is lossy. It is the weaker of the two
  and a backend should prefer the ED interface where present.

`baseLatency` and `outputLatency` are also available and are worth naming here
only to rule them out: they are latency estimates, not glitch counts, and the ED
warns that `currentTime` minus `getOutputTimestamp().contextTime` "cannot be
considered as a reliable output latency estimation". They diagnose, they do not
count.

**MDN vs. normative text.** MDN's pages track the ED interface
(`AudioPlaybackStats`) while `AudioRenderCapacity` remains in the published TR;
neither MDN page contradicts normative text, but a reader consulting only MDN
would not learn that the two interfaces coexist across documents. The normative
split above is the fact to carry.

**Narrowest edit: none.** ADR-0007 already scopes `underruns()` to the ring
underrun and pushes the device/file-stream case to R7 one tier up, which is
precisely the scoping that makes the clause backend-independent. If anything is
added, add a note, not a clause: *a backend may fold device-reported underruns
into the count where the platform exposes them (`AudioPlaybackStats.
underrunEvents`), but the mandated count is of ring underruns, which the backend
observes directly.*

---

## Summary

| # | Clause | Verdict | Edit needed |
|---|---|---|---|
| 1 | Cursor continues across device change | **supplied** | none; optional one-phrase clarification that the cursor may stall, not merely skew |
| 2 | Cursor untouched by reload | **supplied** (under the code-swap mapping) | one parenthetical defining "reload" as the in-process code swap |
| 3 | Underruns counted and reported | **supplied** | none |

No clause is divergent and none is unobservable. The ticket's flagged
worst case — a recreated `AudioContext` as a new clock — is not what the spec
does: `setSinkId()` suspends and resumes the same context, and `currentTime`
neither resets nor decreases. The two residual risks are both about wording
rather than capability: an unsolicited device change is unspecified territory
that only implementation behaviour covers, and "reload" needs pinning to the
code swap so that no backend reads it as a document reload.

### Citations

- Web Audio API editor's draft — https://webaudio.github.io/web-audio-api/
  (`currentTime`, `sampleRate`, `resume()`, `getOutputTimestamp()`,
  `baseLatency`, `outputLatency`, `setSinkId()` and its control-message
  algorithm, `sinkchange`, `AudioPlaybackStats` and the *update audio stats*
  algorithm, and its privacy considerations). Quoted from the specification
  source, `WebAudio/web-audio-api@main:index.bs`.
- Web Audio API 1.1 — https://www.w3.org/TR/webaudio-1.1/
  (`AudioRenderCapacity`, `AudioRenderCapacityEvent.underrunRatio`,
  `averageLoad`, `peakLoad`).
- MDN — `AudioContext.setSinkId()`, `AudioContext.sinkId`,
  `AudioContext.sinkchange` (behaviour and support notes only).
- Chrome for Developers, *Change the destination output device in Web Audio* —
  https://developer.chrome.com/blog/audiocontext-setsinkid (M110; the clock
  advancing across a sink change).
- Chromium issue tracker — 153056, 557620 (unsolicited output-device changes
  handled by the implementation where the spec is silent).
