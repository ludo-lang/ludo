---
status: accepted
---

> **Amended by [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md):**
> §10's delta adds `debug_image` without subtracting `checkerboard` and omits the `Clip`
> type §5 introduces; both are folded into the reconciled count.

# Assets are declared, not loaded

Issue #42 named `Image` and `Font` in the mandated drawing facade and explicitly
declined to spell their loaders. Two facade types therefore existed that no
program could obtain. This ADR spells the mechanism that produces them — and
finds that most of it was already decided, in three places, by decisions taken
for other reasons.

The ticket asked three questions: what does a load call take, what happens when
the file is missing, and is loading synchronous. The third dissolves, the second
turns out to be a fault rather than a value, and the first is answered by a
declaration that no call site ever spells.

## 1. An asset reference is a declaration whose name is fixed at compile time and whose bytes the runner resolves

`extern image player = "art/player.png"` — the shape of ADR-0008's `extern`
shader declaration and issue #29's `extern "SDL3" fn`, which name their
counterparty in the file with no build configuration anywhere.

Three prior decisions converge here and none of them was about assets:

- **Issue #5 criterion 3** — one spelling per entity, closed compile-time
  namespace. A runtime path string is an entity the oracle cannot see.
- **Issue #4** puts *wrong paths and names* at **43.7%** of agent compile
  errors, the single largest measured failure mode. A declared name is in the
  name table; a string literal inside a call is not.
- **ADR-0014 §12** forbids source naming a resolved *location* for libraries.

**Rejected: a load call, `$.assets.load_image("player.png")`.** It is the shape
this map has refused three times already — issue #29's C-symbol rename,
ADR-0008's shader build step, ADR-0014's library location in source — each time
because it puts an entity outside the compiler's knowledge. There is no reason
for assets to be the exception.

**Rejected: the compiler reads the file and embeds the bytes.** Tempting,
because it makes a missing file a *compile error* and deletes §3 and §4 outright.
It fails on two counts: every art edit becomes a compiler invocation, which is
the content build step issue #12 ruled out wearing different clothes; and it puts
megabytes into the artifact issue #17's runner swaps at a frame boundary. Note the
contrast with ADR-0008, where the compiler genuinely *must* read the `.wgsl` to
run its three-comparison interface check. It has no such reason to read a PNG.

**The path is relative to the source file that declares it.** R1-locality holds,
a directory moves without editing its contents, and — falling straight out of
ADR-0014 §9 — **an asset path may not cross a library boundary**, so a library
carries its own content and provenance stays legible at the reference site.
Nothing here is a cache or a build artifact, so issue #19's P3 is untouched.

**A declaration may appear in any module, not only the entry file.** Issue #17
confines `persist` to the entry file because it is *mutable cross-frame state*
and R1 forbids reading it invisibly. A declared asset is immutable content, so
the argument does not transfer — and a library that ships its own default font is
unwritable under the stricter rule.

## 2. One mechanism for every content type

`Image`, `Font`, `Clip` and raw bytes share one declaration shape, differing only
in the declared type. The extern shader declaration stays the odd one out, with
its reason stated: the compiler reads it.

Stating this once is deliberate. Issue #28 answered a graphics-shaped question
and audio inherited the blind spot, which cost issue #32 an entire ticket to
repair. *What about music* must not be a later decision.

## 3. Nothing is synchronous or asynchronous, because loading is not a program-visible operation

**Every declared asset is resolved and decoded before top-level code runs.**

The program therefore never observes a loading state: no `?Image`, no
handle-in-flight, no readiness poll, no progress value. ADR-0006 R1 — nothing in
the platform-layer API may block — is satisfied **by construction rather than by
care**, which is R1's own stated preference. R1 already anticipated this answer:
it resolves the expensive asset-loading case by pointing at issue #26's
run-to-completion top level, and says a browser can satisfy it with the tab's own
asynchronous machinery before it ever calls into ludo.

**Stated cost, and it is real: you cannot write a progress bar for the declared
set.** A large game shows a window and nothing else until it is ready.

That cost is paid, not dodged, and §5 is what makes it payable: a loading screen
is written by streaming bytes yourself through ADR-0006 R7's request-and-poll
file IO and decoding them. This is the identical move ADR-0007 made for the raw
sample push — a narrow escape hatch justified by *a library that owns its own
thing*, never offered as the ergonomic answer.

## 4. A missing declared file is a bug, not a value

There is no call site, so there is nothing to `rescue`; and a fallible
declaration would tax every use site with a condition that is constant for the
process's life.

**A declaration that cannot be resolved or decoded at startup is an issue #18
fault at top level.** Issue #18 already specifies that case exactly: the
simulation never starts and the prior `persist` is untouched.

The dev loop then handles it without a single new mechanism: missing file →
fault → **pause, not terminate** → put the file back → reload → resume. The
answer is loud without being fatal, which is the experience contract paying for
itself.

**Rejected: substitute the debug pattern and carry on.** The checkerboard works
because a *wrong* image is visible. A missing file is not a wrong image; it is a
program that cannot run as written — the same class as ADR-0008 §9's unnameable
WGSL type, which is a hard error naming its counterparty.

**No new diagnostic machinery.** Issue #18's fault report already mandates a
closed fault-kind enum, source location, concrete values and a call chain in both
modes. This ADR adds one clause — **asset resolution failure is a spec-named
fault kind**, carrying the declaration's location and the path it tried — which
lands it in issue #19 P5's stable code namespace automatically, so an agent reads
`path` as a field rather than parsing English. The report's *content* is spec;
its *sink* is the runner's, on the same split as every other diagnostic. A
spec-mandated log file would be P3's project-layout rule broken, wearing a
diagnostics hat. **No logging concept enters the language.**

## 5. Content the program produces is decoded from bytes, and there is no decode-from-path

```
$.graphics.decode_image(allocator, bytes: []u8) -> Image rescue DecodeError
$.graphics.decode_font(allocator, bytes: []u8) -> Font rescue DecodeError
$.audio.decode_clip(allocator, bytes: []u8) -> Clip rescue DecodeError
```

This covers what declarations cannot: a level pack chosen at runtime, a mod
folder, a player-supplied image, an atlas whose member list is data.

**It is not a second spelling of the declaration**, and the precedent is
ADR-0007's caller-authored clip: content the *program* produces, handed to the
ordinary playing path, sitting beside declared sounds without anyone calling it a
duplicate. `decode_image` is that move for pixels. Note what it does not contain:
**a path**. The bytes come from R7 file IO, and bundling a read with a parse is
what makes a decode-from-path call a path-resolution call in disguise.

**An allocator is a parameter** per issue #8 — decoding produces an object of
unknown size, and allocation is by absence.

**The declared set is runner-owned memory the program never names.** It must
outlive reload the way issue #17 §4's program-constant blob does, and making a
beginner name an allocator for `player.png` restores the ceremony issue #15
protected `{1,2,3}` from.

## 6. The checkerboard is demoted from a mandated behaviour to a mandated value

ADR-0007 kept DragonRuby's checkerboard, mandated as a named debug pattern for
the case DragonRuby handles: **rendering a `nil` pixel array**.

**Two decisions have since removed every path to that state.** Issue #9 deleted
null, so an absent `Image` cannot reach a draw call. §4 above makes a missing
declared file a fault, so the facade is never holding a not-there image at draw
time. The checkerboard was an implicit fixup for a state that is now
unrepresentable — and an implicit fixup is what criterion 5 forbids in any case,
so it should not go looking for a new job.

What survives is real, and §5 is what creates it: `decode_image` is fallible, and
the common handling at a call site is *carry on with something visible*.

> **`$.graphics.debug_image`** — a spec-named pattern with normative dimensions,
> colours and cell size, so it is conformance-testable and recognisable across
> implementations, reached **explicitly**:
> `image = decode_image(a, bytes) rescue $.graphics.debug_image`.
> Nothing in the facade ever substitutes it for you.

Every word of ADR-0007's argument survives — *it works because a wrong image is
visible* — moved from a fixup to a spelling. **ADR-0007's mandated-behaviour
sentence comes out**; that ADR is stamped as amended.

## 7. There is no debug clip, and ADR-0007's reasons for that are corrected

ADR-0007 gave two reasons against a debug tone: shipping one into a player's ears
is hostile, and silence is indistinguishable from success. **§6 breaks both.**
The first argues against an *implicit* substitution — nothing ships into anyone's
ears if the developer wrote `rescue $.audio.debug_clip` themselves. The second
argues *for* an audible marker, not against one.

The conclusion stands on an argument neither ADR made, and it is about the
substitute's **parameters**:

A debug substitute must invent the values it does not know, because the decode
that failed is exactly what would have supplied them. The checkerboard invents
**dimensions**; a debug clip invents **duration**. These degenerate differently.
A wrong-size checkerboard is visibly wrong and behaviourally harmless — nothing
downstream depends on its size. A wrong-duration clip is **behaviourally
load-bearing**: game code times against clip length, so a 0.2 s beep for a
four-minute track fires the music-ended logic immediately, and anything long for
a footstep drones under the level. There is no honest duration to pick, and every
choice changes what the program *does* rather than what it sounds like.

This is ADR-0007's own *`stop` is the interrupt, not the note-off* shape: audio's
degenerate cases move the simulation, drawing's do not.

**No `$.audio.debug_clip`.** ADR-0007's empty clip — plays as a zero-length
one-shot and finishes immediately — remains the only degenerate clip, honest
precisely because zero is not a guess. The two superseded reasons are recorded
here so this is not re-proposed the moment someone notices drawing has a debug
value and audio does not.

## 8. Editing an asset file reloads it live, at the next frame boundary

**This reverses the position this ADR's own drafting first reached**, which
deferred asset reload as *issue #17's field-path merge problem in new clothes*.
It is not that problem, and the difference is worth stating because it is what
makes this the cheapest reload on the map:

- **The program never holds the bytes.** `Image` is an opaque handle to
  runner-owned memory (§5), so no user structure contains an image's layout and
  there is nothing to merge.
- **It is immutable** — no half-updated state, nothing to make quiescent.
- **The name is fixed at compile time** (§1). Issue #17 forbids a *function*
  being reachable from `persist` because re-resolution by name can silently call
  the wrong code; here the name resolves to **content**, and content that changed
  is exactly what was asked for.
- **Changed dimensions are not a hazard**, because no dimension was ever copied
  anywhere: code reads `image.width` at the moment it draws. A sprite that got
  bigger draws bigger, which is the point of the edit.

So the runner swaps the bytes behind the handle at the same quiescent frame
boundary issue #17 already owns. Audio is click-free for free, because ADR-0006
R8 already makes the ring runner-owned.

**This is mandated, not optional in spirit.** Issue #16 made the felt loop the
artifact, and the tightest loop a gamedev has is *nudge the art, look at it*.
Shipping state-preserving code reload while making an artist rebuild to see a
sprite would be the experience contract missing its most-used case.

**A failed re-decode retains the last-good asset, reports an error value, and
does not fault** — ADR-0008's failed-shader-recompile clause, for the identical
reason: you are mid-edit, and a half-written file is the normal state of a file
being saved. The asymmetry with §4 is deliberate: **missing at startup is a
program that cannot run; broken at reload is a program already running fine.**

This attaches to issue #19 as a **new property at full conformance**, P1's
sibling: *editing a declared asset's file updates it at the next frame boundary
without restarting the process or resetting `persist`* — precondition, action,
observable.

## 9. `Font` needs nothing `Image` does not

ADR-0005 identifies the default font by **name + version + hash**, and
`measure_text` takes the font as a parameter, so size is a draw-time value and
not a load-time one. `extern font` is `extern image` with a different type, and
rasterization stays where issue #28 put it.

## 10. Issue #24 delta

**Grammar: zero new keywords.** `extern` is spent by issue #29 and reused by
ADR-0008. The declaration adds no production the budget has not already paid for
— the third decision to be rewarded by the budget rule rather than taxed by it.

**Stdlib root-name companion count:**

- `$.graphics` — +2 functions (`decode_image`, `decode_font`), +1 value
  (`debug_image`), +1 type (`DecodeError`)
- `$.audio` — +1 function (`decode_clip`)

ADR-0007's mandated-checkerboard-behaviour clause is **removed**, making this the
second decision on the map to delete a clause while raising the count.

## Lenses

**Simplicity — strong, and the strongest since ADR-0013.** The beginner's first
sprite is one declaration and zero concepts: no loader, no handle-in-flight, no
readiness check, no allocator, no error handling, no path string. The costs are
concentrated where they are chosen — the progress bar, and the decode path's
`[]u8`, which is the pixel array's toll buying the same thing.

**Robustness — strong.** Three bug classes are deleted rather than diagnosed: an
asset name that does not exist is a compile error, a use-before-loaded state is
unrepresentable, and an implicit visual fixup no longer exists to be mistaken for
success. The deployment failure is loud, attributable and resumable.

**Agent-friendliness — strong.** Every asset name is in the oracle's name table,
which is the direct mitigation for issue #4's largest measured failure mode; the
fault kind is spec-named with the path as a field; and the one place a runtime
string still appears — `decode_image` over bytes — takes no path at all, so there
is no path-shaped call for a model to reach for.
