# Chapter 7 — Diagnostics

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where another chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under this chapter's envelope. (ADR-0018.)

**0.3** This chapter owns **the diagnostic stream**: the envelope both producers
write into, the required fields, the closed severity set, the rules governing
codes, the location and entity fields, the suggestion field, the mandatory
encoding and its evolution rule, stream discipline, and the standing rule that
the spec never mandates a warning. It is the referent of every *chapter 7's
envelope* pointer in chapters 1–6.

**0.4** This chapter does not own **what a fault report contains** (chapter 5
§6.5, from #18 §8), **what any individual diagnostic is about** — every
mandated diagnostic is minted by the chapter that states the rule it enforces —
or **the experience contract's properties**, including P5, P9 and P12
(chapter 8, from #19). Where a clause reaches one of them, it names the routing.

**0.5** Every clause cites its source. A clause with no citation is a defect.

**0.6 This chapter specifies output, not language.** Nothing here has a
spelling a program can write, and the reference program is therefore not
extended by it — the first chapter for which that is true of its whole subject
rather than of a section. `coverage/07-diagnostics.md` §11 records it.
(ADR-0018 §12, which states the same fact as *deliberately invisible*.)

---

## 1. What the spec owes, and what it does not

**1.1** The spec owes **what a message carries** and **what its code means**.
(ADR-0018 §1.)

**1.2** The spec does **not** own, and this chapter MUST NOT be read as
constraining: the command-line switch that enables machine-readable output, the
human rendering, colour, the ordering of messages, or the transport. (ADR-0018
§1, §10.)

**1.3** **Diagnostic message design — wording, colour, ordering — is out of
scope for this spec.** (ADR-0018 §1; the map's *Out of scope*.)

**1.4** The **capability** is required and **no switch is named**. A conforming
implementation MUST be able to emit its diagnostics in the form this chapter
fixes; how it is asked to is an implementation's own affair. (ADR-0018 §10.)

---

## 2. One envelope, two producers

**2.1** The compiler and the runner write into **one message shape**. There are
**not two top-level formats**. (ADR-0018 §3.)

**2.2** The two producers are distinguished by one required field, and by
nothing else: **`compile`** for a diagnostic about a program that does not
compile, **`fault`** for the report of a faulted program. (ADR-0018 §3.)

**2.3** A `fault` message carries every field §3 requires, **plus** the fault
report's contents. Those contents are **chapter 5 §6.5's**, not this chapter's;
this chapter binds only that they ride in this envelope. (#18 §8; ADR-0018 §3;
ch5 §6.5.3.)

**2.4** **The `fault` message is emitted in both modes**, dev and release. An
implementation MUST NOT emit the structured report in dev only. (ADR-0018 §11;
#18 §4; ch5 §6.3.1.)

**2.5** In release, a faulted program **terminates abnormally by the host's
convention**, and does so *after* emitting the `fault` message. ADR-0018 §11's
original wording — *release still exits non-zero* — is the third site of a claim
a hosted target cannot satisfy literally, and reads as this clause. (ADR-0018
§11, as amended by ADR-0041 §3; ch5 §6.3.3.)

---

## 3. The required fields

**3.1** Every message, from either producer, MUST carry:

- the **format version** (§8);
- the **producer** discriminant, `compile` or `fault` (§2.2);
- the **severity** (§4);
- the **stable code** (§5);
- the **primary source location** (§6.1);
- the **named entity**, under §7's condition;
- the **message text** (§9).

(ADR-0018 §3.)

**3.2** An implementation **MAY add fields**. It **MUST NOT omit or rename a
required field**. (ADR-0018 §2.)

**3.3** The optional fields this chapter fixes are the **secondary locations**
array (§6.2) and the **suggestion** (§10). An implementation that emits either
MUST emit it in the form fixed here; both are optional to emit and neither may
be renamed. (ADR-0018 §6, §9.)

---

## 4. Severity is a closed enum of three

**4.1** Severity is **`error`, `warning`, `note`**, and the set is **closed**.
An open severity string is dialect variance under another name. (ADR-0018 §4;
#5 criterion 4.)

**4.2** **`note` is not standalone.** It attaches to a parent message's code,
and is the mechanism by which a *defined here* or *moved here* span rides along
with the error it explains. (ADR-0018 §4; §6.2.)

**4.3** **`warning` stays in the enum, and the spec mandates no warning.** The
severity exists so that a vendor diagnostic has a truthful severity to carry;
deleting it was considered and rejected. (ADR-0018 §4; ADR-0023 §1.) §12 states
the rule and its consequences.

---

## 5. Codes

**5.1** A **code** is the short stable identifier a consumer matches on. It is
the one field whose stability matters more than its contents. (ADR-0018 §5.)

**5.2** Every diagnostic the spec **requires** an implementation to produce has
a **spec-assigned code**, and that code is **stable forever**. (ADR-0018 §5.)

**5.3** A retired code is **tombstoned and never reused**. Reuse would silently
change the meaning of a consumer's existing match arm. (ADR-0018 §5.)

**5.4** An implementation **MAY** emit additional diagnostics of its own. Their
codes **MUST** live in a **distinct vendor-namespaced code space**. (ADR-0018
§5.)

**5.5** **A vendor-coded diagnostic MUST NOT be the sole reason a conforming
program fails to compile.** This is #5 criterion 4 on the output side: if a
vendor code could fail a build alone, a program would compile on one
implementation and not another. (ADR-0018 §5; #19 P9, which forbids the same
thing on the syntax side.)

**5.6** A consumer MUST be able to tell a spec-owned code from a vendor code
**by the code's shape alone**, with no lookup table it would have to keep
current. (ADR-0023 §6.)

**5.7 The concrete code strings are not this spec's.** The spec fixes the rules
of §5.1–§5.6; **assigning the codes** — the namespace's contents, and the string
each mandated diagnostic carries — is the follow-on toolchain effort's
deliverable, which inherits §5.3's tombstoning rule and §8.2's additive-only
rule with it. (ADR-0018 §14; #19 §E.) The **shape** that §5.6 requires is
§13.1's marked gap.

**5.8** Where a chapter mints a mandated diagnostic it states the **count and
the severity**, and this chapter states the rules those codes obey. Chapter 1
§11.6's four `#explicit` codes are the corpus's canonical case: four
spec-assigned codes, severity `error`, strings unassigned. (ADR-0029 §6;
ADR-0037 §4, which mints one more for a shader declaration against a
facade-only target.)

---

## 6. Location

**6.1** The **primary location is required**, and is **file, byte offset and
length**. Line and column are derivable from offsets and the source; the reverse
requires agreement about tabs, about Unicode width, and about whether columns
count code points or code units. (ADR-0018 §6.)

**6.2** **Secondary locations are an optional array**, each carrying its own
`note` message. (ADR-0018 §6; §4.2.)

**6.3** A diagnostic that is **inherently two-place** MUST NOT force its second
place into prose. The canonical case is chapter 4's duplicate root-name claim,
where two libraries claim one name; #15's moved-on-assignment diagnostic is the
other. (ADR-0018 §6; ADR-0014; ch4 §6.5.)

---

## 7. The named entity

**7.1** #5 criterion 5 guarantees every failure is attributable to a **named
entity**. The field carrying it is **required for every semantic diagnostic**.
(ADR-0018 §7.)

**7.2** The field is **optional for lex and parse diagnostics only**, because a
lexical or parse diagnostic can occur before any name has been parsed.
(ADR-0018 §7.)

**7.3** **The code says which class the message belongs to**, so a consumer
knows whether to expect the field before it looks. (ADR-0018 §7.)

**7.4** An implementation MUST NOT invent an entity in order to fill the field.
Inventing one is guessing, and §10.2 makes guessing a conformance failure.
(ADR-0018 §7, §9.)

---

## 8. Encoding and evolution

**8.1** An implementation **MUST be able to emit its diagnostics as JSON, one
object per message, newline-delimited**. It **MAY** emit other encodings in
addition. Newline-delimited rather than one enclosing array, because both
producers emit incrementally and a consumer that must wait for a closing bracket
cannot act on the first error while the rest are still coming. (ADR-0018 §2.)

**8.2** **Evolution is additive only.** The format carries a version; a version
bump **MAY add optional fields**. **Removing or repurposing a field is a new
format, not a new version.** (ADR-0018 §2.)

**8.3** **No JSON schema is frozen into this spec.** The fields of §3 are
normative and the encoding of §8.1 is mandatory; the layout is not. (ADR-0018
§2, which records the rejection.)

---

## 9. Message text

**9.1** The human-facing **message text is required to be present** and is
**not required to say anything in particular**. (ADR-0018 §8.)

**9.2** It is **not stable, not versioned, and not a field a consumer may key
on**. Everything a consumer needs is the code, the location, the entity and the
suggestion. (ADR-0018 §8.)

---

## 10. The suggestion, and the never-guess rule

**10.1** A **suggestion is an optional field** carrying a **source range and
replacement text** — applicable by a tool without reading English. (ADR-0018
§9.)

**10.2** **A diagnostic MUST omit the field rather than emit a low-confidence
suggestion.** (ADR-0018 §9; #19 P5's never-guess clause, which this field makes
checkable.)

**10.3** **Helpful prose inside the message text is not a suggestion.** A *did
you mean* sentence carries no obligation and is not machine-applicable by
construction; it is where an implementation puts a hunch that is not the field a
consumer will apply blind. (ADR-0018 §9.)

**10.4** A suggestion that is present and, when applied, does not resolve its
diagnostic is a **conformance failure**. (ADR-0018 §9; #19 P5, whose failable
form this is; ch8 owns the property's wording.)

**10.5** The never-guess rule reaches diagnostics minted elsewhere without
restating itself: chapter 4's unresolvable-`use` diagnostic **may not suggest a
name the compiler has not confirmed exists**. (ADR-0027 §1; ADR-0018 §9; ch4
§7.8.)

---

## 11. Stream discipline

**11.1** When machine-readable output is enabled, the stream carrying it
contains **messages and nothing else**. Human rendering, progress lines, timing
summaries and build chatter go elsewhere. (ADR-0018 §10.)

**11.2** A single non-message line interleaved into the stream breaks every
consumer exactly once, and the failure is **silent for the producer**; §11.1 is
stated as an obligation for that reason. (ADR-0018 §10.)

---

## 12. The spec never mandates a warning

**12.1** **A spec-owned diagnostic is an error, or it does not exist.** This is
a **standing rule**, not a description of the corpus as it stands: any future
spec-mandated diagnostic MUST land as a hard error or not at all. (ADR-0023 §1.)

**12.2** The spec mandates **zero** warning-severity diagnostics. Every
diagnostic this spec requires is a hard error. (ADR-0023 §1.)

**12.3** **Every warning is therefore vendor-space**, governed by §5.4 and
§5.5: a vendor-namespaced code, never the sole reason a conforming program fails
to compile. (ADR-0023 §1.)

**12.4 Criterion 4 governs accept/reject, not output.** Its normative reading
is fixed as:

> **The set of programs that compile is identical across conforming
> implementations. Diagnostic output is not constrained.**

(ADR-0023 §2; #5 criterion 4.)

**12.5** **Warnings sit outside criterion 4 by construction** — a warning never
changes whether a program compiles. Any mechanism that promotes a warning to an
error moves it inside. (ADR-0023 §2.)

**12.6** **No flag may change the set of programs that compile.** A compiler
flag under which the compiler **rejects** a program another conforming
implementation accepts is forbidden, and this includes a vendor's own `-Werror`
over its own vendor warnings. (ADR-0023 §3.)

**12.7** **An external gate is permitted.** A consumer that reads §8.1's stream,
finds warnings and exits non-zero is conforming: the compile **succeeded**, the
artifact is byte-identical to the one produced without the gate, and a pipeline
chose to fail on its contents. This needs **no spec surface at all**. (ADR-0023
§3.)

**12.8 Warnings have no language surface.** There is **no `#allow(...)`, no
`#deny(...)`, and no in-source silencing of any kind**. A per-site attribute
could only ever name a vendor code, which would put identifiers that mean
something on one implementation and nothing on another into ludo source.
(ADR-0023 §4; #19 P9; #5 criterion 4.)

**12.9** **Named warning groups do not exist.** Grouping exists to make bulk
silencing ergonomic; with no silencing surface and no mandated warnings there is
nothing to group. Recorded so it is not re-opened as an oversight. (ADR-0023
§5.)

**12.10** **The word *lint* does not appear in this spec.** A lint is the middle
position §12.1 found has no occupant. (ADR-0029 §6; ADR-0023 §1; ch1 §11.6.)

---

## 13. Marked gaps

Per ADR-0044 §8, one gap this chapter writes down rather than blocks on. It is
filed as a ticket that owns the repair of this chapter's text and its
`coverage/` rows in one commit (ADR-0049).

**13.1 The code space has no stated shape, so §5.6's test cannot be applied.**
ADR-0023 §6 requires a consumer to separate spec-owned from vendor-owned codes
**by the code's shape alone**, and grounds the requirement on ADR-0018 §5
*already* granting it. ADR-0018 §5 requires a *distinct vendor-namespaced code
space* and fixes no shape for either space, and a namespace that is distinct by
assertion is not distinguishable by inspection: with no shape rule, the only way
to classify a code is the lookup table §5.6 exists to avoid. The requirement is
therefore transcribed at §5.6 as the obligation it is, and the shape that
discharges it is unwritten. Filed as
[#117](https://github.com/ludo-lang/ludo/issues/117), which owns the repair of
§5.6, §5.7, this entry and the coverage rows in one commit. Until it closes, an
implementation choosing a vendor namespace SHOULD assume the spec's own codes
will claim a reserved prefix and choose accordingly.

---

## 14. What this chapter does not decide

Recorded so the boundary is legible, and so a later chapter is not read as
having inherited a silence.

- **The contents of a fault report** — the closed fault-kind enum, the concrete
  values, the call chain, the frame number, the `persist` names and types.
  Chapter 5 §6.5's, from #18 §8. This chapter carries only the envelope they
  ride in (§2.3). The enum's membership is chapter 5 §6.5.1's, seven members,
  closed there ([#113](https://github.com/ludo-lang/ludo/issues/113), resolved).
- **The experience contract's properties and their wording** — P5 (machine-
  readable diagnostics and the never-guess clause), P9 (no vendor syntax
  extensions) and P12 (no flag changes the accepted program set). Chapter 8's,
  from #19. §10.4 and §12.6 state the obligations those properties test; the
  properties themselves are numbered and worded there.
- **Which diagnostics exist.** Every mandated diagnostic is minted by the
  chapter stating the rule it enforces — chapter 1 §11.6's four, chapter 6
  §9.3's shader-rung error, and every *compile error* clause in chapters 1–6.
  This chapter states no diagnostic of its own.
- **The concrete code strings and the diagnostic-code namespace's contents** —
  the follow-on toolchain effort's (§5.7; ADR-0018 §14; #19 §E).
- **The CLI, the human rendering and the transport** — §1.2, permanently.
- **The bootstrap compiler's own warning configuration**, which is a different
  language and not this spec's business. (ADR-0023's opening; ADR-0020.)
