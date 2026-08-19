# Coverage: chapter 6 — the standard library

**Non-normative.** This file is the closing test for
[`../06-stdlib.md`](../06-stdlib.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §17 below says which
happened.

**Chapter 6's boundary is every name under `$.`** ([#99](https://github.com/ludo-lang/ludo/issues/99)),
which is why the blessed math set (ADR-0016) is here and not in a chapter about
types: `$.vec2` is a `$.`-rooted stdlib type, not a facade.

---

**Absorbed here:** ADR-0004, ADR-0005, ADR-0006, ADR-0007, ADR-0008, ADR-0009, ADR-0010, ADR-0011, ADR-0012, ADR-0016, ADR-0022, ADR-0026, ADR-0034, ADR-0038.

Read by `tools/check-absorption.py`, which asserts that each carries the
ADR-0051 §4 stamp naming this chapter, and that no ADR claims this chapter
without appearing here (ADR-0055 §4). ADR-0004 is here though §0 predates the entry: the mandated facade over a delegated renderer is transcribed as rules in this chapter, and no other chapter claims it (chapter 8 §7 *reaches* its core/full split without absorbing it).

## 0. Source check (#72, #73)

Run over **this chapter's sources only** — ADR-0005, ADR-0006, ADR-0007,
ADR-0008, ADR-0009, ADR-0010, ADR-0011, ADR-0012, ADR-0016, ADR-0022, ADR-0026,
ADR-0034 and ADR-0038 — before transcription, per `docs/agents/domain.md`.

`tools/check-stamps.py` reports zero findings over the whole corpus both before
and after this chapter. §0.1's three findings are of the silent kind and were
found by reading every ADR that cites these thirteen — with a twist worth
recording, because it is a **new** way for the floor to be blind (§0.4).

### 0.1 Direction 1 — stamps owed and missing

Applied retroactively in this chapter's commit; stamps are additive metadata,
not prose.

| Record stamped | What moved it, and what moved |
|---|---|
| **ADR-0009** | **ADR-0043 §11.** `TextDesc { text: string }` → `text: []u8`. Declared in ADR-0043's own list and never stamped. |
| **ADR-0010** | **ADR-0043 §11.** Same descriptor, same change, same omission. |
| **ADR-0011** | **ADR-0043 §11.** `$.input.text() -> string` → `-> []u8`, with the frame-lifetime clause. Recorded only in ADR-0011 §294's prose, never in the header. |

### 0.2 Direction 2 — stamps **not** owed, recorded so they are not re-derived

- **ADR-0024** confirms ADR-0006 R5 and gives its rule a general statement
  (*"ADR-0006 R5's 'stands for everything else' is now closed"*). It closes
  #29's residual conflict; **no clause of ADR-0006 moves**. Transcribed at
  ch6 §10.7.
- **ADR-0019** reads ADR-0005, ADR-0007, ADR-0009, ADR-0010 and ADR-0011 as
  *the normative surface a claimant conforms to*, and ADR-0006 R1–R8 as
  backend-facing. Framing for chapter 8; no clause moves.
- **ADR-0028** cites ADR-0006 R6 and ADR-0007 as the reason `$.audio.cursor()`
  and `$.audio.sample_rate()` pass the admission test, and cites ADR-0016 for
  *the shipped quantity list is closed* when refusing a `PhysicalPixels` type.
  Precedent in both directions. ADR-0028's own §6 row for the render-scale
  getter is ADR-0038's amendment, already stamped on ADR-0028.
- **ADR-0035** cites ADR-0007's filter coefficients and ramp as things the
  render ceiling keeps well-defined. Precedent; the ramp clause is unchanged.
- **ADR-0037** amends ADR-0022 §1 and ADR-0008, and **both carry the stamp**.
  Its reading of ADR-0006 R3 (a rung is build-time, not runner-time) is
  derivation.
- **ADR-0042** cites ADR-0016 §6.1's alignment mandate as the reason an
  allocator takes `align` as a **parameter** rather than reading a type
  attribute. It constrains the allocator, not the blessed types; no clause of
  ADR-0016 moves.
- **ADR-0052** cites ADR-0016 §2's *a column is a `[]T` view*. Precedent.
- **ADR-0017** cites ADR-0016 §3's operator exception as the class its own
  `fn`-type rule belongs to. Precedent.
- **ADR-0029** cites ADR-0008 §4 for the attribute mechanism's existence and
  records that ADR-0016 depends on `#align(n)`. Precedent; ADR-0029's own
  correction of ADR-0024 §6 was stamped by chapter 4.
- **ADR-0039** and **ADR-0040** both build on ADR-0038 — §10's paying-in-writing
  precedent, §5's weakened MUST, §8's grid — and **neither amends it**. Their
  own amendment lists name ADR-0025, ADR-0028, ADR-0030, ADR-0031 and ADR-0036
  and no source of this chapter. `$.video` therefore stays at **four
  functions**: ADR-0039 refuses `stretch`/`set_stretch` rather than shipping
  them.
- **ADR-0032** overturns ADR-0005's caller-declared logical canvas and **carries
  the stamp**; ADR-0030 and ADR-0031 likewise. All three are chapter 5's
  sources, and ch6 §4.2.7 routes rather than restates.
- **ADR-0027**, **ADR-0014**, **ADR-0030** and **ADR-0032** cite ADR-0012 for
  *the spec's one constraint on how an implementation compiles*. Precedent; the
  contingency ADR-0033 opened was discharged by ADR-0034 §5, and both stamps are
  on ADR-0012 already.
- **ADR-0036** cites ADR-0006 R8's ring, ADR-0011's pointer input and ADR-0008's
  fullscreen shader case. Precedent; its own §7 no-screen-readback clause is
  transcribed at ch6 §4.9.11 as ADR-0038 §11 restates it.
- **ADR-0015** demoted the checkerboard and ADR-0033 renamed it; **both stamps
  are on ADR-0007, ADR-0009 and ADR-0010** where owed. ch6 §4.9.10 transcribes
  the post-rename rule only.
- **ADR-0047** restamped `get_pixels`'s parameter mark on ADR-0009 and ADR-0022;
  both stamps were applied by chapter 3.

### 0.3 Issue sources against `SOURCES.md` (#73)

This chapter cites issues only as **grounds** — #4 for the naming rules, #5 for
one-spelling-per-entity, #8 for the `O(1)` audio line, #10 for errors as values,
#11 for static dispatch, #12 for the absent asset pipeline, #15 for view
lifetime, #19 for the properties chapter 8 owns, #24 for the budget chapter 8
owns, and #22 for the oracle. **No clause of chapter 6 rests on an issue as its
sole authority**: every rule above has an ADR behind it, which is the shape
§1–§13 below record.

Two `SOURCES.md` §1 rows were checked and needed no edit:

- **#15** — its Q10 view rule reaches ch6 §4.9.9 (a slice must not outlive its
  parent) through ADR-0009, and ADR-0047's re-cut does not touch that clause.
- **#60** — its blanket ban on program-visible backend state is partly withdrawn
  by ADR-0025 §8 and replaced by ADR-0028's admission test. Chapter 6's only
  backend-reading queries are `$.video`'s pair and `$.audio`'s two device facts,
  and **all four were run through ADR-0028's test by their own ADRs**. No clause
  here cites #60.

`SOURCES.md` is therefore unedited by this chapter.

### 0.4 A new blind spot in the checker, recorded

`tools/check-stamps.py` did not see §0.1's three findings, and the reason is
**not** the documented one (*an ADR that amends without declaring it*).
ADR-0043 **does** declare them — under a heading spelled `## 11. The stamp
list` rather than `## Amendments this ADR makes`. The checker matches the
heading, so a declared amendment under a differently-spelled heading is invisible
to it, which is a strictly worse failure than the known one because the author
did everything right except pick the conventional heading.

This is a **code** repair, so it goes behind a branch and a PR under ADR-0046
§7 rather than into this docs commit. Filed as its own ticket.

### 0.5 Absorbed-by stamps (ADR-0051 §4)

All thirteen sources carry an **Absorbed by [spec ch6 …]** stamp applied in this
chapter's commit, owed whether or not the chapter changed the rule. Their
`STATUS.md` rows move to `absorbed-by-ch-6`.

---

## 1. ADR-0005 — the drawing facade's surface

| Source clause | Landed |
|---|---|
| Coordinate system: top-left, Y-down, not configurable | §4.1.1 |
| Handedness is fixed by prior conformance (the argument) | Rationale. **Explicitly dropped**, see §17 |
| A transform does not buy Y-up | §4.1.3 |
| Origin location fixed top-left; centre-origin via the transform; no origin mode | §4.1.2 |
| Straight alpha, sRGB | §4.1.4 |
| Premultiplied and linear-space rejected; both reachable at the renderer tier | Rationale + a delegated-tier fact. **Explicitly dropped**, see §17 |
| Web backend: opaque presentation surface | §4.1.5 |
| Web backend: decode images itself, upload raw bytes | §4.1.6 |
| Target is opaque, passed, never ambient | §4.2.1 |
| `style:` is one token, `crisp` \| `smooth`, `crisp` default | §4.2.2 |
| `crisp` is bit-exact across the backend matrix | **Deleted by ADR-0022 §2.** Replaced by §4.11.2–§4.11.3 |
| Target carries a 2D transform | §4.2.4 |
| Caller-declared logical canvas | **Overturned by ADR-0032.** Routed at §4.2.7 |
| Offscreen-target shape reserved | §4.2.5 |
| Two normalized anchors, both defaulting to centre | §4.7.1 |
| The byte-exact blit paragraph | **Deleted by ADR-0034 §1.** §4.11.7 records the deletion |
| The texel-centre and tie-break convention | **Phantom.** ADR-0034 §4; §4.11.8 states the replacement |
| `measure` returns advance and bounds, accounts for kerning | §4.8.3, §4.8.4 |
| Per-glyph positions excluded | §4.8.5 |
| Default font by name, version and content hash | §4.8.6 |
| Blend is a closed enum of three modes | §4.4.8 |
| Composable blend factors stay at the renderer tier | §4.4.8, §4.10.2 |
| Coverage-not-colour tolerance, `smooth` only | **Deleted by ADR-0022 §3.** Replaced by §4.11.6 |
| `crisp` is the default and the conformance-testable case | §4.2.2; the exactness half is §4.11.3 |
| The literal spelling is still open | Discharged by ADR-0009. No clause owed |
| `@125`/`@150`/`@200` variant selection excluded | §4.10.1 |
| The `nil`-pixel-array checkerboard, recorded for later | Discharged by ADR-0007 → ADR-0009 → ADR-0015 → ADR-0033. Landed at §4.9.10 |

## 2. ADR-0006 — the platform layer's forbidden and required shapes

| Source clause | Landed |
|---|---|
| Two forcing arguments (browser; ludo off the frame thread) | Rationale. **Explicitly dropped**, see §17 |
| Scope: this designs no API | §10.1 |
| R1 nothing blocks | §10.2 |
| R1's asset case resolved by top level before any frame | §10.3 |
| R1 rejected: a blocking read in loading only | §10.2 (second sentence) |
| R2 one struct of non-capturing function pointers | §10.4 |
| R2 language consequence: function-pointer type | §10.4, routed to ch2 §7 |
| R3 no conditional compilation; target selection at build | §10.5 |
| R3 language consequence: target-selected module set | §10.5, routed to ch4 §10 |
| R4 identical signature on every target | §10.6 |
| R4 a divergent signature is a **type error** (ADR-0014) | §10.6 |
| R5 audio pushed from the frame entry; nothing calls back | §10.7 |
| R5 audio DSP spends frame budget | §10.8 |
| R5 no voice cap / stealing policy / voice model in the spec | §5.1.3 |
| R6 push is a copy, no mapped region | §5.5.5, indexed at §10.9 |
| R6 device reports its rate; f32 interleaved | §5.5.6, indexed at §10.9 |
| R6 monotonic play cursor in sample frames | §5.7.1, indexed at §10.9 |
| R6 caller polls free space | §5.5.2, indexed at §10.9 |
| R6 latency is a ring depth the program chooses | §10.10 |
| R7 file IO is request-and-poll; we decode; underrun outputs silence | §10.11, §5.7.6 |
| R8 runner-owned ring survives reload; click-free reload | §10.12 |
| R8 after a fault: drain, then silence, never a loop | §10.12 |
| R8 attaches to existing properties at full conformance | §10.12, routed to ch8 |
| R8 core conformance is headless | §10.13 |
| The audio engine tier is unmandated | §5.6.4 |
| The facade's audio altitude was wrong (the argument) | Rationale. **Explicitly dropped**, see §17 |
| Mandated addition: voice, closed waveform enum, envelopes, one-shot/loop, forward/reverse | §5.4.3, §5.4.1, §5.4.2, §5.4.6 |
| Mandated addition: filter as a voice parameter, closed enum, cutoff and resonance, stated formulas | §5.10.1, §5.10.2 |
| Mandated addition: raw push and play cursor | §5.5.1, §5.7.1 — on ADR-0007's **corrected** justification, §5.4.8 |
| The line: `O(1)`, spec-sized, allocates nothing, no routing | §5.6.1 |
| The line sorts: delay/reverb out, mix bus out, LFO out on routing alone | §5.6.2 |
| Caller-supplied memory is fine | §5.6.3 |
| The earlier across-time/across-voices draft was wrong | Rationale. **Explicitly dropped**, see §17 |
| Tolerance: sample-exact where exact, stated tolerance elsewhere | §5.8.1, §5.8.2 (ADR-0007 supplies the numbers) |
| Ramping is mandated; per-frame modulation is the mechanism | §5.3.1, §5.3.4 |
| "Drawing needs eight verbs" | A wrong count, corrected by ADR-0033; decorative. **Explicitly dropped**, see §17 |
| The rotated sprite's texel-centre tie-break rule | **Phantom**, corrected by ADR-0034 §4. §4.11.8 |

## 3. ADR-0007 — the audio facade's surface

| Source clause | Landed |
|---|---|
| Voices play; they do not render | §5.1.1 |
| The sum into one device is not routing | §5.1.2 |
| The render shape falls out of voices plus push | §5.5.1 |
| There is no voice cap | §5.1.3 |
| `play` returns `Voice`, not `?Voice` | §5.2.3 |
| `get` returns `?Voice`; the handle goes gone | §5.2.4 |
| No stealing policy | §5.1.3 |
| The voice set is runner-owned; the facade allocates nothing | §5.1.4 |
| The real limit is frame budget | §5.1.5 |
| Mono voice, continuous `pan`, stereo out, f32 interleaved at device rate | §5.1.6 |
| Constant-power pan, −3 dB centre, with the formula | §5.1.7 |
| A stereo clip: `pan` is a balance | §5.1.8 |
| Naming rules 1–6 | §1.1.1–§1.1.6 |
| Rule 6 makes `stop` two calls | §1.2 |
| The mandated set of ten functions | §5.2.1 (with ADR-0016's `cursor` and `seconds`) |
| Five types | §5.2.2, **corrected to six** by ADR-0033 (`Clip`) |
| "A clip adds no root name" | **Superseded by ADR-0033.** §5.2.2 |
| `set(v, patch)`, `?T` fields, absent means unchanged | §5.2.5 |
| Five per-parameter setters rejected | §5.2.5 (final sentence) |
| Ramping: linear over one frame, five named parameters | §5.3.1 |
| Amplitude ramps in amplitude, frequency in Hz | §5.3.2 |
| Discrete parameters switch at the frame boundary | §5.3.3 |
| `stop`/`stop_all` ramp to zero over one frame, then free | §5.3.5 |
| The handle goes gone immediately; the slot frees a frame later | §5.3.6 |
| A release segment runs instead | §5.3.7 |
| Amplitude envelope is ADSR | §5.4.1 |
| Pitch envelope is a two-number decay-to-target, curve linear by mandate | §5.4.2 |
| A clip is caller-owned `[]f32` + channels + rate, never a path | §5.4.4 |
| One loop region in sample frames | §5.4.5 |
| Direction forward or reverse | §5.4.6 |
| Resampling to the device rate is the facade's job, toleranced | §5.4.7, §5.8.2 |
| The clip is the procedural path; the push is not | §5.4.8 |
| The push's narrower justification: a library with its own mixer | §5.5.1 |
| `free_space` describes the ring, not the voices | §5.5.2 |
| `push` returns nothing; three alternatives rejected | §5.5.3 |
| Overrunning the ring is a `$.assert` bug | §5.5.4 |
| `cursor` is a developer call | §5.7.1 (the clause); the audience note is rationale |
| Cursor monotonic, in frames, never resets, process lifetime | §5.7.1 |
| Underrun: the cursor advances through the silence | §5.7.2 |
| Device change: the cursor continues; re-read `sample_rate` | §5.7.3 |
| Reload: untouched; *reload* defined (ADR-0022 §5) | §5.7.4 |
| Ring underrun outputs silence; `underruns()` counts | §5.7.5 |
| File-stream underrun stays R7's, one tier up | §5.7.6 |
| Keep the checkerboard for drawing as a named debug pattern | Superseded twice (ADR-0015, ADR-0033); landed at §4.9.10 |
| Audio has no equivalent; no debug tone | §5.7.10 |
| Absent clip: unreachable; empty slice is a zero-length one-shot | §5.7.7 |
| NaN/infinite/negative frequency, negative amplitude, NaN pan are `$.assert` bugs | §5.7.8 |
| Amplitude above 1.0 legal, clamped at output | §5.7.9 |
| Sample-exact list | §5.8.1 |
| Toleranced list at ±1/32768 | §5.8.2 |
| xorshift32, with the formula | §5.9.1 |
| Per-voice seed from `VoiceDesc`, spec-stated default | §5.9.2 |
| RBJ biquad, three members, resonance as `Q` | §5.10.2 |
| Direct form I | §5.10.3 |
| Coefficients in f64, at the ramp rate, applied per-sample | §5.10.4 |
| `alpha = sin(w0)/(2Q)`, `w0 = 2·PI·cutoff/sample_rate` | §5.10.5 |
| Cutoff clamped to `(0, rate/2)`; at or past Nyquist is a bug | §5.10.6 |
| `$.audio.cursor() -> int` | **Superseded by ADR-0016 §5.** §5.2.6 |

## 4. ADR-0008 — shader ludo and the extern shader declaration

| Source clause | Landed |
|---|---|
| Shaders are opt-in | §9.1 |
| §1 the map owns the dialect and the checker | §9.4 |
| §1 the IR and emitters are renderer-repo, unmandated | §9.4 |
| §1 the dialect may not exceed the IR's ceiling | §9.5 |
| §1 rejected: the map owns the IR / owns none of it | Rationale. **Explicitly dropped**, see §17 |
| §2 strict subset binds in both directions | §9.6 |
| §2 removals are free (the list) | §9.7 |
| §2 additions are excluded rather than granted syntax | §9.8 |
| §2 textures/samplers are `distinct` opaque `$.` types | §9.9 |
| §2 builtins and sampling are free functions in a shader-only module | §9.9 |
| §2 address spaces ride on the binding declaration | §9.9 |
| §2 uniformity is a checker rule with a named diagnostic | §9.9 |
| §3 four locked guarantees have no GPU implementation | The four are handled individually below; the enumeration is rationale. **Partly dropped**, see §17 |
| §3 indexing is deletable; the guarantee is vacuous | §9.11 |
| §3 sampling is not indexing; clamp-vs-repeat is a sampler parameter | §9.12 |
| §3 vertex and fragment only; compute is `.wgsl` | §9.10 |
| §3 integer overflow is an enumerated exception | §9.13 |
| §3 rejected: emit checks in the shader with a readback | §9.13 (final sentence) |
| §4 `#vertex`/`#fragment` attributes, no new keyword, never a magic name | §9.14 |
| §4 the marker puts the checker into shader mode | §9.15 |
| §5 an unmarked in-subset function is callable from both | §9.16 |
| §5 calling a marked function from CPU code is a compile error | §9.17 |
| §6 blessed math stays library types; the emitter recognises by name | §9.18 |
| §6 rejected: vectors become built-in types | §9.18 (final sentence) |
| §6 the veto that fixes the blessed set | §9.19, §3.2 |
| §7 `sample` / `sample_level`, with the straight-line restriction | §9.20 |
| §8 a `.wgsl` file is named by an `extern shader` declaration | §9.21 |
| §8 "asset" is retired; no pipeline, no build step | §9.22 |
| §8 no `unsafe` on the declaration | §9.23 |
| §9 three comparisons and nothing more | §9.24 |
| §9 declaration surface only, never expression semantics | §9.24 |
| §9 layout decidability rests on #25 | §9.25 |
| §9 an unnameable WGSL type is a compile error naming type and field | §9.26 |
| §9 reported at the ludo declaration, WGSL named as counterparty | §9.27 |
| §10 failed recompile retains last-good pipeline, error value, no fault | §9.28 |
| Compute in shader ludo stays unwritten; the route back is additive | §9.10 states the rule; the route is rationale. **Partly dropped**, see §17 |
| ADR-0037's rung and its compile-error clause | §9.2, §9.3 |

## 5. ADR-0009 — the drawing facade's call spellings

| Source clause | Landed |
|---|---|
| The module is `$.graphics`, an area noun, not `$.draw` | §1.5 |
| Root names are constructors for blessed types; modules are areas | §1.4 |
| Input is its own module | §6 (the whole section) |
| Verb-first | §1.1.2 |
| Three verbs and the adjudication rule | **Amended to two by ADR-0010.** §4.4.3 |
| `fill_text`, not `draw_text`; the pair reserved | §4.8.8 |
| The nine-call mandated set | **Superseded by ADR-0010's ten.** §4.4.1 |
| The six further names | §4.4.2, with ADR-0033's rename |
| `stroke_` for every closed shape | §4.4.1 |
| A stroke is the shape minus its inset shape | **Demoted to a consequence by ADR-0010.** §4.6.1 |
| Degenerate inset yields the filled shape | §4.6.2 |
| The triangle corner is the miter, stated; no miter limit | §4.6.3 |
| A stroked ellipse's band is not constant width | §4.6.4 |
| Two overlapping draws composite twice | §4.6.5 |
| A descriptor struct per call, one type per call | §4.5.1 |
| Descriptor fields are math types, not scalars | §4.5.2 |
| The descriptor table | §4.5.3 (as amended by ADR-0010 and ADR-0043) |
| `color: Color` and `blend: Blend` on every descriptor | `color` **replaced by ADR-0010**; `blend` at §4.5.3 |
| `blend` is per-call, not on the target; no ambient draw state | §4.5.4 |
| Stroke descriptors repeat fields flat | §4.5.5 |
| `thickness` not `width`; `font_size` not `size` | §4.5.7 |
| The ellipse is inscribed in a bounding box | §4.6.6 |
| `width`/`height` are full axis lengths | §4.6.6 |
| Rejected: `{center, radius}`, foci and eccentricity | Rationale. **Explicitly dropped**, see §17 |
| Shapes are axis-aligned; sprites rotate | §4.6.7 |
| `stroke_line` does not collapse into `fill_rect` | §4.6.8 |
| `Target` is the type name; not `Canvas`, not `Screen` | §4.2.1 |
| The runner passes the target into the frame entry; no `screen()` | §4.2.6 |
| UFCS gives `target.fill_rect({...})` | §1.7 |
| `Image` is the type; not `Texture`, not `Sprite` | §4.9.1 |
| This ADR names the type and does not spell its loader | §4.9.2 |
| `Image` is the same type the pixel array produces | §4.9.3 |
| No pixel-array draw call | §4.9.4 |
| `slice_image` over a `source: Rect` field | §4.9.8 |
| A slice must not outlive its parent | §4.9.9 |
| Flip booleans; negative `size` rejected | §4.7.2 |
| `measure_text` is pure and takes no target | §4.8.1 |
| Same descriptor type as `fill_text` | §4.8.2 |
| `TextMetrics { advance, bounds }`; logical units | §4.8.3 |
| `default_font` reaches ADR-0005's identified font | §4.8.6 |
| Types are `PascalCase`; enum members `snake_case` | §1.3 |
| `Style` and `Blend` live in `$.graphics` | §1.6 |
| The checkerboard is a value, not a behaviour | §4.9.10 (renamed by ADR-0033) |
| `fill_sprite` not mandated here | **Mandated by ADR-0010.** §4.7.3 |
| `stroke_sprite` is renderer-tier | **Reversed by ADR-0010.** §4.7.8–§4.7.10 |
| No point primitive, no `draw_point` | §4.4.6 |
| `circle` is not a name | §4.4.5 |
| The hit flash is byte-exact on a `crisp` target (§372) | **Stale**, withdrawn by ADR-0022 §4 and routed to #72 by ADR-0034 §7. §4.7.4 states the withdrawal |
| `TextDesc { text: string }` | **Amended by ADR-0043 §11** to `[]u8`. §4.8.7 |
| `get_pixels(image: Image)` | **Restamped by ADR-0047 §5** to `!Image`. §4.4.2 |

## 6. ADR-0010 — `Paint` and the two-verb facade

| Source clause | Landed |
|---|---|
| Rejected: collapsing `draw_sprite` into `fill_rect` | Rationale. **Explicitly dropped**, see §17 |
| `Paint = Color \| Texture`; `Texture`; `Mapping` | §4.3.1 |
| A pattern is not a third variant | §4.3.2 |
| `stretch` and `tile`, stated against the bounding box | §4.3.3 |
| The tile origin is the shape's | §4.3.4 |
| `offset` shifts the mapping; no `scale` field | §4.3.5 |
| The facade exposes no texture matrix, sampler, address mode or UV set | §4.3.6 |
| `paint: Paint` on all calls; `Color` no longer a descriptor field | §4.3.7 |
| No implicit `Color`→`Paint` conversion | §4.3.8 |
| `fill_sprite` fills the alpha silhouette with `paint`, modulating | §4.7.3 |
| The default paint draws the ordinary sprite | §4.7.3 |
| The colour-paint hit flash | §4.7.4, **with its byte-exactness withdrawn** (ADR-0022 §4) |
| A different image is a legal masked fill | §4.7.5 |
| `SpriteDesc` keeps both `image` and `paint` | §4.7.6 |
| `color_mode` rejected | §4.7.7 |
| `stroke_sprite` threshold fixed at `alpha >= 0.5` | §4.7.8 |
| `stroke_sprite` thickness in logical units | §4.7.9 |
| Feathered artwork outlines fat; cost stated | §4.7.10 |
| A stroke is a path with thickness; round caps for open paths | §4.6.1 |
| `fill_line` does not exist; the set is fills and strokes | §4.4.4 |
| No polyline | §4.4.7 |
| The ten-call mandated set | §4.4.1 |
| The descriptor table with `paint` and `blend` | §4.5.3 |
| `SpriteStrokeDesc` is a flat repeat | §4.5.5 |
| A `thickness`-selects-the-operation descriptor is rejected | §4.5.6 |
| The flat rule strains at nine fields (recorded honestly) | Rationale. **Explicitly dropped**, see §17 |
| The byte-exact blit restated and strengthened | **Deleted in full by ADR-0034 §1.** §4.11.7 |
| The six further names unchanged | §4.4.2 |

## 7. ADR-0011 — the input facade

| Source clause | Landed |
|---|---|
| The complaint (AZERTY, role not layout) | Rationale. **Explicitly dropped**, see §17 |
| The virtual controller is the primary surface; raw is the escape hatch | §6.1.1 |
| A game written against it never names a key | §6.1.2 |
| Rebindable actions rejected | §6.1.3 |
| The binding table is mandated (a convention, not a mechanism) | §6.1.4, §6.7 |
| `Button` is twelve ordinals | §6.2.1 |
| A control is in the set iff every conforming pad has it | §6.2.2 |
| The ordinal table | §6.2.3 |
| South is `one`; no Switch confirm remap | §6.2.4 |
| Integer indices and bitmasks rejected | §6.2.5 |
| `direction(side, player) -> Vec2`; `Side = left \| right` | §6.3.1 |
| Keys synthesise 1.0; the diagonal is unit length | §6.3.2 |
| A stick's magnitude passes through, clamped to 1.0 | §6.3.3 |
| The dead zone is fixed and rescaled to the rim | §6.3.4 |
| `direction(.left)` unions d-pad, stick and keys | §6.3.5 |
| `direction(.right)` falls back to face buttons | §6.3.6 |
| Why the dead zone is not a knob | Rationale; the cost (a fighting game drops a tier) is a delegated-tier fact. **Explicitly dropped**, see §17 |
| One number, not three (no raw/perc/angle) | §6.3.7 |
| Analog triggers are not exposed | §6.2.6 |
| `_down`/`_pressed`/`_released` for buttons, keys, pointer | §6.4.1 |
| Edge state is the runner's | §6.4.2 |
| No auto-repeat; never a flag on `pressed` | §6.4.3 |
| Players are slots, max four, `player: int = 0`; no handle | §6.4.4 |
| An absent device is present and idle | §6.4.5 |
| Unplugging does not renumber | §6.4.6 |
| The keyboard is player 0; two-on-one-keyboard is uncovered | §6.4.7 |
| `connected` exists for "press Start to join" | §6.6.1 (in the set); its purpose is rationale |
| Pointer position in logical-canvas space, camera not applied | §6.4.9 |
| A pointer in the bars reports outside the canvas rect; no clamping | §6.4.10 |
| Pointer lock is a request that can fail | §6.4.11 |
| Scroll is a `Vec2`; horizontal exists | §6.4.12 |
| Keys identified by what they print; position where nothing prints | §6.5.1 |
| `Key` is closed; unnamed keys are dropped | §6.5.2 |
| International keys in; F13–F24, media, vendor out | §6.5.3 |
| No `unknown(u32)` variant | §6.5.4 |
| Modifiers are `Key` members, left/right distinguished; no bitset | §6.5.5 |
| `text()` returns the frame's composed text | §6.5.6 |
| No general event queue | §6.5.7 |
| `key_label` rejected by name; the same-frame rebind idiom | §6.5.8 |
| `mapping_for` not mandated | §6.5.9 |
| No controller name | §6.5.10 |
| The 22-function mandated set and five types | §6.6.1 |
| No descriptor struct anywhere in `$.input` | §6.6.2 |
| `last_source()` drives glyph switching | §6.6.3 |
| `to_world` lives in `$.graphics` | §4.2.8, §6.6.4 |
| The binding table | §6.7.1 |
| Players 1–3 have no keyboard bindings | §6.7.2 |
| Input is read by ambient calls | §6.8.1 |
| Latched once per frame; every query sees one snapshot | §6.8.2 |
| The reasoning does not rescue `Target`; the conflict raised | §6.8.3 (resolved by ADR-0013) |
| The web obligation discharged (four degradations) | Each is a clause elsewhere: §6.4.11, §6.4.8, §6.5.8, §6.2.6. The discharge statement is rationale |
| Trim candidates if a later ADR needs them | Rationale. **Explicitly dropped**, see §17 |
| `$.input.text() -> string` | **Amended by ADR-0043 §11** to `[]u8`. §6.5.6 |

## 8. ADR-0012 — the wrap-the-facade idiom

| Source clause | Landed |
|---|---|
| §1 the intent is rationale and binds no implementation | §2.1 |
| §2 a forwarding function is a single call expression, no captures | §2.2 |
| §2 an implementation must not emit a call to one | §2.3 |
| §2 not the general promise "the compiler inlines well" | §2.2 (the predicate is structural); the contrast is rationale |
| §2 an explicit inline marker rejected | §2.7 |
| §3 stated structurally, not comparatively, not by a cost model | §2.4 |
| §3 it constrains how an implementation compiles, deliberately and narrowly | §2.6 |
| §4 wrappers are never blessed into the reserved root | §2.8 |
| §4 a universal wrapper means the facade is at the wrong altitude | §2.8 |
| §5 the project publishes no wrapper library; forbids no one else's | §2.9 |
| §5 the count measures the reserved root | §2.9, routed to ch8 |
| §6 the docs carry a small illustrative set | Teaching material, not conformance. **Explicitly dropped**, see §17 |
| §7 teaching order, with the `$.audio` exception | Teaching material. **Explicitly dropped**, see §17 |
| §8 normatively uniform across the facades | §2.5 |
| ADR-0034 §5: the guarantee is again the spec's **one** constraint | §2.6 |

## 9. ADR-0016 — the blessed math set

| Source clause | Landed |
|---|---|
| §1 concrete `f32` types, no element parameter | §3.1 |
| §1 the three grounds (emitter, dimensional algebra, root surface) | Rationale. **Explicitly dropped**, see §17 |
| §1 the named cost: the unit discipline stops at scalars | §3.11 |
| §2 vec2/3/4, mat2/3/4, integer vectors at three widths | §3.2 |
| §2 integer vectors ship | §3.2; **spelled `$.ivec2`/`$.ivec3`/`$.ivec4`, `i32` components, at §12.3 ([#119](https://github.com/ludo-lang/ludo/issues/119))** |
| §2 `mat2` ships despite redundancy | §3.3 |
| §2 non-square matrices and quaternions rejected | §3.4 |
| §3 operators on the blessed types and nowhere else | **Chapter 2 §16.1** (#99). Named at §3.6 |
| §3 the operator restrictions (no vector `/`, no elementwise `*`) | **Chapter 2 §16.1.** Named at §3.6 |
| §4 `Radians`, `Seconds`, `SampleFrames`; `Color` unchanged | §3.7; **stated to be root names, with the qualified-spelling rule, at §12.4 ([#119](https://github.com/ludo-lang/ludo/issues/119))** |
| §4 each shipped quantity appears in a `$.` return type | §3.7 |
| §4 `PxPerSec`, `PxPerTick`, `Ticks` dropped | §3.7 |
| §5 `cursor()` returns `SampleFrames` | §5.2.6 |
| §5 `$.audio.seconds(frames, rate)`; rate never a constant | §3.7 (the type), §5.2.7 (the call) |
| §6 no SIMD lowering obligation | **Chapter 8** (#99). Named at §3.9 |
| §6 clause 1 alignment specified on the blessed types | §3.8 |
| §6 clause 2 the batch shape is `[]T` views | Already true by ch3 §6.1. Named at §3.9 |
| §6 clause 3 float evaluation exactly as written | **Chapter 8**, with ch2 §2.5 (#99). Named at §3.9 |
| §7 no blessed batch functions | §3.10 |
| §7 positional constructors ship | §3.5 |
| §9's #24 delta — 9 types and 3 quantity types counted as root names | §12.4.1, and ch8 §9.3.1's table. Previously **bookkeeping dropped by §16**, which is how thirteen counted names reached no chapter's list ([#119](https://github.com/ludo-lang/ludo/issues/119)) |

## 10. ADR-0022 — the spec promises only what is derivable

| Source clause | Landed |
|---|---|
| §0 the rule | §4.11.1 |
| §0 widening a tolerance is refused | §4.11.6 (no number may be invented) |
| §0 a published measurement is refused | §4.11.9 |
| §1 `region` parameter rejected | §4.9.6 |
| §1 taking the view is the signal | §4.9.5 |
| §1 `get_pixels` signature unchanged | §4.4.2 (with ADR-0047's mark) |
| §1 backends upload from a strided host array | §4.9.7 |
| §1 the excluded-renderer sentence, as restated by ADR-0037 | §4.9.7, rung wording routed to ch8 |
| §2 the `crisp` bit-exactness headline is deleted | §4.11.2 |
| §2 the interior/exterior/boundary replacement | §4.11.3 |
| §2 the texel-selection sentence (added by ADR-0034 §3) | §4.11.4, §4.11.5 |
| §3 the coverage tolerance is deleted, no replacement number | §4.11.6 |
| §3 the fully-covered/uncovered/boundary rule | §4.11.6 |
| §3 the non-normative note to the conformance-suite author | Chapter 8's business, and non-normative there. **Explicitly dropped**, see §17 |
| §4 the byte-exact blit and copy-not-quad | **Deleted in full by ADR-0034 §1.** §4.11.7 |
| §4 the hit flash's byte-exactness withdrawn | §4.7.4 |
| §5 `copyExternalImageToTexture` forbidden | §4.1.7 |
| §5 reload defined | §5.7.4 |
| §6 the count is unchanged | Chapter 8's ledger. **Routed**, see §16 |

## 11. ADR-0026 — a program writes bytes to declared storage slots

| Source clause | Landed |
|---|---|
| §1 the Destination's "asset IO" was not a decision | Map bookkeeping. **Explicitly dropped**, see §17 |
| §2 the runner cannot own game data | §8.17 states the resulting split |
| §3 `extern storage <name>`; the kind-noun slot; no grammar cost | §8.1 |
| §3 names fixed at compile time; no runtime key, path or lookup | §8.2 |
| §3 multi-slot games declare multiple slots | §8.3 |
| §4 `write` and `read`, two functions, one root name | §8.4 |
| §4 UFCS resolves; the spec teaches the qualified spelling | §1.7 |
| §4 the payload is `[]u8`; no blessed serializer | §8.5 |
| §4 `?[]u8` carries first launch; no `exists` | §8.6 |
| §5 the read resolves before top level | §8.7 |
| §5 the write is submit-and-forget; flushed at the latest at the bound | §8.8 (with ADR-0041's restatement) |
| §5 both halves are async-tolerant; R7 untouched | Backend-facing; the rule a program sees is §8.7–§8.8. **Partly dropped**, see §17 |
| §6 MUST accept 1 MiB; MAY refuse more; MUST NOT truncate | §8.9 |
| §6 the anchor (512×512×4) | Rationale. **Explicitly dropped**, see §17 |
| §6 the conformance test lives in core, headless | §8.18, rung wording routed to ch8 |
| §7 oversize is an error value at the call | §8.10 |
| §7 IO failure is never reported to the program; the runner reports it | §8.11 |
| §7 no status channel, no did-my-write-land query | §8.11 |
| §8 slots resolve once per process, never re-resolved | §8.12 |
| §8 read-after-write returns what was written | §8.13 |
| §8 reload preserves contents; the runner flushes before the swap | §8.14 |
| §8 the quiescence predicate gains resolved storage contents | §8.15, predicate routed to ch5 |
| §8 a fault does not discard a submitted write | §8.16 |
| §9 no settings concept; ADR-0025 §6 survives; no `$.settings` | §8.17 |
| §10 the #24 delta | Chapter 8's ledger. **Routed**, see §16 |

## 12. ADR-0034 — the byte-exact blit is withdrawn

| Source clause | Landed |
|---|---|
| The clause and its implementation condition are withdrawn | §4.11.7 |
| §1 five grounds | Rationale. **Explicitly dropped**, see §17 |
| §2 why withdrawal beats an `opaque`/`none` `Blend` variant | The outcome is §4.4.8 (the enum has three members and no `none`); the argument is rationale |
| §3 the texel-selection sentence | §4.11.4 |
| §3 it promises which texel, never what byte | §4.11.5 |
| §4 the texel-centre convention is deleted, not written | §4.11.8 |
| §4 the fractional and rotated cases are covered honestly | §4.11.3, §4.11.4, §4.11.8 |
| §5 ADR-0012's "exactly one" is restored | §2.6 |
| §6 nothing is owed against #19 | Chapter 8's ledger. **Routed**, see §16 |
| §7 ADR-0009 line 372 is stale and routed to #72 | Handled at §4.7.4 and at §0 above |
| §7 the research document is not patched | Corpus hygiene. **Explicitly dropped**, see §17 |
| §7 ADR-0030 §4–§6 untouched | Chapter 5's. **Routed** |

## 13. ADR-0038 — the render scale

| Source clause | Landed |
|---|---|
| It is the spec's only performance escape hatch | §7.2 |
| §1 the arithmetic, the range, the quantisation, the offscreen resample | §7.3 |
| §1 at `s = 1` nothing changes | §7.3 |
| §1 the world drawn is identical at every scale | §7.4 |
| §2 down only; `s > 1` refused by the ordering | §7.5 |
| §3 not confined to the shader rung | §7.6 |
| §4 MUST NOT require an intermediate target at `s = 1`; MAY below | §7.7 |
| §4 at most one, never larger than the window | §7.7 |
| §4 ADR-0030 §3 is scoped, not overturned | The scoping is chapter 5's; the rule a backend follows is §7.7 |
| §5 the runner MUST provide an affordance | §7.8 |
| §5 the program may set it | §7.8 |
| §5 the scale is not the program's to cache | §7.9 |
| §5 runner-adaptive is refused | §7.10 |
| §6 the admission test, limbs one and two | ADR-0028's machinery; the outcome is the calls existing at §7.1. **Explicitly dropped**, see §17 |
| §7 the upscale filter is linear, on both style tokens | §7.14 |
| §7 it does not touch content sampling | §7.15 |
| §7 a `crisp` game at `s = 0.5` looks soft | §7.14's consequence; stated in ADR-0038 as a cost. **Explicitly dropped**, see §17 |
| §8 continuous in shape, quantised to thirteen values; the getter returns the grid value | §7.11 |
| §8 out-of-range input is clamped silently, never faulted | §7.12 |
| §8 no new quantity type | §7.13 |
| §9 survives relaunch; no file, format or location named | §7.16 |
| §9 provenance is not tracked | §7.17 |
| §9 it cannot go stale | A property of §7.11's fixed grid, not a separate obligation. **Explicitly dropped**, see §17 |
| §10 the four calls under `$.video` | §7.1 |
| §10 getters, not values | §7.9 |
| §10 neither belongs in `$.graphics` | §7.21 |
| §10 the rejected roots | §7.22 |
| §10 `$.video` is not a precedent | §7.20 |
| §11 no readback | §7.18, §4.9.11 |
| §11 no effect on input mapping | §7.19 |
| §12 P15 and its core-runnability; no property for the image | Chapter 8's. **Routed**, see §16 |
| §13 the #24 delta | Chapter 8's ledger. **Routed**, see §16 |

---

## 14. What chapter 5 owns and this chapter routes

Named here so the boundary is checkable rather than assumed. Each is a clause of
one of this chapter's sources that **lands in chapter 5**, not in this one:

- ADR-0005's caller-declared logical canvas, as overturned by ADR-0032, and the
  fit (ADR-0030, ADR-0031) — ch6 §4.2.7.
- The frame entry's `screen: !Target` and its per-frame freshness (ADR-0013) —
  ch6 §4.2.6.
- `to_logical` and `to_world`'s semantics (ADR-0013 §8, ADR-0030 §4) — ch6
  §4.2.8. **Their module is fixed here**: `$.graphics`.
- The loader that produces an `Image` (ADR-0015) — ch6 §4.9.2.
- The quiescence predicate ADR-0026 §8 amends — ch6 §8.15.
- Top level running to completion before any frame (#26) — ch6 §10.3.

## 15. What chapter 2, 3 and 4 own and this chapter routes

- ADR-0016 §3's operators → **ch2 §16.1** (ch6 §3.6).
- ADR-0016 §6 clause 3's float rule → **ch2 §2.5** and ch8 (ch6 §3.9).
- `#align(n)` as a mechanism → **ch3 §8.3** (ch6 §3.8).
- View lifetime, which ch6 §4.9.9 leans on → **ch3 §6**.
- Function-pointer types (ADR-0006 R2) → **ch2 §7** (ch6 §10.4).
- The target-selected module set (ADR-0006 R3) → **ch4 §10** (ch6 §10.5).
- `$.` as a module root and its two privileges → **ch4 §9** (ch6 §0.7).

## 16. What chapter 8 owns and this chapter routes

- Every **#24 companion-count delta** in this chapter's sources: ADR-0007's ten
  functions and six types, ADR-0009's and ADR-0010's `$.graphics` counts,
  ADR-0011's 22 functions and five types, ADR-0026's `+1` root, ADR-0038's `+1`
  module and `+4` functions, ADR-0008's zero-grammar result, ADR-0016's nine
  types and three quantities, and ADR-0022's and ADR-0034's zero deltas.
  **Routing a delta to chapter 8 is not the same as listing the names it
  counts**, and ADR-0016's row is where the difference bit: thirteen counted
  types had no list in either chapter until §12.4.1
  ([#119](https://github.com/ludo-lang/ludo/issues/119)).
- The **conformance rungs** (core ⊂ full ⊂ shader) every clause above names:
  ch6 §4.9.7, §5.8, §8.18, §9.2, §9.3, §9.28, §10.12.
- The **property list**: ADR-0006 R8's two full-conformance observables (P1, P2),
  ADR-0008 §10's shader-reload property, ADR-0038 §12's P15, ADR-0034 §6's
  finding that nothing is owed, and ADR-0022's addition of none.
- ADR-0016 §6's SIMD non-obligation and float-evaluation obligation.

## 17. Phantom clauses, and what was explicitly dropped

**Two phantom clauses were met, and both were already resolved before this
chapter began.** No new phantom was found, and — as this chapter originally
shipped — no clause was authored here.

**That last claim did not survive chapter 8** ([#119](https://github.com/ludo-lang/ludo/issues/119)).
Chapter 8 §9.3 is obliged to *count* the root, and a count found what reading
this chapter's sources had not: three rules with no surface — `$.assert` and
`$.panic` (ch2 §8.5), the integer vector spellings (ADR-0016 §2), and the
thirteen types ADR-0016 §9 counts (§16's dropped bookkeeping). §12 authors all
three, so **chapter 6's authored count is one section, not nothing**, and §0.7's
completeness claim is discharged at §0.7.1. The original *nothing authored, no
marked gap* result was cited as evidence that the standard library was the
best-specified region of the corpus; what it actually measured is that
transcription cannot find a missing surface, and a count can. Recorded here
rather than overwritten, because that is the transferable finding.

1. **The texel-centre and tie-break convention.** Cited by ADR-0005, ADR-0006
   and ADR-0010; never written. ADR-0034 §4 deleted the three citations rather
   than authoring the rule. Chapter 6 transcribes the replacement — §4.11.3,
   §4.11.4 and §4.11.8 — and carries no citation of the convention.
2. **The byte-exact blit.** Not a phantom in the strict sense (it was written,
   four times) but the same failure mode: a clause preserved by citation past
   the point its premise held. Withdrawn by ADR-0034 §1; §4.11.7 states the
   withdrawal as the rule, because a reader needs to know the promise is not
   there.

**Explicitly dropped, by class:**

- **Rationale about a rejected alternative** — the bulk of every "dropped" row
  above. The rejection is transcribed **only** where it produces a rule a
  program or an implementation can violate; those cases are ch6 §4.10.2,
  §6.5.8, §7.22, §2.7 and §2.8.
- **Delegated-tier facts** — what a fighting game or a racing game does when it
  drops below the facade (ADR-0011), what the renderer or audio engine tier may
  offer (ADR-0005, ADR-0006). Not obligations on a conforming implementation.
- **Teaching material** — ADR-0012 §6 and §7 in full. The spec is normative;
  the tutorial is not this document.
- **Corpus and map bookkeeping** — ADR-0026 §1, ADR-0034 §7's research-document
  clause, ADR-0022 §3's note to the suite author, and every `#24 delta` and
  amendment list.
- **Arguments for a number that is itself transcribed** — ADR-0026 §6's 1 MiB
  anchor, ADR-0038 §8's two quantisation grounds, ADR-0007's pan-law derivation.
  The number is normative; the derivation is not.

## 18. Induced holes opened by this chapter

Per `docs/agents/domain.md`, one line per hole opened by **writing a repair**
rather than by reading.

- **None.** Chapter 6 authored no clause (§17), so it opened no induced hole.
  The one defect it found — §0.4's checker blind spot — was found by reading,
  is a tooling gap rather than a corpus hole, and is filed as its own ticket.
- **§12's repair, landed by [#119](https://github.com/ludo-lang/ludo/issues/119),
  opens none.** Every clause is cited from ch2 §8.5, ch5 §6.5.1 or ADR-0016; the
  one unsourced decision is the `ivec` prefix against §1.1.4, recorded in place
  at §12.3.3 with the precedent bounded to the blessed math set. No ADR is
  written: nothing here reverses a decision (ADR-0044 §6).
