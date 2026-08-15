---
status: accepted
---

> **Amended by [ADR-0023](0023-the-spec-never-mandates-a-warning.md):** §4's parked
> fog — *whether warnings are configurable* — is settled; the spec mandates no
> warning at all, so every warning is vendor-space under §5. Stamp added
> retroactively by spec ch7; see [`coverage/07-diagnostics.md`](../spec/coverage/07-diagnostics.md) §0.1.

> **Amended by [ADR-0041](0041-the-record-stops-assuming-a-process-and-re-entrancy-is-by-task.md):**
> §11's *Release still exits non-zero* is restated as abnormal termination by the
> host's convention — the third site of a claim a tab cannot satisfy literally.

> **Absorbed by [spec ch7](../spec/07-diagnostics.md):** the chapter is normative;
> this ADR keeps the argument — the envelope, the fields, the codes, the
> encoding and stream discipline are transcribed there.

# One diagnostic stream for the compiler and the faulted program

Issue #53 asked whether a conforming implementation owes a **structured format**
for the diagnostics it emits, and if so, one format for both producers or two.

The ticket did not start from zero. The experience contract's **P5** (#19)
already binds machine-readable diagnostics carrying a stable code, a source
location, the named entity #5 criterion 5 guarantees exists, and a severity, in a
documented, versioned serialization — and already carries the never-guess clause,
on the measured finding that a confidently wrong root-cause analysis *lowered*
DeepSeek-Coder's correct patches by 46.6% (Java) and 22.6% (Python). #18 §8
independently fixed the **contents** of a faulted program's fault report and
explicitly deferred the format question here.

So this ADR does two things P5 could not do inside a test-shaped property: it
fixes the field set and the code namespace rules, and it decides that the
compiler and the runner write into **one stream shape**, not two.

## 1. The obligation is the spec's, and it stops at the fields

The spec owes **what a message carries** and **what its code means**. It does not
own the command-line switch that turns machine-readable output on, the human
rendering, the colours, the ordering, or the transport.

This is the same split the map has already taken twice: hot-reload semantics are
in scope and the mechanism is out (#17), and the language-level preconditions for
tooling are in scope while the LSP itself is out (#22). Diagnostic **message
design** — wording, colour, ordering — was already out of scope on this map and
stays out.

The argument for the spec owning this at all is #4's finding that the agent's
loop is read-diagnostic → edit. #22 already committed the spec to an oracle
contract; the diagnostic stream is the oracle's other half. A per-implementation
format would mean an agent must learn N formats, and "machine-readable" would buy
nothing that a regex over English does not already buy.

## 2. Fields are normative; one serialization is mandatory; the shape is not frozen

Three postures were on the table:

- **fields only, serialization free** — rejected: it discharges the letter of P5
  and none of its purpose (§1);
- **a full JSON layout written into the spec** — rejected: P5 already recorded
  that a schema in a language spec fossilises worst;
- **fields normative plus one mandatory encoding** — taken.

An implementation MUST be able to emit its diagnostics as **JSON, one object per
message, newline-delimited**. It MAY emit other encodings in addition. It MAY add
fields. It MUST NOT omit or rename a required field.

Newline-delimited rather than one enclosing array: the compiler and the runner
both produce messages incrementally, and a consumer that must wait for a closing
bracket cannot act on the first error while the rest are still coming.

Evolution is **additive only**. The format carries a version, and a version bump
may add optional fields; removing or repurposing a field is a new format, not a
new version.

## 3. One envelope, two payloads

The compiler and the runner share a **reader** — the agent, and #4's loop — and
differ only in **producer**: one is static, one has a live faulted process behind
it.

Every message, from either producer, carries:

- **format version**;
- **producer** — the discriminant, `compile` or `fault`;
- **severity** (§4);
- **stable code** (§5);
- **primary source location** (§6);
- **named entity** (§7, required for semantic diagnostics);
- **message text**, human-facing, carrying no obligation (§8).

A `fault` message additionally carries #18 §8's contents: the closed fault-kind
enum, the concrete values that made it a fault, the call chain to the frame
entry, the frame number, and the names and types of the entry file's `persist`
declarations.

Rejected: **two top-level formats**. They would make an agent write two parsers
for one loop. Two payload variants cost it a `match` on a discriminant it already
has to perform on the code.

## 4. Severity is a closed enum of three

`error`, `warning`, `note`. Closed, because an open severity string is dialect
variance under another name and #5 criterion 4 forbids that.

`note` is **not standalone** — it attaches to a parent message's code. That is
the mechanism by which a "defined here" or "moved here" span rides along with the
error it explains (§6).

**Named consequence:** this is the first place the spec admits warnings exist at
all. This ADR binds only that a warning is a severity a message may carry.
Whether warnings are configurable was settled afterwards by
[ADR-0023](0023-the-spec-never-mandates-a-warning.md): the spec never mandates a
warning, warnings have no language surface, and no flag may change the set of
programs that compile.

## 5. The spec owns the codes for everything it mandates

A **code** is the short stable identifier an agent matches on, and it is the one
field whose stability matters more than its contents.

- Every diagnostic the spec **requires** an implementation to produce has a
  **spec-assigned code**. It is stable forever.
- A retired code is **tombstoned, never reused**. Reuse would silently change the
  meaning of an agent's existing match arm.
- An implementation MAY emit additional diagnostics of its own. Their codes MUST
  live in a distinct vendor-namespaced code space, and such a diagnostic **MUST
  NOT be the sole reason a conforming program fails to compile.**

The last clause is #5 criterion 4 applied here: if a vendor code could fail a
build on its own, a program would compile on one implementation and not another,
which is dialect variance arriving through the diagnostic channel rather than
through the grammar. P9 (#19) already forbids vendor *syntax* extensions; this is
the same rule on the output side.

## 6. Location is a primary span plus optional secondaries

The primary location is **required**: file, byte offset, and length.

Byte offsets rather than line and column: line/column is derivable from offsets
and the source, while the reverse requires agreeing about tabs, about Unicode
width, and about whether columns count code points or code units — three
disagreements this spec does not need to have.

**Secondary locations are an optional array**, each carrying its own `note`
message. A conflicting-definition diagnostic, or #15's moved-on-assignment
diagnostic, is inherently two-place; forcing the second place into prose defeats
§8 and §9. The two-libraries-claim-one-name hard error (ADR-0014) is the
canonical two-place case.

## 7. The named entity is required for semantic diagnostics only

#5 criterion 5 guarantees every failure is attributable to a named entity. A
lexical or parse diagnostic can occur **before any name has been parsed**.

So: the field is **required for every semantic diagnostic** and **optional for
lex and parse diagnostics only**. The code says which class the message belongs
to, so a consumer knows whether to expect the field before it looks.

Requiring it unconditionally would force an implementation to invent an entity
for a message about a stray brace — which is guessing, and §9 makes guessing a
conformance failure.

## 8. Message text carries no obligation

The human-facing text is required to be present and is **not** required to say
anything in particular. It is not stable, not versioned, and not something an
agent may key on. Everything an agent needs is in the code, the location, the
entity and §9's suggestion field.

This is deliberate: it keeps error-message design out of scope (§1) while leaving
the field in the envelope, so a rendering tool has something to print.

## 9. The suggestion is a structured field, and silence beats a guess

P5's never-guess clause was prose. Here it gains teeth.

A **suggestion is an optional field** carrying a source range and replacement
text — applicable by a tool without reading English. A diagnostic MUST omit the
field rather than emit a low-confidence suggestion.

Helpful prose inside the message text — a "did you mean" sentence — is **not a
suggestion**, carries no obligation, and is not machine-applicable by
construction. That is the whole point of the split: it gives an implementation
somewhere to put a hunch that is not the field an agent will apply blind.

This is the cheapest place to make P5's measured 46.6% regression finding
enforceable, because "the field is present and the replacement does not resolve
the diagnostic" is a checkable failure and "the prose was over-confident" is not.

## 10. Stream discipline: the machine-readable stream carries nothing else

The spec **requires the capability** and **names no switch** — naming a flag
would be the spec deciding a CLI, which §1 put out of scope.

It does require this: when machine-readable output is enabled, the stream
carrying it contains **messages and nothing else**. Human rendering, progress
lines, timing summaries and build chatter go elsewhere. A single progress line
interleaved into the stream breaks every consumer exactly once, and the failure
is silent for the producer.

## 11. The fault report is emitted in both modes

Dev and release both emit the `fault` message.

#18 §4 fixed that fault semantics are identical in dev and release — a bug ends
the simulation in both — and #18 §9 already pays for the call chain in release.
Emitting the structured report only in dev would make the same program report
differently by mode, which is the variance #18 §4 refused.

Release still exits non-zero. It now says why in a parseable form first. The
difference between the modes stays what #12 blessed: the runner's liveness, not
the semantics.

## 12. The three lenses

**Simplicity — neutral, and deliberately invisible.** Nothing here appears in the
language a beginner writes. The beginner-facing effect is second-order: §9 means
a suggestion offered is a suggestion that works, and §6 means "the other place"
is shown rather than described.

**Robustness — moderate, on the toolchain rather than the program.** It deletes
no bug class in user code. It closes one in the ecosystem: §5's vendor-code rule
removes a path by which implementations could diverge on what compiles, and §10
removes a class of consumer breakage that is silent on the producing side.

**Agent-friendliness — strong; this is the section that earns the ADR.** §3 gives
the read-diagnostic → edit loop one parser for both of its inputs. §5 gives it a
stable key. §7 tells it when a field is guaranteed. §9 converts the never-guess
finding from an aspiration into a failable property. Every one of these traces to
#4's measured failure modes rather than to taste.

**No beginner-versus-agent conflict to name.** The two audiences do not meet
here — which is itself worth recording, since #6's axis has bitten most other
tickets on this map.

## 13. #24 delta

- **Core grammar: unchanged.** **Type sublanguage: unchanged.** This ADR adds no
  syntax.
- **Stdlib root names: none added.** The diagnostic stream is toolchain output,
  not a program-visible API.
- **Recorded rejections on the budget's own terms:** a frozen JSON schema in the
  spec (§2), two top-level formats (§3), an open severity string (§4), an
  unconditionally-required entity field (§7).

## 14. What this hands on

- **#19 P5** gains a pointer to this ADR and keeps its test-shaped form. The
  field set, the code namespace rules and the stream discipline live here.
- **The follow-on toolchain effort** already inherits "the diagnostic code
  namespace" as a deliverable (#19 §E). It now also inherits §5's tombstoning
  rule and §2's additive-only evolution rule.
- **Whether warnings are configurable** is fog on the map, not a decision this
  ADR took by implication.
