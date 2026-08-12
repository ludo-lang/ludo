# Project structure

Where the project's repositories live, what each holds, and when each comes into
existence.

This is **operational, not a language decision**. It is deliberately not an ADR:
nothing in the spec depends on GitHub, and an ADR about repository layout would
sit oddly in a series that is otherwise grammar, memory and conformance. It is
written down because three accepted ADRs promise that something "lives in its own
repo" and bless it **by reference**, and until now no document said which repo.

The promises this discharges:

- **ADR-0001** — "The platform layer lives in its own repo, versioned
  independently and blessed by reference."
- **ADR-0004** — the platform layer and the WebGPU-model renderer "stay ours,
  blessed by reference in their own repo, on their own clock, **unmandated**."
- **ADR-0006** — the audio engine tier gets "the same treatment the renderer gets
  under ADR-0001 and ADR-0004: ours, own repository, blessed by reference,
  **unmandated**."
- Issue #49 — the bootstrap compiler (ADR-0013) gets its own repo, outside the
  spec tracker.

## The organisation

**https://github.com/ludo-lang**

An organisation rather than a second personal account: a separate account gives a
second login, no shared ownership and no members, which is strictly worse for the
same isolation. The repository count already justifies one — the platform layer,
the renderer, the audio engine and the compiler are four siblings of one project.

The handle is `ludo-lang` because `github.com/ludo` belongs to a real user since
2008 and `ludolang` sits on a dormant empty account reclaimable only through
GitHub's name-squatting process. `ludo-lang` is also the conventional shape:
`rust-lang`, `ziglang`, `odin-lang`.

Note the discoverability cost this exposed, which belongs to issue #34 rather than
here: *ludo* is a board game (the Pachisi variant) and a common given name, so
domain availability and search discoverability are one problem, not two.

## Naming

**Bare nouns.** The organisation is the namespace, so `ludo-lang/ludo-renderer`
stutters. Precedent is unanimous: `rust-lang/{rust,cargo,rfcs}`,
`ziglang/{zig,zig-bootstrap}`, `golang/{go,tools}`.

The known objection — a cloned directory called `renderer` says nothing about
which project it belongs to — is real, cheap to fix at clone time, and was
overruled by every project named above.

**`ludo-lang/ludo` is the toolchain, not the spec.** It is the URL a visitor
guesses, and what they want is the thing that compiles their game, not the ADR
archive. Same split as `rust-lang/rust` (compiler) versus `rust-lang/rfcs`
(design work).

## A repo exists when it has a first commit

No placeholder repositories. Four of the repositories below cannot hold anything
until a compiler exists — ADR-0001 says so directly ("nothing here is built yet…
the platform layer is written in ludo, so it cannot exist before the compiler
does").

An ADR may therefore name an intended URL that currently 404s. **A 404 is more
honest than an empty repo**, which advertises a surface that does not exist.

The organisation profile (`.github`) waits until there are at least three real
repositories to profile.

## The repositories

### `ludo-lang/ludo` — the toolchain

Compiler and runner. The bootstrap prototype (ADR-0013, issue #49) is its early
history, and the keeper grows out of it rather than starting clean — "the
prototype's frontend carries forward" is exactly the claim a fresh repository
makes you re-litigate, and ADR-0013 rejected the minimal-throwaway
(Rust-from-OCaml) move for fighting it. *Throwaway* describes the interpreter and
the driver, not the repository.

The frontend is **library-shaped from the first commit** — nothing inside reads
`argv`, writes to stderr or calls `exit`, and diagnostics are returned as values.
Issue #22's oracle is an in-process caller, not a subprocess.

The **runner ships here too**, not in a repo of its own: `ludo build` and
`ludo run` are one CLI to the user, sub-second reload needs the compiler's
incremental frontend, and splitting them means two versioned artifacts that must
agree exactly on issue #19's reload contract.

*Exists now.*

### `ludo-lang/spec` — the specification

Today's `adamico/ludo`: the map, the ADRs, `CONTEXT.md`, the research corpus.
Transferred **and renamed**, since `ludo` goes to the toolchain.

*Trigger: when the map is complete* — every ticket closed, not merely an empty
frontier. The transfer churns every in-repo cross-link and a map body of roughly
130KB of `adamico/ludo` URLs, so it happens once, against a finished document.
In-repo links get rewritten in a single pass; issue links are left to GitHub's
redirects.

### `ludo-lang/stdlib` — the `$.` root

ADR-0014 rejected a baked-in stdlib: `$.` is an ordinary library directory
claiming the reserved root, resolved by the same mechanism as any third-party
library, at the stated cost that the bootstrap must resolve modules before it can
compile `$.` at all. So the stdlib is a repository, and it is the **first real
consumer of ADR-0014's own mechanism** — the best available test that a library
really is just a directory.

**The mandated facade lives here**, not in `platform` or `renderer`.
`$.graphics` (ADR-0009) and `$.audio` (ADR-0007) are conformance surface
implemented in ludo on top of an *unmandated* platform layer and renderer.
Shipping them from an unmandated repository would version a conformance-critical
surface on the renderer's clock, inverting the blessing relationship.

*Trigger: a compiler that can resolve modules.*

### `ludo-lang/platform` — the platform layer

ADR-0001's own repo: window, input, audio device, GPU device and swapchain,
across the backend matrix. ADR-0006's eight rules bind its API.

*Trigger: a compiler — it is written in ludo.*

### `ludo-lang/renderer` — the WebGPU-model renderer

ADR-0004's delegated tier. Ours, blessed by reference, **unmandated**.

*Trigger: a compiler.*

### `ludo-lang/audio` — the audio engine tier

Synthesizer, mod-slot effects chain, MML, mixer buses, streamer. Ruled out of the
spec by issue #32 and given the renderer's exact treatment: ours, own repository,
blessed by reference, **unmandated**. Reachable from conforming ground, exactly as
the renderer is beneath the drawing facade.

*Trigger: a compiler.*

## Why three tiers are three repositories

ADR-0004's wording ("their own repo") left one-or-each open. Three, because the
tiers have different **mandate status** and different **clocks**: the platform
layer carries the conformance-critical backend matrix, the renderer is
blessed-but-unmandated, and the audio engine was pushed further out still. One
repository would put an unmandated DSP stack's CI beside the backend matrix —
precisely the concentration ADR-0001 evicted the platform layer from
`adamico/ludo` to avoid.

The boundary is already hard regardless: ADR-0006 R2 makes the backend seam one
struct of non-capturing function pointers.

## Not repositories yet

- **Conformance harness and corpus** — the map's out-of-scope line splits this:
  the spec owes the assertions (issue #19 writes each property in
  precondition/action/observable form, plus a three-size reference set defined by
  shape), the follow-on effort owes the harness and the corpus. It cannot be
  written before there is a compiler to run it against. Note issue #49's
  completion test is a *spec* test done by hand, and is not this.
- **Build system and package manager** — ruled out of the spec by ADR-0014, which
  nonetheless owes that effort a contract (§9). Issue #30 (scaffold generator) and
  issue #31 (sharing libraries, snippets, templates) are very likely the same
  repository's problem rather than three.
- **Website and API docs** — issues #34 and #36, both un-designed. Creating an
  empty `website` repo would decide by accident: #34's answer might be a docs site
  generated from the spec repo, and #36's might be a tool that dictates layout.

## Homeless issues

Issues #30, #31, #35, #36 and #37 are tooling, not language design; they survived
the map by being outside its scope. They stay on the spec tracker and move to
their own repository's tracker as that repository is created — not before.

A dedicated `meta` tracker for homeless issues sounds tidy and reliably becomes a
graveyard. While the count is five, the spec repo is a fine holding pen.
