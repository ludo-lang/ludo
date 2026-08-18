---
status: accepted
---

# The repository becomes a public monorepo in the ludo-lang org, under Zlib

Not a ticket. This ADR was written from a session that opened as *chapter 1 is complete, we
should introduce a workflow to sync the work with one of the `ludo-lang` org repos*, and ended
by deleting the workflow rather than designing it. The premise was that the spec and the
bootstrap compiler would live in different repositories and therefore need machinery between
them. They will not, so there is nothing to sync.

[ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §5
scheduled this moment: *"`ludo-lang/ludo` gets its first commit the day `grammar.ebnf` is
done."* [`docs/spec/grammar.ebnf`](../spec/grammar.ebnf) exists as of
[#85](https://github.com/adamico/ludo/issues/85). This ADR decides what that repository
actually is.

At the time of writing: **45 ADRs, 86 issues (19 open), one wayfinder map
([#1](https://github.com/adamico/ludo/issues/1)), zero lines of code.** The `ludo-lang`
organisation exists and holds **no repositories**. `adamico/ludo` is **private**, has no
`LICENSE`, and has no `.github/` — no CI has ever run against this project.

## 1. The sync workflow was an artifact of a split nobody had chosen

ADR-0044 §5 names `ludo-lang/ludo` as the C bootstrap's home while the spec is written in
`adamico/ludo`, and in the same breath requires that *"the C frontend's test suite consumes
the same file"* — `grammar.ebnf`, normatively. Two repositories, one normative file, therefore
a pinned copy, a version, a bump, and a rule about when to bump it.

That machinery is real and it has a real cost, and it exists only because of the repository
boundary. Remove the boundary and the frontend reads `docs/spec/grammar.ebnf` by relative
path. **The correct answer to "how should these two repositories sync" was that there should
not be two repositories.**

This is worth recording in its own right: the session spent its first round designing a
release-cutting workflow for the split, and none of that survived the question of whether the
split should exist. The requirement — *the test suite consumes the normative file* — was
satisfied more cheaply by a path than by any version scheme considered.

## 2. Nobody runs a language across two accounts

The immediate complaint was ergonomic: *managing two separate repos on two different accounts
is a hassle.* Checked against the practice of established languages, and the result is
unanimous — every one of these lives wholly under a single organisation, never straddling a
personal account and an org:

| Language | Compiler | Spec / reference | Design record |
|---|---|---|---|
| Go | `golang/go` | **same repo** — `doc/go_spec.html` | issues + `golang/proposal` |
| Zig | `ziglang/zig` | **same repo** — `doc/langref.html.in` | issues, same repo |
| Odin | `odin-lang/Odin` | same repo; no formal spec | `PROPOSAL-PROCESS.md`, same repo |
| Rust | `rust-lang/rust` | `rust-lang/reference`, `rust-lang/spec` — **vendored back as a git submodule** (`src/doc/reference` is a gitlink) | `rust-lang/rfcs` |
| Kotlin | `JetBrains/kotlin` | `Kotlin/kotlin-spec` | KEEP |
| Swift | `swiftlang/swift` | `swiftlang/swift-book` | `swiftlang/swift-evolution` |

Two findings, and only one of them is the obvious one.

**Spec-with-compiler versus spec-apart is genuinely 50/50.** Go and Zig keep the normative
document inside the compiler repository; Rust and Kotlin keep it out. There is no industry
answer to inherit, so it is decided below on this project's own facts.

**The split that mature languages reliably do make is design-record versus implementation** —
`rust-lang/rfcs`, `swiftlang/swift-evolution`, `dotnet/csharplang` all sit apart from the
compiler. That line maps precisely onto this repository's 45 ADRs and map #1 versus the C
bootstrap, which makes it the tempting split to copy.

It is not copied, for the reason in §3.

## 3. One repository, because the split is a scale artifact

**`ludo-lang/ludo` holds everything**: `docs/adr/`, `docs/spec/`, `CONTEXT.md`,
`docs/agents/`, the C bootstrap, and one issue tracker carrying both design tickets and
compiler bugs. The Go and Zig model.

The RFC-repo split exists to give an external contributor a design surface that is not the
compiler's issue tracker. That is a governance problem, and this project does not have it:
one author plus agents, no external contributors, no implementation. **Rust did not start
split** — `rfcs` separated as the contributor base grew, which is the same move available
here later and for the same reason. Splitting now would be paying a governance cost against a
governance need that does not exist.

Two arguments specific to this repository sit on top of the scale one.

- **The tracker's graph is load-bearing and cannot cross a repository boundary.** Map #1's
  child tickets are GitHub sub-issues and its blocking edges are GitHub's native issue
  dependencies (`docs/agents/issue-tracker.md`). Both are **same-repository relations**. A
  design-versus-implementation split forces a choice about which side the tracker lives on and
  severs the graph on the other. `/wayfinder` runs on that graph.
- **ADR-0044 §5's coupling degrades to a path.** The frontend's conformance input stops being
  a versioned dependency with a bump policy and becomes a file in the same working tree. A
  grammar edit and the parser change it implies land in one commit, and the test suite is
  green or it is not — which is the fastest possible hole-finder, and hole-finding is the
  entire justification for building the parser now.

**The cost, accepted:** one tracker holds both a design ticket and a segfault. That is a
labelling problem (§8), not a topology one, and it is cheaper than a severed dependency graph.

**Rejected: `ludo-lang/ludo` for compiler-and-spec plus `ludo-lang/design` for ADRs, map and
issues.** This is the Rust shape inside one org, and it answers the account complaint. It is
rejected because it severs the tracker graph — the design side would keep it and the compiler
side would start a second one — while buying only a separation this project has no reader for.

**Rejected: spec vendored as a submodule**, the literal `rust-lang/reference` arrangement.
Same objection, plus a submodule bump standing between a grammar edit and the test that
validates it.

## 4. The repository is transferred, not recreated

`adamico/ludo` is **transferred** to the `ludo-lang` organisation as `ludo-lang/ludo`. It is
not re-created and pushed.

GitHub's repository transfer preserves issue numbers, comments, labels, sub-issue links and
issue dependencies, and installs a permanent redirect from every previous URL. Each of those
matters concretely here:

- **The tracker's graph survives.** §3's argument is only honoured if map #1's sub-issue and
  dependency edges arrive intact, and they do — precisely because every issue moves at once,
  making the relations still same-repository on the far side. Issue-by-issue migration would
  shred them.
- **The corpus's links keep resolving.** These 45 ADRs cite issues as absolute
  `https://github.com/adamico/ludo/...` URLs, and so does map #1, hundreds of times over. The
  redirect means **not one of them has to be rewritten**, and none are rewritten by this ADR.
  Links authored from here on use the new location; the old ones are left alone as the
  historical record they are, exactly as ADR prose is left alone under
  `docs/agents/domain.md`.
- **Issue numbers are stable**, so `#85` continues to mean chapter 1, in the ADRs, in the
  coverage files, and in map #1's Decisions-so-far.

**Rejected: a fresh repository with a squashed or replayed history.** It breaks map #1's
children, every dependency edge, and every cross-link in the corpus, in exchange for a history
cosmetic. The corpus's dense self-citation — the thing ADR-0044 §5 identified as this
project's defining hazard — is exactly what makes link preservation non-negotiable.

## 5. Public, because on a free org that is where the features are

The repository is **made public as part of the transfer**.

The working assumption was that a free GitHub organisation mandates public repositories. It
does not: **GitHub Free for organisations includes unlimited private repositories** — "with a
limited feature set." The limits are the decision:

| | Private, Free org | Public, Free org |
|---|---|---|
| Branch protection / rulesets | **unavailable** — restrictions apply "in public repositories owned by a GitHub Free organization, and in all repositories" on Team | available |
| GitHub Actions | 2,000 minutes/month, metered, overage billed | **unlimited** on standard runners |
| GitHub Pages | unavailable | available |

So nothing forces public, and everything downstream wants it. §7's PR gate on compiler code is
**unenforceable** on a private free-org repository — it degrades to convention. CI against the
bootstrap, which is the mechanism by which the grammar finds holes, would meter against 2,000
minutes a month for a compiler test suite. And [#34](https://github.com/adamico/ludo/issues/34)
(the official website) wants Pages, which private-on-Free does not have.

The alternative that keeps every feature is paying for GitHub Team to hold 45 design ADRs in
secret. **Nothing in this corpus wants secrecy** — it is language design, the peers in §2 all
did this work in the open, and being readable is consistent with a project whose stated
audience includes AI agents.

This is the one **genuinely one-way** step: an indexed repository cannot be un-indexed. It is
taken deliberately and separately from the transfer mechanics, and it is what makes §6 urgent
rather than housekeeping — **a public repository with no `LICENSE` is all rights reserved by
default**, so the licence lands in the same operation, not after it.

## 6. Zlib, because a shipped game should owe nothing

The repository is licensed **Zlib**, one licence covering the specification and the code.

MIT and Zlib are both permissive and differ, for this project, in one clause. MIT requires the
copyright and permission notice in "all copies or substantial portions of the Software",
**including binary distribution**. A shipped ludo game links the runtime and the standard
library, so every game built with ludo would inherit an attribution obligation — a licences
file, a credits-screen entry, or the tooling Rust users maintain to generate them. Zlib
requires attribution only in source distributions, and says so explicitly: *"If you use this
software in a product, an acknowledgment in the product documentation would be appreciated but
is not required."*

The decisive argument is this project's own, not a general preference between licences.
[#16](https://github.com/adamico/ludo/issues/16) fixed that **the experience is the artifact**
— the felt loop of writing, running, and being caught early. Handing every user of that loop a
compliance chore for the runtime linked into their binary is friction in exactly the place the
destination promises there will be none.

The neighbourhood agrees, and it is the neighbourhood ludo binds to —
[ADR-0006](0006-forbidden-and-required-shapes-of-the-platform-layer-api.md)'s platform layer
and [ADR-0019](0019-claimants-conform-alone-and-extern-is-the-only-door-below-the-facade.md)'s
`extern "SDL3"`:

- **Zlib** — SDL, raylib, GLFW, sokol, and **Odin**, this project's closest peer in both
  domain and scale.
- **MIT** — Box2D, Dear ImGui.

SDL3, the library the platform layer names verbatim, is itself Zlib.

Zlib is OSI-approved and GPL-compatible, so the permissiveness costs nothing in reach. MIT's
advantage is recognition by corporate legal review, which buys little against an audience of
indie game developers.

**A consequence accepted knowingly:** Zlib requires that altered source versions be marked as
altered. A fork calling itself ludo must say it is modified. That is wanted — chapter 8
(conformance) can lean on it, and it costs nothing to anyone shipping a game.

**Rejected: a split licence** — code under Zlib, specification under CC-BY. That is machinery
for a specification with multiple independent implementers, and ludo has none. Revisit at
chapter 8, which is where a second implementer first becomes a real prospect.

**Rejected: Apache-2.0** (Rust, Swift). Its distinguishing feature is an explicit patent
grant, which addresses a corporate-contribution risk this project does not carry, at the cost
of the longest notice obligation of the candidates.

## 7. The tree, and where the branch-and-PR exemption stops

**The C bootstrap lands in `src/`**, with `core/` reserved for the ludo standard library when
it exists — Odin's layout, the peer closest to where this is going.
[ADR-0020](0020-the-bootstrap-compiler-is-written-in-c.md) requires the frontend be *"a
library, not a binary's internals"*; `src/` therefore separates frontend from driver **from
the first commit**, because ADR-0020 already records that retrofitting that seam is a frontend
rewrite.

**Amended by [#96](https://github.com/ludo-lang/ludo/issues/96): the tree is four directories, and
the platform layer is injected.** `frontend/` and `driver/` alone had no place for the
tree-walking interpreter [#49](https://github.com/ludo-lang/ludo/issues/49) scopes in, and this
map's destination — a *played* reference program — made the omission load-bearing rather than
tidy: `reference.ludo` reaches all five facades, so the interpreter that reaches the destination
does I/O. The tree becomes **`frontend/`, `interp/`, `platform/`, `driver/`**, with `core/` still
reserved.

- `interp/` — the tree-walking interpreter, a **library with no ambient I/O**, on the same rule as
  `frontend/`. It never links SDL3.
- `platform/` — the SDL3 host ([ADR-0057](0057-the-delegated-platform-layer-is-sdl3.md)), the tier
  `CONTEXT.md` calls the platform layer.
- `driver/` — unchanged: the CLI, one consumer among several, which wires the three together.

**The facades are reached through an injected host interface, not through ambient calls.** The
caller hands `interp/` a table of function pointers; `interp/` calls the platform layer only
through it. ADR-0020's rule survives in the form that fits a program with real I/O in it — *no
I/O the caller did not supply*, rather than *no I/O*. **`interp/` declares the interface** and
`platform/` implements someone else's header, so the dependency arrow points at the pure
component and a stub host is a few lines in a test file. That is what makes one evaluator serve
two consumers: the hole-finder passes an empty host and needs no window; the runner passes the
SDL3 host.

**An effect-list design was considered and rejected on the reference program's own call sites.**
Five drawing calls on the draw target return nothing and could have been deferred, but ten or
more — `$.graphics.measure_text`, `$.input.button_pressed`, `$.audio.play`'s voice handle,
`$.audio.cursor`, `$.video.render_scale`, `$.time.now`, `save1.write` — must answer mid-expression
with a value the program branches on. Deferring those needs a suspend-and-resume evaluator, which
is a large machine bought for a prototype. Injection answers them with an ordinary call.

**This is an amendment, not a reversal**, so it carries no ADR of its own
([ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §6):
the seam stands, the no-ambient-I/O rule stands, and the injected host is what lets both stand.
What crosses the `frontend/` seam is still open — this fixes only that the AST must cross, and
leaves its shape to [#130](https://github.com/ludo-lang/ludo/issues/130).

Map #1's Notes exempt documentation from review — *"Docs and research commit straight to
`main`… the repo is spec-only, so a review cycle for a Markdown file adds a review surface
with nothing to review against"* — and close with **"Revisit once the repo contains code."**
The transfer is that moment.

**The exemption is split by path, not reversed.** Documentation — ADRs, spec chapters,
coverage files, research — continues to commit straight to `main`. **Code goes behind a
branch, a pull request, and CI.** The original argument was that Markdown has nothing to
review against; C has a compiler and a test suite, so the argument does not transfer, and
neither does the exemption. This is the smallest change that respects why the exemption
existed.

## 8. One tracker, a `bootstrap` label, and no second map

§3 accepted that design tickets and compiler bugs share a tracker. They are separated by a
**`bootstrap` label**, alongside the five triage labels of `docs/agents/triage-labels.md` and
the five `wayfinder:*` labels.

**The bootstrap does not get its own wayfinder map.** ADR-0044 §5 calls it a parallel track,
and `/wayfinder` charts a route through fog — decisions not yet made. Writing a C parser from
a finished normative grammar has no fog in it: the route is visible end to end, and the skill's
own test says an effort with no fog does not need a map. The parser is **execution**. What it
produces for the map is holes, and those arrive as ordinary tickets on map #1, which is the
mechanism ADR-0044 §5 wanted from it in the first place.

Map #1 keeps its admission rule — every new design ticket names the line of the reference
program it unblocks (ADR-0044 §7). **This ADR's own work fails that test**, which is precisely
why it is an ADR and a sequence of acts rather than tickets on map #1: it is not a step on the
route to the reference program, it is the ground the route is walked on.

## 9. What this amends

[ADR-0044](0044-the-map-ends-at-an-artifact-and-the-corpus-is-consolidated-into-a-spec.md) §5
assumed `ludo-lang/ludo` was the bootstrap's own repository, standing beside a separate home
for the spec, with the normative grammar crossing between them. `ludo-lang/ludo` is now the
whole project. The schedule §5 set is unchanged and is being honoured — the first commit lands
the day `grammar.ebnf` is done — and so is its reasoning: the parser starts in parallel with
the remaining chapters, gated on the grammar and nothing else. **Only the topology changes.**

ADR-0044 is stamped accordingly, per `docs/agents/domain.md`, in the same commit as this ADR.

[ADR-0020](0020-the-bootstrap-compiler-is-written-in-c.md) is untouched: it decides the
bootstrap's host language and the frontend's shape, neither of which depends on where the
repository lives. §7 above is that ADR's frontend-as-library requirement expressed as a
directory layout.

## 10. What this ADR does not decide

- **The public name.** [#34](https://github.com/adamico/ludo/issues/34) owns it. Transferring
  to `ludo-lang/ludo` does not settle it, and a later rename carries its own redirect.
- **What CI actually runs.** §7 requires code to pass CI; the jobs are the bootstrap's own
  business and are specified when there is code to run them against.
- **Whether the reference program is checked mechanically before a frontend exists.**
  `docs/spec/README.md` says it is checked by hand until then; that stands.
- **Anything about the language.** No clause of the specification is touched by this ADR.

## 11. Order of operations

The steps are ordered by what each one makes possible, and two of them are irreversible or
outward-facing enough to be taken deliberately rather than incidentally.

1. **Transfer** `adamico/ludo` to `ludo-lang/ludo`. Preserves the graph and installs the
   redirects (§4).
2. **Add `LICENSE` (Zlib)**, before or in the same operation as step 3 — §5's default of all
   rights reserved makes ordering here non-optional (§6).
3. **Make the repository public** (§5). One-way.
4. **Enable branch protection on `main`** for the code paths §7 gates, now available (§5).
5. **Create the `bootstrap` label** (§8).
6. **Create `src/`** with the frontend/driver separation ADR-0020 requires (§7).
7. **Update map #1's Notes** to record the split branch policy (§7) and the new home.

Steps 1 and 3 are the author's to trigger. Nothing in this ADR authorises an agent to transfer
or publish the repository on its own initiative.
