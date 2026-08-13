---
status: accepted
---

# The byte-exact blit is withdrawn, and a texel-selection rule replaces it

Issue [#69](https://github.com/adamico/ludo/issues/69) was split out of
[#67](https://github.com/adamico/ludo/issues/67) / ADR-0030 §7, which
substantiated a challenge to ADR-0010's byte-exact blit on three independent
grounds and then declined to decide it, on the reasoning that a facade-wide
withdrawal buried inside a letterbox ADR is how the original defect happened.
ADR-0033 §10 declined it a second time on the same reasoning. This ADR is the
one whose subject it is.

The clause, in ADR-0010's paint vocabulary:

> **A `stretch` texture paint whose source extent equals its destination extent
> in device pixels, at integer logical coordinates, unrotated, under an identity
> or integer-translation transform, on a `crisp` target, is a byte-for-byte
> blit.**

Plus ADR-0022 §4's implementation condition: *"Backends must implement this case
as a texture-to-texture copy, not a rasterised quad."*

**Both are withdrawn.** ADR-0022 §2's interior/exterior rule stands as the
corpus's statement of geometric exactness, and gains one sentence covering
texture sampling, which it never covered.

## 1. Five grounds, of which two are new

#67 established three. Two more arrived during this grilling, and one of them is
the plainest of the five.

**(1) It is unsatisfiable in the facade's own vocabulary.** ADR-0022 §4 derives
the guarantee from a condition list that includes *no blending*. ADR-0009 gives
every descriptor a `blend: Blend` **defaulting to `alpha`**, over a closed enum
`{ alpha, additive, multiply }` with **no `none` or `opaque` mode**. Every
expressible draw is therefore blended, rides the rasterised path, and inherits
WebGPU §23.2.7's implementation-defined float-to-unorm store rounding — which is
the exact reasoning ADR-0022 §4 used to withdraw the tinted hit flash. It
withdrew the tint case for shader multiply and missed that alpha blending
disqualifies the untinted case identically.

**(2) It is false on its face for any source with alpha < 1.** *(New.)* The
clause as ADR-0010 states it carries **no blend condition at all**. Read
literally, it promises a byte-for-byte blit for a semi-transparent texture drawn
over a non-black background — which is not merely underivable from WebGPU, but
plainly untrue arithmetically, on every backend, for reasons that have nothing to
do with any graphics API. ADR-0022 §4 derived the guarantee under *no blending*
and then blessed a restatement that omits the condition; the omission was never
noticed because the clause was read as being about sprites, where opacity is the
common case.

This ground is worth recording separately from (1) because it is the one a reader
can check without opening a specification. Grounds requiring three cross-document
derivations are how a false clause survives four ADRs.

**(3) Where it is not vacuous, it mandates an implementation with a performance
cliff.** A texture-to-texture copy is a command issued outside a render pass, so
a pixel-art game drawing 2000 sprites at integer coordinates breaks its batch
2000 times — in the exact case the facade exists to make fast. It is also a
**second** constraint on how an implementation compiles, alongside ADR-0012's
forwarding function.

**(4) Its protective content is duplicated, derivably.** ADR-0022 §2 already
promises interior and exterior exactness on a `crisp` target, and boundary
exactness for axis-aligned geometry whose edges lie on integer framebuffer
coordinates. A sprite quad at integer coordinates *is* that geometry. The
remainder is the texture-sampling half — which §3 below states, because it turns
out nothing stated it.

**(5) It is vacuous at every window size but one.** *(New in force, if not in
observation.)* ADR-0030 §3 made the logical canvas a coordinate mapping rather
than an intermediate framebuffer, so the clause's *source extent equals
destination extent in device pixels* precondition self-disables whenever the fit
scale `k ≠ 1`. ADR-0031 then deleted integer scaling, making `k` real, and
ADR-0032 fixed the canvas at 1280×720. The composition is that `k = 1` holds
**only when the window is exactly 1280×720** — not a common case, not a case a
program can bring about, and not one a player has any reason to arrange. The
guarantee is unreachable in ordinary play.

## 2. Why withdrawal beats making it satisfiable

The fork was withdraw, or add an `opaque`/`none` variant to `Blend` so the
clause's derivation has an expressible precondition.

The second branch is refused, and the map's own priority ordering decides it
rather than any judgement made here:

> **Performance and frame stability above rendering fidelity.** Where a fidelity
> guarantee costs throughput, frame pacing, or stability on a major platform, the
> guarantee loses.

Adding `opaque` makes the **ordinary opaque sprite** the mandated
copy-not-quad case. That is not a marginal cost paid in a corner: it is a batch
break per sprite in the single hottest path the facade has, paid on every
platform, to buy a guarantee that ground 5 shows is unreachable unless the player
happens to size their window to 1280×720. It also costs a `Blend` variant against
[#24](https://github.com/adamico/ludo/issues/24).

This is ADR-0031's bite applied a second time, and it is again not close. ADR-0022
withdrew the hit flash on materially identical reasoning; this is the case that
reasoning missed.

## 3. What replaces it: one sentence, about texel selection

ADR-0033 §3 deleted ADR-0004's bit-exactness bullet, leaving **ADR-0022 §2 as the
sole surviving statement of graphics exactness in the corpus**. §2 is about
*geometry coverage* — which pixels a shape covers. It says nothing about **texel
selection** — which source texel nearest sampling reads. #67's ground 3 asserted
that nearest filtering at exact texel centres "supplies" that half, and it does,
but no clause anywhere states it. Withdrawing the blit without adding it would
leave texture sampling entirely unspoken.

So ADR-0022 §2 gains:

> Under `crisp`'s nearest sampling, where a sample point lies strictly inside a
> texel, that texel is selected. Where a sample point lies on a texel boundary,
> selection is **implementation-defined**.

Three things about its shape are deliberate:

- **It mirrors §2's own interior/boundary split**, so the corpus has one rule
  shape for exactness rather than two. Strictly-inside is determined; on the
  boundary is not.
- **It promises which texel, never what byte.** The value half died with the
  copy-not-quad condition: without a copy, §23.2.7's store rounding is live, and
  §0's rule forbids promising through it. Selection is a function of coordinates
  alone and survives.
- **It is derivable**, which is the whole test. It is the half of ADR-0022 §4's
  own derivation that does not depend on the draw being a copy.

## 4. The phantom texel-centre convention is deleted, not written

ADR-0005 closes its sprite section with *"Fractional and rotated cases get the
explicit texel-centre and tie-break convention."* ADR-0010 §"The byte-exact blit,
restated" says those cases *"keep ADR-0005's texel-centre and tie-break convention
unchanged."* ADR-0006 §"Specifiability" cites *"a rotated sprite, which needed a
texel-centre tie-break rule"* as a thing that existed.

**No such convention was ever written.** A search of `docs/adr/` and `CONTEXT.md`
finds three citations and no statement. ADR-0005 promised it in the future tense
and never delivered it; ADR-0010 preserved a thing that was not there; ADR-0006
reasoned from it.

This is the **third** phantom clause the corpus has produced, after ADR-0011's
pointer-in-bars clause and the never-written "integer scale", both found by #67.
The pattern is now established well enough to name: **a clause cited by a later
ADR reads as existing, and the citation is what makes it unfalsifiable.** Nobody
greps for a rule three documents agree they are relying on.

The three citations are deleted rather than the convention being authored, for the
reason §0 gives. §2 already declares boundary coverage implementation-defined for
fractional and rotated geometry, and §3 above declares boundary texel selection
implementation-defined. A tie-break convention would have to pin down exactly the
rounding mode and reassociation freedom that killed the `crisp` bit-exactness
headline. Writing it now would author a promise on the same ground ADR-0022 cleared.

**The fractional and rotated cases are therefore covered, and covered honestly:**
interior determined, boundary implementation-defined, on both the coverage and the
sampling axis.

## 5. ADR-0012's pending stamp is discharged, by appending

ADR-0033 §6 stamped ADR-0012 recording that the forwarding-function guarantee is
*one of two* constraints on how an implementation compiles, the second being
copy-not-quad — **contingent, not false**, and pending this issue. `CONTEXT.md`'s
*Forwarding function* entry was amended to match, reading "one of two, pending
#69".

§1 withdraws copy-not-quad, so **ADR-0012's "exactly one" is restored** and the
contingency is discharged.

The mechanism is worth stating because it was grilled. ADR-0033's stamp is **not
rewritten and not reverted**. This ADR appends a further stamp beneath it. That
is the corpus's established convention — ADR-0005 carries five stacked stamps,
ADR-0025 four — and the precedent is exact: when ADR-0025's ADR-0030 stamp went
stale, ADR-0031 **appended** a stamp reading *"the stamp above is superseded"*
rather than editing it. ADR-0033 §2 states the rule outright: *a stamp is itself
amendable content.*

Reverting would erase that a second compilation constraint was proposed, shipped,
and withdrawn. ADR-0012 §1's *"the spec owes exactly one normative thing"* is
stronger with that history attached than without it, because the next person
proposing a second constraint should meet the one that failed.

`CONTEXT.md` is treated differently, and the difference is principled: it is the
live single-context document, not a dated record, so it is **edited in place** to
read *one* constraint again, pointing here.

## 6. Against issue #19: nothing is owed

#19's property list (P1–P13) was read in full. **No property tests the blit**, and
none tests graphics exactness at all — ADR-0022 added none when it wrote §2 and §3.
The withdrawal is therefore free against the experience contract.

§3's new sentence gets **no property either**, and the consistency argument is the
reason: pixel exactness has always been conformance-suite territory rather than
P-list territory, the P-list being behaviour-contract shaped — reload, faults,
diagnostics, the fit. Adding a property for a sentence introduced here, while §2's
older and larger claims have none, would make the list incoherent. #19's own line
holds: **this spec owes the assertions, the follow-on effort owes the harness.**

## 7. What this ADR does not do

**ADR-0009's line 372** still asserts that the hit flash is *"byte-exact on a
`crisp` target"* — a claim ADR-0022 §4 withdrew and never stamped at that citation
site. It is stale and it is in this clause family, which argues for fixing it here.
It is nonetheless **routed to [#72](https://github.com/adamico/ludo/issues/72)**,
whose whole subject is missing amendment stamps, on ADR-0033 §10's rule that
facade-wide corrections buried in unrelated ADRs are how these defects propagate.
Recorded here so it is not rediscovered.

**`docs/research/07-browser-semantics-of-the-mandated-facade.md` is not patched.**
It records what was true of the corpus when it was read, including its finding
that clause 2c was "supplied". Research documents are dated findings, not
normative text, and editing one to match a later decision destroys the evidence
the decision was made against.

**ADR-0030 §4–§6 are untouched.** The fit clause and the uniform-scale rule stand
on grounds independent of this outcome, as #69 said when it was filed.

## Against issue #24: zero delta

Nothing here adds or removes a keyword, an operator, a facade call or a stdlib
root name. The rejected branch would have cost a `Blend` variant; the accepted one
costs nothing. Second data point for ADR-0022 §6's observation that **a conformance
re-cut can be free against the grammar budget**, because the budget counts surface
and this ADR changed only what the surface promises.

## Amendments this ADR makes

- **ADR-0005** — the *byte-exact blit is normative in its own right* paragraph is
  **deleted** (§1); its trailing citation of the texel-centre and tie-break
  convention is deleted with it (§4).
- **ADR-0006** — the parenthetical citing *"a rotated sprite, which needed a
  texel-centre tie-break rule"* is corrected: no such rule exists (§4).
- **ADR-0010** — §"The byte-exact blit, restated and strengthened" is **deleted**
  in full (§1), including its texel-centre citation (§4).
- **ADR-0012** — the pending contingency is **discharged**: the forwarding-function
  guarantee is again the spec's **one** constraint on how an implementation
  compiles (§5).
- **ADR-0022** — §4 is **deleted**, both the guarantee and the copy-not-quad
  implementation condition (§1); §2 gains the texel-selection sentence (§3).

## How it fares on the three lenses

- **Simplicity.** A net deletion: one fewer promise to learn, and one fewer
  vocabulary of conditions — *1:1 device extent, integer coordinates, unrotated,
  identity-or-integer-translation, `crisp`* — that a beginner had to check before
  knowing whether a guarantee applied to their draw. What replaces it is the rule
  they already met in §2, extended to a second axis.
- **Robustness.** Another net loss in stated guarantees for a net gain in true
  ones, ADR-0022's trade taken a second time. The specific gain is that the
  spec no longer contains a sentence that is arithmetically false for the ordinary
  semi-transparent sprite (§1 ground 2).
- **Agent-friendliness.** The direct win, and it is ADR-0022's exactly: an agent
  reading the withdrawn clause writes a test asserting byte equality, and that test
  fails on conforming implementations at every window size but one. Deleting the
  clause deletes the failure mode. §3's replacement names its condition in its own
  sentence, which is criterion R1 applied to prose.

**Where the lenses conflict:** they do not, here, and that is itself worth
recording. The usual tension — a guarantee that helps the pixel-art author against
a cost paid by the implementation — dissolves once ground 5 is seen, because the
guarantee was not reaching the pixel-art author either. A clause that costs
throughput and delivers nothing is not a trade-off; it is a defect, and the three
lenses agree about defects.
