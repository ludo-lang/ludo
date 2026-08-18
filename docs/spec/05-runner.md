# Chapter 5 — The program and the runner

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where this chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under chapter 7's envelope (ADR-0018).

**0.3** This chapter owns **what a program is at run time and what the runner
owes it**: the two modes, the entry file and its top level, the frame entry and
its signature, the fixed simulation rate and the presentation clause, `persist`
and reload, faults, quiescence, declared assets, the fixed canvas and the fit,
and the window and display the runner opens. [Chapter 1](01-grammar.md) owns the
spelling of every form named here, and [`grammar.ebnf`](grammar.ebnf) remains
the authority on syntax.

**0.4** This chapter does not own the **type system** (chapter 2), **memory,
views and the allocator's shape** (chapter 3), **modules, `use` and name
resolution** (chapter 4), the **names under `$.`** (chapter 6, whose boundary is
every such name), the **diagnostic envelope and its codes** (chapter 7), or
**conformance and the property list** (chapter 8). Where a clause reaches one of
them, it names the routing.

**0.5** Every clause cites its source. A clause with no citation is a defect.

**0.6 Two `$.graphics` names are spelled here, by routing.** Chapter 6's
boundary is every name under `$.`, and it routed `set_canvas` and `canvas_size`
to this chapter with §4.2.7, because both are meaningless without the fixed
canvas and the fit that §9 fixes. They are the only `$.`-rooted spellings this
chapter authors. (ADR-0032 §5, §6; ch6 §4.2.7.)

**0.7 The one contradiction this chapter inherited is resolved at §4.3.** #9
requires every binding to initialise at its declaration; ADR-0045 §1 and §8
write a `persist` declaration with no initialiser. Chapter 1 §14.5 recorded the
tension and deferred it here. §4.3 requires the initialiser, and the repairs
that decision owes — chapter 1 §5.13 and §14.5, chapter 2 §9.6 and §17.6, both
`coverage/` files, ADR-0045's stamp and the reference program — land in this
chapter's commit, per ADR-0049.

---

## 1. The two modes

**1.1** A ludo program is **compiled to a native artifact and runs standalone**.
It is never a guest scripted into a host, and there is no bundled engine. (#12
call 1.)

**1.2** The toolchain ships a **runner**, and the **bare command is the
runner**: `ludo main.ludo` runs the program. An implementation MUST NOT require
a `run` subcommand, a scaffolding step, or any out-of-source project file to run
a single file. (#12 call 2; #3's *a program is a file you run*.)

**1.3** **`ludo build` drops the runner** and emits a shipping artifact. (#12
call 2.)

**1.4** The two modes are called **dev** (the runner owns the process) and
**release** (the artifact runs alone) throughout this chapter.

**1.5** **The two modes are two modes of one language.** No semantics differ
between them. Where this chapter states a mode-dependent behaviour, it is a
property of *the runner's liveness*, never of the program's meaning — §5.1,
§6.3.2 and §6.3.3 are the whole of that list. (#12 call 2; #5 criterion 4.)

**1.6** In **release** the artifact is a single statically linked binary on a
target that has such a thing. (#12 call 2; #17 §1.)

**1.7** A **hosted target need not be a process.** Where this chapter names an
observable that a process supplies — an exit status, a moment of exit, a
readable call stack, memory outside the swapped code — the clause is stated in
terms every host can satisfy, and the browser's convention is fixed rather than
left open. (ADR-0041 §1; §5.2, §5.9, §6.3.3, §6.5.2.)

---

## 2. The entry file and its top level

**2.1** A program has exactly one **entry file**: the file named to the runner
or the build. (#12 call 2; ch4 §13.6.)

**2.2** **The entry file's top level executes**, in source order, and it is the
only place statements run outside a frame entry. (#26 call 1.)

**2.3** **An imported module's top level never executes.** That rule and its
consequences are chapter 4's (ch4 §13.1–§13.3). A statement — as opposed to a
declaration — at the top level of an imported module is a **compile error**
naming the module and the statement's location. (#26 call 2; ch4 §13.6, which
routes the diagnostic here.)

**2.4 The startup order is fixed:**

1. Every **declared asset** is resolved and decoded (§8.3).
2. The **entry file's top level** runs to completion.
3. `persist` **initialisers evaluate at their declaration site**, in source
   order with the surrounding top-level statements, on a **cold start only**
   (§4.4).
4. If a **frame entry is declared**, the runner begins calling it. If none is
   declared, the program **exits normally**.

(#26's ordering rule; ADR-0015 §3 for step 1.)

**2.5** **A file with no frame entry is a complete program.** It runs and exits.
Adding one frame declaration upgrades it into a game; this MUST NOT require any
other edit. (#26 call 1; #6's monotonic addition.)

**2.6** **Top-level bindings are immutable constants from a frame entry's
view.** A frame entry MAY read them and MUST NOT mutate them. General top-level
mutable state does not exist; `persist` is the only mutable cross-frame channel
(§4.1). (#26 call 3.)

**2.7** **Top-level code cannot draw.** No `Target` exists at top level, because
the target exists only as a parameter of the frame entry (§3.4). A program that
renders one image and exits is a frame entry that exits, not a top-level
statement. (ADR-0013 §7.)

**2.8** The smallest useful program is one statement with **no declarations**,
no `main`, no setup callback and no loop concept present. An implementation MUST
NOT require more. (#26 call 1; #3.)

---

## 3. The frame entry

**3.1** The per-frame entry is a **marked declaration, never a magic name**. An
implementation MUST NOT resolve the entry by looking up an identifier such as
`update` or `tick`. Its spelling is chapter 1 §5.11. (#12 call 4.)

**3.2** **At most one frame entry may be declared in a program.** A second
declaration is a **compile error**. (#12 call 4, which requires a duplicate
entry to be an error.)

**3.3** **The frame entry may be declared only in the entry file.** It is the
unit of work the runner drives, and §2.3 makes an imported module's top level
declarations-only with nothing that runs. (#12 call 4; #26 call 2.)

**3.4 The entry's parameter list is exactly `screen: !Target, scratch:
!Scratch`,** in that order, with both `!` marks required. Any other parameter
list is a **compile error**. This fixes chapter 1 §5.11's admitted list.
(ADR-0013 §1, §2, as amended by ADR-0042 §6.)

**3.5 The rule that governs the list:** *the entry's parameters are exactly the
values only the runner can supply.* The list grows only when the runner acquires
a new exclusive property, which is a decision of its own. (ADR-0013 §3; ADR-0042
§6, which adds a member and does not amend the rule.)

**3.6** **A fetch is not available.** There is no call that returns the screen
target, and an implementation MUST NOT provide one. (ADR-0013 §1; ADR-0009, *the
target arrives at the frame entry*; ch6 §4.2.6.)

**3.7** **A `Target` MUST NOT be held in `persist` or in a top-level binding.**
The runner owns the window, and a stored screen target would put a value the
user never authored into the reload set. (ADR-0013 §1.)

**3.8 The target is fresh each frame, with its transform reset to identity.** A
transform set during one frame MUST NOT survive into the next. (ADR-0013 §6.)

**3.9** A camera that persists across frames is therefore a `persist` value the
entry applies to the target each frame. (ADR-0013 §6.)

**3.10** `scratch` is the **per-frame arena**, and its lifetime is exactly one
entry call. The runner resets it at the frame boundary; the program MUST NOT
reset it, and there is no call that does. Chapter 3 owns the `Scratch` type and
what may be allocated from it. (ADR-0042 §6; ch3 §9.9.)

**3.11 The entry takes no time parameter.** There is no `dt`, and there is no
ambient simulation clock. (#26, *the frame entry takes no parameters*.)

---

## 4. `persist`

### 4.1 What it is

**4.1.1** **`persist` is the only mutable cross-frame channel.** Every piece of
mutable state that outlives a frame entry is a `persist` declaration. (#26 call
3.)

**4.1.2** **The reload set is exactly the `persist` declarations of the entry
file**, with nothing else to discover (§5.3). (#17 §2.)

**4.1.3** `persist` means the same thing in release: **state that lives across
frames, allocated once, its initialiser run at startup**. Reload is the runner
doing something extra with that state, not a second meaning, and a beginner need
not be told about reload to use `persist`. (#17 §9.)

### 4.2 Where it may appear

**4.2.1** **A `persist` declaration may appear only at the top level of the
entry file.** A `persist` declaration in an imported module is a **compile
error**, and so is one in a function body. (#26's amendment from #17; #17 §2;
ch4 §13.4.)

**4.2.2** Multiple `persist` declarations in the entry file are legal and carry
no ordering rule beyond §2.4's source order. (#17 §2.)

**4.2.3** Consequently **a library cannot own private cross-frame state**; it
exports a type and an initialising function, and the consumer declares the state
and passes it. (#17 §2; ch4 §13.5.)

**4.2.4** Aggregating every `persist` value under one named root is a
**documented idiom and not a language rule**. Nothing distinguishes it from N
flat declarations, and an implementation MUST NOT prefer either shape. (#17 §2.)

### 4.3 The initialiser is required

**4.3.1** **A `persist` declaration MUST carry an initialiser.** `persist
<name>: <Type> = <expr>` is the whole of the form; a declaration that omits the
initialiser is a **compile error** naming the declaration. This narrows chapter
1 §5.13's permissive grammar, which was written so that this chapter could
decide either way without a grammar change. (#9, as transcribed at ch2 §9.6; #26
call 4; ch1 §5.13, §14.5.)

**4.3.2 The corpus contradiction this resolves.** #9 requires every binding to
initialise at its declaration and states that uninitialised memory does not
exist in the safe layer. ADR-0045 §1 and §8 write `persist name: TextBuf[32]`
with no initialiser. The tension is resolved **for #9**, on three grounds that
the omission has nothing to set against them:

- ADR-0045 **argues for the integer generic parameter and never for the
  omission**. Its §8 line exists to display `TextBuf[32]`, and a `persist`
  declaration is the position ADR-0043 §5 had already put that type in.
- **#17 §8's reload machinery requires an initialiser to exist.** On a reload
  where a declaration's type changed, the runner evaluates *that declaration's
  own initialiser* in the new build to supply the fresh value for every new,
  renamed or type-changed field path. A declaration with no initialiser has no
  source of those values, and #9's rejection of zero values (ch2 §9.3) leaves
  nothing to substitute — so the exemption would silently disable the field-path
  reset for exactly the declarations that took it.
- **ADR-0048 §7 presumes the initialiser.** Its rule — allocation failure in a
  `persist` initialiser is a #18 fault — is stated over a position that an
  exemption would make optional.

(#9; ADR-0045 §1, §8; #17 §8; ADR-0048 §7.)

**4.3.3 ADR-0045 §8's reference-program line is amended to carry an
initialiser**, and the spelling is `= {}`: the mandated `TextBuf[N: usize]`
declares a default for both of its fields, so the empty literal names every
field that has no default, of which there are none. This is not an implicit
zero — the default is written in the type's declaration, which is exactly the
distinction chapter 2 §9.6 draws. The decision ADR-0045 makes is unaffected.
(ADR-0045 §8; ch3 §5.7; ch2 §9.6, §17.6.)

**4.3.4** An initialiser is an ordinary expression and **may be fallible**. What
happens when it fails is §4.6.

### 4.4 When the initialiser runs

**4.4.1** **A `persist` initialiser evaluates on a cold start only.** It is
skipped on every reload. (#26 call 4.)

**4.4.2** It evaluates **at the declaration's site**, in the top level's source
order (§2.4 step 3). (#26 call 4.)

**4.4.3** The one exception is **drift**: on a reload where that declaration's
type changed, the initialiser is re-evaluated in the new build to supply
defaults (§5.7). A declaration whose type did not change runs no initialiser on
any reload. (#17 §8.)

**4.4.4** A consequence, which is the general rule and not a special case: after
a reload a `persist` value still holds what it held before, including text —
because initialisers do not re-run. (#17 §4.)

### 4.5 Reload-safety

**4.5.1** **The compiler computes reload-safety transitively over every
`persist` declaration, and rejects a declaration that can reach a pointer into
the reloadable image.** The error names the offending field path. (#17 §3.)

**4.5.2** This is a **diagnostic, not a type property**. The user never writes
it, it never appears in a signature, it never propagates into a generic
constraint, and it never reaches the oracle path. (#17 §3.)

**4.5.3** **A `persist` declaration may not reach a function value.** (#17 §5.)

**4.5.4** An implementation MUST NOT re-resolve a stored function value by name
on reload. (#17 §5.)

**4.5.5** **The check is on in release builds too.** Release has no swap and
nothing to protect, and the check applying in one mode and not the other would
be dialect variance and would let an agent write shipping code that cannot be
reloaded. (#17 §3; #5 criterion 4.)

**4.5.6** **`unsafe` lifts the check, per declaration.** An `unsafe`-marked
`persist` declaration is exempt from §4.5.1. The downgrade is per declaration,
never per program, and there is **no second, narrower spelling** meaning *this
points at the heap*. (#17 §6.)

**4.5.7** The **mandated allocator implementations are runner-owned**, so their
code is not in the reloadable image and a `persist` container backed by one
passes §4.5.1 unchanged. **A `persist` container backed by a user-written
allocator is rejected** by that same check, and §4.5.6 lifts it for an author
who owns the consequence. In release the predicate is vacuous and still passes.
(ADR-0042 §7; ch3 §9.10.)

**4.5.8** **`heap` is legal only in a `persist` initialiser, in the entry
file**, and nowhere else. Chapter 3 owns the allocator; this clause is the
position rule. (ADR-0042 §8; ch3 §1.6, §9.12.)

**4.5.9** **Program constants are reload-stable.** The compiler emits literal
data into a blob the **runner** holds rather than into the swapped code, and
blobs from earlier builds stay alive for the session, so a view into constant
data stays valid across any number of reloads. (#17 §4; ADR-0043 §1, which
narrows what such a view's type is.)

### 4.6 Allocation failure in a `persist` initialiser

**4.6.1** **Allocation failure in a `persist` initialiser is a §6 fault** of
kind **allocation failure** (§6.5.1, item 7), and the fault report names the
declaration — its name and its type. It is not a `rescue`-able error at that
position. (ADR-0048 §7; [#113](https://github.com/ludo-lang/ludo/issues/113).)

**4.6.2** The carve-out is **by position, not by call**: everywhere else, an
allocating call is fallible and its failure is a value the call site handles.
Chapter 3 owns that rule. (ADR-0048 §6, §7; ch3 §11.10.)

**4.6.3** A `rescue <fallback>` at this position would compile and would start
the game against an empty world, which is silent wrong behaviour under familiar
notation. An implementation MUST NOT accept the declaration on that basis.
(ADR-0048 §7.)

---

## 5. Reload

**5.1 Mechanism.** In **dev** the compiler emits a **shared library** rather
than a program; the runner owns the process, loads the library and drives the
frame entry. On a save it builds a new library, unloads the old one, loads the
new one and calls the next frame entry. In **release** there is no runner, no
library and no swap. (#17 §1.)

**5.2 Where `persist` lives.** `persist` lives in **runner-controlled memory
that outlives the swapped code**. On a native target the runner holds it outside
the swapped library; on a hosted target where the program's own linear memory is
the only addressable memory, the runner holds that memory and instantiates each
build against it. (ADR-0041 §5, correcting #17 §1.)

**5.3** **The state a reload preserves is exactly the `persist` declarations not
marked `unsafe`** (§4.1.2, §4.5.6). (#17 §2, §6.)

**5.4 A reload happens between two frame entry calls, never mid-frame.** The
runner finishes the current entry, swaps, and calls the next one. (#17 §7; §7.2
is the predicate that makes the boundary safe.)

**5.5 A reload re-runs the entry file's top level**, skipping `persist`
initialisers (§4.4.1). Top-level bindings are recomputed from the new code,
which is what makes editing a top-level constant take effect. **Top-level side
effects therefore happen again on every reload.** (#17 §9.)

**5.6 A build that fails to compile never touches `persist`**, and the program
keeps running the old code. (#17 §9.)

**5.7 Type drift is resolved by field path.** Where a `persist` declaration's
type changed between builds, the runner evaluates that declaration's initialiser
in the new build to produce a fresh value, then walks the old and new types
together, field path by field path:

- name **and** structural hash both match → **the old memory is copied**;
- new, renamed, or type-changed → **the fresh value is kept**;
- the walk **recurses through nested structs**;
- a **pool or array whose element type changed is taken fresh, whole**;
- a declaration whose type **did not change** is untouched, and no initialiser
  runs for it.

(#17 §8.)

**5.8** Two consequences of §5.7 are stated rather than left to be discovered:
matching is by name plus structural hash, so **a renamed field reads as
delete-plus-add** and takes the fresh value; and the initialiser re-runs on a
drift reload, allocating a fresh value of which only the changed parts are kept,
which is a **dev-only leak bounded by how often a type is edited**. (#17 §8.)

**5.9 A build whose initialised data would land over the live `persist` region
is non-conforming.** A conforming implementation's reload path **MUST NOT** place
a new build's initialised data over that region. On a host that copies a module's
active data segments into imported memory before any code runs, the discharge is
to place that data under the build's own control after instantiation instead.
(ADR-0041 §6.)

**5.10 Declared assets reload too**, at the same boundary; §8.6 states it.

**5.11** An implementation MUST NOT ship **different `persist` structure in dev
and in release**. (#17 §8.)

---

## 6. Faults

### 6.1 What faults

**6.1.1** A **fault is a detected bug, and nothing else**. The bug class is
chapter 2's: out-of-bounds, a failed `$.assert` or `$.panic`, integer overflow,
integer divide-by-zero, and `unsafe` use-after-free — but see §6.1.8, which is
not reportable. (#18 §1; ch2 §8.3, the list; ch2 §8.6 routes what happens next.)

**6.1.2** **An expected failure is never a fault.** Errors are values in the
return type and must-use makes ignoring one a compile error, so no unhandled
error value can reach the runtime. There is **no unwinding**, no handler stack,
and no `catch` frame. (#18 §1; #10.)

**6.1.3** Not faults: a **stale handle**, which reads as absence; and **NaN or
infinity**, which are ordinary values. (#18 §1.)

**6.1.4** **Asset resolution failure is a spec-named fault kind** (§8.4).
(ADR-0015 §4.)

**6.1.5** **Allocation failure in a `persist` initialiser is a fault** (§4.6.1).
(ADR-0048 §7.)

**6.1.6** §6.1.4 and §6.1.5 each add a kind to a list #18 §8 published as
**closed**, and only the first says so. **The enum is still closed, and its
membership is now seven** — #18 §8's five, plus asset resolution failure and
allocation failure. **§6.5.1 is the authoritative list**; #18 §8's five are a
superseded prefix of it. An implementation MUST report each of the two added
kinds as itself, and MUST NOT report either under one of #18 §8's five.
(#18 §8; ADR-0015 §4; ADR-0048 §7;
[#113](https://github.com/ludo-lang/ludo/issues/113).)

**6.1.7 Closed means closed against this document, not against an issue.** A
kind is added only by an edit to §6.5.1, and per
[ADR-0051](https://github.com/ludo-lang/ludo/blob/main/docs/adr/0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md)
§3 that edit is a spec change with an ADR behind it. This is what the two
undeclared extensions cost: #18 §8's list lived on an **issue**, and an
amendment whose target is an issue cannot be stamped, so the corpus recorded
neither extension. Relocating the list here is the repair, not a weakening of
the closure. ([#113](https://github.com/ludo-lang/ludo/issues/113).)

**6.1.8 A use-after-free inside `unsafe` is not a fault kind.** Chapter 2 §8.3
lists it in the bug class this section's first paragraph cites, but §6.2.5 voids
every guarantee for it: it is a bug that is **not detected**, so no conforming
report can name it and it is absent from §6.5.1 by construction. The enum is the
list of **detected** bugs, and that is why it is exhaustively handleable.
(#18 §3; ch2 §8.3; §6.2.5.)

### 6.2 What a fault does

**6.2.1** **A fault ends the simulation.** The faulting frame is abandoned —
the rest of it never runs — and **no further frame entry is called**. (#18 §2,
§4.)

**6.2.2** An implementation MUST NOT resume at the faulting statement, and MUST
NOT unwind to the caller. (#18 §2.)

**6.2.3** **`persist` is left exactly where the broken frame stopped.** There is
no rollback and no snapshot, and an implementation MUST NOT mark the state
suspect or ask the user about it. The world may be logically inconsistent; it is
never memory-corrupt. (#18 §5.)

**6.2.4** **Every runtime-detected bug is pausable, and nothing else is.** The
line is the boundary of detection, not a judgement about severity, and it is the
same boundary `unsafe` draws. (#18 §3.)

**6.2.5** Undefined behaviour reached through `unsafe` is **not a fault at all**,
and no contract is offered about it. **The guarantees of this section are void
for a program using `unsafe`.** (#18 §3.)

**6.2.6 A fault before any frame runs.** Top level can fault — on cold start, or
on §5.5's re-run during a reload. There is no frame to abandon and **the
simulation never starts**. On a reload re-run the previous `persist` is
untouched, because the failed top level never reached it. (#18 §10.)

### 6.3 What the process does next

**6.3.1** **Fault semantics are identical in dev and release.** The only
difference is the runner's liveness. (#18 §4; §1.5.)

**6.3.2** In **dev** the runner keeps the process alive so state can be read and
a fix loaded. Loading a fix is a **reload** (§5), carrying `persist` across, and
the frame entry starts again from the top. Nothing resumes the old run. (#18 §2,
§4.)

**6.3.3** In **release** the program **terminates abnormally by the host's
convention**. On a hosted target with no exit status, the convention is fixed
rather than left open: **the trap surfaces as an exception through the host and
the frame entry does not return.** (#18 §4, as restated by ADR-0041 §3; ADR-0018
§11, the third site of the same claim.)

**6.3.4** A faulted program is **quiescent forever** (§7.4), which is what lets
a reload rescue it. On a host that may reclaim a hidden page without running
code or firing an event, the rescue is a dev-loop affordance the host may take
away first; the predicate is unaffected. (ADR-0024 §3; ADR-0041 §8.)

### 6.4 The paused runner's surface

**6.4.1** **The runner offers named operations, never evaluation.** There is no
console that runs program expressions, and an implementation MUST NOT provide
one. (#18 §6.)

**6.4.2** The operations are: **read a `persist` value**; **reset one `persist`
declaration**, using §5.7's field-path machinery with a manual trigger;
**cold-restart**; and **edit the source and reload**. (#18 §6.)

**6.4.3** **`persist` is readable recursively, to arbitrary depth, by a
structural walk over known layout that runs no program code.** The runner holds
the build's type information; the walk needs no cooperation from the program.
(#18 §7.)

**6.4.4** **The dump executes no user code** — no formatting hook, no
`to_string`. (#18 §7.)

**6.4.5** Three exclusions: an **`unsafe` `persist` declaration** renders as an
address and stops; a **pool** is walked by its active set, so a released slot
renders as free and never as stale bytes; and **depth and size limits are a
runner concern**, not a semantic one. (#18 §7.)

**6.4.6** Chapter 8's conformance surface, not this chapter, decides what a
harness may assert about the presentation of a dump. (#18 §7, which routes it.)

### 6.5 The fault report

**6.5.1** A fault report MUST carry:

- the **fault kind**, drawn from a **closed enum fixed by the spec** — closed so
  that an agent can handle it exhaustively and a new kind is a spec change
  rather than a surprise. **The membership is this list, and it is seven**:

  1. **out-of-bounds** — an index outside a container's bounds;
  2. **assert** — a failed `$.assert` (ch2 §8.5);
  3. **overflow** — a checked integer overflow (ch2 §8.3);
  4. **divide-by-zero** — an integer divide by zero;
  5. **explicit panic** — a `$.panic` (ch2 §8.5);
  6. **asset resolution failure** — a declared asset that cannot be resolved or
     decoded at startup, carrying **the declaration's location and the path it
     tried** (§6.1.4, §8.4; ADR-0015 §4);
  7. **allocation failure** — an allocating call in a `persist` initialiser
     whose allocation fails, carrying **the name and type of the declaration**
     (§6.1.5, §4.6.1; ADR-0048 §7). It is the **only** position that raises this
     kind: everywhere else allocation failure is a value the call site handles,
     never a fault (ch3 §11.10; ADR-0042 §3 as narrowed by ADR-0048 §7). A
     report carrying it therefore always names a `persist` declaration of the
     entry file, which is what lets an agent locate it without seeing the
     runner;

  The **concrete identifier strings** are no more this chapter's than the
  diagnostic code strings are chapter 7's (ch7 §5.7); what is fixed here is the
  membership and what each member carries;
- the **source location** of the faulting operation;
- **the concrete values that made it a fault** — index *and* length; both
  operands *and* the operation, for overflow; the asserted expression's source
  text *and* its message. A bare *index out of range* is non-conforming;
- the **call chain** to the frame entry, one source location per level;
- the **frame number**;
- the **names and types of the entry file's `persist` declarations**.

(#18 §8.)

**6.5.2** **The call chain is guaranteed in both modes.** An implementation MUST
NOT emit a chain in dev only. The price is a native price on a native target
(binary size, and the most aggressive frame-pointer omission foreclosed) and a
**per-call runtime price** on a target with no instruction that reads its own
call stack, where the discharge is a shadow stack in release as well as dev.
Neither price buys the guarantee back. (#18 §9, as amended by ADR-0041 §4.)

**6.5.3** The report's **contents** are this chapter's. Its **envelope, codes
and sink** are chapter 7's, and whether a machine-readable format is obliged is
the experience contract's. (#18 §8; ADR-0018.)

---

## 7. Concurrency and quiescence

**7.1** **There is no concurrency surface**: no threads, no job system, no
coroutines, no `async`/`await`, and no data-parallel call over a function-pointer
body. Concurrency is a **stated non-goal**. (ADR-0024 §1.)

**7.2 Quiescence is a state predicate:**

> **A program is quiescent when no ludo code is on any stack, no `extern` call is
> in flight, and the only live ludo state is `persist` plus declared assets** —
> plus the resolved contents of any declared storage slots, which resolve once
> per process.

(ADR-0024 §3, as amended by ADR-0026 §8; ch6 §8.15, which routes the predicate
here.)

**7.3 Where the predicate holds** is between two invocations of the frame entry,
and also **after top level and before the first frame**, which §2.4 makes a
quiescent point with no frame on either side of it. The location is where; the
predicate is why it is safe, and only the predicate tells an implementer what to
check. (ADR-0024 §3.)

**7.4** **A faulted program is quiescent forever**, since no ludo code will run
again. This is a theorem of §7.2, not a special case. (ADR-0024 §3.)

**7.5** Three mechanisms cite one definition: the reload swap (§5.4), the asset
byte-swap behind a handle (§8.6), and the backend re-point (ch4). (ADR-0024 §3.)

**7.6** **A multi-frame sequence is a `persist` state machine**, since there is
no coroutine to suspend. (ADR-0024 §2.)

**7.7** **An implementation MAY use threads; the program MUST NOT be able to
observe them.** The observable behaviour MUST be as if single-threaded. A
backend may run a device thread, a decode thread or a compile thread. (ADR-0024
§4.)

**7.8** **Ludo code may only be entered from the frame task.** A C thread MUST
NOT call in, and neither may any other host-scheduled callback — an event
listener, a promise reaction, a storage completion handler, or a display
callback the runner did not schedule. (ADR-0024 §5, as widened by ADR-0041 §7,
which makes this a rule rather than a repair.)

**7.9** `async`, `await`, `go`, `spawn`, `coroutine` and `yield` are **ordinary
identifiers** and MUST NOT be reserved. (ADR-0024 §7; ch1 §2.3.)

---

## 8. Declared assets

**8.1** **An asset is a declaration whose name is fixed at compile time and
whose bytes the runner resolves.** Its spelling is chapter 1 §5.14's `extern
<class> <name> = "<path>"`. There is no load call, and an implementation MUST
NOT provide one. (ADR-0015 §1.)

**8.2** **The path is relative to the source file that declares it**, and **an
asset path MUST NOT cross a library boundary**. A declaration MAY appear in any
module, not only the entry file, because a declared asset is immutable content
and #17's argument for confining `persist` does not transfer. (ADR-0015 §1;
ADR-0014 §9; ch4 §3.6.)

**8.3** **Every declared asset is resolved and decoded before top-level code
runs** (§2.4 step 1). The program therefore **never observes a loading state**:
there is no optional asset, no in-flight handle, no readiness poll and no
progress value. (ADR-0015 §3.)

**8.4** **A declaration that cannot be resolved or decoded at startup is a
fault** at top level, under §6.2.6: the simulation never starts and the prior
`persist` is untouched. Asset resolution failure is a **spec-named fault kind**,
carrying the declaration's location and the path it tried. An implementation
MUST NOT substitute a placeholder and carry on. (ADR-0015 §4.)

**8.5** One mechanism serves **every content class**, differing only in the
declared type. (ADR-0015 §2.)

**8.6 Editing a declared asset's file updates it at the next frame boundary**,
without restarting the process and without resetting `persist`. The runner swaps
the bytes behind the handle at §7.2's quiescent boundary. This is mandated, not
optional. (ADR-0015 §8.)

**8.7** **A failed re-decode retains the last-good asset, reports an error, and
does not fault.** The asymmetry with §8.4 is deliberate: missing at startup is a
program that cannot run, and broken at reload is a program already running fine.
(ADR-0015 §8.)

**8.8** The declared set is **runner-owned memory the program never names**, and
it outlives a reload the way §4.5.9's constant blob does. (ADR-0015 §5.)

**8.9** The **decode-from-bytes calls** that cover content the program produces,
and the mandated debug value a call site may `rescue` to, are names under `$.`
and are chapter 6's. What is stated here is that neither takes a path.
(ADR-0015 §5, §6; ch6 §4.9.2.)

---

## 9. The canvas and the fit

**9.1 The logical canvas is fixed at 1280×720** for every program. It is not
declared, not configurable and not defaulted: it is a constant of the language.
(ADR-0032, overturning ADR-0005's caller-declared canvas.)

**9.2** **`$.graphics.canvas_size` is a constant**, not a call, so that no
program types `1280`. It is folded at compile time and reads no backend state.
(ADR-0032 §6; ch6 §4.2.7, which routes the spelling here.)

**9.3** **`$.graphics.set_canvas(<style>)` is a top-level statement** that takes
the style token directly; there is no descriptor type and there is no size
argument. (ADR-0013 §5, as amended by ADR-0032 §5; ch6 §4.2.7.)

**9.4** **The style token is immutable for the process's life.** An
implementation MUST reject a `set_canvas` call anywhere but the entry file's top
level, and MUST reject a second call. A canvas property that can differ between
two frames would make every guarantee in this section conditional on the program
not having changed it. (ADR-0013 §5.)

**9.5 The fit is a uniform scale followed by a translation, and nothing else.**
Given the canvas `(W, H) = (1280, 720)` and a window of `(w, h)` device pixels:

> `k = min(w/W, h/H)`. The canvas image occupies a rect of `(kW, kH)` device
> pixels, centred at `origin = (round((w − kW)/2), round((h − kH)/2))`. **The
> same `k` applies on both axes.**

(ADR-0030 §4 as restated by ADR-0031 §2; ADR-0032 §4. Chapter 6 §7.3 restates
this arithmetic where the render scale composes with it.)

**9.6** **Aspect ratio is preserved.** No conforming implementation may scale the
axes independently, shear, or stretch to fill. (ADR-0030 §4.)

**9.7** **The refusal binds the presentation, whoever asked for it.** There is no
program-facing fit field and **no player preference for a non-uniform fit**; a
runner MUST NOT present a stretched or a cropped image through its own
affordance either. A future proposal for any non-similarity presentation
transform argues from scratch. (ADR-0039 §1, §6, §7; ADR-0030 §9.)

**9.8** `k` is **real, with no branch on the style token, no floor and no
special case for a window smaller than the canvas.** A window or a display
smaller than 1280×720 downscales; there is no minimum window size and no refusal
to launch. (ADR-0031 §2; ADR-0036 §6.)

**9.9** **`crisp` is nearest sampling at every scale**, integer or not. A style
token MUST NOT mean different things on different monitors. The stated cost is
that at `k < 1` a `crisp` target drops texels rather than blurring them.
(ADR-0031 §3; ADR-0036 §6.)

**9.10** **The device pixels outside the canvas rect are the bars.** They are
**opaque black**, spec-fixed and not implementation-defined. (ADR-0030 §4, §6.)

**9.11** **Rendering is clipped to the canvas rect.** No drawing call may affect
a device pixel outside it, so the bars are never program-reachable. (ADR-0030
§6.)

**9.12** A program whose play area is not 16:9 **letterboxes itself inside the
canvas**, and those regions are ordinary canvas it may draw into — unlike §9.10's
bars, which are outside the canvas. (ADR-0032 §3.)

**9.13** Under fullscreen the window is the display and §9.5 is unchanged. That
is the whole of the interaction. (ADR-0030 §4.)

**9.14** **The canvas is a coordinate mapping, not an intermediate
framebuffer.** Content rasterises at native device resolution. At a render scale
of 1 an implementation MUST NOT require an intermediate target; the render scale
is the one optional, player-invoked exception, and it is chapter 6's. (ADR-0030
§3, as scoped by ADR-0038; ch6 §7.)

**9.15** **The real window size is not exposed**, and neither is a display size,
a density or a display count. There is no setter either. (ADR-0013 §9; ADR-0036
§7.)

**9.16** The **coordinate-mapping calls** that invert this transform for a
pointer position, and their module, are chapter 6's; their semantics are §9.5's
transform and its inverse. (ADR-0013 §8; ADR-0030 §4; ch6 §4.2.8.)

---

## 10. Frame rate and presentation

**10.1** **The simulation runs at a fixed 60Hz.** The step is a compile-time
constant. (#26.)

**10.2** **The render rate equals the simulation rate, permanently.** One frame
entry call is one simulated step and one presented image. There is **no second
entry, no `alpha` parameter and no interpolation**. (ADR-0035 §1.)

**10.3 Exactly one image is presented per returned frame entry.** The runner
MUST NOT present a frame twice, and MUST NOT skip presenting a frame that was
entered. (ADR-0035 §3.)

**10.4** **A frame that overruns its budget runs late and the simulation slows
down.** An implementation MUST NOT run the entry twice to close a gap. Composed
with §10.3, an overrunning frame yields exactly one entry call and exactly one
late image. (#26; ADR-0035 §6.)

**10.5** **Pacing is runner discretion.** How the runner waits — vsync on or
off, how it sleeps, whether it free-runs — is not specified, and an
implementation MUST NOT be required to enable vsync. *How many images the player
sees per simulated step* is not discretion, and §10.3 fixes it. (ADR-0035 §4.)

**10.6** **There is no ambient simulation clock.** Elapsed time and frame count
are a `persist` counter the program keeps, which is visible, greppable and
automatically correct across a reload. Wall-clock time is genuine ambient I/O
and is a name under `$.` (chapter 6). (#26.)

**10.7** **`frame` has one referent.** The entry call, the simulated step and
the presented image are one thing, definitionally. (ADR-0035 §9.)

---

## 11. The window and the display

**11.1** Everything in this section binds **the runner**. None of it is
program-visible, and the program has no getter and no setter for any of it,
except fullscreen (§11.9). (ADR-0036 §7; ADR-0040 §5.)

**11.2 First launch is windowed.** Every program opens in a window the first
time it runs on a given machine, and the developer gets no say. (ADR-0025 §7.)

**11.3 The window opens maximal, at the canvas aspect.** On that first launch
the runner opens a window of **the largest size with the canvas's 16:9 aspect
that fits within the usable work area** of the display it opens on — the display
area minus platform chrome. The window **MUST NOT** exceed that work area.
(ADR-0036 §2.)

**11.4** Whether the runner reaches that size by setting bounds or by asking the
platform to maximize is **runner discretion**. (ADR-0036 §4.)

**11.5 The window's size survives relaunch.** The runner restores the size the
window was left at on exit. If that size no longer fits, the runner **clamps it
to the work area, silently, and persists the clamped value** — no fault, and no
reset to §11.3's default. §11.3 therefore fires exactly once per program per
machine, and leaving fullscreen returns the window to the **last windowed
size**. (ADR-0036 §4.)

**11.6 The window may be resized to any size and any aspect**, and the runner
**MUST NOT** constrain the drag to the canvas aspect. 16:9 is the initial shape,
not an invariant. Resize is a permission, always granted, and is not a setting.
(ADR-0025 §1; ADR-0036 §5.)

**11.7 Position within a display is not persisted.** A runner MAY persist it;
the spec is silent. (ADR-0036 §4.)

**11.8 A program reopens on the display it last occupied.** On first launch on a
given machine the runner places it on the platform's primary display, at
§11.3's size. Thereafter the runner restores the display the window was on at
exit; if that display is no longer attached, the program opens on the primary
display, **silently**. (ADR-0040 §2.)

**11.9 Fullscreen is a player preference the program may read and set.** There
are **two legal mutators**: the runner, which **MUST** provide an affordance —
the spec fixes no key, chord or chrome, only that one exists — and the program,
through its own menu. The program **MUST NOT cache the mode**, because the
runner can change it without telling the program. The two calls are names under
`$.` and are chapter 6's. (ADR-0025 §2, §3, §4; ch6 §7.)

**11.10 The fullscreen mode survives relaunch**, restored to what the player
last chose for that program, whichever mutator produced it. (ADR-0025 §6.)

**11.11 There is exactly one mutator for the display**: the player's own window
drag, using the affordance the platform already provides. **No new runner MUST
is minted**, because a MUST is earned by the failure it prevents and this one
would prevent nothing. §11.2 guarantees the window that drag needs. (ADR-0040
§4.)

**11.12** §11.5, §11.8 and §11.10 are **observables**. The spec names **no file,
no format, no location and no ordinal**; how a runner re-identifies a physical
display across a relaunch is its business, and a runner that cannot falls
through to §11.8's primary-display clause. (ADR-0025 §6; ADR-0036 §4; ADR-0040
§2, §3.)

**11.13** **Bordered versus borderless is runner discretion** and the spec is
silent; nothing observable depends on it. (ADR-0025 §1.)

**11.14** There is **no resolution selection**. A video menu's resolution row
has neither a mechanism nor a motive here: the canvas is fixed (§9.1), content
rasterises at native device resolution (§9.14), windowed the player drags, and
fullscreen `k` is forced. (ADR-0036 §7.)

**11.15** A **render scale** is the one performance escape hatch the spec ships,
and it is a player preference under §11.9's pattern. Its surface, its range, its
quantisation and its filter are chapter 6's. (ADR-0038; ADR-0036 §8; ch6 §7.)

---

## 12. What this chapter does not decide

Recorded so the boundary is legible, and so a later chapter is not read as
having inherited a silence.

- **The experience contract's wording and its property list.** Every clause here
  that a harness could check — reload preserving state, the fault observable,
  P13's fit, P14's window, the asset-reload property — is worded and numbered by
  chapter 8. (#19; ADR-0035 §5; ADR-0030 §8; ADR-0036 §9; ADR-0015 §8.)
- **The diagnostic envelope, codes and severity** of every fault and compile
  error named here. Chapter 7's. (ADR-0018; ADR-0023.)
- **The C boundary.** `extern "<lib>" fn` and the rules on calling out are #29's
  and chapter 4's; §7.8 states only the direction that binds this chapter.
- **Every name under `$.`** other than §0.6's two: the fullscreen pair, the
  render scale, the coordinate-mapping calls, the decode calls, the debug value,
  the storage surface and wall-clock time. Chapter 6's.
- **The `Scratch` and `Allocator` types**, and what `heap` is. Chapter 3's;
  §3.10 and §4.5.8 state only their positions.
- **The `#24` companion count** for §0.6's two names. Chapter 8's ledger.

---

## 13. Marked gaps

Per ADR-0044 §8, two gaps this chapter wrote down rather than blocked on. Each is
filed as a ticket that owns the repair of this chapter's text, its `coverage/`
rows and the reference program in one commit (ADR-0049). **One of the two is now
closed** and is kept, struck through, as the record of the repair.

**13.1** ~~**The fault-kind enum is published as closed and has been extended
twice.**~~ **Closed.** [#113](https://github.com/ludo-lang/ludo/issues/113)
resolved: the enum keeps its closure and gains a **seventh** member, **allocation
failure**, alongside ADR-0015 §4's **asset resolution failure**. §6.5.1 now
carries the authoritative membership and what each member reports, §6.1.6–§6.1.7
state that a kind is added only by editing §6.5.1 — which relocates the list off
an issue, the thing that let two extensions go unrecorded — and §6.1.8 states
that an `unsafe` use-after-free is a bug that is never a fault kind, because it
is never detected. Mapping the `persist` fault onto an existing kind was
rejected: no existing kind can carry a declaration's name and type, and folding
it into **explicit panic** would report a bug the program never wrote, which
§6.5.1's concrete-values rule forbids and which costs an agent the one field
that locates the failure.

**13.2** ~~**There is no spelling for a zero-filled fixed array.**~~
**Closed.** [#116](https://github.com/ludo-lang/ludo/issues/116) resolved in
chapter 3, whose territory it was: **ch3 §5.10.1's fill arm `[_] = v`** gives
`[N]T` a value of `N` equal elements, and **ch3 §5.10** states for the first
time that a fixed-array literal otherwise supplies exactly `N` items. The author
writes the value and the compiler repeats it, so #9's ban on an *implicit* zero
is untouched and `{}` against `[N]T` stays an error (ch3 §5.10.6). §4.3.1's
mandate is now satisfiable at any `N`; before it, the escape was to move the
buffer out of `persist` rather than write 4096 elements. **Zero #24 delta** —
the shape already parses (ch3 §5.10.2). Both marked gaps this chapter opened are
now closed.

