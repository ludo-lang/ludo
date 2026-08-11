---
status: accepted
---

# The wrap-the-facade idiom, and the forwarding-function guarantee that makes it free

ADR-0007, ADR-0009, ADR-0010 and ADR-0011 all spent verbosity deliberately: no
abbreviations, a descriptor struct past two parameters, no overloads, no aliases
(`circle` was refused because there is no `square` beside `rect`). The reason is
measured — issue #4 puts wrong-or-non-existent names at 43.7% of agent compile
errors, and the full unabbreviated word is what a model guesses correctly.

The counterpart to that choice was never stated anywhere: **the user is expected
to wrap it.**

```
pixel(target, p, c)     -> $.graphics.fill_rect(target, {position = p, size = $.vec2(1, 1), paint = c})
footstep(surface)       -> $.audio.play({...}) with the envelope and filter for that surface
jump_pressed()          -> the $.input query for whichever binding jump is on
```

A three-line wrapper written once turns the verbose, guessable, agent-safe call
into the short one a human types all day. The agent writes the qualified,
unabbreviated call it cannot get wrong; the human writes their own three-letter
name over it. Both get the surface they want out of one language — which is
issue #6's layering answer (progressive disclosure over fixed semantics) showing
up in the standard library instead of in the grammar.

Unstated, this reads as an omission: a reader meets ADR-0009's descriptor
literals and concludes the language is ceremonious. It is the obvious criticism
of ADR-0007 and ADR-0009 — *these calls are long* — and the honest answer is:
yes, once.

## Decision

### 1. The spec owes exactly one normative thing

The intent ("the mandated facades are verbose because they are meant to be
wrapped") is **rationale**, and lives in the spec's front-matter beside the
existing non-goals, plus the teaching material. It binds no implementation and is
not conformance.

The one part an implementation can *violate* is the cost of the wrapper, so that
— and only that — is normative. Without it, "wrap it" is advice with a
performance asterisk in the one domain where per-frame cost is the entire point.

### 2. The forwarding-function guarantee

A **forwarding function** is a function whose body is a single call expression,
with no captures. This is a structural predicate, decidable by inspection, not a
judgement about optimisation quality.

A conforming implementation must not emit a call to a forwarding function.

It is deliberately *not* the general promise "the compiler inlines well". Issue
#11's ban on `dyn` already removes the case that would make the predicate
undecidable.

An explicit inline marker was rejected. It spends grammar — against issue #24's
budget rule — on something the definition already decides, and it makes the
beginner write ceremony in order to undo ceremony.

### 3. How the guarantee is stated as a testable property

Issue #19's contract is written in precondition / action / observable form, and
"must not cost more" is not observable. Three forms were considered:

- **Comparative** (wrapped program and hand-inlined program have identical timing)
  — a benchmark. Benchmarks are not conformance, and are unownable across
  eighteen backends.
- **Cost model** (the spec defines a model in which a forwarding call costs zero)
  — elegant, but the spec has no cost model and inventing one for a single clause
  is a large new surface for a small guarantee.
- **Structural** (no call to a forwarding function appears in the emitted
  program) — checkable without a stopwatch, by the same harness the follow-on
  conformance effort already owes.

**Structural wins.** Its cost is named rather than hidden: it constrains *how* an
implementation compiles, which this map has otherwise kept clear of. It is
therefore written as narrowly as the predicate allows, and stands as a
deliberate, bounded exception to "the spec does not prescribe implementation".

### 4. Wrappers are never blessed into the reserved root

Issue #5 reserves the `$.` root and fixes one spelling per entity. Promoting a
wrapper — `$.graphics.pixel` beside `fill_rect` with a 1×1 size — would create a
second spelling for one entity. It does not happen, and the reason is worth
stating: it is *why* the facade is verbose.

The escape hatch already exists and is better. A wrapper that turns out to be
universal is evidence the **facade call is at the wrong altitude**, and the fix is
the move issue #41 already made for audio — change the facade, and record the
delta in issue #24's companion count. The pressure stays visible in the count
instead of hiding inside a blessed alias.

### 5. The project publishes no wrapper library — and forbids no one else's

Two different things were nearly conflated here, and the distinction is
load-bearing:

- **What this project does.** It does not ship a blessed wrapper library, not
  even "blessed by reference" in the manner of the renderer (ADR-0004) or the
  audio engine tier. Blessing by reference would recreate the second spelling of
  §4 through the back door, and would make *conforming* quietly come to mean
  *has the sugar library* — which is ADR-0004's rejected full-cover argument with
  different nouns.
- **What anyone else does.** Not the spec's business. A third party publishing a
  wrapper package is exactly the ecosystem this language wants, and nothing here
  forbids, discourages or constrains it. Such a package is an **opt-in
  dependency**, not shared vocabulary: it is not de-facto surface, in the same
  way no other package is.

Accordingly, issue #24's companion count is unaffected by wrappers of either
kind. The count measures the **reserved root** — names a package may not occupy
and every program therefore shares. A wrapper is per-project or opt-in, and
shadows nothing. The definition should say so explicitly, because the one thing
that *would* corrupt the count is precisely the option §5 declines: a
blessed-by-reference wrapper library would be shared vocabulary that never
appears in the count.

### 6. The docs carry a small illustrative set, not a standard

Issue #5's corpus-sizing conflict lands here concretely: a *small* corpus prevents
memorised-bug replication (44.4% of agent output byte-identical to training data)
and *causes* the ~30-point unfamiliarity tax.

The teaching material carries a deliberately small illustrative set, presented as
**examples of the shape** and never as names to depend on. The examples are
chosen to be obviously project-flavoured — `footstep`, `jump_pressed` — rather
than plausibly universal (`circle`), so that nothing reads as a de-facto standard
name.

Smallness is the point, and it is the one place the unfamiliarity tax is free:
the tax is paid against *wrapper names*, which are supposed to differ per project
anyway. Agents will imitate something; giving them a set that is visibly local
beats both a universal-looking set and nothing at all.

### 7. Teaching order: the real call first, with one exception

- **The reference and tutorial teach the facade plainly first.** Teaching the
  wrapper first is a trap: a beginner who meets `pixel(t, p, c)` before
  `fill_rect` has learned a name that exists in no other program, and the first
  time they read someone else's code — or ask an agent — nothing matches. That is
  issue #4's naming failure re-created by the teaching material itself.
- **A dedicated chapter** ("the facade is verbose on purpose; here is what you do
  about it") comes early enough that no reader forms the ceremony conclusion
  first.
- **The exception is `$.audio`**, which gets its wrapper example inline at first
  contact. A `VoiceDesc` literal is not something anyone should meet un-named.

Showing a wrapper beside every call in the reference was rejected: it doubles the
reference and makes wrapping look mandatory.

### 8. Normatively uniform across the three facades

The guarantee of §2 is a property of *functions*, not of facades, so it cannot be
per-facade without becoming incoherent. It applies identically to `$.graphics`,
`$.audio` and `$.input`.

The teaching asymmetry of §7 is not normative, and its cause is worth naming
because it is a good line for the chapter:

- **Drawing** — the wrapper is purely a shorter name.
- **Input** — the wrapper names a *binding*, a genuine indirection: the binding
  can change without the call sites changing.
- **Audio** — the wrapper is not sugar at all. It **is** the sound design. A named
  `footstep` is the game's vocabulary and the `VoiceDesc` behind it is a
  parameter table nobody memorises.

## Consequences

- The language core gains one normative obligation it did not have: the
  forwarding-function guarantee, and with it a bounded exception to the spec's
  otherwise clean separation from implementation.
- The conformance effort gains one more assertion to harness, of a kind it
  already handles (inspect the emitted program, no timing).
- Issue #24's count definition gains a clause naming what it excludes and why.
- The front-matter non-goals gain a companion rationale paragraph.
- This ADR exposed, but does not answer, a question the map had never asked:
  **what a third-party library is as an artifact.** §5 leans on packages existing
  without the spec ever having said what one is. That is now its own ticket.
