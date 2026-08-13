---
status: accepted
---

# The window opens maximal, and belongs to the player

[ADR-0025](0025-fullscreen-is-a-player-preference-the-program-can-read-and-set.md)
§7 guarantees that every ludo game's first launch is windowed, and names a size
it never defines:

> Every ludo game opens in a window the first time, at the runner's default size.

There is no such size anywhere in the spec. Issue
[#71](https://github.com/adamico/ludo/issues/71) asks whether that silence was
deliberate. It was not — it was a phrase written to finish a sentence about
fullscreen, and this ADR supplies what it promised.

## 1. Half the question dissolved before it was asked

#71 originally asked two coupled questions: what canvas a program gets when it
declares none, and what window the runner opens.
[ADR-0032](0032-the-canvas-is-fixed-at-1280x720.md) fixed the canvas at 1280×720
for every program, so there is no declaration and therefore no default to choose.
What survives is the window, and it is a narrower question than it was: a single
arithmetic choice against one known constant rather than a policy that has to
work for any canvas a program might declare.

Two facts settled before grilling and are recorded so they are not re-litigated.
The runner reading the display is **not** governed by
[ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md)'s
admission test, which binds *program-facing* queries only — the runner is not a
ludo program and has always read the display to compute the fit. And a window
smaller than the canvas already has defined behaviour under
[ADR-0031](0031-integer-scaling-loses-to-the-priority-ordering.md) §2, so no new
machinery is needed for small displays (§6).

## 2. The window opens maximal, at the canvas aspect

> On the first launch of a program on a given machine, the runner opens a window
> of the largest size with the canvas's 16:9 aspect that fits within the usable
> work area of the display it opens on — the display area minus platform chrome
> such as taskbars and menu bars. The window **MUST NOT** exceed that work area.

Four candidates were weighed and three rejected.

**1280×720 exactly** is `k = 1` wearing a different name. The map's standing
ordering says *k = 1 is not a state to engineer toward*, and ADR-0031 deleted the
integer scaling that was the only thing `k = 1` ever bought. It is also wrong at
both ends of the display range: a postage stamp on a 4K panel, and larger than
the work area on a 1366×768 laptop.

**The largest integer multiple that fits** was the original proposal in #71 and
died with ADR-0031: its entire justification was integer scaling, which no longer
exists. At 1080p it yields exactly 1280×720 — the previous candidate, arrived at
by a longer route.

**Runner discretion, explicitly** was live longer than the others, because
nothing here is program-observable: [#5](https://github.com/adamico/ludo/issues/5)'s
no-dialect-variance criterion governs divergence a *program* can see, and the
window is invisible to the program (ADR-0013 §9). Variance here is a player-UX
difference, not a semantic one. It loses on the concrete case rather than on
principle — a beginner who ships a game has no way to correct a runner that opens
it small, and *small* is what an undefined default produces in practice.

**Maximal wins on the player's actual behaviour**, which is the evidence that
decided this ticket and is recorded in §3.

A margin — *fill some fraction of the work area* — was proposed and dropped.
A fraction in the spec is a magic constant no conformance test can justify, and
with the maximal rule there is no margin left to quantify: the clamp to the work
area is the whole of what a margin was protecting against.

## 3. The evidence: nobody resizes a game window down, except to share the screen

The grilling turned on an account of what a player actually does with a ludo
game's window. They go fullscreen; or they leave the window at whatever large
size it opened at, possibly borderless for a quick alt-tab; or they deliberately
shrink it to put a guide, a map or a video beside it.

That list decides three things at once.

**It decides §2.** If the player's first act on a small window is to enlarge it,
opening small costs a gesture and buys nothing. Maximal is also the shorter rule.

**It decides §5.** The one reason to resize is *sharing the display with another
window*, and a window sharing a display is not 16:9.

**It disposes of the resolution dropdown entirely** (§7). Nothing in that list is
a resolution choice.

## 4. It is a size, not a maximized window state, and it survives relaunch

The spec says how large the window is. Whether the runner reaches that by setting
bounds or by asking the platform to maximize is runner discretion, exactly as
borderless is under ADR-0025 §1. The distinction matters because the platform's
maximized state carries a separate restore size, and §5's persisted state would
then have to record two sizes and a flag for no player-visible gain.

**The size survives relaunch**, joining the runner-owned per-program state that
already exists — [#17](https://github.com/adamico/ludo/issues/17)'s dylib,
[ADR-0006](0006-forbidden-and-required-shapes-of-the-platform-layer-api.md) R8's
ring buffer, ADR-0025 §6's fullscreen mode. As with the mode, this is stated as
an **observable**, and the spec names no file, no format and no location.

ADR-0025 §6's test is met: runner-owned state works where the runner knows what
the state means. A size is a size. This is where it differs from the display
*ordinal* of [#68](https://github.com/adamico/ludo/issues/68), which goes stale
ambiguously when a monitor is unplugged — **a persisted size that no longer fits
has exactly one sensible answer, and the runner takes it: clamp to the work area,
silently, and persist the clamped value.** No fault, no reset to the §2 default.

**Position is not persisted.** It is ADR-0025 §1's borderless case — cosmetic,
nothing observable depends on it — and persisting it adds a multi-monitor
staleness case for nothing. A runner may persist it anyway; the spec is silent.

Consequently **§2's rule fires exactly once** in a program's lifetime on a given
machine. This includes leaving fullscreen: the window returns to **the last
windowed size**, not to a recomputed default.

## 5. Resize is free of aspect, and the fit already handles it

The window may be resized to any size and any aspect. The runner **MUST NOT**
constrain the drag to the canvas aspect.

ADR-0030/0031's `k = min(w/W, h/H)` handles every aspect with no special case,
and #19's P13 already tests windows both narrower and wider than the canvas. An
aspect-locked drag would fight the player in the one case they actually resize
for (§3), and would make ADR-0025 §2's always-granted resize permission
conditional for the first time. **16:9 is the initial shape, not an invariant.**

## 6. A window smaller than the canvas is ordinary, and the cost is named

> A window, or a display, smaller than 1280×720 is not a special case. `k` falls
> below 1 and the canvas is downscaled. There is no floor, no minimum window
> size, and no refusal to launch.

This is derivable from ADR-0031 §2, which deleted both the `max(1, …)` floor and
the too-small-window case. It is stated explicitly anyway, because the reader who
needs it is an implementer deciding whether to clamp `k` at 1 — and
[ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md)
shipped exactly that floor once already, which is evidence that the wrong
instinct is the common one.

**The cost, which the corpus has not previously named.** At `k < 1`, `crisp` is
still nearest sampling — ADR-0031 §3 fixed that unconditionally, precisely so a
style token cannot mean different things on different monitors. So a downscaled
pixel-art game **drops texels** rather than blurring them. This is ADR-0032's bet
resurfacing: fixing the canvas at 1280×720 weighed panels larger than it, and
panels smaller than it pay. It is rendering fidelity, so the standing ordering
accepts it — but it is now recorded rather than discovered.

## 7. Nothing is program-visible, and the resolution row does not exist here

**ADR-0013 §9 — *the real window size is not exposed* — stands untouched**, and
the program gets no setter either. A setter is a program writing a hardware
magnitude and fails the same guard as the getter: ADR-0028's admission test, with
ADR-0025 §4's "zero bits about the hardware" as its operative clause. A window
size is nothing but bits about the hardware.

So there are **two mutators, neither of them the program**: the player's own
resize (ADR-0025 §2) and the runner. ADR-0025 §3's no-caching rule is inherited
for free, since a program cannot read the value to cache it.

Every candidate use was audited against the corpus and each is already answered:

- **Layout and HUD** — the canvas is fixed and ADR-0032 §6 ships
  `$.graphics.canvas_size`.
- **Pointer input** — [ADR-0011](0011-the-input-facade-a-virtual-controller-over-ordinal-buttons.md)
  reports in logical space, and ADR-0013 §8 ships `to_world`/`to_logical` as a
  pair. The transform is exposed as **a function you call, never as numbers you
  multiply**.
- **Readback** — `get_pixels` takes an `Image`, not the screen. Nothing in the
  corpus reads back the presented framebuffer except P13's harness.
- **Aspect-adaptive layout** — the canvas cannot change shape, and ADR-0030 §6's
  bars are unreachable.
- **Snapping a camera or a hairline to the device-pixel grid, and art LOD by
  scale** — both want `k`. Both are rendering fidelity, last in the ordering, and
  both are the resolution-dependence ADR-0013 §9 exists to prevent.

**The video menu's resolution row was the one case that survived that audit, and
it dissolves rather than being rehomed.** The argument that nearly carried it:
on an LCD the display mode is effectively fixed at native, so what a game's
resolution row *actually does* is set how many device pixels are rasterised — the
window size when windowed. It is a window-size control wearing a different word,
and ADR-0025 §4 gave fullscreen a getter on exactly the reasoning that a finished
game shows the row and sets it.

It fails because **the row has nothing to select here.** Other engines need it
because their canvas *is* their framebuffer; ADR-0030 §3 made ours a coordinate
mapping, so content rasterises at native device resolution whatever the window
is. Windowed, the player drags. Fullscreen, `k` is forced. There is no third
state to name. Combined with §3 — nobody drags a window smaller to gain frames —
the row has neither a mechanism nor a motive.

## 8. Resolution-for-performance is a real question, and it is not this one

The motive the row *did* have is fill rate, and it survives the row's death: a
fullscreen fragment shader ([ADR-0008](0008-the-shader-ludo-dialect-and-the-extern-shader-declaration.md))
at 4K shades four times the pixels it does at 1080p, with no change to the game's
content. It is not a 3D-only concern, and it bites exactly where the player has
no window to drag.

That makes it a **render scale** question — rasterise at `s·k` and upscale — which
shares no mechanism with first-launch geometry. It goes to its own ticket,
[#75](https://github.com/adamico/ludo/issues/75), with four candidates open:
runner-only, unitless-and-program-readable, adaptive, or out of scope on the
ground that ADR-0004 delegated the renderer tier. Whichever wins owes ADR-0030 §3
an answer, since any scale below 100% reintroduces the intermediate framebuffer
that section was pleased to avoid.

It is a ticket rather than map fog because the question is precisely statable
now and nothing blocks it — which is the fog test, and the reason it differs from
the stretch-to-fill and display-choice items that remain fog.

## 9. Conformance: P14, at full conformance only

A new property rather than clauses on an existing one. ADR-0025 §6 attached its
window behaviour as clauses to the single-command-run property, but that was a
boolean a test can read *through the program*. These are geometric facts about a
window that only an external harness can observe — the same shape as P13, which
is why P13 is a property.

> **P14 — the window opens maximal at the canvas aspect, and its size persists.**
> *Precondition:* a program that has never been run on the target machine, and a
> display whose usable work area is known. *Action:* launch it and measure the
> window; resize the window to a different size and a different aspect; exit;
> relaunch. *Observable:* on first launch the window's aspect is 16:9 and its
> size is the largest such size fitting the work area, and it does not exceed the
> work area. On relaunch the window has the size left at exit, clamped to the
> work area if it no longer fits.

**Full conformance only.** Core conformance is headless, so it imposes nothing on
an implementation with no window. It is the second property core cannot run,
after P13.

## 10. Against issue #24: zero delta

No keyword, no operator, no type, no facade call, no stdlib root name. Every
clause here binds the runner, and §7 is the section that keeps it that way: the
cheapest possible answer to a question that other engines answer with an API.

## 11. How it fares on the three lenses

- **Simplicity.** Nothing to configure and nothing to learn. The beginner writes
  no window code and gets a window the size of their screen. The one sentence a
  player-facing manual needs — *it opens as big as it fits, and remembers what
  you leave it at* — is the whole behaviour.
- **Robustness.** Positive. The resolution-dependence bug family stays closed
  because no number is exposed, and §6 removes the floor an implementer would
  otherwise invent. The one hazard, a stale persisted size, has a defined
  resolution (§4) instead of an implementation-defined one.
- **Agent-friendliness.** Neutral by construction, which is the best available
  outcome: there is no surface for an agent to write, correctly or otherwise.
  §7's audit is the work — every plausible reason to reach for `getWindowSize()`
  has a named call that is the right answer instead, which is
  [#4](https://github.com/adamico/ludo/issues/4)'s naming finding applied
  preventively.

## 12. The priority ordering

Neutral on the geometry — a window size costs no throughput and no frame pacing.
The ordering does appear twice: it rejects `1280×720`-as-default in §2 on the
*`k = 1` is not a state to engineer toward* clause, and it accepts §6's dropped
texels as fidelity yielding. §8's ticket is the one place where the ordering will
have to do real work, and this ADR deliberately does not pre-empt it.

## Amendments this ADR makes

- **ADR-0025** — §7's *"at the runner's default size"* is **defined**: the largest
  16:9 window fitting the work area (§2), persisted across relaunch (§4). §2's
  resize permission gains an explicit no-aspect-constraint clause (§5). §3's
  two-mutator pattern is applied to window size with the program excluded (§7).
- **ADR-0031** — §2's deletion of the too-small-window case is made explicit as a
  positive clause, and its cost at `k < 1` under `crisp` is named (§6). No change
  in substance.
- **ADR-0013** — §9 is unchanged and gains the audit that shows nothing needs it
  (§7).
