# Project structure

Where the project's code and documents live, and what is allowed to become a
second repository later.

This is **operational, not a language decision**. It is deliberately not an ADR:
nothing in the spec depends on GitHub, and an ADR about repository layout would
sit oddly in a series that is otherwise grammar, memory and conformance. It is
written down because a reader needs one file that answers *is there a `stdlib`
repo?* with a plain no.

## One repository

**[`ludo-lang/ludo`](https://github.com/ludo-lang/ludo) holds everything**:
`docs/adr/`, `docs/spec/`, `docs/research/`, `CONTEXT.md`, `docs/agents/`, the C
bootstrap under `src/`, and one issue tracker.
[ADR-0046](adr/0046-the-repository-becomes-a-public-monorepo-in-the-ludo-lang-org.md)
§3 settled this, and
[#95](https://github.com/ludo-lang/ludo/issues/95) confirmed it against the four
own-repo promises that predate it. There is no `ludo-lang/spec`, no
`ludo-lang/stdlib`, no `ludo-lang/platform` and no `ludo-lang/renderer`, and the
earlier plan to rename this repository is dead: `ludo-lang/ludo` is the
toolchain **and** the spec.

**Why the earlier multi-repo chart collapsed.** It rested on two arguments, and
both were paid off elsewhere:

- **Scale.** ADR-0001 kept the platform layer out of the design tracker so that
  *"an eighteen-backend conformance matrix"* could not dominate it. There is no
  matrix:
  [ADR-0056](adr/0056-the-platform-layer-is-delegated-to-a-third-party-library.md)
  delegates the platform layer to a third-party library and
  [ADR-0057](adr/0057-the-delegated-platform-layer-is-sdl3.md) makes it SDL3.
  What is left to host is a thin facade over somebody else's backends.
- **Mandate status.** ADR-0004 and ADR-0006 blessed their tiers *unmandated*, and
  a separate repository was how that status was signalled. Of those tiers, the
  platform layer is no longer ours, and the renderer is now a facade implementing
  **normative** spec ch6 §4 over SDL_GPU's ceiling (ADR-0057). The distinction
  that survives is carried by directory, not by repository: **`docs/spec/` is
  normative, `src/` is one implementation of it.**

One unmandated tier is genuinely left, and it keeps its promise — see below.

## The organisation

**https://github.com/ludo-lang**

An organisation rather than a second personal account: a separate account gives a
second login, no shared ownership and no members, which is strictly worse for the
same isolation. It stands on the shared-ownership and namespace argument alone
now that the repository count no longer supplies one.

The handle is `ludo-lang` because `github.com/ludo` belongs to a real user since
2008 and `ludolang` sits on a dormant empty account reclaimable only through
GitHub's name-squatting process. `ludo-lang` is also the conventional shape:
`rust-lang`, `ziglang`, `odin-lang`.

Note the discoverability cost this exposed, which belongs to issue #34 rather than
here: *ludo* is a board game (the Pachisi variant) and a common given name, so
domain availability and search discoverability are one problem, not two.

**Naming, should a second repository ever exist: bare nouns.** The organisation is
the namespace, so `ludo-lang/ludo-renderer` stutters. Precedent is unanimous:
`rust-lang/{rust,cargo,rfcs}`, `ziglang/{zig,zig-bootstrap}`, `golang/{go,tools}`.

## A repo exists when it has a first commit

No placeholder repositories. An ADR may name an intended URL that currently 404s.
**A 404 is more honest than an empty repo**, which advertises a surface that does
not exist.

This rule is what makes the section below cost nothing.

## The one promise still standing

### `ludo-lang/audio` — the audio engine tier

[ADR-0006](adr/0006-forbidden-and-required-shapes-of-the-platform-layer-api.md)'s synthesizer,
mod-slot effects chain, MML, mixer buses and streamer: ours, own repository,
blessed by reference, **unmandated**. Ruled out of the spec by issue
[#32](https://github.com/ludo-lang/ludo/issues/32), and out of scope on
[map #129](https://github.com/ludo-lang/ludo/issues/129).

This is the last tier whose *unmandated* status a repository boundary would
actually signal, and the promise is preserved rather than reversed — **dormant,
not live**. It has no first commit, nothing on the route to a playable reference
program needs it, and under the rule above a dormant promise costs nothing until
someone writes the first line of it. Whoever does should re-ask the question
rather than assume the answer, since the tier arrives long after the facade it
sits beneath.

`$.audio` in the spec is the **mandated facade** (ch6 §5), not this tier, and it
is implemented in `src/` like every other normative surface.

## Not repositories

- **The keeper compiler.** Issue [#49](https://github.com/ludo-lang/ludo/issues/49)
  excluded the keeper from *its own* scope and an earlier chart gave it a repo;
  #95 put it here. ADR-0020 keeps one host language so the prototype's frontend
  carries forward, and the frontend reads `docs/spec/grammar.ebnf` by **relative
  path** — one working tree, one path, no vendoring. Both break at a repository
  boundary. *What* carries forward from the prototype is a separate and still-open
  question.
- **The platform layer and the renderer.** SDL3 and a facade over it (ADR-0056,
  ADR-0057). Third-party code is a dependency, not a repository of ours.
- **The stdlib.** ADR-0014 makes `$.` an ordinary library directory claiming the
  reserved root, resolved by the same mechanism as any third-party library — which
  makes it the first real consumer of that mechanism, and a directory in this tree
  is a perfectly good test of it. It is normative (spec ch6), so no
  mandate-status argument pulls it out either.
- **Conformance harness and corpus** — the spec owes the assertions (issue #19),
  the follow-on effort owes the harness and the corpus. It cannot be written
  before there is a compiler to run it against. Issue #49's completion test is a
  *spec* test done by hand, and is not this.
- **Build system and package manager** — ruled out of the spec by ADR-0014, which
  nonetheless owes that effort a contract (§9). Issues #30 and #31 are very likely
  one repository's problem rather than three.
- **Website and API docs** — issues #34 and #36, both un-designed. Creating an
  empty `website` repo would decide by accident.

## Homeless issues

Issues #30, #31, #35, #36 and #37 are tooling, not language design; they survived
map #1 by being outside its scope. They stay on this tracker and move to their own
repository's tracker if and when such a repository is created — not before.

A dedicated `meta` tracker for homeless issues sounds tidy and reliably becomes a
graveyard. While the count is five, this repo is a fine holding pen.
