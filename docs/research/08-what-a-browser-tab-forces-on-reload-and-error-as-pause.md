# What a browser tab forces on reload and error-as-pause

Research for [#78](https://github.com/adamico/ludo/issues/78).

The web question has been resolved in four pieces and this is the fifth.
[#28](https://github.com/adamico/ludo/issues/28) settled the obligation — **no
mandated web target, but nothing in the mandated facade may be unimplementable in
a browser**. [#32](https://github.com/adamico/ludo/issues/32) / ADR-0006 shaped
the API. [#56](https://github.com/adamico/ludo/issues/56) / ADR-0022 checked the
facade against real browser semantics. [#74](https://github.com/adamico/ludo/issues/74)
/ ADR-0037 removed the last vendor dependency by admitting a wasm software
rasterizer. What was never checked is the **runner** half — the two hardest
halves of the experience contract, [#17](https://github.com/adamico/ludo/issues/17)'s
state-preserving reload and [#18](https://github.com/adamico/ludo/issues/18)'s
error-as-pause.

This document checks them against normative text: the WebAssembly core and JS API
specifications, the WHATWG HTML Standard's event loop, the IndexedDB
specification, the WebGPU specification, and the Web Audio API editor's draft.
Where the platform specifies nothing, that is said rather than guessed —
ADR-0022's posture, applied here.

## Summary of verdicts

| # | Clause or property | Source | Verdict |
| --- | --- | --- | --- |
| 1 | A code swap preserves `persist` across builds | #17 §1 | **Supplied, by a different mechanism** |
| 2 | "`persist` memory belongs to the runner's process and is never touched by the swap" | #17 §1 | **Description falsified** |
| 3 | Program constants are reload-stable | #17 §4 | **Supplied, monotonically** |
| 4 | Sub-second save-to-visible | #19 P1, MUST-MEASURE | **Not derivable; nothing in any spec bounds it** |
| 5 | A faulted program's process stays alive and is re-enterable | #18 §4, P2 | **Supplied, explicitly** |
| 6 | `persist` is readable after a fault by a walk running no user code | #18 §7 | **Supplied** |
| 7 | The call chain, in dev and in release | #18 §9, P2 | **Not supplied by the platform** |
| 8 | #18 §9's stated cost of the chain | #18 §9 | **Falsified for wasm** |
| 9 | The closed fault-kind enum covers what a tab can do to you | #18 §8 | **Divergent** |
| 10 | "release exits non-zero" | #18 §4, P2 | **No browser analogue** |
| 11 | Quiescence is undisturbed by the tab | ADR-0024 §3 | **Supplied** |
| 12 | "a faulted program is quiescent forever, so reload can rescue it" | ADR-0024 §3 | **Supplied, then defeated by discard** |
| 13 | "ludo is entered only from the frame thread" | ADR-0024 §5 | **Satisfiable; the word `thread` is wrong** |
| 14 | The fixed 60Hz step against `requestAnimationFrame` | #26, P7 | **Supplied, at the cost of running slow** |
| 15 | Exactly one image presented per returned entry | ADR-0035 §3 | **Supplied, structurally** |
| 16 | "Pacing is runner discretion" | ADR-0035 §4 | **Discretion is empty on the web** |
| 17 | A fault does not discard a submitted write | ADR-0026 §7 | **Supplied, with one runner discipline** |
| 18 | "The runner flushes it — at the latest, at exit" | ADR-0026 §5 | **Falsified** |
| 19 | Unsolicited device changes are unspecified | inherited, research/07 | **Confirmed, with a refinement** |

## The shape of the answer

**A browser tab does not make the runner's contract unmeetable. It makes three
sentences in the corpus untrue, and it moves two guarantees from *the platform
gives you this* to *your compiler emits this*.**

The two hardest halves came out opposite to expectation. **Error-as-pause is the
easy one** — the WebAssembly JS API says, in as many words, that *"traps do not,
in general, prevent future execution"*, which is #18 §4's whole mechanism handed
over as a normative sentence. And `persist` after a trap is a plain
`ArrayBuffer` a JS walk reads without running a byte of ludo, which is #18 §7
exactly. The pause is *cheaper* in a tab than natively, because there is no
signal handler, no guard page and no core file — just a caught exception and a
live buffer.

**Reload is the hard one, and not for the reason the ticket expected.** A wasm
module is not a dylib, but that is a mechanism detail with at least three working
answers. The real bite is that in wasm, `persist` **cannot** live where #17 §1
puts it. Linear memory is the only thing ludo code can address, so `persist` is
*inside* the artefact being swapped over rather than in host memory beside it.
The guarantee survives — a `WebAssembly.Memory` is explicitly shareable across
instances — but #17 §1's *description* of why it survives is false on the web,
and a reader who implements from that sentence walks into the active-data-segment
hazard in §1.3 below.

The two guarantees that move from platform to compiler are the **call chain** and
the **fault-kind enum**. WebAssembly has no instruction that reads its own call
stack, and `Error.prototype.stack` is not in any standard. So #18 §9's chain is
not something a web runner can ask for; it is something the ludo compiler must
emit, at a per-call runtime cost — which falsifies #18 §9's own account of what
the chain costs.

The three sentences the tab makes untrue are all sentences nobody had reason to
doubt: `persist` lives outside the swapped image (§1.3), the runner flushes
storage at exit (§5.3), and a released program exits non-zero (§2.5). None is
load-bearing for a decision; all three are wrong as written.

---

## 1. Reload

### 1.1 What the platform actually offers, and what it does not

[#17](https://github.com/adamico/ludo/issues/17) §1 makes reload a **dev-mode
dylib swap owned by the runner**. WebAssembly has no dylib. Three candidate
mechanisms exist and they are not equally real.

**(a) A second `WebAssembly.Instance` over a retained `Memory`.** The JS API
states plainly:

> "A Memory object represents a single memory instance which can be
> simultaneously referenced by multiple Instance objects."
> — [WebAssembly JS Interface](https://webassembly.github.io/spec/js-api/), *Memory objects*

So the runner compiles the new build as a module that **imports** its memory
rather than defining one, instantiates it against the `Memory` the old build was
using, discards the old `Instance`, and calls the new build's frame export. This
is the closest analogue to the dylib swap and it is fully specified. Its hazards
are §1.3 and §1.4.

**(b) Module-level dynamic linking.** The `dlopen`-shaped route Emscripten
supports. It is not a WebAssembly feature. The ABI document that defines it says
of itself:

> "Note: This ABI is still a work in progress. There is no stable ABI yet."
> — [WebAssembly/tool-conventions, `DynamicLinking.md`](https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md)

The mechanism is a `dylink.0` custom section plus imported `env.__memory_base` /
`env.__table_base` immutable globals against which all side-module code is
compiled position-independent. Emscripten's own documentation adds that dynamic
linking costs performance — *"This can add overhead, so for best performance
static linking should still be preferred"* — and that main modules disable dead
code elimination by default. Nothing in either document describes **unloading**
or **replacing** a loaded side module, which is precisely the operation reload
needs. This route is available, undocumented for the operation we want, and
sitting on a self-declared unstable ABI. Recorded so nobody proposes it as the
obvious one because it has the familiar name.

**(c) Full instantiate-and-copy.** A fresh `Memory`, a fresh `Instance`, and a
byte copy of the old memory into the new one at identical offsets. Every internal
offset stays valid because nothing moved. This is the only route with **no**
cross-build aliasing at all, and its cost is one `Uint8Array.prototype.set` over
the live heap — tens of milliseconds at heap sizes a #12-scoped game reaches. It
is not a dylib swap in any sense, and it is the most obviously correct of the
three.

**Verdict: supplied, by a different mechanism.** #17 §1's *outcome* — a new build
runs against untouched `persist` at a frame boundary — has at least two sound web
mechanisms. Its *mechanism* has none.

### 1.2 The retained `Memory` gotcha nobody would guess: `grow` detaches

If the runner holds a JS view onto `persist` (for #18 §7's structural dump, say)
and the wasm heap subsequently grows, that view dies. The JS API is explicit:

> "If IsFixedLengthArrayBuffer(buffer) is true, Perform ! DetachArrayBuffer(buffer,
> \"WebAssembly.Memory\"). Let buffer be the result of creating a fixed length
> memory buffer from memaddr."
> — [WebAssembly JS Interface](https://webassembly.github.io/spec/js-api/), *grow the memory buffer*

The escape is the resizable-`ArrayBuffer` path in the same algorithm, where the
existing buffer's data block and byte length are updated **without** detaching.
Either way this is a runner-implementation fact, not a spec-clause fact, and it
is noted here only because "the dump reads `persist` through a retained view" is
the obvious implementation and the obvious implementation is the broken one.

### 1.3 The one that falsifies a sentence: `persist` is inside the swapped image

#17 §1 says:

> "`persist` memory belongs to the runner's process and is never touched by the
> swap."

On the web the runner's process **is** the tab, and the only memory ludo code can
address is the module's linear memory. `persist` therefore lives in the same
`WebAssembly.Memory` the incoming build will be instantiated against. The
guarantee is still deliverable — see §1.1(a) and (c) — but two consequences
follow that the native sentence hides:

**Active data segments write into the shared memory at instantiation.** The core
specification's instantiation algorithm copies active data segments into the
module's memory as part of instantiation, before the start function runs, and can
trap partway if a segment is out of bounds
([WebAssembly core specification](https://webassembly.github.io/spec/core/exec/modules.html),
*Instantiation*). If the incoming build's constant data is emitted as active
segments at the offsets the old build used, **instantiating the new build
overwrites live `persist` bytes** — silently, before any ludo code runs. A web
runner must therefore either emit all build data as **passive** segments it
copies deliberately, or lay each build's data out in a region no live state
occupies. This is the single most implementable-looking way to get web reload
wrong.

**Verdict: description falsified.** No decision changes; a sentence does. The
narrowest correct restatement is that `persist` lives in **runner-controlled
memory that outlives the swapped code**, which is true natively and on the web,
where "the runner's process" is not.

### 1.4 #17 §4's constant blob is supplied, and it only grows

#17 §4 requires that *program constants are reload-stable*: literal data goes
into a runner-held blob, and blobs from earlier builds stay alive for the
session, so a `persist` `str` view survives any number of reloads.

Web mechanism: each build's literals go into a distinct region of the shared
linear memory, and no region is ever reclaimed. That works. What it costs is
stated by a spec absence — the core instruction set has **`memory.grow` and no
counterpart**; there is no instruction and no JS API to shrink a memory
([WebAssembly core specification](https://webassembly.github.io/spec/core/exec/modules.html);
[JS API](https://webassembly.github.io/spec/js-api/), `Memory.prototype.grow`).
So a long dev session's memory is monotonically non-decreasing in reload count,
with a hard ceiling of 4 GiB in wasm32.

**Verdict: supplied, monotonically.** #17 §4 already accepts the native version
of this leak ("blobs from earlier builds stay alive for the session"). The web
version is the same leak in an address space that cannot be given back.

### 1.5 P11's stable trampoline has a better web answer than a native one

[#19](https://github.com/adamico/ludo/issues/19) P11 requires a C-held ludo
callback pointer to remain valid across a reload and dispatch to the new build,
with the runner owning a stable trampoline.

WebAssembly gives this for free and by construction: a `WebAssembly.Table` of
`funcref` is importable, shareable across instances exactly as `Memory` is, and
**mutable at runtime** through `Table.prototype.set`. Callbacks are handed out as
table indices; `call_indirect` dispatches through the table; the swap rewrites the
slot. The index is stable, the target is not. There is no trampoline to write.

**Verdict: supplied, and cheaper than the native answer.**

### 1.6 Latency: the MUST-MEASURE is intact and nothing bounds it

#19 posture 1 makes reload latency a **MUST-MEASURE** against three reference
sizes with a published 1-second target, deliberately not a hard MUST. That
posture is the reason this section has no verdict to give: **no specification
anywhere bounds wasm compile or instantiate time**, and none should be read as
implying one.

What can be said from first-party vendor material, and only as an existence
argument rather than a number: V8 ships a two-tier wasm pipeline whose baseline
compiler, Liftoff, exists specifically for compile throughput, with eager tier-up
to the optimising compiler on background threads afterwards
([V8 blog, *Liftoff*](https://v8.dev/blog/liftoff)). That blog post publishes
*relative* compile-speed graphs and *no* absolute throughput figure, and it
publishes execution-quality figures instead — Liftoff code runs "on average around
50% slower than TurboFan code on the desktop machine and 70% slower on the
MacBook" on Unity benchmarks. A reload path that instantiates and runs
baseline-compiled code immediately is therefore fast to *start* and measurably
slower to *run* until tier-up completes.

Two web-specific costs the reference-size table does not currently model:

- **Where the compiler runs.** #17's loop assumes a compiler on the same machine
  producing a dylib. A browser-hosted implementation has to produce the wasm
  module somewhere and get it into the tab; a native implementation with a web
  backend is doing an ordinary `ludo build`. These are different measurements
  and the table names neither.
- **The 8 MiB synchronous-compile ceiling.** Emscripten's documentation records
  that "Chromium does not support synchronous compiling of Wasm modules over 8Mb
  on the main thread"
  ([Emscripten, *Dynamic Linking*](https://emscripten.org/docs/compiling/Dynamic-Linking.html)),
  which pushes a large build's compile onto the streaming/async path. That is a
  vendor limit, not a spec one, and it lands squarely on the 50k-line reference
  size.

**Verdict: not derivable.** The MUST-MEASURE posture is exactly right and needs
no change; a web claimant reports its number like everyone else.

---

## 2. Error-as-pause

### 2.1 The sentence that does most of this section's work

The WebAssembly JS API's error-mapping section states:

> "Running WebAssembly programs encounter certain events which halt execution of
> the WebAssembly code. WebAssembly code (currently) has no way to catch these
> conditions and thus an exception will necessarily propagate to the enclosing
> non-WebAssembly caller (whether it is a browser, JavaScript or another runtime
> system) where it is handled like a normal JavaScript exception. If WebAssembly
> calls JavaScript via import and the JavaScript throws an exception, the
> exception is propagated through the WebAssembly activation to the enclosing
> caller. **Because JavaScript exceptions can be handled, and JavaScript can
> continue to call WebAssembly exports after a trap has been handled, traps do
> not, in general, prevent future execution.**"
> — [WebAssembly JS Interface](https://webassembly.github.io/spec/js-api/) §7, *Error Condition Mappings to JavaScript*

Read against #18 §4 — *the dev runner keeps the process alive so state can be read
and a fix loaded* — this is the guarantee handed over verbatim. A trap in a ludo
frame becomes a `WebAssembly.RuntimeError` thrown at the runner's rAF callback;
the runner catches it; the instance stays callable; a reload re-enters it. The
core specification's own framing agrees, and is worth quoting for the half about
catching:

> "Under some conditions, certain instructions may produce a *trap*, which
> immediately aborts execution. Traps cannot be handled by WebAssembly code, but
> are reported to the outside environment, where they typically can be caught."
> — [WebAssembly core specification](https://webassembly.github.io/spec/core/intro/overview.html), *Overview*

**Verdict: supplied, explicitly.** This is the strongest single result in this
document. The corpus's most distinctive property is the one the web spec goes out
of its way to promise.

Two notes on what "abandoned frame" means down here. First, a trap is
administrative: *"Traps are bubbled up through nested instruction sequences,
ultimately reducing the entire program to a single trap instruction, signalling
abrupt termination"*
([core specification](https://webassembly.github.io/spec/core/exec/runtime.html),
*Administrative instructions*) — there is no unwinding to run, no landing pad, no
destructor, which matches #10's no-unwinding error model and #18 §1's "the
mechanism is far smaller than DragonRuby's" more closely than native does.
Second, the store is threaded through those reduction rules unchanged: **no rule
restores an earlier store**, so every memory write performed before the trap is
still there. That is #18 §5's *`persist` is trusted as-is: no rollback*, obtained
for free. The spec never says "the store is not rolled back" in those words; it
is derivable from the reduction rules and from §2.2 below, and it is flagged as
derived rather than stated.

### 2.2 `persist` is readable after a trap, and the dump runs no ludo code

#18 §7 requires `persist` to be readable recursively by a structural walk over
known layout that executes no user code.

On the web this is not merely possible, it is the *only* thing available, which
makes it hard to get wrong. `persist` is bytes in a linear memory; the memory is
exposed to the runner as a `WebAssembly.Memory` whose `buffer` is an ordinary
`ArrayBuffer`; a trap does not detach it (the only detach in the JS API is the
`grow` path in §1.2); the runner holds the build's type information because the
runner *is* the toolchain's other half. The walk is JS reading typed-array views
over a byte range. There is no formatting hook to accidentally call, because JS
cannot call into wasm except through an export the runner chose to call.

**Verdict: supplied.** #18 §7's exclusions carry over unchanged: `unsafe`
declarations render as an address, pool slots render by active set, depth limits
stay a runner concern.

### 2.3 The call chain is not something a browser gives you

#18 §9 guarantees the fault report's **call chain to the frame entry, in dev and
in release**. There are exactly two ways to get one in a tab and neither is a
platform guarantee.

**From the host.** `WebAssembly.RuntimeError` inherits `stack` from `Error`. MDN's
status banner on that property reads:

> "**Non-standard:** This feature is not standardized. We do not recommend using
> non-standard features in production, as they have limited browser support, and
> may change or be removed."
> — [MDN, `Error.prototype.stack`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Error/stack)

with the caveat that it is de facto implemented everywhere, is the subject of a
TC39 proposal, and that *"You cannot rely on the precise content of the stack
string due to implementation inconsistencies"* — MDN's own examples show three
different formats across JavaScriptCore, SpiderMonkey and V8. The JS API
specification contains no requirement about stack traces of any kind. The
rendering of *wasm* frames within such a string is a vendor matter on top of a
non-standard property.

**From inside.** The core instruction set contains no instruction that reads the
call stack. A wasm function cannot see its callers. This is not a gap the spec
notes; it is derivable from the instruction index, and it is absolute.

So the chain must be **emitted by the ludo compiler** — a shadow stack in linear
memory, pushed and popped per call, read by the runner from the trapped memory.
That is entirely implementable and it is what a wasm backend will do.

**Verdict: not supplied by the platform.** The property is not falsified. Its
cost is.

### 2.4 #18 §9's stated cost is wrong for wasm

#18 §9 states its own price as:

> "Costs binary size and forecloses the most aggressive frame-pointer omission."

That is a native cost model. In wasm there are no frame pointers to omit and no
unwind tables to consult; §2.3's shadow stack is a **store on every call and a
decrement on every return**, in the hot path, in **release** builds too — because
§9 deliberately made the chain mode-independent to keep #5 criterion 4 literally
true.

This does not argue against the guarantee. It argues that the guarantee's price
is misdescribed in the one place the corpus states it, and a future ticket
weighing "should the chain be dev-only?" would weigh it against a number that is
wrong by a category.

**Verdict: falsified for wasm.** Narrowest correction: the chain costs binary size
and frame-pointer omission on native targets, and per-call runtime on targets with
no stack introspection.

### 2.5 Two things a tab does that #18's closed enum has no member for

#18 §8 fixes the fault kind as a **closed enum**: out-of-bounds, assert, overflow,
divide-by-zero, explicit panic. The JS API defines two conditions that halt a wasm
program and are **not** traps:

> "**7.1. Stack Overflow.** Whenever a stack overflow occurs in WebAssembly code,
> the same class of exception is thrown as for a stack overflow in JavaScript. The
> particular exception here is implementation-defined in both cases. Note:
> ECMAScript doesn't specify any sort of behavior on stack overflow;
> implementations have been observed to throw RangeError, InternalError or Error.
> Any is valid here."
>
> "**7.2. Out of Memory.** Whenever validation, compilation or instantiation run
> out of memory, the same class of exception is thrown as for out of memory
> conditions in JavaScript. The particular exception here is implementation-
> defined in both cases. Note: … implementations have been observed to throw
> OOMError **and to crash**. Either is valid here."
> — [WebAssembly JS Interface](https://webassembly.github.io/spec/js-api/) §7

Three observations, in ascending order of seriousness.

- **Stack exhaustion** is a real ludo bug (unbounded recursion) that produces no
  ludo fault kind. It arrives as an implementation-defined JS exception the runner
  must classify by vendor. This is a pre-existing gap in #18 §8 — native stack
  exhaustion is equally unenumerated — that the web makes impossible to ignore,
  because the exception is *typed* and the type is *not specified*.
- **Out-of-memory** is worse: it can occur during **instantiation**, which is
  reload's own operation, and the specification blesses **crashing** as a
  conforming response. A crash is not a fault; nothing is reported and nothing is
  rescued.
- Both are cases where #18 §4's "the dev runner keeps the process alive" is
  outside the runner's control.

**Verdict: divergent.** Not a browser defect — the same holes exist natively — but
the browser is where they are written down.

### 2.6 "release exits non-zero" has no browser analogue

#18 §4 and #19 P2 both state the release half as: *the same program ends its
simulation identically and **exits non-zero***.

A tab has no exit status. There is no process to exit, no waiting parent, and
nothing in the HTML Standard that lets a document report a numeric status to
anything. A released ludo program running in a tab can satisfy every other clause
of P2 — the frame is abandoned, no further entry is called, `persist` holds its
value at the fault, the ADR-0018 `fault` message is emitted — and cannot satisfy
that one, ever.

Two ways out exist and this ticket picks neither: read "exits non-zero" as
*terminates abnormally by whatever the host's convention is*, or scope the clause
to hosted targets. Both are re-cuts; both are #19's to make.

**Verdict: no browser analogue.** This is the only place where a **#19 property's
literal observable** is unmeetable in a tab.

---

## 3. Quiescence

### 3.1 Nothing a tab does disturbs the predicate

ADR-0024 §3:

> "A program is quiescent when no ludo code is on any stack, no `extern` call is
> in flight, and the only live ludo state is `persist` plus declared assets" —
> plus, per ADR-0026 §8, resolved storage contents.

The relevant platform fact is the event loop's task discipline: a task runs to
completion, and the runner's frame entry is one task's worth of work. Between two
rAF callbacks there is no ludo code on any stack because there is no ludo code
running at all — the JS stack is empty and the wasm activation is gone. The
predicate holds at exactly the boundary ADR-0024 names.

The interesting cases are the ones a tab has and a native process does not, and
all four come out the same way:

- **Hidden tab.** The document's animation frame callbacks are not run (§4.1). No
  ludo code runs. Quiescent.
- **Frozen page.** Chrome's page-lifecycle documentation describes the frozen
  state as one where "the browser suspends execution of freezable tasks" and
  "JavaScript timers and fetch callbacks don't run"
  ([Chrome for Developers, *Page Lifecycle API*](https://developer.chrome.com/docs/web-platform/page-lifecycle-api)).
  No ludo code runs. Quiescent.
- **Back/forward cache.** The document is kept whole and inert. Quiescent, and
  memory intact.
- **Discarded page.** *"No tasks, event callbacks, or JavaScript of any kind can
  run in this state"* (same source). Quiescent, and see §3.2.

**Verdict: supplied.** A quiescence predicate stated over *ludo stacks* rather
than over *time* is exactly the formulation that survives an environment which can
stop running your code for arbitrary intervals. ADR-0024 §3's choice of a state
predicate over a location pays off here in a way its own reasoning did not
anticipate.

### 3.2 The theorem holds; what it was for does not

ADR-0024 §3 derives:

> "**A faulted program is quiescent forever.** #18 ends the simulation, so no ludo
> code will run again. This is what lets **reload rescue a faulted program** — the
> recovery story that makes error-as-pause worth having."

The theorem is true in a tab and the *rescue* is not guaranteed, because the tab
can be taken away between the fault and the fix. Chrome's page-lifecycle
documentation is explicit that the hidden → discarded and frozen → discarded
transitions fire **no observable event**, and that a discarded page runs no
JavaScript at all; the page learns it happened only retroactively, via
`document.wasDiscarded` after a subsequent load. The WHATWG HTML Standard does not
describe tab discarding at all — it defines destroying and unloading documents but
grants no user-agent latitude to discard for resource pressure, so **the whole
behaviour lives outside the standard**, in vendor policy.

A faulted ludo program in a tab is therefore in the *most* discardable state a
page can be in: it is hidden as soon as the developer switches to their editor, it
is doing no work, and it is holding a large linear memory. Nothing warns it, and
its `persist` — the entire content of the recovery story — goes with it.

**Verdict: supplied, then defeated by discard.** Not a falsification: ADR-0024's
sentence is about quiescence and quiescence holds. It is a scope note the corpus
does not carry, and it is what §5.3's falsification is a special case of.

### 3.3 "The frame thread" names something a tab does not have

ADR-0024 §5 adds:

> "**ludo code may only be entered from the frame thread.**"

with the hazard named as *a C worker thread calling a ludo callback while the
runner swaps the dylib underneath it*.

A tab has one agent and one event loop; there is no second thread to call in from
unless the page creates a Worker, and a Worker cannot call a non-shared
`WebAssembly.Memory`'s instance at all. So the *thread* hazard is absent by
construction. The *re-entrancy* hazard is not, and it arrives through a door
ADR-0024 does not name: the JS API states that when wasm calls JS via an import,
*"the exception is propagated through the WebAssembly activation"* — and by the
same token, an import is ordinary JS that may call an export straight back,
putting ludo frames on the stack beneath an `extern` call in flight. Every browser
callback the runner registers — event listeners, promise reactions, IndexedDB
success handlers — is a place from which ludo can be re-entered outside the frame
task.

This is satisfiable: it is a rule about which tasks the runner calls exports from,
and the runner owns all of them. But the clause as worded tells a web implementer
to check a property (*thread*) that is trivially true on their platform, while the
property they must actually maintain (*task*) is not mentioned.

**Verdict: satisfiable; the word is wrong.** Narrowest restatement: ludo is
entered only from the runner's frame task, on whatever the host calls a thread.

---

## 4. The frame thread and the 60Hz step

### 4.1 A hidden document gets zero frames, and this is normative

The common belief — from MDN, and true as far as it goes — is that *"`requestAnimationFrame()`
calls are paused in most browsers when running in background tabs or hidden
`<iframe>`s"*
([MDN, `Window.requestAnimationFrame()`](https://developer.mozilla.org/en-US/docs/Web/API/Window/requestAnimationFrame)).
The HTML Standard is stronger than "most browsers". In the *update the rendering*
steps, before any callback runs:

> "**Filter non-renderable documents:** Remove from docs any Document object doc
> for which any of the following are true: doc is render-blocked; **doc's
> visibility state is \"hidden\"**; doc's rendering is suppressed for view
> transitions; or doc's node navigable doesn't currently have a rendering
> opportunity."

and only later:

> "For each doc of docs, **run the animation frame callbacks for doc**, passing in
> the relative high resolution time given frameTimestamp and doc's relevant global
> object as the timestamp."
> — [WHATWG HTML Standard](https://html.spec.whatwg.org/multipage/webappapis.html#event-loop-processing-model), *Update the rendering*

A hidden document is removed from the list before the callbacks step. This is not
throttling and not a vendor choice: **animation frame callbacks are specified not
to run for a hidden document.** A ludo game whose tab is in the background is not
running slowly; it is not running.

### 4.2 The rate is the user agent's, and the specification says so twice

> "A navigable **has a rendering opportunity** if the user agent is currently able
> to present the contents of the navigable to the user, accounting for hardware
> refresh rate constraints and user agent throttling for performance reasons, but
> considering content presentable even if it's outside the viewport."
>
> "A navigable's rendering opportunities are determined based on hardware
> constraints such as display refresh rates and other factors such as page
> performance or whether its active document's visibility state is \"visible\".
> … **This specification does not mandate any particular model for selecting
> rendering opportunities.** But for example, if the browser is attempting to
> achieve a 60Hz refresh rate, then rendering opportunities occur at a maximum of
> every 60th of a second (about 16.7ms). **If the browser finds that a navigable
> is not able to sustain this rate, it might drop to a more sustainable 30
> rendering opportunities per second** for that navigable, rather than
> occasionally dropping frames. Similarly, if a navigable is not visible, the user
> agent might decide to drop that page to **a much slower 4 rendering
> opportunities per second, or even less**."
> — [WHATWG HTML Standard](https://html.spec.whatwg.org/multipage/webappapis.html#event-loop-processing-model)

#26 fixes the simulation at 60Hz as a **compile-time constant step**, which is
what makes the frame entry's zero-parameter signature coherent. Composed with the
above: if the user agent offers 30 rendering opportunities per second, a ludo
program in that tab executes 30 steps of 1/60 s each per wall-clock second and
**runs at half speed**.

This is not a violation. It is #19 **P7** — *frame overrun degrades, never catches
up* — arriving from the environment instead of from the program. P7's observable
is "exactly one frame entry call has occurred for that step; the simulation runs
late and slows down, and no catch-up call is made", and that is precisely correct
behaviour for a throttled navigable. P7 is the property that makes a ludo web
target behave sanely under throttling, and it was written for an entirely
different reason.

**Verdict: supplied, at the cost of running slow.** Worth stating on the record
because *the game runs in slow motion when the browser is busy, and stops entirely
when the tab is hidden* is a user-visible consequence of two decisions
(#26's fixed step, P7's no-catch-up) neither of which mentions a browser.

### 4.3 The timer escape hatch does not exist

The obvious counter-move — drive the simulation from `setTimeout` at 60Hz and only
draw on rAF — fails on two counts.

Mechanically, the HTML Standard's timer initialisation steps wait until the global
has been *fully active* for the requested duration and then permit padding:

> "Optionally, wait a further implementation-defined length of time. This is
> intended to allow user agents to pad timeouts as needed to optimize the power
> usage of the device."
> — [WHATWG HTML Standard](https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html), *Timers*

The permission is open-ended and browsers use it aggressively for background
pages. So the timer does not rescue a hidden tab.

Structurally, it also breaks ADR-0035 §3 (§4.4): a simulation ticking at 60Hz
against a presentation happening at 144Hz or at 30Hz would produce entries with no
presentation and presentations with no entry, which is the clause's exact
prohibition.

**Verdict: closed.** The only sound web frame source is the rendering-opportunity
clock, and that is the user agent's.

### 4.4 One present per entry is *structurally* guaranteed on the web

ADR-0035 §3:

> "**Exactly one image is presented per returned frame entry.** The runner does
> not present a frame twice, and does not skip presenting a frame that was
> entered."

This was the clause most likely to be unimplementable in a tab, because a web
runner has no present call — it never asks for a swap. It turns out that is
exactly *why* the clause holds. The WebGPU specification hooks canvas presentation
into the same event-loop step as the animation frame callbacks:

> "… updating the rendering of a WebGPU canvas … occurs before getting the
> canvas's image contents, in the following sub-steps of the event loop processing
> model: \"update the rendering or user interface of that Document\"; \"update the
> rendering of that dedicated worker\"."
> — [WebGPU](https://www.w3.org/TR/webgpu/) §21.3, *HTML Specification Hooks*

The animation frame callbacks step and the canvas-presentation step are in one
pass over one list of documents. If the runner calls the frame entry from an rAF
callback and draws in it, **the resulting image is presented exactly once**, in
that same pass, by the platform. Skipping is impossible without skipping the
callback that produced it; double presentation is impossible because there is one
presentation step per pass.

The 144Hz case ADR-0035 §1 accepts behaves identically to native. On rendering
opportunities where the runner declines to run an entry, no new image is produced
and WebGPU's `[[lastPresentedImage]]` is what the compositor shows — which is
scan-out of an unchanged frame, the same thing a 144Hz panel does to a 60fps
native game, and not a second presentation of a frame.

**Verdict: supplied, structurally.** The strongest result in §4, and the one that
most vindicates ADR-0035 §5's decision to ship the clause as prose rather than as
a conformance property: on the web the clause is not merely unobservable, it is
not the runner's to violate.

### 4.5 ADR-0035 §4's discretion is empty in a tab

> "**How** the runner waits — vsync on or off, how it sleeps to hit its clock,
> whether it free-runs — is **not specified**. … *How you wait* is environment."

In a tab the runner does not wait at all. It registers a callback and the user
agent decides when, at a rate the HTML Standard explicitly declines to model
(§4.2). There is no vsync toggle, no sleep, and no free-running.

This does not falsify the clause — ADR-0035 §4 grants discretion, it does not
require any to be exercised — but it is worth recording that the clause's premise
(that a backend has a knob here, and should be trusted with it) is false for the
web, and that the consequence is §4.2: the environment can move a ludo program's
apparent speed, and P7 is the only reason that is legal.

---

## 5. Storage

### 5.1 The clause under test

ADR-0026 §7:

> "**A #18 fault does not discard a submitted write, including one submitted in
> the faulting frame.** The frame is abandoned, but the submit already happened.
> Discarding it would make save durability depend on unrelated code later in the
> same frame."

with §5's backing: the write is submit-and-forget, and a web backend uses
IndexedDB with no async reaching the program.

### 5.2 Against a trap, the clause holds

IndexedDB's commit discipline is task-shaped, not exception-shaped. A transaction
becomes inactive when the task that created it finishes, and commits when its
request list drains:

> "It is not normally necessary to call `commit()` on a transaction. A transaction
> will automatically commit when all outstanding requests have been satisfied and
> no new requests have been made."
>
> "To **commit a transaction** … Set transaction's state to committing. Run the
> following steps in parallel: Wait until every item in transaction's request list
> is processed. … The implementation must atomically write any changes to the
> database made by requests placed against the transaction."
>
> "For each transaction transaction with cleanup event loop matching the current
> event loop: Set transaction's state to inactive. … NOTE: These steps are invoked
> by [HTML]. They ensure that transactions created by a script call to
> `transaction()` are deactivated once the task that invoked the script has
> completed."
> — [Indexed Database API](https://w3c.github.io/IndexedDB/) §3.1, §5.4

So: `$.storage.write` in the faulting frame becomes an `IDBObjectStore.put()`
placed against a transaction during the rAF task. The frame then traps. The trap
becomes a JS exception, the runner catches it, the task ends, the transaction
deactivates and commits. The write lands. **The abandoned frame and the submitted
write are on different mechanisms and the fault does not touch the write.**

**One runner discipline is load-bearing.** IndexedDB *does* have an
abort-on-exception path, and it is scoped to event handlers:

> "If this transaction was aborted due to a failed request, this will be the same
> as the request's error. **If this transaction was aborted due to an uncaught
> exception in an event handler, the error will be a \"AbortError\" DOMException.**"
> — [Indexed Database API](https://w3c.github.io/IndexedDB/), `IDBTransaction.error`

The mechanism is the `legacyOutputDidListenersThrowFlag` threaded through IDB's
event dispatch. It fires when a listener on an IDB `success`/`error` event throws
— not when an rAF callback throws. So the clause holds **provided the runner never
calls into ludo from an IndexedDB event handler**, which is §3.3's re-entrancy
discipline in its most concrete form: violate it, and a fault inside a ludo
callback invoked from an IDB `success` handler aborts the very transaction the
program's earlier write was sitting in — losing a save for a reason with no
relation to saving, which is the exact outcome ADR-0026 §7 exists to prevent.

**Verdict: supplied, with one runner discipline.** Worth writing down: the clause
is safe, and there is precisely one way to implement it that makes it unsafe.

### 5.3 The clause that does not hold: "at the latest, at exit"

ADR-0026 §5:

> "**The write is submit-and-forget.** It never blocks and returns no completion.
> **The runner flushes it — at the latest, at exit.**"

A browser does not guarantee that anything runs at exit. This is
vendor-documented, consistently, and it is the reason `unload` is deprecated:

> "Especially on mobile, the `unload` event is not reliably fired. For example,
> the `unload` event is not fired at all in the following scenario: A mobile user
> visits your page. The user then switches to a different app. Later, the user
> closes the browser from the app manager."
>
> "The best event to use to signal the end of a user's session is the
> `visibilitychange` event."
> — [MDN, `Window: unload` event](https://developer.mozilla.org/en-US/docs/Web/API/Window/unload_event)

and, on the state that has no event at all:

> "A page is in the **discarded** state when it is unloaded by the browser in
> order to conserve resources. No tasks, event callbacks, or JavaScript of any
> kind can run in this state."
>
> [the hidden state is] "the last time developers can reliably save app and user
> data."
> — [Chrome for Developers, *Page Lifecycle API*](https://developer.chrome.com/docs/web-platform/page-lifecycle-api)

The WHATWG HTML Standard, correspondingly, defines no discard operation for a
user agent to perform under memory pressure — the whole behaviour is outside the
standard, which is why the citation here is a vendor one and cannot be anything
else.

So a browser runner has no "at exit". It has a **last reliable moment**
(`visibilitychange` to hidden) and after that a page that may simply stop
existing. A pending write held for a lazy flush at exit is lost, silently, with no
program-visible signal — ADR-0026 §7's *IO failure … the program never learns of
it* covers the reporting, but the durability the section fought for is gone.

The fix is not hard and this ticket does not choose it: flush on submit, or flush
each frame, or flush at `visibilitychange`. What is falsified is the sentence.

**Verdict: falsified.** Narrowest correction: *the runner flushes it — at the
latest, at the last moment the host guarantees it can run code*, which is exit on
a hosted target and the transition to hidden in a tab.

### 5.4 Durability is a hint, and the default is not "on disk"

Even a committed transaction is not necessarily on persistent media:

> "A transaction has a **durability hint**. … \"strict\": The user agent may
> consider that the transaction has successfully committed only after verifying
> that all outstanding changes have been successfully written to a persistent
> storage medium. \"relaxed\": The user agent may consider that the transaction
> has successfully committed as soon as all outstanding changes have been written
> to the operating system, without subsequent verification. **\"default\": The
> user agent should use its default durability behavior for the storage bucket.
> This is the default for transactions if not otherwise specified.**"
> — [Indexed Database API](https://w3c.github.io/IndexedDB/) §3.1

This is not a defect — a native `write()` without `fsync` is in the same position,
and ADR-0026 §7 already declines to give the program a status channel. It is
recorded because "the write is submitted" and "the write survives a power cut" are
different claims and the corpus makes only the first one, correctly.

**No verdict; scope note.** §6's 1 MiB floor is unaffected: IndexedDB's capacity
is quota-governed and orders of magnitude above it, which is exactly why ADR-0026
§5 named IndexedDB rather than `localStorage`.

---

## 6. The inherited fact, confirmed with one refinement

The fact carried forward from [#56](https://github.com/adamico/ludo/issues/56) /
[research/07](07-browser-semantics-of-the-mandated-facade.md) is that **an
unsolicited device change is unspecified in every browser** — the specification
covers only the solicited change (`setSinkId()`), and says nothing about the user
unplugging headphones or the system default device moving underneath a live
`AudioContext`.

This is confirmed, and it has moved slightly. The Web Audio API editor's draft now
carries an `onerror` handler on `AudioContext`:

> "The event type of this handler is `error` and **the user agent can dispatch this
> event** in the following cases: When initializing and activating a selected audio
> device encounters failures. **When the audio output device associated with an
> AudioContext is disconnected while the context is running.** When the operating
> system reports an audio device malfunction."
> — [Web Audio API editor's draft](https://webaudio.github.io/web-audio-api/), `AudioContext.onerror`

Read precisely: there is now a **hook** where research/07 found none, and it
changes nothing about the underlying finding. The verb is *can*, not *must*, so no
browser owes the event. And the specification still says nothing about what
happens to the graph, to the node connections, or — the clause that matters to
ADR-0007 — to `currentTime` when the device goes away. The **play cursor's
behaviour across an unsolicited device change remains unspecified**, which is
exactly what research/07 recorded, and its verdict (clause supplied, because
ADR-0006 R8 makes the ring runner-owned and the cursor is therefore ours rather
than the device's) is unaffected.

**Verdict: confirmed, with a refinement.** The refinement is worth carrying:
*there is a spec-mentioned error event for device disconnection, it is permissive,
and it carries no semantics.*

---

## What this falsifies

Four items, in descending order of consequence. None changes a decision; all four
change text, and all four are re-cut work rather than reopened questions.

1. **ADR-0026 §5 — "The runner flushes it — at the latest, at exit."** A browser
   guarantees no exit hook; a discarded page runs no JavaScript at all. (§5.3)
2. **#19 P2 / #18 §4 — "release exits non-zero."** A tab has no exit status. This
   is the only #19 property whose literal observable is unmeetable in a browser.
   (§2.6)
3. **#18 §9 — the call chain "costs binary size and forecloses the most aggressive
   frame-pointer omission."** In wasm it costs per-call runtime, in release, because
   there is no stack introspection and `Error.prototype.stack` is non-standard.
   (§2.3, §2.4)
4. **#17 §1 — "`persist` memory belongs to the runner's process and is never
   touched by the swap."** In wasm `persist` is inside the linear memory the new
   build is instantiated against; the guarantee survives, the description does
   not, and the active-data-segment hazard it hides is the easiest way to get web
   reload wrong. (§1.3)

Two further items are scope notes rather than falsifications and are recorded so a
re-cut ticket can decide whether they want wording:

- **ADR-0024 §3's rescue story** is defeated by tab discard, which the HTML
  Standard does not describe and which fires no event. The quiescence theorem
  holds; what it was *for* does not survive a discarded tab. (§3.2)
- **ADR-0024 §5's "frame thread"** names a hazard a tab does not have and misses
  the one it does: re-entrancy is by *task*, not by thread. (§3.3)

## Summary table

| Area | What a browser tab forces | Verdict |
| --- | --- | --- |
| Reload mechanism | Not a dylib. Retained-`Memory` re-instantiation or instantiate-and-copy; Emscripten's `dlopen` route has a self-declared unstable ABI and no documented unload | Supplied by other means |
| `persist` location | Inside the linear memory, not beside it | #17 §1's description falsified |
| Constants across reloads | Distinct per-build regions in a memory that can grow and never shrink | Supplied, monotonically |
| P11's trampoline | A shared mutable `WebAssembly.Table`; no trampoline needed | Supplied, cheaper than native |
| Reload latency | Nothing in any spec bounds compile or instantiate time | MUST-MEASURE stands |
| A trap | Becomes a catchable JS exception; *"traps do not, in general, prevent future execution"* | Supplied, explicitly |
| `persist` after a trap | A plain `ArrayBuffer` walked by JS; no ludo code runs | Supplied |
| The call chain | Compiler-emitted shadow stack, because the platform has neither introspection nor a standard `stack` | Supplied by us, not by the platform |
| Fault-kind enum | Stack exhaustion and OOM are implementation-defined non-traps; OOM may crash | Divergent |
| Release exit status | Does not exist | No analogue |
| Quiescence | Task-to-completion; hidden, frozen, bfcached and discarded pages all run no ludo code | Supplied |
| The rescue | A discarded tab takes `persist` with it and fires no event | Defeated |
| Frame source | `requestAnimationFrame`; hidden documents are *filtered out* of the callbacks step normatively | Zero frames when hidden |
| Frame rate | The user agent's, explicitly unmodelled, may be 30/s or 4/s | Game runs slow; P7 makes it legal |
| Timers as a fallback | Padding is open-endedly permitted; breaks one-present-per-entry | Closed |
| One present per entry | WebGPU presentation is hooked into the same event-loop pass as the rAF callbacks | Supplied structurally |
| Pacing discretion | There is no knob | Discretion empty |
| A fault vs a submitted write | IDB commits on task end, not on exception | Supplied |
| Flush at exit | No guaranteed exit hook | Falsified |
| Unsolicited device change | A permissive `AudioContext.onerror` exists; semantics still absent | Confirmed, refined |

## Citations

Normative specifications, quoted from the sources named:

- **WebAssembly core specification** — https://webassembly.github.io/spec/core/
  (*Overview* on traps; *Administrative instructions* on trap propagation and the
  threading of the store; *Instantiation* on active data and element segments, the
  start function, and instantiation trapping partway; the instruction index, for
  the absence of any stack-introspection or memory-shrinking instruction).
- **WebAssembly JavaScript Interface** — https://webassembly.github.io/spec/js-api/
  (§7 *Error Condition Mappings to JavaScript*, including §7.1 Stack Overflow and
  §7.2 Out of Memory, and the sentence *"traps do not, in general, prevent future
  execution"*; *Memory objects* on multi-instance references; *grow the memory
  buffer* on `DetachArrayBuffer`; `Table.prototype.set`; instantiation and import
  reading. The specification contains no requirement of any kind about stack
  traces).
- **WHATWG HTML Standard** — https://html.spec.whatwg.org/
  (*Event loop processing model* → *Update the rendering*: the "Filter
  non-renderable documents" step, the "run the animation frame callbacks" step,
  and the definition of *rendering opportunity* with its 60/30/4-per-second
  examples; *Timers* on the optional implementation-defined padding; *Destroying
  documents* and *Unloading documents*, which define no discard-under-pressure
  operation).
- **Indexed Database API** — https://w3c.github.io/IndexedDB/
  (*Transaction* on the durability hint and on automatic commit; *Committing a
  transaction*; *cleanup Indexed Database transactions* and its note about being
  invoked by HTML at task end; `IDBTransaction.error` on the AbortError produced
  by an uncaught exception in an event handler).
- **WebGPU** — https://www.w3.org/TR/webgpu/
  (§21.3 *HTML Specification Hooks*: "updating the rendering of a WebGPU canvas"
  occurring in the event loop's update-the-rendering sub-steps; *Expire the current
  texture* and `[[lastPresentedImage]]`).
- **Web Audio API editor's draft** — https://webaudio.github.io/web-audio-api/
  (`AudioContext.onerror` and its permissive "can dispatch" listing, including
  device disconnection during a running context).

Non-normative and vendor sources, used only where nothing normative exists and
labelled as such in the text:

- **MDN** — `Error.prototype.stack` (non-standard status banner; cross-engine
  format divergence); `Window.requestAnimationFrame()` (background-tab pausing);
  `Window: unload` event (unreliability, mobile scenario, `visibilitychange`
  recommendation, bfcache incompatibility).
- **Chrome for Developers, *Page Lifecycle API*** —
  https://developer.chrome.com/docs/web-platform/page-lifecycle-api
  (the frozen and discarded states, the transitions that fire no event, and
  "the last time developers can reliably save app and user data"). Cited because
  tab discarding is not described by any specification.
- **Emscripten, *Dynamic Linking*** —
  https://emscripten.org/docs/compiling/Dynamic-Linking.html
  (`MAIN_MODULE`/`SIDE_MODULE`, `dlopen`, the static-linking performance
  preference, the 8 MiB synchronous-compile limit in Chromium).
- **WebAssembly/tool-conventions, `DynamicLinking.md`** —
  https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
  (the `dylink.0` custom section, `__memory_base`/`__table_base`, and the
  document's own "There is no stable ABI yet").
- **V8 blog, *Liftoff: a new baseline compiler for WebAssembly in V8*** —
  https://v8.dev/blog/liftoff (two-tier pipeline and eager tier-up; relative
  compile-speed graphs with no absolute throughput figure; Liftoff execution
  50–70% slower than TurboFan on Unity benchmarks).
