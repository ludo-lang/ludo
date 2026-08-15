---
status: accepted
---

> **Amended by [ADR-0041](0041-the-record-stops-assuming-a-process-and-re-entrancy-is-by-task.md):**
> §5's flush bound is restated as the last moment the host guarantees it can run
> code, since a discarded browser tab has no exit hook.

> **Absorbed by [spec ch6 §8](../spec/06-stdlib.md):** the chapter is normative; this ADR
> keeps the argument.

# A program writes bytes to declared storage slots

Issue #64 recorded a hole nothing in the map had noticed: **nothing under `$.`
writes a byte.** A ludo game could not save a high score, let alone a save slot.
ADR-0006 R7's file IO lives below `$.`, where only implementations reach.
ADR-0015 made assets declared, read-only, resolved before top level — one
direction. `persist` (#26) is cross-frame state that dies with the process.

The answer is a **program-visible byte sink in `$.`**, deliberately minimal:
`extern storage <name>` declares a slot, `$.storage.read` and `$.storage.write`
move `[]u8` in and out, and no file is ever named.

## 1. The destination's "asset IO" was not a decision

#64 read the Destination's stdlib scope line — *"math/vec/matrix, allocators,
containers, strings, asset IO"* — as evidence that reading had been weighed
against writing and chosen. It had not. The line is a pre-design scope sketch
written before any of this was designed, and ADR-0015 reasoned about **declared
assets**, a different subject from game state. The line is amended by this
decision, not overruled by it.

Recorded because the same trap is available to any future ticket: a summary
sentence in the Destination is not a resolution, and the resolutions are the only
things that bind.

## 2. The runner cannot own game data, and the reason is meaning

The tempting cheap answer was to generalise ADR-0025 §6 — the runner already
persists window mode across relaunch without any program-visible write surface,
so let it persist everything.

It does not generalise, and the boundary is sharp: **runner-owned state works
only where the runner knows what the state means.** Fullscreen qualifies because
the spec defines fullscreen and the runner is the thing that implements it; it
can store and restore a mode without ever consulting the program. "Level 7, 3
lives, 12400 points" has no spec-defined meaning. The runner can hold bytes on
the program's behalf, but it cannot produce them, interpret them, or restore them
without the program handing them over and reading them back — **and that handover
is exactly the program-visible write surface** the alternative claimed to avoid.

So a settings concept was never a substitute for this decision. §7 disposes of it
as an addition, too.

## 3. Slots are declared, and the declaration costs no keyword

A slot is declared in the same form as an asset, minus the source string:

```ludo
extern storage save1
extern storage save2
```

`extern` is reused and `storage` occupies the kind-noun slot `image` occupies in
ADR-0015, so **the core grammar gains nothing** (§9). Names are fixed at compile
time. There is no runtime key construction, no path, no directory, no string
lookup — a filesystem in disguise was the one shape considered and refused
outright, because it reintroduces every question the spec has spent five ADRs
refusing to answer, and because #5's Tier 1 wants a name an agent cannot get
wrong.

Multi-slot games declare multiple slots. Programs that want one, declare one.

## 4. The calls, and the payload is bytes

```
$.storage.write(save1, bytes) -> !
$.storage.read(save1)         -> ?[]u8
```

Two functions under one new root name. UFCS (`save1.write(bytes)`) resolves per
#11, but the spec teaches the qualified spelling.

**The payload is `[]u8` and the spec blesses no serializer.** ADR-0025's economy
argument applies: a serializer is a subsystem — versioning, schema evolution,
endianness — and ADR-0025 §5 already made every struct C-compatible with fully
specified layout, so a beginner can memcpy a struct and a veteran can hand-roll a
format. Serialization is a library's job under ADR-0014.

`?[]u8` carries first-launch for free through #9: a slot never written reads
`nil`, and the check is the bind. No `exists` predicate is added.

## 5. Nothing polls, because the read is resolved and the write is submitted

ADR-0006 R7 made file IO request-and-poll **below** `$.`. The live question was
whether that shape is forced upward onto the program surface. It is not, and
polling in the frame loop would have been the ugliest thing this decision could
add.

- **The read resolves before top level runs**, exactly as ADR-0015 resolves
  assets. There is no loading state, so there is nothing to poll.
- **The write is submit-and-forget.** It never blocks and returns no completion.
  The runner flushes it — at the latest, at exit.

Both halves are async-tolerant, which is what keeps #28's negative obligation
satisfied: a web backend uses **IndexedDB** (async-only, hundreds of MB) with no
async ever reaching the program. `localStorage` was the reason an earlier draft
of this decision feared a capacity ceiling; under this shape it is not the
backend a browser implementation would pick, and the ceiling is not real. R7 is
untouched and stays a backend shape.

## 6. One MiB per slot, and the anchor is stated

An implementation **MUST** accept a write of at least **1 MiB** per slot. A
larger write **MAY** be refused, reported as an error value, and **MUST NOT** be
silently truncated.

The number is anchored rather than asserted: the largest artifact a program built
on this facade can plausibly save is a raw tile grid, and 512×512 tiles at 4
bytes is exactly 1 MiB. #12 ruled out scene graphs, asset pipelines and streaming
worlds, so the language gives a program no way to build anything structurally
larger. Above that line a program is saving data it did not get from here.

A tighter 64 KiB cap was seriously considered — it covers every counter, flag and
inventory save, and says a 1 MiB save is a design smell the spec should not
bless. It lost because a beginner memcpy-ing their world array is precisely the
felt-loop case (#16), and hitting a spec cap there is the kind of wall this
language exists not to have.

Conformance test: write 1 MiB, relaunch, read it back identical. This lives in
**core (headless)** conformance under ADR-0004 — persistence needs no window, no
GPU and no audio device, and headless is where you would want to test it.

## 7. What can fail, and what the program never learns

Two failure classes, split by whether the program could possibly act:

- **Oversize is knowable at the call**, so `write` returns an error value per
  #10's model, at the call site.
- **IO failure — disk full, quota denied, storage evicted — happens after the
  call returns**, and the program never learns of it. The **runner** reports it
  on the ADR-0018 diagnostic stream.

No status channel, no "did my last write land" query. A program cannot handle
disk-full meaningfully, and the only way to expose it is a poll next frame —
which is the shape §5 exists to keep out.

## 8. Reload, quiescence, and faults

**Slots resolve once per process and are never re-resolved.** An earlier draft
had reload re-read them, on the asset analogy; it is wrong. Reload preserves the
live game, and handing a running program stale or foreign state mid-session is
not what a developer editing a source file asked for. The asset analogy governs
**timing only** — resolved before top level — and stops there.

Consequently:

- Read-after-write within a process returns what was written.
- A #17 reload preserves resolved contents exactly as it preserves `persist`.
  The runner's only reload obligation is to **flush pending writes before the
  dylib swap**.
- **ADR-0024's quiescence predicate is amended**: live state is `persist`, plus
  declared assets, plus **resolved storage contents**. Stated explicitly rather
  than left implied.

**A #18 fault does not discard a submitted write, including one submitted in the
faulting frame.** The frame is abandoned, but the submit already happened.
Discarding it would make save durability depend on unrelated code later in the
same frame — a program that saves and then divides by zero would lose the save
for a reason with no relation to saving. The alternative (mirror "the frame is
abandoned" all the way down) is coherent and was weighed; durability won.

## 9. No settings concept ships, and ADR-0025 §6 survives

#64 flagged the adjacent generalisation #62 deliberately did not take: a general
**settings** concept — resolution, volume, key bindings surviving relaunch — of
which fullscreen would be one instance. It does not ship, and ADR-0025 §6 stands
untouched.

The split earns its place on §2's boundary. Fullscreen is runner-owned because
the runner implements it. Volume and key bindings are **program-defined
behaviour** — the runner has no idea what a binding means to your game — so a
program that wants them persistent declares a slot and writes it, with exactly
the surface above. Two mechanisms would be one too many; there is no
`$.settings`.

## 10. The three lenses, and the #24 delta

- **Simplicity**: a Lua user recognises "save a file" instantly; what they must
  unlearn is naming the file. Two functions and a declaration form they have
  already met on assets.
- **Robustness**: compile-time-fixed slot names delete the whole class of
  runtime path bugs; `?[]u8` makes first-launch unmissable; no silent truncation.
- **Agent-friendliness**: Tier 1 clean — the slot name is declared and local
  (R1), the effect is in the signature, one spelling per entity, no dialect
  variance. #4's finding that agents fail at *naming things that exist* is
  answered by there being nothing to name but a declared identifier.

**#24 delta.** Core grammar: **0**. Type sublanguage: **0**. Stdlib root-name
companion count: **+1** (`$.storage`), plus two functions and one `extern` kind
noun. No sugar, so the tiered payment rule does not bite. Among the cheapest
facades added.
