---
status: accepted
---

> **Amended by [ADR-0026](0026-a-program-writes-bytes-to-declared-storage-slots.md):** the
> quiescence predicate's live state gains the declared storage slots, which resolve once per
> process.

> **Amended by [ADR-0041](0041-the-record-stops-assuming-a-process-and-re-entrancy-is-by-task.md):**
> §5's *frame thread* is widened to *frame task*, since a browser re-enters by
> task rather than by thread; §3's rescue story gains a tab-discard scope
> sentence.

# Concurrency is a non-goal, and quiescence is a state predicate

Issue #61 inherited an obligation rather than choosing one. Issue #29 recorded a
structural conflict — a real SDL audio device callback runs off the frame thread,
and issue #17's dylib swap is safe only at a quiescent boundary, so a
runner-owned trampoline re-pointed mid-call is a race. That conflict was handed
forward as a hard prerequisite: **define what quiescence means for multi-threaded
ludo, or say why none is needed.**

Most of the answer was already spent. ADR-0006's second forcing argument states
it plainly: the platform layer is written in ludo (ADR-0001), and #17 swaps a
dylib at a quiescent frame boundary, therefore **no ludo code may run off the
frame thread, on any platform**. ADR-0006 R5 spent that rule to make audio
push-only and closed #29's gap for audio specifically, while recording that the
conflict "stands for everything else".

This ADR settles everything else, and it settles it by subtraction.

## Decision

### 1. There is no concurrency surface

No threads, no job system, no coroutines, no async/await. **Concurrency is a
stated non-goal**, recorded in front-matter alongside ADR-0017's closures.

Threads and a job system were already foreclosed by ADR-0006's rule and are
confirmed here rather than decided. A data-parallel `parallel_for` over a
non-capturing function-pointer body — the shape ADR-0017 would have permitted —
is **rejected on the same rule**: the body is ludo code, and it would run off the
frame thread.

### 2. Coroutines are the interesting rejection, and they are rejected

ADR-0006's rule does **not** reach coroutines: they are single-threaded, so
nothing runs off the frame thread. They fail for a different reason, and it is
worth recording because coroutines are the one construct in this space a Lua user
positively expects.

A coroutine holds a **suspended stack that lives across the frame boundary** —
which is exactly the state #17's reload must swap. ADR-0017 already deleted
closures and anonymous functions, so a coroutine would become **the only
construct in the language holding live state that is neither `persist` nor on the
current frame's own stack**. Reload would then have a second state channel to
reason about, and #17's reload set — *exactly the entry file's `persist`
declarations* — would stop being exact.

It also breaks §3's predicate directly: a suspended coroutine means ludo frames
are on a stack at the moment the frame boundary is reached, so the boundary would
no longer be quiescent.

**The replacement is a `persist` state machine.** A multi-frame sequence — a
cutscene, a tween, an enemy behaviour — is a `persist` value advanced by the
frame entry. This is #26's call 3 applied consistently: the sequence's position
becomes **greppable state** rather than an implicit program counter inside a
suspended stack.

**The cost is real and is accepted.** This is the largest simplicity loss on this
map after closures: a Lua user writes `coroutine.yield` and a ludo user writes a
state enum plus a `match`. Against the three lenses: **simplicity** loses
outright; **robustness** gains, since the sequence's state is inspectable in a
fault report (#18 lists `persist` names and types, and can list nothing about a
suspended stack); **agent-friendliness** gains strongly, since a state machine's
whole state is visible at one place while a coroutine's is spread across
suspension points — #4's non-local-reasoning failure mode exactly.

### 3. Quiescence is a state predicate

> **A program is quiescent when no ludo code is on any stack, no `extern` call is
> in flight, and the only live ludo state is `persist` plus declared assets.**

Three artifacts cite quiescence and now cite one definition: #17's dylib swap,
ADR-0015's asset byte-swap behind the handle, and ADR-0014's backend re-point.

**Where the predicate holds** is between two invocations of the frame entry. That
is the location; the predicate is the reason it is safe, and only the predicate
tells an implementer what to check. Two boundaries follow that "between frames"
states badly:

- **After top level, before the first frame.** #26 runs top level exactly once,
  so this is a quiescent point with no frame on either side of it.
- **A faulted program is quiescent forever.** #18 ends the simulation, so no ludo
  code will run again. This is what lets **reload rescue a faulted program** —
  the recovery story that makes error-as-pause worth having, and it is a theorem
  of the predicate rather than a special case bolted on.

### 4. The implementation may use threads; the program may never observe them

ADR-0006's rule constrains **ludo code**, not the backend's C. A backend may run
a real device thread draining ADR-0006 R8's runner-owned ring, a decode thread
for assets, or a compile thread for reload.

**The observable behaviour must be as if single-threaded.** Stated explicitly,
because "no ludo code may run off the frame thread" is otherwise read as "no
threads" — leaving a backend author either over-constraining themselves or
quietly violating a rule they believe they are following. R8's ring buffer only
makes sense with something draining it.

### 5. `extern` is the escape hatch, and a C thread may never call in

A user who genuinely needs a threaded pathfinder binds a C library. This is
already governed: `unsafe` at the call site, wrapper-mediated, a fault barrier,
and per ADR-0019 the only door below `$.`.

**One clause is added: ludo code may only be entered from the frame thread.**
#29 already requires callbacks out to C to be non-capturing, but nothing said the
*calling thread* mattered. Without this, the escape hatch reopens the precise
quiescence hazard the rule exists to prevent — a C worker thread calling a ludo
callback while the runner swaps the dylib underneath it.

### 6. Field-level `#align` is declined permanently

Issue #25 declined field-level `#align` because its real use is false-sharing
avoidance, and parked it — *revisit if this produces threads*. It did not, and
under §1 it never will: with no program-visible concurrency, false-sharing has no
program-visible cause.

The item moves from parked to settled, so nobody reads it as a live question.
`#align(n)` remains the one shipped **type-level** attribute. (As originally
written this sentence said "the one shipped attribute", which was a drafting
error: `#explicit` (#6) and the shader markers `#vertex` / `#fragment` (ADR-0008
§4) are also shipped attributes. Corrected by
[ADR-0029](0029-explicit-marks-a-module-and-forbids-a-spelling-that-omits-a-name.md)
§7.)

### 7. No keywords are reserved

`async`, `await`, `go`, `spawn`, `coroutine` and `yield` stay **ordinary
identifiers**.

Reserving them would spend #24 budget now to buy an option on a stated non-goal,
and would steal six plausible identifiers — `spawn` and `yield` are ordinary
gamedev words (a spawner; a resource yield). If concurrency is ever added it is a
breaking change taken then; the language has no version story yet, so reserving
is guessing at the shape of a decision that has been declined.

## #24 companion count delta

**Zero.** No keywords, no operators, no functions, no types, no module. §1 and §6
subtract a candidate each; §3 defines a term used by the spec's prose and by
implementers, not a name in the language.

## Consequences

- **#29's structural conflict is discharged in full**, not merely for audio.
  ADR-0006 R5's "stands for everything else" is now closed.
- **#17's reload set stays exact** — `persist` and nothing else — because §2
  refuses the one construct that would have added a second state channel.
- **#18 gains a property worth naming**: a faulted program is quiescent, so
  reload can rescue it.
- **#25's parked item is closed** (§6).
- ADR-0006's rule is confirmed and given its general statement; the ADR's own
  warning — that nobody should relax a rule believing it was the browser's when
  it was #17's — now applies to the whole concurrency question.
- **The single-threaded model becomes a load-bearing assumption of the spec.**
  Reversing it would reopen ADR-0006 R5, #17's reload set, §3's predicate and
  ADR-0019's door count at once, so it is a redraw of the destination rather than
  an increment.
