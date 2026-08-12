---
status: accepted
---

# The spec never mandates a warning, and no flag changes what compiles

Issue #59 asked whether warning configurability — promotion to errors, per-site
or per-build silencing, named warning groups — is a language surface or a
toolchain flag.

The question exists because [ADR-0018](0018-one-diagnostic-stream-for-the-compiler-and-the-faulted-program.md)
§4 fixed severity as a closed enum of three and, in doing so, admitted warnings
exist on this map for the first time. It deliberately decided nothing else about
them and parked the rest as fog.

**One fact reframed the ticket before any of it could be answered.** A sweep of
every ADR and of `CONTEXT.md` finds that the spec mandates **zero**
warning-severity diagnostics. Not one. Every diagnostic this map has produced is
a hard error: #17's reload-safety check, #10's error model, #8's must-use, #53's
own mandated codes. The only warning-shaped thing in the repository is
[ADR-0020](0020-the-bootstrap-compiler-is-written-in-c.md)'s warning
configuration for the **bootstrap C compiler**, which is a different language and
not this spec's business.

So configurability was never the root question. It is downstream of whether
spec-mandated warnings exist at all — and they do not.

## 1. The spec never mandates a warning

**A spec-owned diagnostic is an error, or it does not exist.**

This is a standing rule, not a description of today. Any future proposal for a
spec-mandated diagnostic must land as a hard error or not at all.

The rule follows from what this language claims to be. A warning is a diagnostic
the spec cares about enough to require and not enough to enforce, and in a
language whose whole pitch is being caught early, that middle position has no
occupant: anything worth warning about should be an error, and anything not worth
an error has no business being an obligation on every implementation.

Stated as a rule rather than a snapshot because it is self-defending. A mandated
warning creates immediate demand to silence it — that is the entire history of
warning configurability in every language that has one — so admitting a single
warning re-opens this decision by construction.

The `warning` severity **stays in ADR-0018's enum**. Implementations emit
warnings, and those warnings need a severity to carry. Deleting it was considered
and rejected: it would strand real vendor output with nowhere to go, buying an
economy of one enum member at the cost of pushing legitimate diagnostics into a
severity that lies about them.

Every warning is therefore **vendor-space**, governed by ADR-0018 §5: a
vendor-namespaced code, and never the sole reason a conforming program fails to
compile.

## 2. Criterion 4 governs accept/reject, not output

[#5](https://github.com/adamico/ludo/issues/5) criterion 4 forbids dialect
variance. This ADR fixes its reading:

> **The set of programs that compile is identical across conforming
> implementations. Diagnostic output is not constrained.**

The alternative reading — that two conforming compilers produce the *same*
diagnostics — was rejected twice over. It is unenforceable the moment vendor
warnings are legal at all, and it would freeze diagnostic quality permanently: no
implementation could ever ship a better warning than the one the spec imagined.

The consequence is the one that decides this ticket. **Warnings sit outside
criterion 4 by construction** — a warning never changes whether a program
compiles, so it cannot produce dialect variance. Any mechanism that promotes a
warning to an error moves it *inside*.

## 3. A promotion may gate a pipeline, never a compile

`-Werror` is a real need and banning it outright is a rule people route around.
The line is drawn by effect, not by name:

- **Forbidden**: a compiler flag under which the compiler rejects a program that
  another conforming implementation accepts. This includes a vendor's own
  `-Werror` over its own vendor warnings.
- **Permitted**: an external gate that reads ADR-0018's machine-readable stream,
  finds warnings, and exits non-zero. The compile **succeeded**; the artifact is
  byte-identical to the one produced without the gate; a pipeline chose to fail
  on its contents.

CI gets everything it actually wants, and criterion 4 is untouched. Note that
this needs **no new spec surface at all** — ADR-0018 already mandates a
newline-delimited JSON stream, and the gate is an ordinary consumer of it. The
capability falls out of what is already there rather than being added.

The spec continues to name no flags, no CLI switches and no rendering — the same
in/out split ADR-0018 took, and #17 and #22 before it. What this ADR adds is a
constraint on an **effect**: no flag may change the set of programs that compile.
That is criterion 4 speaking about the toolchain, not the spec growing a CLI.

## 4. Warnings have no language surface

**No `#allow(...)`, no `#deny(...)`, no in-source silencing of any kind.**

With §1 settled, a per-site attribute could only ever name a *vendor* code. That
puts vendor identifiers into ludo source — source that means something on one
implementation and nothing on another — which is exactly what
[#19](https://github.com/adamico/ludo/issues/19)'s P9 no-vendor-syntax rule and
criterion 4 exist to prevent.

Two alternatives were considered. Attributes over spec-owned codes only is a
surface with an empty set to name. Attributes that tolerate unknown codes
legalises vendor names in source through the back door.

This is what makes the ticket's title resolve: **warning configurability is not a
language surface, because warnings are not a language surface.**

## 5. Named warning groups are dissolved, not deferred

The fog patch named three sub-questions. §3 and §4 answer promotion and
silencing. **Warning groups are dissolved**: grouping exists to make bulk
silencing ergonomic, and with no silencing surface and no mandated warnings there
is nothing to group.

Recorded explicitly so it is not re-opened later as an oversight.

## 6. Vendor codes are structurally distinguishable

An agent's read-diagnostic loop must be able to tell spec-owned from vendor-owned
messages **by the code's shape alone**, without a lookup table it would have to
keep current.

ADR-0018 §5 already requires vendor codes to live in a "distinct
vendor-namespaced code space", which grants this. This ADR records it as a
consequence and **adds no field** to the envelope.

## 7. What this hands to other tickets

- **[#19](https://github.com/adamico/ludo/issues/19)** gains **P12**: compile a
  corpus under every flag combination an implementation offers and assert an
  identical accept/reject set. Criterion 4 has been cited across this map a dozen
  times and has never once been *failable* — it has been an argument, not a test.
  This is the cheapest place it becomes one, and without it §3's gate-not-compile
  line is unenforceable, since a vendor `-Werror` that rejects programs is
  indistinguishable from a conforming compiler unless something checks.
- **ADR-0018 §4** gains a pointer here, replacing its parked-fog paragraph.

Nothing new enters the fog. The patch closes with no residue.

## The three lenses

- **Simplicity.** Invisible, and that is the point: nothing in this decision
  appears in the language. There is no attribute to learn, no pragma, no
  configuration file, and no question of whether a build is "strict". A beginner
  never meets a warning the spec put there, because the spec put none there.
- **Robustness.** The rule is the robust choice rather than the lenient one.
  Every spec-owned diagnostic enforces itself; there is no category of "the
  compiler told you and you shipped anyway" for anything the spec cares about.
  P12 converts the map's most-cited principle from an argument into a test.
- **Agent-friendliness.** Strong, on #4's evidence. An agent's compile signal is
  binary — it compiled or it did not — and cannot be made vendor-dependent by a
  flag it does not know about. Every message worth acting on is spec-owned and
  structurally identifiable (§6), so the read-diagnostic → edit loop has a stable
  key and a filterable stream.

**Where the lenses conflict:** they do not, here. The beginner-versus-agent
tension this map watches for does not appear — neither audience wants a
configurable warning, for different reasons that happen to point the same way.
