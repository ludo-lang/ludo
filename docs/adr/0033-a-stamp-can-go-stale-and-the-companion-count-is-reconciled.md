---
status: accepted
---

> **Amended by [ADR-0034](0034-the-byte-exact-blit-is-withdrawn-and-the-texel-rule-replaces-it.md):**
> §6's pending stamp on ADR-0012 is **discharged** — the copy-not-quad condition is
> withdrawn, so the forwarding-function guarantee is the spec's **one** compilation
> constraint again — and §10's *deliberately unfixed* listing of the byte-exact blit is
> closed. §2's rule that a stamp is itself amendable content is the mechanism ADR-0034 §5
> follows, appending rather than reverting. ADR-0034 §4 finds a **third phantom clause** —
> the texel-centre and tie-break convention, cited by three ADRs and never written — which
> is this ADR's failure mode in its purest form.
>
> **Amended by [ADR-0037](0037-a-backend-clears-a-floor-and-shaders-are-the-third-rung.md):** §10's second
> *deliberately unfixed* item — ADR-0022 §1's WebGL2 mention — is **discharged**. §10 read
> the sentence correctly and could not fix it, because the fix needed a decision about the
> admissible set and there was no set; ADR-0037 §1 supplies that absence as the decision.

# A stamp can go stale, and the companion count is reconciled

A full read of `CONTEXT.md` against all thirty-two ADRs found seven live
contradictions between accepted documents, four count deltas that do not
reconcile, and seven softer incoherences. None of them is a decision anyone took
— every one is a claim that was true when written and was falsified later by a
decision that did not reach back far enough.

This ADR is therefore mostly bookkeeping, and it says so up front. It takes
exactly two substantive calls (§4 and §5), states one new rule about the record
itself (§2), and otherwise quotes falsified sentences and replaces them so that
the corpus stops asserting things it does not believe.

**It rewrites no earlier ADR's prose.** `docs/agents/domain.md` fixes that an ADR
is an immutable record and that a later decision reaches an earlier one through a
stamp. That rule is what this ADR uses, and §2 is what it learned about it.

## 1. Why the corrections travel together

Each finding is individually small and individually arguable as *not worth an
ADR*. They are here as one decision because they share a cause, and the cause is
worth naming: **a claim is checked against the issue that produced it rather than
against the ADR that superseded it.**

The corpus cites issues and ADRs interchangeably for the same decisions — #28 for
ADR-0004, #40 for ADR-0005, #42 for ADR-0009, #41 for ADR-0007, #33 for ADR-0008.
An author verifying "the facade has three verbs" against #42 finds it true. The
same check against ADR-0010 finds it false. Every Tier-1 finding below has that
shape, and §10 records why this ADR does not fix the aliasing itself.

## 2. A stamp can go stale, and stamps accumulate rather than merge

ADR-0025 §1 leans on *"ADR-0005's letterbox + integer scale"*. ADR-0030 stamped
it:

> the citation is now true, with integer scale holding on `crisp` targets only

ADR-0031 then deleted integer scaling outright. **So ADR-0025's header now
asserts a rule that does not exist** — and it asserts it in the one place a
reader trusts most, because a stamp reads as verified.

This is the worst failure shape the record can produce, and the fix is a rule
about stamps rather than about ADR-0025:

> **A stamp is amendable content.** A stamp that carries a claim about what a
> later decision holds is itself subject to correction, and is corrected the same
> way anything else is — by a further stamp, never by editing the existing one.
> Stamps accumulate in date order. A reader takes the *last* stamp on a subject
> as current.

`docs/agents/domain.md` already says stamps accumulate rather than merge; what it
did not say is that a stamp can be the thing that goes stale. It can, because a
stamp with a one-clause gist necessarily restates a piece of the amending
decision, and that piece is as falsifiable as any other.

The narrow consequence: **ADR-0025 §1's citation is now true only as to
letterbox.** There is no integer scale, on any target, at any scale. The clause
ADR-0025 §1 relies on is ADR-0030 §4 as restated by ADR-0031 §2 — a uniform
scale plus a centring translation, `k = min(w/W, h/H)`, with `W` and `H` fixed at
1280×720 by ADR-0032. ADR-0025 §1's actual conclusion — resize changes nothing
the program can observe — survives all three, and survives them more cleanly than
it did under the two-branch rule.

## 3. ADR-0004's bit-exactness sentence is deleted, on ADR-0022's own reasoning

ADR-0022 §2 deleted the headline `crisp` claim and §3 deleted the `smooth`
coverage tolerance. Its amendment list stamps ADR-0005 for both. It does not
reach ADR-0004, where the same two claims live in their original form:

> **The conformance predicate is bit-exact with one carve-out.** Interior and
> exterior pixels must match exactly; boundary pixels may differ within a stated
> tolerance **only** for targets that opted into AA. A crisp-target program is
> bit-exact across the whole backend matrix.

**That bullet is deleted**, and nothing numeric replaces it. ADR-0022 §0's rule
governs: the spec promises only what is derivable, and #56 falsified this in
three citable places — WebGPU declines a top-left fill rule, WGSL specifies no
rounding mode, float-to-unorm store is implementation-defined. What stands in its
place is exactly what ADR-0022 §2 and §3 wrote for ADR-0005, unchanged and not
restated here.

This correction adds no argument. It is ADR-0022 finishing a job it did three
quarters of, and it is recorded because a false normative sentence in an ADR's
*Consequences* is not softened by a true one three files away.

**ADR-0004's `crisp`-is-the-default consequence bullet survives**, since ADR-0022
left `crisp`'s operational meaning intact — no anti-aliasing, nearest sampling —
and ADR-0031 §3 later made it unconditional at every scale.

## 4. The debug value is `debug_image`, and the mandated sets are corrected

ADR-0015 §6 replaced the checkerboard-as-behaviour with `$.graphics.debug_image`
and stated that ADR-0007 *"is stamped as amended"*. It was not, and neither were
the two ADRs that publish the mandated set: ADR-0009 lists
`$.graphics.checkerboard` as a value, and ADR-0010 re-affirms it in "the same six
further names … unchanged". Two names for one mandated value have been live since
ADR-0015 landed.

**`debug_image` wins**, and this is a decision rather than a bookkeeping call, so
it is argued:

- ADR-0015 §6's reasoning reaches the **behaviour**, not the name. It deleted an
  implicit fixup and produced an explicitly-reached value; the rename came along
  unexamined, which is why it did not propagate.
- The name should say what the value is *for*. `checkerboard` describes what it
  looks like, and ADR-0015 gave it a normative appearance — dimensions, colours,
  cell size — so the appearance is already spec'd elsewhere and does not need to
  be the name.
- `debug_image` sits beside `decode_image` and `Image`, which is the vocabulary
  the surrounding calls already use.

The counter-argument, recorded: `checkerboard` is the more guessable name for a
model, and ADR-0009 spent real effort on corpus-guessable naming. It loses
because the guess is only better if the reader already knows the DragonRuby
behaviour it descends from, and ADR-0015 deleted that behaviour.

So: **ADR-0009's and ADR-0010's `$.graphics.checkerboard` is
`$.graphics.debug_image`**, and ADR-0007's mandated-behaviour sentence is out,
per ADR-0015 §6.

## 5. `Clip` is a root type, and ADR-0007's five becomes six

ADR-0007 states:

> A clip is the caller's own `[]f32` plus a rate and a channel count, so it adds
> no root name.
>
> Five types: `Voice`, `VoiceDesc`, `VoicePatch`, `Waveform`, `FilterType`.

ADR-0015 §2 then lists `Clip` among the declarable content types and §5 ships
`$.audio.decode_clip(allocator, bytes) -> Clip`, while its §10 delta records the
function and not the type.

**`Clip` is real**, and ADR-0007's sentence is superseded. The forcing argument
is ADR-0007's own: it makes **resampling to the device rate the facade's job**,
which requires the facade to know the clip's source rate, and it puts a loop
region in sample frames, which requires knowing the channel count. A bare `[]f32`
carries neither. The triple was always a type; ADR-0007 declined to name it and
ADR-0015 named it without noticing that it had.

`$.audio`'s type count is therefore **six**, and the count is reconciled in §9.

Nothing else in ADR-0007 moves. A clip is still caller-owned memory, `play` still
takes no path, and §5's decode path is still the only way to obtain one from
bytes.

## 6. The forwarding-function uniqueness claim is contingent, not withdrawn

ADR-0012 §3 records the forwarding-function guarantee as *"a deliberate, bounded
exception to 'the spec does not prescribe implementation'"*, and `CONTEXT.md`
sharpened that to "the spec's one bounded constraint". ADR-0030 §7.2 declares it
false in passing:

> ADR-0012 records the forwarding-function guarantee as "the spec's one
> deliberate constraint on how an implementation compiles", and this is a second
> one, so that claim is already false.

The second constraint is ADR-0022 §4's *backends must implement this case as a
texture-to-texture copy, not a rasterised quad*. **That condition is under review
in [#69](https://github.com/adamico/ludo/issues/69)** — ADR-0030 §7 routed it
there and gave three independent grounds for withdrawing it.

So the claim is **contingent, not false-and-standing**. It is not withdrawn here,
because withdrawing a claim that #69 may restore means writing it back. What
changes is that ADR-0012 gets a stamp saying so, and `CONTEXT.md`'s *Forwarding
function* entry says "one of two, pending #69" rather than "one".

Rejected: withdrawing it now on ADR-0030's word. ADR-0030 itself declined to
amend ADR-0010 or ADR-0022 on this point, on the grounds that burying a
facade-wide withdrawal inside a letterbox ADR is how the original defect
happened. The same reasoning forbids burying it in an audit ADR.

## 7. Resolved conflicts and stale worked examples

Four corrections with no argument attached, listed rather than sectioned.

- **ADR-0011's closing line** — *"The ADR-0009 / #26 frame-entry conflict is
  raised, not resolved"* — was resolved by ADR-0013, which is entirely about it.
  ADR-0011 carried no stamp for it. It does now.
- **ADR-0009's `Target` naming rationale** — *"not `Canvas`, which collides with
  the logical canvas that is a field inside it"* — describes a field ADR-0032
  removed. The naming conclusion survives on its second ground (`Screen` is
  falsified by the reserved offscreen shape); the collision argument does not.
- **ADR-0029 §4's worked example** — *"the mandated spelling is
  `$.graphics.set_canvas({...})`, which is target-typed"* — is stale: ADR-0032 §5
  removed `CanvasDesc` and the call takes the `style` token directly. The
  section's *rule* is unaffected, because entry 2 of its closed list is about
  aggregate literals generally and the facade supplies many others. The example
  should read `$.graphics.fill_rect(screen, {...})`.
- **ADR-0006's "drawing needs eight verbs"** was already wrong against ADR-0004's
  nine when written, and is ten after ADR-0010. The sentence is rhetorical — it
  is arguing that call count was never the problem with the audio surface — and
  its argument is untouched. The number is wrong and is noted so nobody counts
  from it.

## 8. `to_world` was mandated and never counted

ADR-0011 closes with *"`to_world` is owed by `$.graphics` … and is not counted
here"*. ADR-0013 §8 ships it — *"with the target in scope this is
`screen.to_world(p)` under UFCS"* — and its delta reads *"+2 functions and +1
type: `set_canvas` and `to_logical`, plus the `CanvasDesc` descriptor."*

`to_world` is mandated surface with no delta in any ADR. It is counted in §9.

This is the mechanism the count exists to catch, caught late: a name owed by one
ADR and shipped by another falls between two deltas, because the shipping ADR
counts what it introduced and the owing ADR counted nothing.

## 9. The reconciled companion count

Every prior delta is applied in order, with §4, §5 and §8's corrections folded
in. This table is the base a future delta is computed against; a decision that
disagrees with it amends this ADR.

### `$.graphics` — 19 functions, 3 values, 21 types

```
fill_rect          stroke_rect         fill_ellipse       stroke_ellipse
fill_triangle      stroke_triangle     stroke_line        fill_sprite
stroke_sprite      fill_text                                              (10, ADR-0010)
measure_text       slice_image         create_pixel_image  get_pixels     (14, ADR-0009)
set_canvas         to_logical          to_world                           (17, ADR-0013, §8)
decode_image       decode_font                                            (19, ADR-0015)
```

Values: `default_font` (ADR-0009), `debug_image` (ADR-0015, §4),
`canvas_size` (ADR-0032).

Types: 16 (ADR-0009) + 4 (ADR-0010: `Paint`, `Texture`, `Mapping`,
`SpriteStrokeDesc`) + 1 (ADR-0013: `CanvasDesc`) + 1 (ADR-0015: `DecodeError`)
− 1 (ADR-0032: `CanvasDesc` removed) = **21**.

**ADR-0032 §1's "all 26 `$.graphics` calls" is wrong**; the figure at the time of
writing was 19 functions, or 22 names counting values. Its argument is unaffected
— the finding was that *none* of them reports the canvas size, and none did.

### `$.audio` — 12 functions, 6 types

```
play  stop  stop_all  get  set  push  free_space  cursor  sample_rate  underruns   (10, ADR-0007)
decode_clip                                                                        (11, ADR-0015)
seconds                                                                            (12, ADR-0016)
```

Types: `Voice`, `VoiceDesc`, `VoicePatch`, `Waveform`, `FilterType` (ADR-0007),
`Clip` (ADR-0015, §5).

`cursor()` returns `SampleFrames`, not `int` (ADR-0016 §5) — an amended
signature, not a name.

### `$.input` — 22 functions, 5 types

Unchanged from ADR-0011. Nothing has amended it.

### `$.storage` — 2 functions, 0 types

Unchanged from ADR-0026. `extern storage` is a kind noun, not a root name.

### Pending, and deliberately not counted

The fullscreen getter and setter (ADR-0025) are two functions whose **module is
unspelled** — ADR-0025's budget section says the count "gains one module name if
these calls live under a new `$.window` root; spelling is left to the
facade-spelling convention of ADR-0009 and is not fixed here." They stay out of
the table until spelled, and this line is here so their absence is deliberate
rather than another §8.

The root itself — `$.rgb8`, `$.rgbf`, the math constructors and types (ADR-0016),
`$.print`, `$.assert`, `$.time.now` — is **not enumerated anywhere in the
corpus**. This table covers the four spelled facade modules only, and says so
rather than implying completeness.

## 10. What this ADR deliberately does not fix

- **The issue/ADR aliasing of §1.** Rewriting every `#40` into an ADR reference
  across thirty-two documents is a mechanical change to immutable records, and
  the citations are *true* — the issue really did settle the thing. What is
  needed is a cross-reference table, which is documentation work rather than a
  decision, and is its own ticket.
- **ADR-0022 §1's WebGL2 mention.** It names WebGL2 unpack parameters and
  excludes WebGL1 "as a backend", which reads as admitting WebGL2 where ADR-0002
  adopts the WebGPU model and names D3D12, Metal, Vulkan and WebGPU. The sentence
  is a floor on how a whole-image upload is done, not a backend admission, and
  ADR-0002's ceiling is unchanged. Recorded because it will be misread again, and
  because the right fix — an explicit statement of the admissible backend set —
  belongs with whoever writes the conformance matrix.
- **The byte-exact blit.** #69's, per ADR-0030 §7 and §6 above.
- **The `CONTEXT.md` entries.** They are patched in the same commit as this ADR;
  the glossary is a living document and is edited in place, unlike an ADR.

## 11. Against issue #24: one name renamed, one type recognised

- **Core grammar: unchanged. Type sublanguage: unchanged.**
- **Stdlib root names: net zero.** §4 renames one value; §5 recognises a type
  that already existed in a signature; §8 counts a function that already shipped.
  Nothing new is admitted.
- The reconciled base (§9) is the deliverable: **19 + 12 + 22 + 2 = 55 mandated
  functions across four modules**, with 3 values and 32 types. The count has
  never been stated as a total before, and stating it is the point — a budget
  nobody can compute the current value of is not a budget.

## The three lenses

- **Simplicity.** No effect on any program. One name changes in the reference.
- **Robustness.** The gain is on the record rather than on the language: four
  sentences that a conforming implementation could have read as normative and
  been wrong about are now marked, and the count that governs future additions
  can be reproduced.
- **Agent-friendliness.** The strongest, and it is why this is an ADR rather than
  a chore. Every finding here is a case where an agent reading one document in
  isolation gets a wrong answer with full confidence — three verbs, a bit-exact
  `crisp`, a `checkerboard` that no longer exists, a clip with no type. #4's
  measured failure mode is naming things that do not exist, and a stale document
  is a machine for producing exactly that.

## Amendments this ADR makes

- **ADR-0004** — the bit-exactness consequence bullet is **deleted** (§3), on
  ADR-0022 §2 and §3's reasoning, which reached ADR-0005 and not this ADR.
- **ADR-0006** — "drawing needs eight verbs" is a wrong count; the argument
  around it stands (§7).
- **ADR-0007** — the mandated-checkerboard-behaviour sentence is **out**, per
  ADR-0015 §6 (§4); "a clip adds no root name" is **superseded** and the type
  count is six (§5); the cursor's `int` return is `SampleFrames`, per ADR-0016 §5.
- **ADR-0009** — `$.graphics.checkerboard` is `$.graphics.debug_image` (§4); the
  `Target`-not-`Canvas` collision argument cites a field ADR-0032 removed (§7).
- **ADR-0010** — the sixth further name is `debug_image` (§4).
- **ADR-0011** — its closing "raised, not resolved" was resolved by ADR-0013 (§7).
- **ADR-0012** — the forwarding-function guarantee is one of **two** constraints
  on how an implementation compiles, pending #69 (§6).
- **ADR-0013** — its delta omits `to_world`, which it ships in §8 (§8).
- **ADR-0015** — its delta adds `debug_image` without subtracting `checkerboard`,
  and omits the `Clip` type (§4, §5, §9).
- **ADR-0025** — §1's citation survives as **letterbox only**; there is no
  integer scale on any target (§2).
- **ADR-0029** — §4's `set_canvas({...})` example is stale under ADR-0032 §5; the
  rule is unaffected (§7).
- **ADR-0032** — §1's "26 `$.graphics` calls" is 19 functions; the argument is
  unaffected (§9).
