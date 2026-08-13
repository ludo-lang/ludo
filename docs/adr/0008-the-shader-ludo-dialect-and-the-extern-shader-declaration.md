---
status: accepted
---

> **Amended by [ADR-0037](0037-a-backend-clears-a-floor-and-shaders-are-the-third-rung.md):** the two
> authoring paths specified here **define the shader conformance rung**. An `extern` shader
> declaration or shader-ludo source, built against an implementation that does not claim
> that rung, is a **compile error naming the declaration** under ADR-0018's envelope — not
> a runtime fault, because ADR-0006 R3 makes the rung known at compile time. Criterion 4 is
> not reopened: `extern` is visible in the signature, per ADR-0019 §2.

# Shader ludo is a strict subset with a vacuous guarantee set; `.wgsl` is reached by an extern declaration, not ingested as an asset

ADR-0003 settled the *shape* of the shader story — two authoring paths, one shader
IR, three emitters — and deliberately left the hardest part open: what shader
ludo actually **is**. It named the dialect as the real cost of the decision and
recorded that it puts genuine pressure on issue #16's *two modes of one language,
not two languages*.

Three things landed after ADR-0003 was written, and they move the ground under it:

- **ADR-0004** delegated the renderer — ours, own repo, blessed by reference,
  **unmandated** — and the mandated drawing facade exposes no shader call at all.
- **ADR-0006** set the precedent for what a decision on a delegated tier produces:
  not the tier's API, but the **rules the API must obey**, plus what those rules
  force onto the language.
- **ADR-0007** confirmed the shape: the tier is delegated, the language-level
  consequences are ours.

**Shaders are opt-in.** A conforming implementation may support none, and a game
may ship without touching one. That is not a caveat — it is what makes every call
below affordable.

## 1. The map owns the dialect and the checker; the IR and the emitters are the renderer's

The shader IR and the SPIR-V/DXIL/MSL emitters are **renderer-repo work: ours,
blessed by reference, unmandated** — the renderer's exact treatment under
ADR-0004, and the audio engine tier's under ADR-0006.

What stays here is what cannot be delegated:

- **The dialect's rules**, because shader ludo is ludo, and nobody outside the
  language spec may define a flavour of it. Delegating this is criterion 4 (*no
  dialect variance*) failing by outsourcing rather than by decision.
- **The checker obligation** on the `.wgsl` path, because it is the compiler's
  job and because #19 P4 (*no unchecked escape path*) does not have an exception
  for optional facilities.

The IR's *existence and ceiling* is recorded as a binding constraint — the
dialect may not exceed what the shared IR expresses, which ADR-0002 fixed at
WebGPU's intersection — but its contents are not specified here.

**Rejected: the map owns the IR too.** Mandating an IR makes *conforming* mean
*shipped three GPU backends*, which is ADR-0004's rejected full-cover argument
with different nouns.

**Rejected: the map owns none of it.** Shader ludo would then be a dialect of
ludo defined outside the language spec. See above.

## 2. Strict subset is a binding requirement, not an aspiration

Everything shader ludo accepts means what it means on the CPU. The requirement
binds in both directions, and the two directions are not symmetric.

**Removals are free.** No allocators, no pools, no handles, no `List`/`Map`, no
`defer`, no `unsafe`, no FFI, no recursion. A restriction is not a second
language; this is what the *two modes of one language* line was always about.

**Additions are the threat**, and the rule is: **anything that cannot be
expressed with existing language machinery is excluded from the dialect rather
than granted new syntax.** In practice every addition this ticket found does fit:

- Texture and sampler handles are **`distinct` opaque types** in the reserved
  `$.` root — #29's shape for a C resource, applied unchanged.
- Stage builtins and sampling are **ordinary free functions** in a shader-only
  stdlib module.
- **Address spaces ride on the binding declaration**, not on the type, so they
  never become a type constructor and never propagate.
- **Uniformity is a checker rule with a named diagnostic, never a type-system
  feature** — the same move #17 made for reload-safety, for the same reason: a
  property computed and reported beats a property carried in the type, which
  would infect every signature it touches.

## 3. The guarantee divergence, and the line it forced

ADR-0003 did not mention this, and it is the sharper attack on #16. Section 2 is
about *features*; criterion 4 is about *semantics*. Four locked guarantees have
no GPU implementation:

1. **#8: no out-of-bounds access, ever.** On a GPU an out-of-range read is
   clamped or returns zero by the API's own rules.
2. **#10: integer overflow is always checked and is a bug.** GPU integer
   arithmetic wraps. There is no trap to install.
3. **#18: a bug ends the simulation, `persist` is kept, the fault report names
   concrete values.** A shader has no faults, no call chain, nothing to pause.
4. **#25 §10: `?T` costs a tag word always.** Reachable, but a tag word in a
   uniform buffer is a real cost for a type with no `rescue`/`or` story worth
   having on the GPU.

So `xs[i]` faults attributably on the CPU and silently reads zero on the GPU:
**a behavioural change with no signature change**, which #4 measures as the worst
class at 38.0%.

**The call: cut it out where possible, write it down where not** — #9's
treatment of `nil` and #15's of iterator invalidation, applied again. Delete the
failure rather than report it.

### Cutting it out

**Indexing is deletable.** If every index into a fixed-size array is provably in
range — a `0..<N` loop over an `[N]T`, which #15 already makes correct by
construction — there is no out-of-bounds case to have a behaviour for. The
guarantee is not violated; it is **vacuous**.

Texture *sampling* is not indexing and survives untouched: clamp-vs-repeat is a
**declared parameter of the sampler**, so the behaviour is in the signature, not
a silent fixup.

What this deletes is **dynamic indexing into a variable-length storage buffer**,
which is the entire point of a compute shader. Hence:

> **Shader ludo covers the vertex and fragment stages. Compute shaders are
> authored as `.wgsl` only.**

Vertex and fragment shaders read fixed-shape inputs and sample textures; they do
not need dynamic buffer indexing. Compute exists to do the thing being deleted.

The cost is real and stated plainly: **someone who wants a compute shader must
learn WGSL.** But that is the *existing* second authoring path, aimed at exactly
this person, not a new limitation — and adding compute to shader ludo later is
additive, never a break.

### Writing it down

**Integer overflow is not deletable** — nothing stops a user multiplying two
large integers — so it lands as a **named, enumerated exception** on #19 P8's
model: a difference written into the spec, never a silent one.

**Rejected: emit the checks in the shader** (clamp, plus an error buffer read
back). It costs a readback per frame, and discovering a bug one frame late is
not error-as-pause.

## 4. The marker is an attribute, not a keyword

A shader function is a **marked declaration, never a magic name** — #12's
argument transfers without modification: misspell a magic name and the program
compiles, runs, and silently does nothing, which is criterion 4 *and* criterion 5
at once.

The spelling is **`#vertex` and `#fragment` attributes**, reusing the mechanism
`#explicit` (#6) and `#align(n)` (#25) already established. **Zero new keywords.**
New keywords were rejected under #24's tiered rule: a semantics-bearing spend must
name a failure class it deletes, and an optional facility on an unmandated tier
has no business spending core surface.

The marker is also what puts the checker into shader mode, so a function is
either checked as CPU ludo or as shader ludo, never ambiguously.

## 5. Shared functions are the whole prize

**An unmarked function that stays inside the subset is callable from both CPU and
shader code.** Write `smoothstep` once, use it in both places. This is why §2's
strictness was worth its cost — without it, the subset requirement would be
decoration.

**Calling a `#vertex` or `#fragment` function from CPU code is a compile error.**
It cannot run: there is no invocation, no fragment position, nothing. A named
error rather than a silent nothing, on #12's magic-name argument — a marked entry
is invoked by the machinery that owns it, never by hand.

## 6. Vectors and matrices: the stdlib fog gains a client with a veto

Shader code is vectors and matrices in almost every line, and the GPU has `vec4`
as a genuine primitive. The map's game-facing stdlib fog still carries *are
vectors a language feature with SIMD lowering, or a library type?*, which #25
pushed toward **library** by giving a library type `#align(n)`.

**They stay library types in the `$.` root, and the shader emitter recognises the
blessed math types by name**, mapping them to GPU primitives. That is the same
privileged compiler knowledge already granted for compiler-derived `Eq`/`Hash`/
`Clone` (#15) and the SoA transform (#25) — a third use of an established shape,
not a new one.

**Rejected: vectors become built-in types.** It reopens a settled direction and
spends core grammar for an optional facility.

**Consequence handed to the fog: the blessed math set now has a second consumer
with a veto.** Whatever ships must be expressible on the GPU, which effectively
fixes it at 2/3/4-component float vectors and square matrices.

## 7. Sampling is two calls, because the difference must be in the signature

Sampling with automatic detail-level selection is only defined when neighbouring
pixels take the same code path. Inside an `if` or a loop where they might not,
the result is undefined — **silently**, which is §3's problem returning.

Both HLSL and WGSL solve it identically, and so do we:

- **`sample(tex, uv)`** — automatic detail level, legal only in straight-line
  fragment code.
- **`sample_level(tex, uv, level)`** — caller states the level, legal anywhere.

The restriction on the first is **a checker rule with a named diagnostic**, per
§2. Two calls rather than one satisfies criterion 2: which one was called is
visible at the call site, so the difference is never hidden.

## 8. `.wgsl` is reached by an extern shader declaration — and "asset" is retired

ADR-0003 called a `.wgsl` file an **asset**, in the narrow sense of *a file the
toolchain reads*, and CONTEXT.md recorded the framing as load-bearing: describing
WGSL as "a syntax ludo also accepts" reopens #16 and #24.

**The framing was right and the word was wrong.** "Asset" collides with #12,
which ruled the **asset pipeline out of scope** — no import step, no build step,
no processing stage between a file and your program — a line #40 reinforced when
it excluded DragonRuby's `@150` variant selection *specifically as an asset
pipeline*. One word was carrying two meanings: *not a language dialect* (what
ADR-0003 wanted) and *goes through a pipeline* (what #12 forbids).

> **A `.wgsl` file is named by an `extern` shader declaration in ludo source** —
> the identical shape to #29's `extern "SDL3" fn`, which names library and symbol
> in the file, with no build configuration anywhere. The compiler reads the file
> when it reads the declaration.

This keeps everything ADR-0003 wanted from the asset framing — not a dialect, no
grammar cost, no second ludo syntax — and drops the implication that broke it.
**The term "shader asset" is retired from CONTEXT.md** and replaced by *extern
shader declaration*.

**Rejected: a build step compiles shaders into an intermediate form first.** That
is the pipeline #12 rejected, wearing a shader hat.

**Note the difference from #29 in one respect: no `unsafe`.** #29 puts `unsafe`
at every FFI call site because nothing checks the C side. §9 checks the shader
side, so the hazard #29's mark exists to make greppable does not exist here.

## 9. The interface check: three comparisons, a hard error on the unnameable, reported at the ludo declaration

A `.wgsl` file is not self-contained — it declares what it expects to be handed,
and ludo code hands it over. Disagreement produces no error and a wrong image.
ADR-0003 requires the toolchain to parse the **declaration surface only** —
never expression semantics, never codegen, because reading shader bodies means
owning a WGSL compiler, which ADR-0003 rejected.

**Three things are compared, and nothing more:**

1. **The parameter block's layout** — field order, types, sizes.
2. **The resource list** — how many textures and buffers, in which slots, of
   which kind.
3. **The vertex input list** — the per-vertex data the shader reads.

The first is decidable **only because of #25**: every ludo struct's byte layout
is computable from its declaration text alone, order guaranteed, no reordering,
no niche optimisation. Without that, *does this struct match that struct* would
be unanswerable. Recorded as #25 paying off in a place it was not written for.

**A WGSL type ludo cannot name is a compile error naming the type and the
field** — never accepted-and-skipped, which is the unchecked hole #19 P4 forbids
in the place it is hardest to debug. This is #29's call applied unchanged: a type
that cannot cross the boundary is a named local error, never a silent guess.

**The mismatch is reported at the extern shader declaration in ludo source**,
with the `.wgsl` file and its declaration named as the counterparty. The ludo
side is the code the author is writing, the side the compiler can attribute, and
the side that changes when the mismatch is fixed. The WGSL location appears in
the message; it is not where the error *is*.

## 10. The reload path goes to #19, scoped to full conformance

ADR-0003 put shaders inside #19's reload bound with a **last-good-pipeline
fallback** on failed recompile, and routed it away from #18's error-as-pause: a
broken shader edit is an error value per #10, not a fault. That is observable
behaviour, which makes it #19's currency.

But #19's properties are conformance obligations and shaders sit on an
**unmandated** tier — a core-conformant implementation has no pipelines to keep.
So it lands as **a property scoped to full conformance**, on the core/full split
ADR-0004 established:

> A failed shader recompile retains the last-good pipeline, reports an error
> value, and does not fault.

## Consequences

- **Core grammar cost: zero keywords, zero new productions.** `#vertex`/
  `#fragment` are attributes; `extern` is already spent by #29; textures and
  samplers are `distinct` opaque types; `sample`/`sample_level` are stdlib names
  recorded as a **#24 root-name companion delta**, as #41 recorded `$.audio`'s.
  **#24's budget rule bit correctly** — an optional facility on an unmandated
  tier bought itself no core surface at all. Its second real application after
  the labelled-break rejection.
- **#16's line holds**, and holds by construction rather than by assertion:
  shader ludo differs from CPU ludo only in what is *permitted*, with the one
  semantic difference — integer overflow — enumerated rather than discovered.
- **ADR-0003 is amended twice**: the dialect it left open is specified, and its
  "asset" framing is replaced by the extern shader declaration.
- **CONTEXT.md**: *Shader asset* is retired and replaced by *Extern shader
  declaration*; *Shader ludo* gains the vertex/fragment bound.
- **Handed to #19**: one property, scoped to full conformance (§10). Fourth
  property this map has handed it, after #29's two and #32's.
- **Handed to the game-facing stdlib fog**: the blessed math types now have a
  consumer with a veto (§6).
- **Compute shaders in shader ludo remain unwritten, deliberately.** The route
  back is additive: give the dialect a bounds-checked storage-buffer access whose
  failure has a stated behaviour, and the stage becomes expressible. Nothing here
  forecloses it.

## Lenses

**Simplicity — strong, and cheaper than expected.** The beginner who never writes
a shader meets nothing new; the beginner who does meets ludo with fewer features,
not a second language, and shares `smoothstep` between CPU and GPU. The whole
chapter costs one attribute family and a stdlib module.

**Robustness — strong, with one forfeit named.** The silent-wrong-value class is
deleted structurally rather than reported, and the one place it could not be
deleted (integer overflow) is written down instead of discovered. The forfeit is
compute, and it is a capability forfeit rather than a safety one.

**Agent-friendliness — strong.** The marker is a declaration rather than a magic
name; which sampling call was used is visible at the call site; an unnameable
WGSL type is a named local error rather than wrong marshalling; and the one
remaining CPU/GPU semantic difference is a written sentence rather than something
an agent has to infer from the target.
