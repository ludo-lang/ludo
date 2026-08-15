---
status: accepted
---

> **Amended by [ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md):**
> §8's withdrawal of the blanket ban on reading backend state is **superseded** by the
> admission test, and §4's "zero bits" becomes that test's operative guard.
>
> **Amended by [ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md):**
> §2's citation of "ADR-0005's letterbox + integer scale" named a clause that did not exist;
> it now does, with integer scale holding on `crisp` targets only.
>
> **Amended by [ADR-0031](0031-integer-scaling-loses-to-the-priority-ordering.md):** integer
> scaling is **deleted** on every target, so the stamp above is superseded — the citation
> survives as letterbox only, over a single uniform `k = min(w/W, h/H)`.
>
> **Amended by [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md):**
> the ADR-0030 stamp above is the corpus's one instance of a **stale stamp**, and is the case
> that produced §2's rule that a stamp is itself amendable content.
>
> **Amended by [ADR-0036](0036-the-window-opens-maximal-and-belongs-to-the-player.md):**
> §7's *"at the runner's default size"* is **defined** — the largest 16:9 window fitting the
> display's work area, persisted across relaunch; §2's resize permission gains an explicit
> no-aspect-constraint clause; §3's two-mutator pattern applies to window size with the
> program excluded.
>
> **Amended by [ADR-0038](0038-the-escape-hatch-is-a-player-set-render-scale.md):** the
> Budget section's **conditional module name is resolved** and the fullscreen pair is
> **spelled** — `$.video.fullscreen` / `$.video.set_fullscreen`. The suggested `$.window`
> root is **rejected**, because under ADR-0030, ADR-0032 and ADR-0036 §7 a *window* is a
> noun the spec keeps out of the program's vocabulary and the root would make
> `$.window.size()` an agent's natural next reach. §3's two-mutator rule and §6's relaunch
> persistence now govern a **second** preference, the render scale.
>
> **Amended by [ADR-0040](0040-the-runner-reopens-on-the-last-display-and-names-none.md):**
> §3's two-mutator rule is **pattern, not law** — it does not carry to the display choice,
> which the program cannot see and which has exactly one mutator, the platform's own window
> drag. No new runner MUST is minted, on the rule that a **MUST is earned by the failure it
> prevents**: §7's windowed first launch guarantees the drag affordance already exists.
> §6's relaunch persistence governs a **third** piece of runner-owned state, stated with no
> ordinal.

> **Absorbed by [spec ch5](../spec/05-runner.md):** the chapter is normative; this ADR
> keeps the argument — first launch, the relaunch observables and the two-mutator rule are transcribed there.

# Fullscreen is a player preference the program can read and set

Issue #62 asked a narrow question inherited from #60: fullscreen, windowed and
resize are runner-owned and never program-visible, so **how is the runner told**
— a CLI flag, a manifest, or neither? The answer is none of the three, because
the question's premise does not survive contact with what a player expects.

Two facts closed the field before grilling began. **There is no manifest**:
ADR-0014 §2 killed it outright, twice, as the thing issue #3's simplicity survey
put on the must-keep shortlist — *a program is a file you run*. And a CLI flag is
the launcher-shaped answer: it fixes the mode before the process exists, so a
player who wants to change it must quit, and non-technical players do not type
flags, so someone eventually ships a window that collects them. Both channels
answer at **authoring or launch time** a question the player asks **during
play**.

## 1. The window configuration is three things, not one

The phrase inherited from #60 lumps three unrelated decisions:

- **Resize** is not a setting but a permission, and under ADR-0005's letterbox +
  integer scale it changes nothing the program can observe. Always permitted; no
  surface, no clause.
- **Borderless versus bordered** is a cosmetic variant of windowed that no player
  asks to change mid-session. Runner discretion; the spec is silent, and silence
  here is not dialect variance because nothing observable depends on it.
- **Fullscreen** is the only live axis, and it is live because it is
  **player-facing**.

## 2. Fullscreen belongs to the player, not to the developer

This is the root the rest hangs from. Everything else the runner reads out of a
program is genuinely the developer's: the logical canvas is a coordinate space
they draw into (ADR-0013 §5), assets are theirs (ADR-0015). Fullscreen is the
first thing in this spec that belongs to **the player** — a preference about
their own machine, expressed on hardware the developer has never seen.

That is what makes #60's "no program-visible owner" feel right rather than merely
restrictive. The program has no opinion because it is not the program's business.

## 3. Two mutators, both legal

A player must be able to change the mode, and there are two paths:

- **The runner**, through window decoration or a platform affordance. A
  conforming runner **MUST** provide one. The spec does not fix a key, a chord or
  any chrome — only that an affordance exists. Without this MUST, a beginner's
  game that ships no menu leaves the player with no way out of the window on any
  platform, which is the most likely project shape this language targets.
- **The program**, through a settings menu, which is what players expect of a
  finished game.

Both are legal. The consequence is stated plainly because it is the one thing a
program can get wrong: **the mode is not the program's to cache.** The runner can
change it without telling the program, so a program that stores it in a variable
will display a stale checkbox. Read it when you need it — the same discipline
ADR-0013 put on `Target`.

## 4. The program can read the mode, and #60's fullscreen clause is overturned

A settings menu does not only set the mode, it **shows** it. A write-only setter
therefore breaks the very case that motivated it: the menu goes stale the moment
the player uses the runner's affordance. So the program gets a getter as well as
a setter, and #60's *"a program cannot observe or set them"* is overturned for
fullscreen rather than narrowly amended.

This is a correction to #60, not a reversal of it. #60 grilled one case —
**reading the display in order to pick a canvas size** — and rejected
`$.display.size()` on the ground that it had no job. Fullscreen-as-a-write was
swept in as a one-paragraph consequence and never grilled on its own merits, and
the merit it was never tested against is that every player expects a video
settings menu.

What #60 got right stays intact. Its three killed queries were killed because
**hardware facts leaking into a simulation is a bug family**: a canvas that
varies with the monitor shows more world on a wider screen, making hardware a
gameplay advantage. A fullscreen boolean carries **zero bits about the
hardware** — no size, no scale, no density, no count — so none of that reasoning
reaches it. The resolution-dependence bug family stays dead because there is
still no number to depend on.

## 5. ADR-0019 §3 is not triggered

§3 binds whoever proposes a program-visible surface below `$.`: carry `extern`'s
treatment, or weaken criterion 4 deliberately. Neither is needed here. The
surface sits **in `$.`**, so no door below the facade opens, and criterion 4
survives because nothing about the platform diverges — every conforming
implementation returns the same boolean for the same player action. `extern`
remains the only door below `$.`, visible in the signature.

## 6. The mode survives relaunch, and the spec names no file

Relaunching a program restores the mode the player last chose for it. This is
stated as an **observable**, not a mechanism: the runner stores it wherever the
platform says user state goes, and the spec names no file, no format and no
location.

The runner persists the mode current at exit, whichever mutator produced it. This
requires the runner to key that state per program, which is a new runner
obligation and is stated here so it is not discovered later.

Crucially this is **runner-owned state, not program-owned**. It joins a set that
already exists — #17's dylib, ADR-0006 R8's ring buffer — and therefore does not
wait on ludo having any way for a program to write persistent data. It does not,
and that hole is now issue #64; this decision is independent of it.

## 7. First launch is windowed, and the developer gets no say

A ludo game cannot ship fullscreen-first. Every ludo game opens in a window the
first time, at the runner's default size.

The cost is real and is recorded rather than hidden: a shipped commercial game
opening in a small window looks unfinished. Three things bound it. §6 means the
cost is exactly **one toggle in the lifetime of a player's relationship with a
game**. §3 and §4 mean the game can offer that toggle in its own menu on the
first frame if it wants. And the alternative — letting developers choose — buys
the fullscreen-on-first-launch behaviour players most often complain about.

## 8. The blanket ban on reading backend state is withdrawn, and no perimeter is drawn

#60 as written forbids **all** program-visible platform state. That blanket is
withdrawn: reading backend information is not forbidden in principle.

**No perimeter replaces it.** Display size, DPI and display count are not
readmitted here — they remain unspecified, and #60's reasoning against each still
stands unrebutted. What has changed is that their exclusion no longer rests on a
general prohibition, so any future proposal argues on its own merits rather than
against a blanket rule.

This is deliberate incompleteness. Drawing the perimeter now would mean inventing
a test — "expose it when useful" was considered and is too weak, since nearly
every platform query is useful to someone and that is exactly the test every
engine applies on its way to `getWindowSize()`. Issue #4's finding is that the
only reliable defence against an agent reaching for such a call is the call not
existing, so a permissive test carries a real agent-friendliness cost and
deserves its own grilling. The perimeter returns to the map as fog.

## Three lenses

- **Simplicity** — nothing to configure, nothing to launch with, no file to
  write. The beginner's model is unchanged: declare a canvas, draw, and the
  runner fits it. A settings menu is ordinary game code using two calls.
- **Robustness** — the mode has exactly one home, the runner, and the no-caching
  rule (§3) makes the one available bug explicit. #60's hardware-leak bug family
  stays closed because no number is exposed.
- **Agent-friendliness** — the weakest of the three, and the reason §8 refuses to
  generalise. Two calls are easy to write correctly and the getter is
  self-describing, but withdrawing the blanket ban removes a rule that was doing
  real work against training data full of `getWindowSize()`.

## Budget

**#24 delta: two functions, no keywords, no operators, no types.** Core grammar
and type sublanguage both unchanged. The stdlib root-name companion count gains
one module name if these calls live under a new `$.window` root; spelling is left
to the facade-spelling convention of ADR-0009 (verb-first) and is not fixed here.

**#19: two clauses, not new properties.** The runner affordance (§3) and
relaunch restoration (§6) are observable and testable, and attach to the
existing single-command-run property at the **full** conformance level. Core
conformance is headless, so nothing is imposed on an implementation with no
window.
