---
status: accepted
---

# The runner reopens on the last display, and names none to the program

[ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md) §7
ran the admission test over display count and **admitted** it — *which display
should this run on* reformulates to a player preference structurally identical to
fullscreen, and an ordinal carries no hardware magnitude. §7 then handed on what
it had not settled: **the shape**. Issue
[#68](https://github.com/adamico/ludo/issues/68) was filed for exactly that and
closed unresolved, demoted to map fog on the ground that nothing pressed on it.
[#75](https://github.com/adamico/ludo/issues/75) /
[ADR-0038](0038-the-escape-hatch-is-a-player-set-render-scale.md) discharged the
dependency it was parked behind, and issue
[#77](https://github.com/adamico/ludo/issues/77) asks it again.

The answer is that the shape question was framed one door too far along. §7
assumed that admitting the query meant building a program-facing surface for it,
because at the time those were the only two states a fact could be in: refused,
or exposed in `$.`. [ADR-0036](0036-the-window-opens-maximal-and-belongs-to-the-player.md)
opened a third — **runner-owned state the player controls and the program cannot
see** — and the display choice fits it exactly. So the admission stands, unused:
**a permission is not an obligation**, and this ADR declines to exercise it.

## 1. The motivating case, stated precisely

A player with two displays launches a ludo game. It opens on the wrong panel.
They drag it across and go fullscreen, which is the answer every desktop OS
already gives and needs nothing from this spec. Then they quit, relaunch, and it
opens on the wrong panel again — **every time, forever**, because ADR-0036 §4
persists the window's size and explicitly does not persist its position.

That relaunch half is the entire live complaint. It is not *which display can the
program select*; it is *why does this one forget*. Once stated that way the case
is served without any program-facing surface at all, and the four questions #77
inherited from the fog — is a count exposed, does the two-mutator rule carry, is
it runner-owned state, is there an ADR-0011 analogue — are answered by the door
chosen rather than by a facade design.

## 2. The rule

> **A program reopens on the display it last occupied.** On first launch on a
> given machine the runner places it on the platform's primary display, at
> ADR-0036 §2's maximal size. Thereafter the runner restores the display the
> window was on at exit. If that display is no longer attached, the program opens
> on the primary display, silently.

This is an **observable**, on ADR-0025 §6's terms: the spec names no file, no
format, no location, and — the load-bearing omission — **no ordinal**. It says
*the display it last occupied*, not *display index 2*. How a runner re-identifies
a physical panel across a relaunch is its business; platforms hand out stable
display identifiers, and a runner that fails to match one falls through to the
primary clause above.

**Runner discretion was rejected**, on ADR-0036 §2's precedent rather than on a
general principle: a spec that leaves this silent leaves the complaint in §1
unfixed on some conforming runners and fixed on others, which is the outcome §2
already refused for the initial window size.

## 3. Position within a display stays unpersisted, and ADR-0036 §4 is split

ADR-0036 §4 says:

> **Position is not persisted.** It is ADR-0025 §1's borderless case — cosmetic,
> nothing observable depends on it — and persisting it adds a multi-monitor
> staleness case for nothing.

The clause is **split, not overturned**. What §4 called "position" was two facts
wearing one word: *where on a display* and *which display*. The first stays
unpersisted for §4's stated reason, unchanged — it is cosmetic and no player
complains that a window came back forty pixels left of where they left it. The
second is now persisted, because §1's answer is that it is **not** "for nothing":
reopening on the wrong panel is the one multi-monitor complaint players actually
file.

§4's staleness objection survives the split and is paid, not dodged. It stands
because §4 was reasoning about a **stored ordinal**, which is what #68 was
assumed to require and what §4 called "stale ambiguously" — unplug the monitor
that was index 1 and index 2 now denotes a different physical panel than the one
the player chose, so the stored value silently means something new. §2's rule
never mints that value. Nothing stores a number whose referent can shift; the
runner either re-identifies the panel or it does not, and the failure case has
one answer.

## 4. The two-mutator rule does not carry, and this is the first preference where it does not

[ADR-0025](0025-fullscreen-is-a-player-preference-the-program-can-read-and-set.md)
§3 established two legal mutators over one piece of player state — the runner
**MUST** provide an affordance, and the program may set it through its own menu —
and ADR-0038 §5 carried the pattern to a second preference, weakening the MUST's
*justification* (trapped in a window versus merely running slow) while keeping
its strength. #77 asks whether it carries a third time, and at what strength.

**It does not carry, and no new MUST is minted.** There is exactly one mutator:
the player drags the window, using the affordance their windowing system already
provides. This is not a runner obligation the spec invents, it is the one piece
of window chrome that exists on every desktop platform ludo targets, and ADR-0025
§7 guarantees the window the player needs it on — first launch is windowed, so
there is no state a player can be in where a program has never been draggable.

The MUST-strength question therefore never arises, and the reason it does not is
the same reason ADR-0038 had to argue its strength rather than inherit it:
**strengths must be earned by the failure they prevent.** Fullscreen's MUST
prevents a player being trapped. Render scale's prevents a player running slow
with no recourse. A display MUST would prevent nothing — the recourse is already
there, in the title bar.

The consequence for a player in fullscreen on the wrong panel is stated plainly:
leave fullscreen, drag, re-enter. Three actions, once per machine per program,
because §2 then remembers. That is the same shape of cost ADR-0025 §7 accepted
for its one-toggle-per-lifetime rule, and it is bounded by the same mechanism.

## 5. Nothing is program-visible, and the admission test is not run

**No call ships.** No getter, no setter, no count, no handle, no `$.video`
addition. ADR-0028's admission test **is not run**, because the test binds
program-facing queries and there is no program-facing query — the same reasoning
ADR-0036 §1 already recorded when it observed that the runner reading the display
to compute the fit was never the test's business.

The program-facing door is closed, and the argument is recorded here so it is not
reopened cheaply. All three candidate shapes were run against ADR-0028 §3's
return guard and **all three fail**:

- **An ordinal pair** — `display()` / `set_display(i)`. The getter passes the
  guard on its own: an ordinal carries no size, density or refresh, which is what
  §7 correctly observed. But a settings menu must **enumerate** to let a player
  choose, so the pair is unusable without `display_count()`, and a count **is** a
  hardware magnitude. The pair fails through the call it drags behind it.
- **Cycle-only** — `next_display()`, no getter, no count. It passes the guard
  trivially by returning nothing, and dies on ADR-0025 §4 verbatim: a write-only
  mutator breaks the case that motivates it, because a menu does not only set a
  preference, it **shows** it.
- **An opaque handle set.** Its `len()` is the count wearing a different word,
  and the guard reads what is returned, not what it is called. It fails a second
  time on ADR-0011's refusal of a `Controller` handle: a handle to a device can
  dangle across a hot-unplug.

The failure is **structural, not incidental**: a menu that lets a player choose
must enumerate, and enumeration returns a count. Any future proposal must either
break that chain or re-argue the guard.

## 6. The ADR-0011 analogue holds, in the only form left

#77 asks whether ADR-0011's *an absent device is present and idle* has an
analogue — a vanished display silently resolving to the primary. **It does**, and
§2's last clause is it: no fault, no dialog, no missing state, the same refusal
to make a hardware absence into an error the program or player must handle.

The analogy is weaker than it looks and the difference is worth recording. In
ADR-0011 the idle rule exists so that **a call site conditional on a device is a
branch that never gets tested** — it protects program code. Here there is no
program code to protect, because §5 ships no surface. What carries over is only
the disposition: absence resolves to a defined, unsurprising default rather than
to a failure.

## 7. Dialect variance is not triggered, and only door (b) escapes it

Two conforming runners could disagree about which display a program reopens on
after a topology change, since §2 leaves re-identification to the runner. Under
[#5](https://github.com/adamico/ludo/issues/5)'s criterion 4 that would normally
be a Tier 1 veto.

It is not one here, on ADR-0025 §1's borderless reasoning: **silence is not
dialect variance when nothing observable depends on it.** No ludo program can
detect which display it is on, so no program's behaviour can diverge, and the
divergence is confined to where the pixels land — a fact about the player's desk,
not about the language.

This defence is available **only** because §5 ships nothing. Under a
program-visible shape the same runner disagreement becomes two conforming
implementations returning different values from the same call, which is the
dialect-variance veto arriving through the back door — exactly the failure mode
ADR-0038 §8 named when it fixed the render-scale grid. Door (b) does not merely
avoid the admission test; it is the only door on which §2's runner discretion is
affordable.

## Three lenses

- **Simplicity.** The strongest of the three, and it is the *absence* that
  delivers it: no call, no field, no mode, nothing to learn, nothing in a
  beginner's way. What a player gets is the behaviour they already assumed —
  the game comes back where they left it — and what a developer gets is one
  fewer row their settings menu is expected to carry.
- **Robustness.** The bug family #60 closed stays closed with no new guard, since
  the count that would have been its re-entry point is never minted. The one
  hazard, a display that vanishes between runs, has a defined answer in §2, and
  §3's split means no stored value can silently change referent.
- **Agent-friendliness.** [#4](https://github.com/adamico/ludo/issues/4)'s
  finding in its strongest form: the call an agent's training data reaches for —
  `getDisplayCount`, `setMonitor`, `getScreens` — **does not exist**, so the
  failure mode is a compile error at a name rather than a program that behaves
  differently on the player's second monitor. No lens conflict: unusually, all
  three are paid by the same decision to ship nothing.

## Budget

**#24 delta: zero.** No keyword, no operator, no type, no facade call, no stdlib
root name. §2 constrains runner behaviour only, and adds no surface for the
program or for the developer to configure. This is the third consecutive zero
bought by declining a knob (ADR-0036, ADR-0039, this).

**#19: no new property, and the reason is stated** per ADR-0030 §8's test. §2's
observable is testable in principle, but the test requires **two attached
physical displays and a topology change between runs** — hardware no conformance
run can require, where P13 and P14 need only several window aspects on one
display. P13 and P14 are already full-conformance-only; a display property would
be a fourth tier of one, and #19 gains nothing checkable in exchange. The size
half of relaunch persistence remains covered by P14.

## Amendments this ADR makes

- **ADR-0025** — §3's two-mutator rule is established as **pattern, not law**: it
  does not carry to a preference the program cannot see, and its MUST is earned
  by the failure it prevents rather than inherited (§4).
- **ADR-0028** — §7's hand-off is **discharged**. The admission stands and is
  **not exercised**; the shape question resolves by declining the program-facing
  door, and §5 records the three candidate shapes' failure against §3's return
  guard so the door is closed with an argument rather than by silence.
- **ADR-0036** — §4's *position is not persisted* is **split** (§3): position
  within a display stays unpersisted on §4's own reasoning; **which display is
  now persisted**, because §1 falsifies the "for nothing" half. §4's staleness
  objection is answered by §2 minting no ordinal.
