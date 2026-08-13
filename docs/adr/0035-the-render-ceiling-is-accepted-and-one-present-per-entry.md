---
status: accepted
---

# The 60Hz render ceiling is accepted, and one image is presented per frame entry

Issue [#70](https://github.com/adamico/ludo/issues/70) was filed because the
render ceiling had been **inherited rather than chosen** — the exact outcome
[#28](https://github.com/adamico/ludo/issues/28) set itself an obligation to
prevent, in as many words:

> A permanent 60Hz render ceiling is an acceptable outcome (DragonRuby ships real
> games on it) — it just has to be chosen rather than inherited by accident.

[#26](https://github.com/adamico/ludo/issues/26) fixed the simulation at 60Hz and
explicitly declined to decide render-rate decoupling, routing it to #28 with the
mechanism recorded (accumulator plus `alpha` interpolation, which requires two
entries). #28 then resolved to a thin **immediate-mode** drawing facade — which,
by #26's and #28's own analysis, forecloses splitting rendering out later without
a rewrite. Neither #28's resolution nor ADR-0004 stated that consequence, and
**ADR-0013 §4 still spoke of the split in the future tense**, reading as an open
door that immediate mode had already closed.

This ADR chooses the ceiling, closes the door on the record, and writes the
presentation clause the corpus never had.

## 1. The ceiling is accepted

**The render rate equals the simulation rate, fixed at 60Hz, permanently.** One
frame entry call produces one simulated step and one presented image. There is no
second entry, no `alpha` parameter, and no interpolation.

The map's standing ordering decides this, and it decides it against the intuitive
answer:

> Performance and frame stability above rendering fidelity.

Decoupled render rate is the smoother-motion option, and **smoother motion is
rendering fidelity**. What it costs is **stability**: fixed-timestep-with-
interpolation obliges the program to double-buffer every interpolated quantity,
and anything a developer forgets to double-buffer stutters visibly. That is a
per-game correctness burden — appearing only in motion, only sometimes, and
absent from any test that does not have a human watching — traded for visual
smoothness. It is the trade the ordering exists to resolve, and it is not close.

### The cost, stated rather than buried

On a **fixed-refresh 144Hz panel**, or in a **windowed session where VRR does not
engage**, 60 unique frames against 144 refreshes give a 2.4-refresh cadence that
reads as uneven motion. This is a real cost paid by real players.

It lands on the **out-of-box configuration**, not an edge case:
[ADR-0025](0025-fullscreen-is-a-player-preference-the-program-can-read-and-set.md)
§7 makes every ludo game's first launch **windowed**, and VRR in windowed mode is
unreliable, depending on the desktop compositor. In fullscreen on a VRR display —
near-universal on gaming monitors sold since ~2019, and a fixed 60fps sits inside
a typical 48–144Hz window — there is no judder at all.

Recorded honestly because #70's own established facts include that **the refresh-
rate statistic does not exist**: the Steam Hardware Survey has never collected
monitor refresh rate, so any claim about how common 144Hz is — including the
judder argument that motivated this ticket — is an estimate, not evidence. The
cost is therefore stated as a mechanism, not sized.

## 2. Two rejected alternatives, and one that is not an alternative at all

**Reopening fully** — a second entry taking `alpha`, mandatory double-buffering —
loses to §1's ordering argument directly.

**Splitting the entries now while mandating no interpolation** is the option #70
did not list and the more interesting rejection. It would give `simulate()` zero
parameters (#26's conclusion in the place its argument actually applies) and
`draw(screen: !Target)` the target (ADR-0013 §1), with the runner calling them
1:1 so behaviour is identical to §1 today — but with the facade already shaped for
interpolation, making ADR-0013 §4's future tense true rather than stale.

It is rejected on three grounds:

1. **It does not buy what it appears to buy.** Interpolation needs the renderer to
   read *two* states, which needs the program to double-buffer. Splitting entries
   buys the entry-point shape and nothing else; the hard half — the per-game
   burden that §1 rejects — stays undone and unpaid-for.
2. **It pays #24 and every program's shape for optionality**, which is the
   *keeping the door open* move [#6](https://github.com/adamico/ludo/issues/6) and
   [ADR-0017](0017-the-ceiling-is-a-named-function-pointer-and-closures-are-a-non-goal.md)
   both already rejected in other clothes.
3. **It would leave the spec with two entries and no stated reason for the
   second** — every reader asks why, and the honest answer is "for a feature we
   decided not to have."

**Re-calling the drawing entry at display rate with un-advanced state** is
recorded so nobody proposes it later. It produces a **byte-identical image**, so
it costs GPU time and delivers zero motion. It is not a middle ground; it is
nothing.

## 3. One image is presented per frame entry

> **Exactly one image is presented per returned frame entry.** The runner does not
> present a frame twice, and does not skip presenting a frame that was entered.

This clause is what makes §1's accepted cost **bounded and stateable**. Without
it, "60 unique frames" is not even true — a backend could drop or duplicate
presentations and the accepted cost would silently become dropped input-to-photon
frames rather than uneven cadence. Those are different costs and the spec should
not be neutral between them.

Nothing in the corpus said this before. The frame entry returned and what happened
next was unwritten across all thirty-four prior ADRs.

## 4. Pacing is runner discretion, and that is deliberate

**How** the runner waits — vsync on or off, how it sleeps to hit its clock,
whether it free-runs — is **not specified**.

This is
[ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md)'s rule
applied exactly:

> Environment variance is permitted, implementation variance is not.

*How you wait* is environment — it is the one knob that lets a backend do the
right thing on a panel the spec cannot see. *How many images the player sees per
simulated step* is not, and §3 fixes it.

**Mandating vsync-on is rejected specifically.** On a 60Hz panel it gives the
lockstep you want, but on a 144Hz fixed-refresh panel vsync-on is precisely what
turns §1's 2.4 cadence into visible judder, rather than letting the runner pace to
its own clock. A spec-level vsync mandate would make the worst case worse on the
hardware where the cost already lands.

## 5. No #19 property, and the test that decided it

§3 ships as a **normative clause with no conformance property**.

The corpus has both patterns and an explicit test for choosing between them.
[ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md)
§8 warranted P13 because the invariant is *"testable by one framebuffer
readback"*;
[ADR-0022](0022-the-spec-promises-only-what-is-derivable.md) re-cut clauses
without adding properties, on the principle that the spec promises only what is
derivable.

**Presentation count is not a framebuffer readback.** A black-box conformance test
cannot observe how many times the swap chain presented — it can observe the image,
and the image is identical whether the runner presented once or twice. Adding an
unfalsifiable P14 would be the exact move ADR-0022 declined.

The clause still does its job. It bounds §1's cost and makes §1's claim true; it
does so as a normative statement a backend reviewer reads, not a test a harness
runs.

## 6. #19 P7 is strengthened, not amended

P7 — *frame overrun degrades, never catches up* — was written for a 60Hz
simulation that also renders. **That premise is now permanent rather than
provisional**, which is a strengthening, not a change. Its text is correct and is
not rewritten.

Composed with §3, the behaviour is now fully stated where before it was half
stated: an overrunning frame yields **exactly one entry call and exactly one late
image**. The player sees the framerate drop. P7 alone said what the simulation did
and said nothing about what the player saw.

The composition is recorded here and cross-referenced on #19 rather than left to
inference.

## 7. An argument for the ceiling that no prior ticket stated

[ADR-0007](0007-the-audio-facade-surface.md) defines filter-coefficient
recomputation *"at the ramp rate (**once per frame**)"* and parameter ramping as
linear **over one frame**.

Under a simulation/render split, *frame* becomes ambiguous — sim frame or render
frame — and **the audio ramp rate silently becomes a function of the display**.
The same parameter change would ramp at a different speed on a 144Hz monitor than
on a 60Hz one, which is implementation-visible behaviour driven by hardware: the
thing ADR-0028 §4 forbids.

The ceiling keeps ADR-0007's ramp well-defined by construction. Neither #26, #28
nor #70 noticed this; it is recorded because it is an independent argument for the
same conclusion, and because whoever reopens the ceiling inherits it.

## 8. What gets corrected

The open door is stated in **three** places, not the two #70 assumed:

1. **ADR-0013 §4** — *"If #28 later splits simulation from rendering, the drawing
   half keeps the parameter and the simulation half takes zero parameters"*.
   Future tense → closed door. The sentence's *substance* survives as a
   counterfactual: it correctly identified which half would keep the target, and
   that half is now the only half.
2. **ADR-0004** — gains the clause #28 owed and never wrote: immediate mode
   implies the ceiling, and the ceiling is now chosen.
3. **`CONTEXT.md`, *Drawing entry*** — repeats ADR-0013 §4's future tense
   **verbatim**. The glossary carried the open door too, and a glossary is what an
   agent reads first.

## 9. Glossary: `frame` collapses to one referent

*Render ceiling* enters `CONTEXT.md` as a term. The larger win is elsewhere.

Across the corpus **frame** means three things — the entry call, the simulated
step, and the presented image (plus *sample frames* in audio, already
disambiguated by its own glossary entry). §1 **collapses the first three into one
referent, permanently and definitionally.**

This is worth naming rather than leaving as a side effect: an agent reading
*frame* anywhere in the spec no longer has to work out which sense is meant, which
is [#4](https://github.com/adamico/ludo/issues/4)'s non-local-reasoning finding
addressed at the vocabulary level. A split would have re-forked the word.

## Three lenses

- **Simplicity** — a Lua user recognises one loop running at one rate. The split
  is the thing they would not recognise. `love.update` and `love.draw` exist, but
  LÖVE calls them 1:1 and does not interpolate, so the ceiling is what a LÖVE user
  already has.
- **Robustness** — the ceiling **deletes the forgot-to-double-buffer stutter class
  by construction**. There is no interpolation to forget. This is the strongest
  form of the robustness lens: not a compile-time catch, but an unrepresentable
  bug.
- **Agent-friendliness** — strongest of the three. `frame` collapses to one
  referent (§9), and an agent never has to decide **which entry its code belongs
  in** — the single most common structural error in engine code, and exactly #4's
  non-local-reasoning failure mode.

**No lens conflict.** Unusual enough in this corpus to be worth recording
explicitly: the three lenses and the standing ordering all point the same way, and
the decision's only cost is paid by players on fixed-refresh high-refresh panels,
not by any lens.

## #24 delta

**Zero.** No keyword, no operator, no stdlib root name.

§3 and §4 are **runner obligations with no program-visible surface**, so the
companion count is untouched as well. This is the cheapest available resolution of
#70's fork — the split-entries option costs a declaration form and reopening costs
a parameter, while accepting costs nothing and *deletes* a future tense.
