---
status: accepted
---

# The admission test: reformulate, guard the return, then require necessity

ADR-0025 §8 withdrew #60's blanket ban on program-visible backend state and
deliberately drew **no perimeter** in its place, sending the perimeter back to
the map as fog. Issue #65 is that fog, and this ADR draws the perimeter.

It is called **the admission test**. It has three limbs, applied in order, plus
one clause about what an admitted query may vary with. It binds **whoever next
proposes a query into `$.`**, and binds no implementation.

The test was not invented for this ADR. It was reverse-engineered from three
decisions the spec had already made and never generalised — `$.audio.sample_rate`,
`$.input.connected`, and ADR-0025's fullscreen pair — and it was adopted because
it explains all three and the candidate that arrived with the ticket does not.

## 1. What the test governs

**Only a proposal to add a query that reads platform or backend state to `$.`.**

Additions to `$.` in general are already governed: #24 owns the budget process,
with its tiered payment, its one-in-one-out rule for sugar and its stdlib
root-name companion count. Two processes governing one act is how a spec grows
contradictions, so this test is narrow by construction and defers to #24 on
everything that is not a backend read.

**`extern` is untouched.** A program that calls `SDL_GetDisplayBounds` through an
`extern` declaration is outside this test entirely. That path is `unsafe` at the
call site, visible in the signature, and ADR-0019 already makes it the sole
divergence channel below the facade. A rule binding it would be unenforceable and
would restate ADR-0019 §3. This is written down so it is not re-litigated.

## 2. Limb one — reformulate before you refuse

**Before admitting or refusing a proposed query, restate it.** Ask whether the
motivating case is served by a query that reports a fact about the **player** or
about **program-declared state**, rather than a fact about the machine.

This is not a new invention either; it is the move that produced both good
outcomes in the corpus. ADR-0011 wanted device attachment and shipped
`connected(player)` redefined to mean **"has produced input"**, not "is physically
attached" — a fact about a human, wearing the name of a fact about hardware.
ADR-0025 wanted the fullscreen flag and shipped a **player preference**, which is
why its §3 could give the runner and the program two legal mutators over one
piece of state.

The limb is first, not last, because a reformulated query frequently makes the
other two limbs moot — and because a refusal that never attempted reformulation
has not actually examined the motivating case, only its first phrasing.

## 3. Limb two — the return guard

Reformulation smuggles if it is unguarded. Screen resolution is a **player
preference**: it is the first row of every video settings menu ever shipped. A
reformulation limb with no guard readmits, in one sentence, the exact query #60
killed.

The guard:

> **Reformulation may change what is asked. It may never change what is
> returned. If the returned value still carries a hardware magnitude,
> reformulation has failed and the proposal drops through to limb three.**

Fullscreen returns a `bool` — **zero bits about the hardware**: no size, no
scale, no density, no count. That was stated in ADR-0025 §4 as an observation
about one case; this ADR promotes it to the operative guard clause, because it
is the whole reason the fullscreen reformulation was legitimate and the
resolution reformulation is not. A resolution preference returns a size, fails
the guard, and then fails limb three.

## 4. Limb three — strict necessity

A proposal that survives to limb three ships **only if some facility the spec
itself mandates cannot be computed without it.**

The denominator is **spec-mandated**, not "a plausible game needs it". The loose
reading readmits nearly everything, since for any hardware fact some game wants
it — and that is precisely the test every engine applies on its way to
`getWindowSize()`. #4's finding is that absence is the only reliable defence
against an agent reaching for such a call, so the denominator must be bounded by
something the spec can enumerate, and "the spec's own mandated facilities" is the
only such bound available.

`$.audio.sample_rate` is the corpus's one passing case and shows what necessity
looks like: ADR-0006 R6 makes audio a **push at the device rate**, and ADR-0007
transcribes the RBJ biquad into the spec with `w0 = 2 * PI * cutoff /
sample_rate`. Neither the ring nor the filter can be written without the number.
It leaks — a program *can* branch its simulation on 44100 versus 48000 — and it
ships anyway, because the alternative is a mandated facility that cannot be used.

### Why the defence lives at the door

The obvious alternative is to police the **use** rather than the admission: ship
the numbers and forbid feeding them into anything that determines world extent.
That was considered and does not work.

The canvas channel is closed already — ADR-0013 §5 makes `set_canvas` a top-level
statement, immutable for the process's life — but it is not the only channel.
ADR-0005 gives every frame a fresh `Target` carrying a 2D transform and states
plainly that **"Camera, zoom and screen-shake come free"** from it. A program
that sets camera scale from a hardware number each frame reopens #60's bug
family in full, and the channel it uses is a **blessed feature**, not an
oversight. It cannot be closed without deleting the feature, and a taint rule
spanning every expression that reaches a transform is not checkable.

So the use site cannot be defended, and the door can.

## 5. Environment variance is permitted; implementation variance is not

An admitted query may return different values in different places. It may not
return different values in the same place on two conforming implementations.

> **Every conforming implementation must return the same value in the same
> environment. Only the environment may differ.**

`sample_rate` passes: any conforming implementation on the same device reports
that device's rate. A query that two backends answer differently on one machine
fails, and fails as a **#5 criterion 4 Tier 1 veto** — dialect variance — not as
an admission-test finding.

This clause is what ADR-0025 §5 was reaching for when it cleared fullscreen by
observing that "every conforming implementation returns the same boolean for the
same player action". Stated only for its own case, that reasoning did not carry
forward; stated here, it does. Without it the admission test silently licenses
backend divergence, which is the one thing criterion 4 exists to veto.

ADR-0019 §3's obligation is unaffected and still binds each proposal
individually.

## 6. The corpus, audited

The test is demonstrated against everything currently in `$.` that reads backend
state, so that it is not merely asserted, and so a later addition that fails is
visibly an exception rather than a precedent.

| Query | Limb that decides it | Result |
| --- | --- | --- |
| `$.audio.sample_rate()` | three — necessity | **Admitted.** ADR-0006 R6's device-rate ring and ADR-0007's biquad cannot be written without it. Environment-variant, implementation-invariant (§5). |
| `$.audio.cursor()` | three — necessity | **Admitted.** It is the clock the ring is pushed against; ADR-0007 states it in frames at the current device rate. |
| `$.input.connected(player)` | one — reformulation | **Admitted.** Redefined to "has produced input"; never reaches limbs two or three. |
| fullscreen get / set | one, then two | **Admitted.** Reformulated to a player preference; returns `bool`, zero hardware magnitude. |
| `$.display.size()` | two, then three | **Refused.** Returns a hardware magnitude; not necessary to any mandated facility. #60's original refusal, now on stated grounds. |
| DPI | two, then three | **Refused**, identically. |

## 7. What the test changes: display count moves

Display count was #60's *genuinely borderline* exclusion. Run the limbs and it
does not stay excluded.

Reformulation turns "how many displays are attached" into **"which display should
this run on"** — a player preference, structurally identical to fullscreen. The
return guard passes: a display **ordinal** carries no hardware magnitude, since
index 2 tells a program nothing about size, density or refresh. It therefore
never reaches necessity, and **the test admits it.**

What the test does *not* settle is the shape. A menu that lets a player choose
must enumerate, and enumeration returns a count, which has no `bool` to hide
behind the way fullscreen did. That is a question about the shape of a
preference — ADR-0025 §3's two-mutator territory — not about admissions, and
deciding it here would smuggle a facade design into a rule about doors. It is
handed on as its own ticket.

Recording this is the point: the test is not a restatement of what the spec
already believed. It changed an answer.

## 8. What the test does not do

**The admission test is not what keeps a ludo game's world hardware-independent.**
The logical canvas is. ADR-0005 mandates a caller-declared logical size that the
facade letterboxes and scales to the real window, and ADR-0013 §5 fixes it for
the process's life. In the graphics domain limb three is therefore *trivially*
satisfied — display size and DPI are not merely unnecessary, they have **no
expression that consumes them**, which is #60's own "nowhere to put the answer".

This is stated so nobody mistakes the perimeter for the invariant. And it exposes
a gap that is **not** this ADR's to fill: the word *aspect* appears in no ADR, and
the letterbox sentence lives in an ADR-0005 rationale bullet rather than a
numbered clause — ADR-0022 re-cut the graphics conformance clauses and did not
touch it. By ADR-0022's own lesson, a property nobody wrote as a clause is a
property no implementation owes. That defect gets its own ticket. This ADR does
not block on it, because a door-side defence does not depend on what the use site
guarantees.

## 9. Considered and rejected

- **"Expose it when useful."** Rejected in ADR-0025 §8 and again here: nearly
  every platform query is useful to someone, which makes it the test that
  produces `getWindowSize()`.
- **The leak test** — *expose it when it carries no information a program could
  turn into a hardware-dependent simulation* — which is the candidate issue #65
  carried. **Falsified by the corpus**: `sample_rate` leaks exactly that
  information and ships. Patching it with an enumerated exception list was
  considered and rejected — a test that needs a patch on a corpus of one is not a
  test.
- **A `PhysicalPixels` quantity type.** ADR-0016 already blesses `Radians`,
  `Seconds` and `SampleFrames`, so a nominal quantity for physical measurements
  with **no arithmetic path into logical `Vec2` space** would make the perimeter a
  type rather than a rule, and a hardware number could not reach `set_canvas` or a
  transform without a visible cast. Rejected because it spends grammar budget and
  a stdlib type to police a bug family that refusing the number deletes for free.
  **Recorded rather than dropped**: it becomes the right answer the day a
  *necessary* physical number ships, and whoever faces that should not
  re-derive it.
- **Requiring invariance outright**, which would retroactively condemn
  `sample_rate`. Not live; §5 draws the line at implementation variance instead.

## 10. Standing

This test **binds future spec authorship**, in the shape of ADR-0019 §3. It is
**not** a conformance clause and adds **no #19 property**: ADR-0019 already
forbids program-visible surface below `$.` and makes `extern` the only divergence
channel, so an implementation cannot add a query to begin with, and a conformance
clause here would be a second statement of criterion 4.

**ADR-0025 §8 is superseded** in its operative half. Its withdrawal of #60's
blanket ban stands; its "**no perimeter is drawn**" does not — the perimeter is
drawn here. Its statement that display size, DPI and display count "remain
unspecified" stands for the first two on §6's grounds, and is overtaken for the
third by §7.

## 11. The three lenses, and the #24 delta

- **Simplicity**: nothing reaches a program author. The test is a rule about how
  the spec grows, and a beginner's mental model — *I declare a canvas, the runner
  fits it* — is untouched.
- **Robustness**: it keeps the resolution-dependence bug family dead by keeping
  the numbers out, and §5 closes a backend-divergence hole that the corpus had
  left open by arguing one case at a time.
- **Agent-friendliness**: the strongest of the three, and the reason limb three is
  strict rather than loose. #4's finding is that absence is the only reliable
  defence against an agent reaching for a name from its training data, and
  training data is full of `getWindowSize()`. A permissive perimeter carries a
  real Tier 1 cost; a bounded denominator does not.

**#24 delta: zero.** No keyword, no operator, no stdlib root name, no
source-visible surface. The test spends nothing, which is itself an argument for
it.
