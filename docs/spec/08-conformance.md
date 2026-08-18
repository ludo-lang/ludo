# Chapter 8 — Conformance

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense.

**0.3** This chapter owns **conformance**: what an implementation must be to
call itself ludo. That is the posture (§1), the three rungs (§2), what one
claimant owes (§3), the **experience contract's property
list P1–P17** (§4–§5), the reload-latency measurement and its reference set
(§6), the two conformance halves of the blessed math set (§7), the admission
test as a binding on spec authorship (§8), and the published counts (§9). It is
the referent of every *chapter 8's* pointer in chapters 1–7.

**0.4 This chapter consumes the other seven.** Every clause below is an
obligation about the normative surface chapters 1–7 state; it restates none of
it. Where a property names a rule, the rule's home chapter is cited and the
property carries only the **precondition, action and observable** a harness runs.

**0.5** Every clause cites its source. A clause with no citation is a defect.

**0.6 This chapter specifies obligations on implementations, not language.**
Nothing here has a spelling a program can write, and the reference program is
therefore not extended by it — the second chapter for which that is true of its
whole subject, after chapter 7. `coverage/08-conformance.md` §11 records it.

---

## 1. The posture

**1.1 Conformance is binary.** An implementation conforms or it does not.
**There is no documented-deviation tier.** A deviation tier makes the contract
advisory the moment a property becomes inconvenient, and the properties that
would be deviated on are §5's P1 and P2, which are the whole contract. (#19
posture 3.)

**1.2 The contract binds every conforming implementation.** A batch-only
compiler with no runner is **non-conforming**. The cost is stated rather than
incidental: **a minimal third-party compiler cannot claim the name.** (#19
posture 2; #12; #16.)

**1.3 Invariants bind absolutely; latency binds as a measurement.** Every
property in §5 is an unconditional MUST at the rung §4.3 assigns it. **Reload
latency is a MUST-MEASURE** against §6's procedure and reference set, with a
**published target of 1 second**. An implementation that misses the target
**MUST state its number**; it MUST NOT stay silent. (#19 posture 1; #22 §2.)

**1.4 A conformance suite is implied, and authoring it is a follow-on effort.**
Every property is written **test-shaped** — precondition, action, observable —
so transcribing it into a harness is mechanical rather than a design act. **This
spec owes the assertions; the follow-on effort owes the harness and the
corpus.** (#19 posture 4, §E; ADR-0034 §6, which restates the line.)

**1.5 A rule the suite cannot express is advisory.** #56 checked mandated
clauses against real browser semantics and **falsified two of them**, so a rule
written before an implementation exists is advisory until the suite can express
it. This is the ranking §3.4 applies and the reason §5's properties, not the
prose, are the gate. (#54 finding 2; ADR-0019 *Consequences*.)

**1.6 A property must be falsifiable, and a clause that is not gets none.** A
clause whose observable is a framebuffer readback, or a count no artifact
reports, ships as **normative prose a backend reviewer reads** and mints no
property. Where the corpus declined a property it recorded why, and §5.19 lists
the declinations so a reader does not read one as an omission. (ADR-0030 §8;
ADR-0035 §5; ADR-0038 §12; ADR-0022.)

---

## 2. The three rungs

**2.1** Conformance has three rungs, **totally ordered by containment**:

> **core ⊂ full ⊂ shader**

(ADR-0004; ADR-0037 §2.)

**2.2 Core** is the compiler, the language and the non-visual standard library.
It is **headless**: it requires no device, no window and no GPU. (ADR-0004;
ADR-0037 §2.)

**2.3 Full** is core plus the facades that need a device, over **at least one
backend**. (ADR-0004.)

**2.4 Shader** is full plus chapter 6 §9's two shader authoring paths — shader
ludo and the `extern shader` declaration. (ADR-0008; ADR-0037 §2.)

**2.5** The containment is real and is not an orthogonal capability flag.
Shaders need the facade; the facade needs core. An implementation that shipped
shaders without the facade would be incoherent. (ADR-0037 §2.)

**2.6 The mandated facade needs no GPU.** Every mandated call in `$.graphics` is
satisfiable by a CPU rasterizer, because specifying a shape is not specifying
how it is built. The shader paths are the only place a device's programmable
pipeline is load-bearing. (ADR-0004; ADR-0037 §2.)

**2.7 A rung does not reopen criterion 4.** A program that depends on shaders
**says so in its source**, in a declaration at file scope, so a reader — human
or agent — determines a program's required rung by reading the program.
`extern shader` is an `extern`, and `extern` is visible in the signature, which
is what criterion 4 demands. (ADR-0037 §2; ADR-0019 §2; #5 criterion 4.)

**2.8 A shader declaration against a lower-rung implementation is a compile
error.** An `extern` shader declaration, or shader-ludo source, in a program
built against an implementation that does not claim the shader rung, is a
**compile error naming the declaration**, carrying a spec-owned code in chapter
7's envelope. A runtime fault is wrong here and a silent no-op is dialect
variance outright: target selection is a build-time module set, so the
implementation's rung is known when the compiler runs. (ADR-0037 §4; ADR-0006
R3; ch6 §9.3; ch7 §5.8.)

**2.9 A rung partitions the properties; it adds none.** §5's list is unchanged
in content by the rungs, and the rung decides which properties a given claimant
must pass. (ADR-0037 §7.)

---

## 3. What a claimant owes

**3.1 A conformance obligation is stated on one claimant, never on a pair.**
Each claimant conforms to the spec **alone**. Agreement between two claimants is
a *consequence* of both conforming and is never a thing anyone checks. (ADR-0019
§1.)

**3.2** The rule holds because the pair exists nowhere: the toolchain supplies
exactly one backend and a mapping carrying two entries for one root name is
rejected where it is supplied (ch4 §7.4, §10.2, §10.3), and target selection is
a build-time module set, so **the source never names the backend** (ADR-0006
R3). Any obligation naming a pair describes a comparison no artifact can
perform. (ADR-0019 §1; the first leg re-cited by ADR-0054 §2, which removes the
backend's root-name claim without touching this rule.)

**3.3 This is a shape rule for every future obligation in this spec**, not a
fact about backends. Where a decision is tempted to say *all implementations of
X must agree on Y*, it MUST instead say what **one** implementation owes the
spec. (ADR-0019 §1.)

**3.4 The nominal interface is necessary and insufficient**, and the three
artifacts a reviewer applies to **one** claimant rank as follows:

1. **The nominal interface** (ch4; ADR-0014 §9) — compiler-checked. Catches
   shape; catches nothing behavioural.
2. **ADR-0006 R1–R8** (ch6 §10) — reviewer-checkable. Catches architectural
   divergence before a backend exists.
3. **§5's properties, plus the normative text of chapters 5 and 6** —
   executable. **This is the gate.** A claimant conforms iff it passes.

The ranking is load-bearing and decides which artifact wins when they disagree,
per §1.5. (#54 finding 2; ADR-0019 §1.)

**3.5 The floor is the normative surface, and there is no admissible set.** The
spec names **no graphics API, ever, as a membership test**:

> A renderer component is admissible **if and only if** it can satisfy this
> spec's normative surface at the conformance rung its implementation claims.

Nothing else is a membership test, and there is no list to keep current.
(ADR-0037 §1.)

**3.6** An enumerated roster is the wording shape §3.1 outlaws — a statement
about a *population*, checkable by nobody, stale the day a further API ships. A
capability list authored as a floor is **rejected too**: it is a roster wearing
different clothes, and it invites the failure where a claimant clears every
bullet and still fails a normative property. (ADR-0037 §1.)

**3.7** A renderer component that cannot upload from a strided host array
without repacking **cannot serve a conforming implementation at any rung above
core**. WebGL1 is the known instance. This is a **consequence** of §3.5, not a
membership decree. (ADR-0037 §5, restating ADR-0022 §1; ch6 §4.9.7.)

**3.8 A software rasterizer is admissible, and never mandated.** A claimant MAY
ship a software rasterizer as its renderer component and be **fully
conformant**. It **cannot claim the shader rung**. Mandating one is refused:
full conformance already requires at least one working backend, so software
rendering is a legal choice of one, not a second obligation on everybody.
(ADR-0037 §3.)

**3.9 There is no program-visible surface below `$.`, and `extern` is the only
door.** The only divergence channel below the reserved root is the C FFI, and it
is **visible in the signature** — which is exactly what criterion 4 demands. The
`unsafe` marker on that path *is* the *below `$.` you own your portability*
warning; no second marker is owed. (ADR-0019 §2; ch4 §9.)

**3.10 §3.9 is true because of an absence, and expires with it.** If the
platform layer ever gains a program-visible surface, that decision opens the
first ludo-side door below `$.`, and whoever takes it MUST either carry
`extern`'s treatment — `unsafe` at the call site, visible in the signature — or
re-open criterion 4 and weaken it deliberately. It **may not** open the door and
leave criterion 4 unexamined. (ADR-0019 §3.)

**3.11 Criterion 4 constrains the language, not every library written in it.**
If two third-party authors both claim `physics` and behave differently, the spec
owes nothing: a program that swaps one for the other has changed its
**dependencies**, not its dialect. Mutually-exclusive claimants stay **legal and
unblessed** for third parties, and nothing in this spec may be read as implying
that the language *supports* interchangeable implementations. (ADR-0019 §4.)

---

## 4. The property list: what it is

**4.1** The **experience contract** is a list of properties, each written as a
**precondition, an action and an observable**, that any conforming
implementation MUST deliver. It exists because an implementation could satisfy
every semantic rule in chapters 1–7 and feel nothing like the loop this language
is for. (#19; #16.)

**4.2** The list is **P1–P17**. Each property traces to a decision made
elsewhere; none is invented here, which is what keeps a contract from becoming a
wishlist. (#19 §B.)

**4.3 Each property binds at one rung**, per §2:

| Rung | Properties |
|---|---|
| **Core** (headless) | P1–P12, P15 |
| **Full** (adds a device) | P13, P14, P17 |
| **Shader** | P16 |

Containment means a full-conformance claimant owes P1–P15 and P17, and a
shader-conformance claimant owes all seventeen. (ADR-0004; ADR-0037 §2; ADR-0030
§8; ADR-0036 §9; ADR-0038 §12; ADR-0008 §10; ADR-0015 §8.)

**4.4 A clause may attach to a property instead of becoming one.** Where an
observable is a fact a test reads *through the program itself*, it attaches to
the property it belongs to rather than minting a new one; where it is a fact
only an **external harness** can observe, it is a property. That distinction is
why P13 and P14 exist and why the fullscreen and render-scale affordances do
not. (ADR-0025 §6; ADR-0036 §9.)

**4.5** A **P-token** in this spec names a property that exists. Writing one for
a property that does not is forbidden, because the audit that found P13 missing
from the corpus greps for exactly that token and cannot tell a prose numeral
from a live one. (#19, the P13 note; ADR-0044 §3.)

---

## 5. The properties

### P1 — Reload preserves declared state

**5.1** *(core)*

> **Given** a running program with `persist` declarations holding known values,
> **when** a source edit is saved that does not change any `persist` type,
> **then** the process is not restarted, the frame entry is next called from a
> build containing the edit, and every `persist` declaration not marked `unsafe`
> holds the value it held before the edit.

(#19 P1; #17; ch5 §4, §5.)

**5.1.1 Supporting assertions**, all chapter 5's: the swap occurs **between
frames**, never mid-frame; a build that **fails to compile leaves `persist`
untouched** and the old code running; a **type-changed** declaration is merged
field path by field path, preserving fields whose name and structural hash
match; **program constants are reload-stable**; the entry file's **top level
re-runs** on reload while `persist` initialisers do not. (#19 P1; #17; ch5 §5.)

**5.1.2** **Latency is measured, not asserted** — §6.

**5.1.3 Two clauses attach at full conformance.** The audio ring is
runner-owned and survives reload, so **reload is click-free**; and the scale and
fullscreen preferences a player set persist across relaunch. (ADR-0006 R8;
ADR-0025 §6; ADR-0038 §9; ch6 §5, §7.)

### P2 — A fault pauses rather than terminates

**5.2** *(core)*

> **Given** a running program, **when** a runtime-detected bug occurs — the
> closed class of chapter 2 §8.3 — **then** the current frame is abandoned, no
> further frame entry is called, the process remains alive in dev, and `persist`
> equals its value at the instant of the fault.
>
> **And when** a fix is saved, **then** the program resumes via P1's reload with
> that `persist` intact — the session is not lost.

(#19 P2; #18; ch5 §6.)

**5.2.1 Supporting assertions**: **reading a `persist` value during a pause
executes no user code**; the fault report carries kind, source location,
concrete values and the call chain, in chapter 7's envelope; a **top-level**
fault means the simulation never starts and prior `persist` is untouched. (#19
P2; #18 §8; ch5 §6.5; ch7 §2.3.)

**5.2.2 In release the program terminates abnormally by the host's
convention**, after emitting the `fault` message. On a hosted target that is a
non-zero exit status; in a browser tab the convention is that **the trap
surfaces as an exception through the JavaScript host and the frame entry does
not return**. The property stays binary, mode-independent and universal; what
changes is what it can be **failed** on. (ADR-0041 §3; #18 §4; ch5 §6.3; ch7
§2.5.)

**5.2.3 Scoping P2 to hosted targets is rejected**, on §1.1: it would create a
documented-deviation tier, and the excuse would be permanent, since no future
browser grows an exit code. (ADR-0041 §3.)

**5.2.4** **P10 is the callback case of this property**, and is cross-referenced
rather than left to inference. (#19 P10; #29 §8.)

**5.2.5 One clause attaches at full conformance**: after a fault the backend
**drains what is queued, then outputs silence, never a loop**. (ADR-0006 R8;
ch6 §5.)

### P3 — Single-command run

**5.3** *(core)*

> **Given** a directory containing exactly one `.ludo` file, **when**
> `ludo <file>` is run, **then** the program runs — with no manifest, no config
> file, no project directory, and no init step required as input.
>
> **And given** any build artifact or cache the implementation created, **when**
> it is deleted and the command re-run, **then** observable behaviour is
> identical and only speed differs.

(#19 P3; #3; #12; ch5 §1.)

**5.3.1** Build artifacts and caches are permitted but MUST live **outside the
project directory**. The load-bearing clause is *never change semantics, only
speed*: it makes a stale cache a performance bug rather than a correctness one.
Artifacts landing beside the source is where *a program is a file you run*
becomes *a program is a directory*. (#19 P3.)

**5.3.2 Four clauses attach at full conformance**, on §4.4's test — each is a
fact a test reads through the program or through a relaunch, not a geometric
fact needing a harness: the runner's **fullscreen affordance** and the
**persistence of the fullscreen mode** across relaunch (ADR-0025 §3, §6); the
runner's **render-scale affordance** and the **persistence of the scale** across
relaunch (ADR-0038 §5, §9). (ch5 §11; ch6 §7.)

### P4 — No unchecked escape path

**5.4** *(core)*

> **Given** any conforming implementation, **when** the toolchain is invoked by
> any documented means, **then** there exists no flag, environment variable,
> loader hook, or build mode that executes ludo code with type checking, bounds
> checking, or overflow checking disabled.

(#19 P4; #8.)

**5.4.1** **`unsafe` is explicitly not such a path** and MUST NOT be read as
one: it is per-site, greppable and visible in the source. Bounds checks are
always on, and skipping one is a thing you *write*, not a thing you
*configure*. (#19 P4; #8; ch2 §8.3; ch3 §16.)

### P5 — Diagnostics are machine-readable, and never guess

**5.5** *(core)*

> **Given** a program that fails to compile, **when** the toolchain is asked for
> machine-readable output, **then** each diagnostic carries a stable diagnostic
> code, a source location, the named entity, and a severity, in a documented,
> versioned serialization.
>
> **And when** a diagnostic offers a suggestion, **then** resolution is exact.

(#19 P5; ADR-0018; ch7 §3, §10.)

**5.5.1** The field set, the encoding, the code rules and the severity enum are
**chapter 7's**, and this property is failed on them. A suggestion that is
present and, when applied, does not resolve its diagnostic is a **conformance
failure**. (ADR-0018; ch7 §10.4.)

**5.5.2** The never-guess clause is not a nicety: supplying a wrong root-cause
analysis **lowered** a model's correct patches by 46.6% and 22.6% in the two
measured languages, so a confidently wrong *did you mean* is worse than silence.
(#22; #19 P5; ch7 §10.2.)

### P6 — The oracle answers correctly and incrementally

**5.6** *(core)*

> **Given** a checked project, **when** one file is edited, **then** re-checking
> examines only what the edit affects, and answering a name/type query about one
> file never requires reading the whole program.
>
> **And given** a syntactically incomplete file, **when** the oracle is queried,
> **then** it still answers from a recovered tree.

(#19 P6; #22 §4.)

**5.6.1** The property is a **shape** — incremental and local, hardware-
independent, fast by construction. It carries **no millisecond figure**; the
number belongs to §6's benchmark. (#22 §4.)

### P7 — Frame overrun degrades, never catches up

**5.7** *(core)*

> **Given** a frame entry that takes longer than the fixed 60Hz step, **when**
> it returns, **then** exactly one frame entry call has occurred for that step —
> the simulation runs late and slows down, and no catch-up call is made.

(#19 P7; #26; ch5 §2.)

**5.7.1** Composed with chapter 5's one-present-per-entry rule, the full
observable is: an overrunning frame yields **exactly one entry call and exactly
one late image**. The player sees the framerate drop. (ADR-0035 §3, §6; ch5
§10.)

**5.7.2** P7's premise is **permanent**, not provisional: the 60Hz simulation
that also renders is fixed, not pending a render/simulation split. (ADR-0035
§1; ch5 §2.)

### P8 — Mode-independence, with three enumerated exceptions

**5.8** *(core)*

> **Given** the same program built in dev and in release, **when** both are run
> against the same inputs, **then** observable behaviour is identical **except**
> for exactly three differences: (1) on a fault the dev runner holds the process
> alive where release terminates abnormally; (2) reload exists only in dev;
> (3) the paused-inspection surface exists only in dev.

Anything else differing is **non-conformance**. (#19 P8; #18 §4, §9; #8; ch5
§6.3.)

**5.8.1** The closed list is the useful part: it converts criterion 4 from a
design principle into something a test can fail an implementation on, by running
one corpus both ways and diffing. (#19 P8.)

### P9 — No vendor syntax extensions

**5.9** *(core)*

> **Given** the spec's machine-readable grammar, **when** an implementation is
> tested against it, **then** a program using any construct outside the grammar
> MUST be rejected, and every program the shipped grammar accepts MUST parse —
> the implementation's accepted language is **exactly** the specified one,
> neither narrowed nor extended.

(#19 P9; #24 call 6; #22 §7; ch1 §13.10.)

**5.9.1** The property is testable **directly against the shipped grammar
artifact**, [`grammar.ebnf`](grammar.ebnf), which is why §9.2's count is
co-located with it. (#24 call 7; ch1 §13.5.)

**5.9.2** This is the **implementation-facing half** of the grammar budget. The
author-facing half — the tiered payment rule and the published count — binds
this spec's authors and is chapter 1 §13's. (#24 call 6; ch1 §13.10.)

### P10 — A fault in a C-invoked callback does not unwind through C

**5.10** *(core)*

> **Given** a ludo callback registered with a C library, **when** a bug is
> triggered inside it, **then** the process does not abort, the callback returns
> immediately with the value its author declared in the signature for that case,
> further callbacks within the same C call return it without running, and the
> runner reports the fault with chapter 7's full field set once control returns
> to ludo.

(#19 P10; #29 §8; ch4.)

**5.10.1** The callback boundary is a **fault barrier**. This is the second
construct in the language with a declared behaviour-on-fault, which is why
§5.2.4 cross-references it. (#29 §8.)

### P11 — A C-held callback pointer survives a reload

**5.11** *(core)*

> **Given** a callback registered with a C library at init, **when** the program
> is edited and reloaded, **then** the next C-driven invocation runs the new
> build's code.

(#19 P11; #29 §9.)

**5.11.1** The spec owes the guarantee; the runner owns the machinery. **This is
not a P8 exception** — the indirection differs between dev and release, the
behaviour does not. (#29 §9; #19 P11.)

### P12 — No flag changes the accepted program set

**5.12** *(core)*

> **Given** an implementation offering any set of flags, **when** a corpus is
> compiled under every flag combination, **then** the accept/reject set is
> **identical** across all of them.

(#19 P12; ADR-0023 §3, §7; ch7 §12.6.)

**5.12.1** This is the first place #5 criterion 4 becomes a **failable
property** rather than an argument. A pipeline gate that reads chapter 7's
stream and exits non-zero is permitted and does not violate it — the compile
succeeded — but a vendor `-Werror` under which the compiler **rejects** a
program another conforming implementation accepts does. Without this property
the two are indistinguishable. (ADR-0023 §3; ch7 §12.6, §12.7.)

### P13 — The canvas is fitted, not stretched

**5.13** *(**full** conformance only)*

> **Given** a program — the canvas is fixed at 1280×720 for every program, so
> the aspect is a known 16:9 and not a parameter — **when** it is run in windows
> of several aspects, including one narrower and one wider than the canvas, and
> the presented framebuffer is read back, **then** the canvas image occupies a
> centred rect whose aspect is 16:9, at `k = min(w/W, h/H)`, with every pixel
> outside it opaque black and no program content among them.

(#19 P13; ADR-0030 §8; ADR-0031; ADR-0032; ch5 §9.)

**5.13.1** `k` is **unconditional** — one formula, no `style` branch and no
integer floor. (ADR-0031; ch5 §9.)

**5.13.2** P13 is the **first property core conformance cannot run**: it
requires a presented framebuffer. (ADR-0030 §8.)

### P14 — The window opens maximal at the canvas aspect, and its size persists

**5.14** *(**full** conformance only)*

> **Given** a program that has never been run on the target machine, and a
> display whose usable work area is known, **when** it is launched and the
> window measured, then resized to a different size and a different aspect, then
> exited and relaunched, **then** on first launch the window's aspect is 16:9
> and its size is the largest such size fitting the work area, and it does not
> exceed the work area; and on relaunch the window has the size left at exit,
> clamped to the work area if it no longer fits.

(#19 P14; ADR-0036 §9; ch5 §11.)

**5.14.1** These are geometric facts about a window that only an external
harness can observe, which is why they are a property rather than clauses on P3
— §4.4's test. (ADR-0036 §9.)

### P15 — The render scale quantises to the grid, and clamps rather than faults

**5.15** *(core)*

> **Given** a conforming implementation, at any rung, **when**
> `$.video.set_render_scale(x)` is called and then `$.video.render_scale()`, for
> `x` in turn `1.0`, `0.3`, `0.25`, `0.0`, `-1.0` and `4.0`, **then** the getter
> returns, respectively, `1.0`, `0.3125`, `0.25`, `0.25`, `0.25` and `1.0`. No
> fault is raised for any input, and every returned value is a multiple of
> `1/16` in `[0.25, 1.0]`.

(#19 P15; ADR-0038 §12; ch6 §7.)

**5.15.1** It is **core-runnable** — the first of the device-adjacent
properties that is — because it is a pure API round-trip needing neither a GPU
nor a screen. (ADR-0038 §12.)

**5.15.2** The grid is not an implementation detail: it is what makes the
returned value **implementation-invariant** under §8.5. Without a fixed grid,
`set(0.3)` returns whatever a backend rounded to and two conforming
implementations disagree — a criterion 4 Tier 1 veto arriving through the back
door. (ADR-0038 §12; ADR-0028 §5.)

### P16 — A failed shader recompile retains the last-good pipeline

**5.16** *(**shader** conformance only)*

> **Given** a running program at the shader rung with a compiled pipeline,
> **when** a shader source edit is saved that fails to recompile, **then** the
> last-good pipeline is retained, an **error value** is reported, and the
> program **does not fault**.

(#19; ADR-0008 §10; ADR-0003; ch6 §9.)

**5.16.1 The property was decided by ADR-0008 §10 and never numbered.** ADR-0008
§10 states it in as many words and hands it to the contract; the contract's list
stopped without it, exactly as it stopped before P13. It is numbered here, and
`coverage/08-conformance.md` §9.1 records the finding. (ADR-0008 §10; #19, the
P13 note.)

**5.16.2 Its rung is the shader rung, not full.** ADR-0008 §10 wrote *scoped to
full conformance* against the two-rung split that existed then, and ADR-0037 §2
subsequently made ADR-0008's two authoring paths **define** the third rung. A
full-conformance claimant has no pipelines to keep, which is ADR-0008 §10's own
argument for not making it core. (ADR-0008 §10; ADR-0037 §2.)

**5.16.3** A broken shader edit is an **error value**, not a fault: chapter 2's
bug class is closed and a failed recompile is not in it. (ADR-0008 §10;
ADR-0003; ch2 §8.3.)

### P17 — Editing a declared asset's file reloads it live

**5.17** *(**full** conformance only)*

> **Given** a running program with a declared `extern` asset, **when** that
> asset's file is edited and saved, **then** the runner swaps the bytes behind
> the handle at the next quiescent frame boundary, without restarting the
> process and without resetting `persist`.
>
> **And when** the re-decode fails, **then** the last-good asset is retained, an
> **error value** is reported, and the program does not fault.

(#19; ADR-0015 §8; ch5 §7.)

**5.17.1 The property was decided by ADR-0015 §8 and never numbered**, on the
same route as P16. ADR-0015 §8 calls it *P1's sibling* and states its
precondition, action and observable; the contract's list never gained it.
`coverage/08-conformance.md` §9.1 records the finding. (ADR-0015 §8.)

**5.17.2** It is **mandated, not optional in spirit**: the tightest loop a game
developer has is *nudge the art, look at it*, and shipping state-preserving code
reload while making an artist rebuild to see a sprite would be the contract
missing its most-used case. (ADR-0015 §8; #16.)

**5.17.3** The asymmetry with a **missing** asset is deliberate: missing at
startup is a program that cannot run and is a fault; broken at reload is a
program already running fine and is an error value. (ADR-0015 §4, §8; ch5 §7.)

**5.17.4** It is **full-conformance only** for the same reason P13 and P14 are:
a headless core claimant has no device to swap bytes into. Its full-rung status
follows ADR-0015 §8's own wording, which is unchanged by the shader rung.
(ADR-0015 §8; ADR-0037 §2.)

### 5.18 The list is closed at P17

**5.18** A future decision that mints a property extends this list and states
its rung. A decision that declines one **records why**, per §1.6, so a reader
does not read the absence as an omission. (ADR-0030 §8.)

### 5.19 Declined properties, and why

**5.19** Recorded so no absence below is read as an oversight. (ADR-0030 §8,
which fixed the requirement to record.)

| Clause | Why no property |
|---|---|
| One image presented per returned frame entry | Presentation *count* is not a framebuffer readback, and the image is identical whether the runner presented once or twice. Ships as prose; composed into P7 at §5.7.1. (ADR-0035 §5.) |
| The rasterised image at a given render scale | A framebuffer readback. ADR-0038 §1's arithmetic ships as prose. (ADR-0038 §12.) |
| Graphics exactness, the withdrawn byte-exact blit, the texel rule | No property tested the blit and none tests exactness at all; pixel exactness is suite territory, not list territory. (ADR-0034 §6; ADR-0022 §2, §3.) |
| *ludo is entered only from the frame task* | Would require proving a negative over every callback a host can schedule — an open set that grows with the platform, not the spec. Binds spec authorship and implementation review instead. (ADR-0041 §9.) |
| The passive-segment MUST | Already covered by P1, which fails if a reload overwrites `persist`. A second property would test the mechanism rather than the observable. (ADR-0041 §9.) |
| The admission test | Binds spec authorship, not implementations — §8. A conformance clause here would be a second statement of criterion 4. (ADR-0028 §10.) |
| The display the runner reopens on | Runner-owned state the program cannot see, so there is nothing a program-driven test can observe. (ADR-0040.) |
| Const evaluation, generic integer parameters, view derivation, allocator shape, string representation, removal costs | Compile-time rules whose violation is a compile error chapters 1–4 already mandate. A property would restate a diagnostic. (ADR-0045; ADR-0047; ADR-0042; ADR-0043; ADR-0048; ADR-0050.) |

---

## 6. The reload-latency measurement

**6.1** Latency is a **MUST-MEASURE**, per §1.3. The measured quantity is
**save-to-visible**: file write to the first frame executed from the new build.
(#19 posture 1, §C.)

**6.2** It is reported at **three reference sizes, defined by shape rather than
by a shipped game**, because a single small sample hides the realistic failure —
a rebuild that is fine at 200 lines and useless at 50,000:

| Size | Lines | Modules | `persist` declarations | Other |
|---|---:|---:|---:|---|
| Single file | ~200 | 0 | 1 | — |
| Small game | ~5,000 | ~10 | ~5 | one C library linked |
| Substantial | ~50,000 | ~100 | ~10 | one C library linked |

(#19 §C.)

**6.3** **Cold start** and **whole-project check** are reported at the same three
sizes. The second is P6's number, deliberately kept out of the property. (#19
§C; #22 §4.)

**6.4** The published target is **1 second** for save-to-visible. An
implementation that misses it **MUST publish its number**. (#19 posture 1.)

**6.5 The spec owes this specification; the corpus is the follow-on suite's**,
per §1.4. (#19 §C.)

**6.6 A hard numeric MUST was rejected** — wrong on small hardware,
embarrassing in a decade, and an invitation to game the benchmark. **Shapes
alone were rejected too** — an implementation could satisfy every invariant with
a thirty-second reload, meeting the letter and delivering none of the feeling.
Mandatory measurement with a published target is the form that cannot be hidden
from and does not rot. (#19 posture 1.)

---

## 7. Determinism, and the SIMD non-obligation

**7.1 SIMD lowering is not a conformance obligation.** A conforming
implementation is **not required** to lower operations on the blessed math types
to SIMD instructions. (ADR-0016 §6 clause 2; ch6 §3.9.)

**7.2** A MUST-lower rule was rejected: it makes conformance depend on a
backend's vectoriser, whose failure has **no named entity and no source
location** — *which line failed to vectorise* is not a question a diagnostic can
answer. A MUST-MEASURE was rejected too: SIMD throughput has no single
observable the way save-to-visible does, so the spec would have to freeze a
benchmark corpus. (ADR-0016 §6; #5 criterion 5.)

**7.3 Float evaluation binds every implementation exactly as written.** **No
reassociation, no implicit FMA contraction, no fast-math, in any mode.**
(ADR-0016 §6 clause 3; ch2 §2.5.)

**7.4 §7.3 is a determinism obligation, not a performance one.** A backend that
vectorises with reassociation produces different floats from one that does not,
for identical source — which breaks replay and lockstep, and is a **P8-shaped
divergence** between two builds of the same program. It is the clause that makes
§7.1's freedom safe to grant. (ADR-0016 §6; #19 P8; ch2 §2.5.)

**7.5** What the spec mandates instead of lowering is everything that makes
lowering **possible** and everything that makes it **safe**: alignment declared
on the blessed types (ch6 §3.8), the `[]T` batch shape (ch3 §6.1), and §7.3.
The blessed types are therefore a naming convention plus a layout guarantee plus
an evaluation guarantee; **speed is quality-of-implementation, with the door held
open rather than nailed shut**. (ADR-0016 §6; ch6 §3.8.)

---

## 8. The admission test binds spec authorship

**8.1** The **admission test** governs **only a proposal to add a query that
reads platform or backend state to `$.`**. It binds **whoever next proposes
one** — the authors of this spec — and binds **no implementation**. It adds no
property, because §3.9 already forbids a program-visible surface below `$.`, so
an implementation cannot add such a query to begin with. (ADR-0028 §1, §10.)

**8.2** It is stated here rather than in chapter 6 because it is an obligation on
the spec, in the shape of §3.10, and chapter 6 states the root's contents rather
than the rule for growing it. `extern` is **untouched** by it: that path is
`unsafe` at the call site and visible in the signature. (ADR-0028 §1, §10.)

**8.3 Limb one — reformulate before you refuse.** Before admitting or refusing a
proposed query, restate it: ask whether the motivating case is served by a query
that reports a fact about the **player** or about **program-declared state**,
rather than a fact about the machine. The limb is first because a reformulated
query frequently makes the other two moot, and because a refusal that never
attempted reformulation has examined only the first phrasing. (ADR-0028 §2.)

**8.4 Limb two — the return guard.**

> **Reformulation may change what is asked. It may never change what is
> returned. If the returned value still carries a hardware magnitude,
> reformulation has failed and the proposal drops through to limb three.**

(ADR-0028 §3.)

**8.5 Limb three — strict necessity.** A proposal surviving to limb three ships
**only if some facility this spec itself mandates cannot be computed without
it.** The denominator is **spec-mandated**, not *a plausible game needs it* —
the loose reading readmits nearly everything and is the test every engine
applies on its way to `getWindowSize()`. (ADR-0028 §4.)

**8.6 Environment variance is permitted; implementation variance is not.**

> **Every conforming implementation MUST return the same value in the same
> environment. Only the environment may differ.**

A query two backends answer differently on one machine fails, and fails as a
**criterion 4 Tier 1 veto** — dialect variance — not as an admission-test
finding. This clause is what makes §5.15.2's grid load-bearing. (ADR-0028 §5;
#5 criterion 4.)

**8.7 The admission test is not what keeps a game's world hardware-independent.**
The fixed canvas is. In the graphics domain limb three is *trivially* satisfied,
because display size and DPI have **no expression that consumes them**.
(ADR-0028 §8, as amended; ADR-0032; ch5 §9.)

**8.8** The test's audit of the existing root is ADR-0028 §6's and is not
restated. Its one live hand-off — display count, admitted at limb one and never
given a shape — was **discharged by declining to exercise the admission**.
(ADR-0028 §6, §7; ADR-0040.)

---

## 9. The published counts

**9.1** #24 binds a grammar budget as **a process plus a published, reproducible
count, never as a hard cap**. The process, the counting rules, the comparison
table and the overrule register are **chapter 1 §13's**, computed from
[`grammar.ebnf`](grammar.ebnf), which is where they belong: the count is
reproducible only beside the artifact it is computed from. This section runs the
count **against the finished spec**, which the corpus tracked per decision and
never totalled against a single document. (#24 calls 1, 4, 7; ch1 §13.)

**9.2 The grammar count, totalled.**

| Section | Keywords | Operators | Total |
|---|---:|---:|---:|
| Core grammar | 33 | 41 | **74** |
| Type sublanguage | 5 | 1 | **6** |

Against Lua 5.4's 55, the core grammar stands at **+34.5%**, outside the ~30%
target, under **three** recorded crossings — `>..`, `impl` and `^`. The register
at chapter 1 §13.9.1, not this chapter and not the map, is the authoritative
list, and each crossing names the failure class it deleted. (ch1 §13.6, §13.7,
§13.9.1.)

**9.2.1 The total reconciles.** Every chapter's transcription was checked
against the register and no chapter moved the count without a row: chapters 4, 5
and 7 are zero-delta, chapter 2 carries crossing 2, chapters 1 and 3 carry
crossings 1 and 3, and chapter 6 authored nothing. The published figure is
therefore the whole spec's, not chapter 1's alone. (ch1 §13.9.1; each chapter's
`coverage/` file.)

**9.2.2** Production counts are the machine-checkable companion and are
**sensitive to how the grammar is factored**: 72 core, 23 type sublanguage, 20
lexical. Published with that note so nobody games them by inlining rules. (ch1
§13.2, §13.12.)

**9.2.3 The Odin and Go rows are filled**, closing
[#93](https://github.com/ludo-lang/ludo/issues/93): **Go 73, Odin 99, against
ludo's core 74.** ludo is one token above Go and 25 below Odin. The target is
unaffected — it is stated against Lua and the §9.2.1 overrun is measured against
Lua — but the two rows **bound how that overrun reads**: it lands ludo in Go's
size class, not Odin's. (ch1 §13.7, §13.7.1; #24 call 4.)

**9.3 The stdlib root-name companion count** is published here, with **no
target**, because relocating a feature into the standard library satisfies a
grammar budget by pure relocation and must therefore show up. It is a stdlib
figure, not a grammar figure. (#24 call 5's second consequence; ch1 §13.11.)

**9.3.1 The root names are 31**, of which 5 are facade modules:

| Root name | Kind | Contents |
|---|---|---:|
| `$.graphics` | module | 19 functions, 3 values, 21 types |
| `$.audio` | module | 12 functions, 6 types |
| `$.input` | module | 22 functions, 5 types |
| `$.video` | module | 4 functions, 0 types |
| `$.storage` | module | 2 functions, 0 types |
| `$.vec2` `$.vec3` `$.vec4` | constructors | the `f32` vector set |
| `$.mat2` `$.mat3` `$.mat4` | constructors | the square `f32` matrix set |
| `$.ivec2` `$.ivec3` `$.ivec4` | constructors | the `i32` vector set (ch6 §12.3) |
| `$.rgb8` `$.rgbf` | constructors | both returning `Color` |
| `$.assert` `$.panic` | calls | the two bug-declaring calls (ch6 §12.2) |
| `Vec2` `Vec3` `Vec4` `Mat2` `Mat3` `Mat4` | types | the `f32` blessed set |
| `IVec2` `IVec3` `IVec4` | types | the `i32` blessed set |
| `Radians` `Seconds` `SampleFrames` | types | the shipped quantities |
| `Color` | type | `distinct`, from #28 |

**Functions under the five modules total 59.** (ADR-0033 §9's reconciled base of
55 across four modules, plus ADR-0038's `$.video` at 4; ch6 §4–§8, §3.2, §1.4.)

**9.3.1.1 The figure moved from 13 to 31 when the root was first enumerated**
([#119](https://github.com/ludo-lang/ludo/issues/119); ch6 §12). `+18`: the two
calls chapter 2 §8.5 had placed in the root without a spelling, the three
integer vector constructors ADR-0016 §2 shipped without one, and the thirteen
blessed types ADR-0016 §9 counted as a #24 delta and no chapter had listed.
**No grammar delta accompanies it** — ch6 §12.5.1 — so chapter 1 §13's budget and
its §13.9.1 register are unmoved. This is the count doing the job §9.3 gives it:
a stdlib figure moves and is visible.

**9.3.2 LÖVE2D is the comparator for this count, and not for §9.2's.** It adds
**21 stdlib roots** to LuaJIT for **zero grammar tokens**, which is #24 call 5's
escape route in its purest observed form. A grammar comparison against LÖVE2D is
vacuous — its grammar *is* LuaJIT's exactly — and this is the axis on which the
two are actually comparable. (ch1 §13.11.1; [research/06](../research/06-love-dragonruby-batteries-line.md).)

| | Grammar tokens over its host | Stdlib roots | of which facade areas |
|---|---:|---:|---:|
| LÖVE2D over LuaJIT 2.1 | 0 | 21 | 21 |
| ludo over Lua 5.4 | +19 | 31 | 5 |

**9.3.2.1 The two totals are not like for like, and the fourth column is why.**
Every one of LÖVE2D's 21 roots is a **module** — an area of platform surface.
Only 5 of ludo's 31 are; the other 26 are types, constructors and two calls,
which LÖVE2D's host language does not put at a root because it has no types to
put there. The comparison a reader should run is **5 against 21**, and the
31 is published beside it so that no later chapter has to reconstruct what the
5 excludes. (#24 call 5; ch6 §12.1.)

**9.3.3** The reading the two rows support, and the only one: **ludo bought its
robustness in grammar and not in facade area**, where the closest peer this
language has did the reverse — 21 areas for zero grammar, against 5 areas for
+19 tokens. Neither figure has a target, and this clause states no judgement
beyond what the rows show. The 31-name column supports no such reading in either
direction, because its bulk is the type set a statically typed language must
name somewhere. (#24 call 5; #4.)

**9.3.4 The count is a total, and the root is enumerated at chapter 6 §12.**
This clause previously read *the count is of what is spelled, and the root is not
enumerated anywhere*, and published 13 as a **floor**: `$.assert` and `$.panic`
were placed in the root by chapter 2 §8.5 and absent from chapter 6, and the
integer vector constructors of chapter 6 §3.2 had no spellings. §11.1's gap is
**closed** by [#119](https://github.com/ludo-lang/ludo/issues/119), chapter 6
§12 spells all of them, and the figure above is now a total. The floor is
recorded rather than deleted because ADR-0033 §9 first wrote the absence down
and three documents cite the floor reading. (ADR-0033 §9; ch2 §8.5; ch6 §0.7.1,
§12.)

**9.4 `#explicit` is in neither grammar count**, and chapter 1 §13.4 asserts
that as a checkable property. (#24 call 3; ch1 §13.4.)

---

## 10. What the spec owes, and what the suite owes

**10.1** This spec owes **the assertions**. The follow-on toolchain effort owes
**the harness, the reference corpus of §6.2, and the diagnostic-code
namespace**. (#19 posture 4, §E; ch7 §5.7.)

**10.2 The suite may run on a software renderer**, which makes the full rung
testable in CI on a machine with no GPU. Running a property against an
implementation is permitted and is not the same act as §10.3. (ADR-0037 §6.)

**10.3 A reference implementation may not become a source of promises.** A
software rasterizer is deterministic where a GPU is not, which makes it tempting
to re-promise the pixel exactness this spec withdrew. **Refused.** A reference
implementation is an **artifact**, not a specification, and deriving a promise
from our own rasterizer's behaviour is the same error as a promise wearing a lab
coat — differing only in whether the invented number comes from a measurement or
from a codebase. Chapter 6's texel rule — **which texel, never what byte** —
remains this spec's whole statement of graphics exactness. (ADR-0037 §6;
ADR-0022 §0, §2; ch6 §4.11.)

**10.4 A conformance suite is implied and the map's out-of-scope line stands as
split**: the suite exists because §1.4 makes every property test-shaped; this
spec does not author it. (#19 posture 4.)

---

## 11. Marked gaps

Per ADR-0044 §8, gaps this chapter writes down rather than blocks on. Each is
filed as a ticket that owns the repair of this chapter's text and its
`coverage/` rows in one commit (ADR-0049).

**11.1 The `$.` root is enumerated nowhere, so §9.3's count is a floor —
CLOSED.** Chapter 6 §0.7 stated that it was the reserved root's whole contents
and transcribed the five facade modules and the blessed math set; three things
sat outside that and had no spelling anywhere — `$.assert` and `$.panic`
(ch2 §8.5), the integer vector constructors (ch6 §3.2), and whether a blessed
type is itself a root name (ADR-0016 §9 counted them; no chapter listed them).
ADR-0033 §9 recorded the same absence — *the root itself … is not enumerated
anywhere in the corpus* — and could not fix it, because the fix was chapter 6's
subject and chapter 6's sources did not contain it.

**Closed by [#119](https://github.com/ludo-lang/ludo/issues/119)**, which
authored chapter 6 §12 and moved §9.3.1 from a floor of 13 to a total of 31,
with §9.3.2's comparison re-cut on facade areas and §9.3.4 restated. The interim
obligation this entry carried — *an implementation MUST provide `$.assert` and
`$.panic` with the semantics chapter 2 §8.3 and §8.5 fix* — is discharged by
chapter 6 §12.2, which now carries their signatures and their always-on rule.

Kept, not deleted, because a marked gap is a claim this chapter made about
itself and a reader arriving from ADR-0033 §9 or from an older §9.3.4 must be
able to see that it was closed rather than wonder which text is stale
(ADR-0049).

---

## 12. What this chapter does not decide

Recorded so the boundary is legible, and so a later reader does not treat a
silence here as an inheritance.

- **Any rule of the language, the runner or the standard library.** This chapter
  states obligations *about* chapters 1–7 and restates none of them (§0.4).
- **The concrete conformance suite** — the harness, the assertions' encoding,
  the reference corpus of §6.2, the test runner. The follow-on toolchain
  effort's (§10.1; #19 §E).
- **The diagnostic envelope, codes and severity** that P5 is failed on. Chapter
  7's (ADR-0018; ADR-0023).
- **The concrete diagnostic-code strings**, including §2.8's shader-rung error.
  Chapter 7 §5.7's. The **shape** that separates a spec code from a vendor one
  is chapter 7 §5.6.1–§5.6.5's — a code is `<owner>:<local>` and the owner
  `ludo` is reserved to the spec
  ([#117](https://github.com/ludo-lang/ludo/issues/117), resolved).
- **Which graphics APIs exist or are supported.** §3.5 makes that a category
  error: there is no set (ADR-0037 §1).
- **Whether a given implementation conforms.** This chapter states the
  obligations; §3.4 ranks the artifacts a reviewer applies, and §1.5 fixes which
  wins.
- **Acquisition, registries, versioning and lockfiles**; **the compiler's own
  implementation**; **the build system and package manager**. Permanently out of
  scope for this spec (ch4 §14.1; ADR-0027 §9; the map's *Out of scope*).
