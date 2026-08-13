---
status: accepted
---

# The record stops assuming a process, and re-entrancy is by task

Issue #79 inherited six wrong sentences from [#78](https://github.com/adamico/ludo/issues/78), the research ticket that read the runner half of the experience contract against normative WebAssembly, HTML, IndexedDB and WebGPU text. #78 established *that* they are false and proposed the narrowest correction for each; it had no standing to change them.

**Every one of the six describes a decision that stands.** Not one is reopened here. That is why they are one ADR rather than six tickets — and it is also why they are one ADR rather than three amendment stamps, which was the alternative the ticket named and §1 rejects.

## 1. One cause, which is the finding

The six texts do not share a subject. They span durability (ADR-0026), the fault observable (#19 P2, #18 §4, ADR-0018 §11), the price of the call chain (#18 §9), where `persist` lives (#17 §1), and two clauses of ADR-0024. What they share is a **mistake in kind**: each describes a *native process* and states it as though it described *any host*.

- A process exits, and something waits on it. A tab is discarded, and nothing waits.
- A process has an exit status. A tab has none.
- A process can read its own call stack. WebAssembly has no instruction that does.
- A process owns memory the swapped code cannot address. Linear memory is the only thing ludo code can address.
- A process has threads, so re-entrancy is by thread. A tab has one thread and many tasks.

Three amendment stamps would have landed the word repairs in three documents and the two real rules in a fourth. The pattern — **the corpus wrote a host and called it the world** — would then exist nowhere, and the next backend that is not a native process would find it again from scratch. The pattern is the reusable half, so it gets a document.

This ADR carries all six. §2, §3, §4 and §8 repair prose. §5–§7 change what a conforming implementation must do.

## 2. The flush is bounded by the host's last guaranteed moment, not by exit

ADR-0026 §5's *the runner flushes it — at the latest, at exit* is false in a browser, and false in the exact way that costs the decision what it won. There is no guaranteed exit hook: `unload` is unreliable and deprecated, and a **discarded page runs no JavaScript of any kind and fires no event**. A write held for a lazy flush is lost silently — the durability ADR-0026 §7 fought for and won.

**The clause is restated, not narrowed: the runner flushes a submitted write at the latest at the last moment the host guarantees it can run code.** On a hosted target that moment is exit. In a tab it is the transition to `hidden`, which is the last transition the HTML Standard guarantees will run script before a discard.

The bound now names a **class of moment** rather than a mechanism, which is what preserves ADR-0026 §5's freedom.

**Flush-on-submit was weighed and rejected**, and it is the simpler sentence: every `write` reaches the device before it returns, and the bound disappears. It gives up exactly what §5 bought — the freedom to batch — and prices every `write` at a device round-trip, which on IndexedDB is a transaction per call. A game that writes a slot every frame would then pay per frame for a durability guarantee it did not ask for. The freedom stays; the bound is restated in terms every host can satisfy.

ADR-0026 §7's reload obligation is unaffected: *flush pending writes before the dylib swap* names a moment the runner controls on every target.

## 3. P2 reads as abnormal termination by the host's convention

#19's **P2** and #18 §4 fix that a faulted release program **exits non-zero**. A tab has no exit status, no waiting parent, and nothing in the HTML Standard that reports a numeric status. This is the **only** #19 property whose literal observable is unmeetable in a browser.

**The clause reads as: the program terminates abnormally by the host's convention.** P2 stays binary, and #18 §4's mode-independence is intact.

*By the host's convention* is not a blank cheque, and the browser's convention is stated here so no backend fills it in: **the trap surfaces as an exception through the JavaScript host, and the frame entry does not return.** That is observable, it is what the wasm JS API specifies, and it is what a conformance harness checks in a tab.

**Scoping the clause to hosted targets was rejected.** It creates the documented-deviation tier #19 posture 3 refused — a property that some conforming implementations are excused from — and the excuse would be permanent, since no future browser will grow an exit code.

**The claim lives in three places, not two.** #79 listed #19 P2 and #18 §4. **ADR-0018 §11 states it a third time** — *Release still exits non-zero* — in a document about the diagnostic stream, where no reader of #19 would look for it. This is the second time an audit has found a corpus claim written in three sites where the ticket assumed two ([#70](https://github.com/adamico/ludo/issues/70) found the future tense repeated in the glossary). All three are transcribed together.

## 4. The call chain's price on wasm is per call at runtime, and the record shows a native price

#18 §9 prices the fault report's call chain as *binary size and … frame-pointer omission*. **The guarantee is right and stands.** The price is a native price presented as the price.

WebAssembly has **no instruction that reads its own call stack**, and `Error.prototype.stack` is not standardised in ECMAScript. A web backend therefore emits a **shadow stack: a push and a pop per call, at runtime, in release** — because §9 made the chain mode-independent on purpose, and a release build cannot opt out.

**Only the cost sentence is amended, and it names both target classes.** The amendment states the *category* explicitly — the wasm price is **per call at runtime**, not per binary — because that is the error a reader would not re-derive. Binary size and a shadow stack are not two sizes of the same cost; one is paid once by the artifact and the other is paid by every call the program makes.

Nothing follows for the guarantee. §9's chain is the agent's input under #4's non-local-reasoning finding, and a runtime price on one target does not buy it back.

## 5. `persist` is runner-controlled memory that outlives the swapped code

#17 §1's *`persist` memory belongs to the runner's process and is never touched by the swap* is false on the web. **Linear memory is the only thing ludo code can address**, so `persist` sits inside the memory the incoming build is instantiated against. There is no second region for the runner to hold it in.

**The guarantee survives.** A `WebAssembly.Memory` is explicitly shareable across instances, so the incoming build instantiates against the same memory and the bytes are still there. What fails is the description, and the description was doing work: it told a reader the swap could not reach `persist`, which on this target is exactly backwards.

**The clause is restated: `persist` lives in runner-controlled memory that outlives the swapped code.** True on a native target, where the runner holds it outside the dylib. True in a tab, where the runner holds the `Memory` object and hands it to each instance.

## 6. A build whose active data segments overlap live `persist` is non-conforming

The false description hid a real hazard, and this section is the load-bearing half of §5.

Core WebAssembly instantiation copies **active data segments into the imported memory before any code runs**. A new build whose constants are active segments at the old offsets therefore **overwrites live `persist` silently** — before the first frame entry, with no trap, no diagnostic and no way for the program to detect it.

**This is a MUST, not a hazard note: a conforming implementation's reload path must not place a new build's initialised data over the live `persist` region.** On the wasm path the discharge is passive data segments, copied under the build's control after instantiation rather than by it.

A hazard note was the alternative and protects nobody. The test is [ADR-0040](0040-the-runner-reopens-on-the-last-display-and-names-none.md) §4's — **a MUST is earned by the failure it prevents** — and this one prevents a failure that is silent, that corrupts the exact state the whole reload story rests on, and that no program can observe or defend against. ADR-0040 declined to mint a MUST because a display MUST would have prevented nothing; the same test mints one here.

The condition is a constraint on **the swap**, so it is stated where the swap is named, not in a browser footnote: the swap is a #17 mechanism and a quiescence citer under [ADR-0024](0024-concurrency-is-a-non-goal-and-quiescence-is-a-state-predicate.md) §2.

## 7. Re-entrancy is by task, not by thread

ADR-0024 §5 added *ludo code may only be entered from the frame thread*. In a tab that clause **names a hazard that does not exist and misses the one that does**. There is no second thread that can call in. There are many other places a browser can call in from: an event listener, a promise reaction, an IndexedDB `success` handler, a `requestAnimationFrame` callback the runner did not schedule.

**The clause is widened: ludo code may only be entered from the frame task.** This is a **new rule, not a repair** — "thread" is under-inclusive, so restating it as "task" forbids entries the old clause permitted by silence. It is recorded as an amendment to ADR-0024 §5 accordingly, and §5's C-thread case survives unchanged as an instance of it.

The IndexedDB case is why this is not academic, and it is a live route between two decisions this map already made:

1. An uncaught exception in an IDB event handler **aborts the transaction**.
2. So a #18 fault raised inside ludo code entered from an IDB handler **discards a submitted write**.
3. Which defeats [ADR-0026](0026-a-program-writes-bytes-to-declared-storage-slots.md) §7's *a #18 fault does not discard a submitted write* — by exactly the route that clause exists to close.

Under the widened rule the runner may not enter ludo from that handler at all, and the composition never arises. A thread-shaped clause would have permitted it on every reading.

## 8. Tab discard defeats the rescue in practice, and this is a scope sentence

ADR-0024 §3's *a faulted program is quiescent forever, which is what lets reload rescue a faulted program* **holds as a theorem** and is defeated in practice by tab discard.

A faulted ludo program in a tab is in the most discardable state a page can be in: hidden the moment the developer switches to their editor, doing no work, holding a large linear memory. The user agent may reclaim it, and then there is nothing to reload into.

**This is a scope sentence, not a change**, and the sentence names why it is unfixable rather than merely noting it: **the discard is described by no specification and fires no event**, so no implementation can flush around it, warn before it, or detect that it happened. The theorem is not weakened — a faulted program *is* quiescent forever — but the rescue it enables is a dev-loop affordance on a host that may take the page away first.

## 9. No new experience-contract property, and the reason is stated

§7 mints a rule. §6 mints a MUST. **Neither becomes a #19 property**, and per [ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md) §8 the reason is recorded rather than left as an omission.

**§7 cannot be tested.** A property would have to prove a negative over **every callback a host can schedule** — an open set that grows with the host platform, not with the spec. A harness can check the cases it thinks of and can never establish the rule. It binds spec authorship and implementation review, which is where [ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md)'s admission test also sits.

**§6 is covered by what already exists.** P1's state-preserving reload fails if a build overwrites `persist`, so the failure this MUST prevents is already observable through a property #19 has. A second property would test the mechanism rather than the observable.

§3 is the only item that touches the conformance surface, and it **changes what P2 can be failed on** without adding or removing a property. #19 gains an amendment comment, not a P16.

## Three lenses

**Simplicity — untouched.** Nothing here appears in the language a beginner writes. §7 constrains the runner, §6 constrains the build, and the rest is prose.

**Robustness — the two rules are both silent-failure closures.** §6 stops a corruption no program can see; §7 stops a composition in which a fault eats a durable write. Both are the class of bug this map prefers to make impossible rather than diagnosable, because neither has a diagnosis site.

**Agent-friendliness — this is the lens that argued for one ADR.** #4's finding is that agents fail at non-local reasoning. Six corrections in four documents is the non-local shape; one document that states the cause is the local one. §1's pattern is the sentence an agent needs, and it exists in exactly one place.

**No lens conflict.**

## Budget

**Zero #24 delta.** No keyword, operator, function, type or root name is added or removed by any of the six items. §6 and §7 are obligations on the runner and the build; the program's surface is untouched. This is the fourth consecutive zero.

## Amendments this ADR makes

- **ADR-0026** — §5's *at the latest, at exit* is **restated** as the last moment the host guarantees it can run code (§2). §7's reload obligation is unaffected, and §7's fault clause is **defended** by §7 of this ADR, which closes the IDB route that would have broken it.
- **ADR-0024** — §5's *frame thread* is **widened to frame task** (§7), a new rule rather than a repair. §3's rescue story gains a **scope sentence**: the theorem holds and tab discard defeats it in practice, unfixably (§8).
- **ADR-0018** — §11's *Release still exits non-zero* is **restated** as abnormal termination by the host's convention (§3), the third site of a claim #79 recorded as living in two.
- **Issue #17** — §1's account of where `persist` lives is **corrected** (§5), and the **passive-segment MUST** is added where the swap is named (§6).
- **Issue #18** — §4's exit observable is **restated** with §3; §9's **cost sentence** is amended to name both target classes and the per-call category (§4). §9's guarantee is untouched.
- **Issue #19** — **P2's failure condition changes** (§3). The property stays binary, mode-independent and universal; what it is failed on is now the host's abnormal-termination convention. No property is added (§9).
