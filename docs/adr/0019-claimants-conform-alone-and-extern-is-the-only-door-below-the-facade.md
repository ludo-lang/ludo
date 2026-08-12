---
status: accepted
---

# Claimants conform alone, and `extern` is the only door below `$.`

ADR-0014 §9 made a backend a library claiming a root name, satisfying a nominal
interface, with target selection deciding which mutually-exclusive claimant is in
the build. Issue #5 criterion 4 forbids dialect variance: a behavioural change
must force a signature change.

Those two collide. A nominal interface holds signatures. Two claimants of one
root can satisfy identical signatures and behave differently — dialect variance
arriving through the back door of target selection.

Issue #54 asked what the spec owes here. The answer is **no new rule**, for two
reasons that are worth recording because both are easy to get wrong a second
time. This ADR records only those two. The reviewer-facing detail — what a person
checking a new backend is handed, and in what order — lives in
[issue #54's resolution](https://github.com/adamico/ludo/issues/54) and is
entirely pointers to artifacts that already exist.

## Decision

### 1. A conformance obligation is stated on one claimant, never on a pair

**Each claimant conforms to the spec alone.** Agreement between two claimants is
a *consequence* of both conforming. It is never a thing anyone checks.

An obligation worded on mutually-exclusive claimants — "claimant A and claimant B
must agree" — has no checkable form, because it names a pair that exists nowhere:

- ADR-0014 §6: two claimants of one root name never coexist in one program.
- ADR-0006 R3: no conditional compilation; target selection is a build-time module
  set, so **the source never names the backend**. The platform is unknown at
  authoring time by design.

So there is no point in the system where both claimants are present — not in the
source, not in a build, not in front of a reviewer. Any wording that names a pair
describes a comparison no artifact can perform.

This is a shape rule for how *any* future interface obligation on this map gets
worded, not a fact about backends. When a decision is tempted to say "all
implementations of X must agree on Y", it must instead say what one
implementation owes the spec.

The corollary is that the nominal interface is **necessary and insufficient**. A
claimant of a spec-defined root owes the spec's *behaviour* — the normative text
of ADR-0005, ADR-0007, ADR-0009, ADR-0010 and ADR-0011, made executable by issue
#19's P1–P11 — not merely its signature. ADR-0006 R4 already moved the signature
half from reviewer discipline to a type error; this states that the type error is
not the end of the obligation.

### 2. There is no program-visible surface below `$.`; `extern` is the only door

**Criterion 4 holds unweakened**, and the reason is that the space it would have
to be softened for is empty.

The worry was a program reaching past the mandated
[drawing facade](../../CONTEXT.md#drawing-facade) into a delegated tier — a
[renderer](../../CONTEXT.md#renderer) or the
[platform layer](../../CONTEXT.md#platform-layer) — making that program portable
in signature and unportable in behaviour, since ADR-0004 delegates both tiers and
writes no behaviour for either.

**That program cannot be written today.** A renderer needs the GPU device and
swapchain; the platform layer owns them. ADR-0006's R1–R8 are **backend-facing**
rules constraining who writes a backend, not a surface a program can call.
Whether the platform layer has a program-visible surface at all remains fog on
the map.

The one door below `$.` that does exist is the **C FFI** (issue #29):
`extern "SDL3" fn …` can bind a C renderer directly. That door is already
governed — `unsafe` at every call site, wrapper-mediated, a fault barrier.

Therefore:

- **There is no signature-portable, behaviour-unportable ludo code.**
- The only divergence channel is `extern`, and `extern` is **visible in the
  signature** — which is exactly what criterion 4 demands.
- The `unsafe` marker on the FFI path *is* the "below `$.` you own your
  portability" warning. No new rule is owed, and adding one would duplicate a
  marker that already carries the meaning.

Softening criterion 4 into a language-only rule was the expected outcome and was
rejected: with the space empty, the language-only reading and the general reading
currently coincide, and weakening a Tier 1 veto to buy nothing is a permanent
cost for no gain.

### 3. The obligation this hands forward

§2 is true **because of an absence**, so it expires the moment the absence does.

If the platform layer ever gains a **program-visible surface**, that decision
opens the first ludo-side door below `$.`. Whoever takes it must either carry
`extern`'s treatment — `unsafe` at the call site, visible in the signature — or
re-open criterion 4 and weaken it deliberately. It may not open the door and
leave criterion 4 unexamined.

The same applies to any decision that exposes a delegated tier to programs: a
renderer surface, or a web-export path that reaches around the facade.

### 4. What the spec does not owe

Criterion 4 constrains **the language**, not every library written in it. If two
authors both claim `physics` and behave differently, the spec owes nothing: a
program that swaps one third-party `physics` for another has changed its
**dependencies**, not its dialect. The spec's behavioural obligation extends
exactly as far as the spec's own normative text.

Mutually-exclusive claimants as a *pattern* therefore stay **legal and
unblessed** for third parties. Forbidding the pattern would need a rule the
compiler cannot check — ADR-0014 §3 errors identically on a deliberate
mutual-exclusion pair and an accidental collision. But nothing in the spec should
imply that the language *supports* interchangeable implementations, because that
implies an equivalence obligation §1 shows has no checkable form.

## Consequences

- **No grammar delta.** Nothing is added; issue #24's count is unaffected.
- Criterion 4 stays a **Tier 1 veto** at full strength. Agents may continue to
  rely on behavioural changes being visible in signatures, with `extern` as the
  one exception they read in the signature anyway.
- Future interface obligations on this map are worded against **one**
  implementation. §1 is the rule to cite.
- The **platform-layer fog inherits §3** and cannot be resolved without answering
  it.
- Issue #56 is the standing precedent for §1's ranking: it checked mandated
  clauses against real browser semantics and **falsified two of them**, so a rule
  written before an implementation exists is advisory until the conformance suite
  can express it.
