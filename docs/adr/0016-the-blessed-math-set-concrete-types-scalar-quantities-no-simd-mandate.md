---
status: accepted
---

> **Absorbed by [spec ch6 §3](../spec/06-stdlib.md):** the chapter is normative; this ADR
> keeps the argument.

# The blessed math set: concrete types, scalar quantities, no SIMD mandate

Issue #51 asked three questions the game-facing stdlib fog had carried since
issue #11 — which vector and matrix types ship, which `distinct numeric`
quantities ship, and whether SIMD lowering is a conformance obligation. Four
earlier resolutions had already narrowed it: #11 built the `distinct numeric`
machinery and left the quantity list open, #15 required positional constructors
because struct literals are named-field only, #25 shipped type-level `#align(n)`
so a SIMD-shaped type needs no privileged built-in, and #33 / ADR-0008 §6
confirmed *library, not built-in* and gave the shader emitter a veto over
whatever ships.

The ticket's own framing turned out to be missing its load-bearing question. All
three of its parts hang on one prior call — **are the blessed math types generic
over their element type?** — and answering that first collapses most of the rest.

## 1. The math types are concrete `f32` types, not generic over an element

`$.vec2` holds two `f32` fields. There is no `Vec2[T]`, no `Vec2[PxPerTick]`.

Monomorphisation (#11) would have made a generic vector free at codegen time, so
the objection is not cost. It is threefold:

- **ADR-0008 §6 recognises the blessed types *by name***, mapping them to GPU
  primitives. `Vec2[Radians]` has no GPU counterpart, so a generic vector forces
  the emitter to carry a second rule — *only `Vec2[f32]` crosses* — which is a
  second class of blessed name and a new checker diagnostic, bought for a
  facility the GPU cannot use.
- **A units-correct vector library needs dimensional algebra, and #11 declined
  that line deliberately.** `distinct numeric` is closed under `+` and scalar
  `*`, and `T*T` does not exist — which is exactly where `dot`, `length` and
  `normalize` live. A generic vector would therefore be units-correct for
  addition and silently unit-losing for every other operation on it: the
  plausible-looking wrong line that #10's rejection test exists to catch.
- It multiplies the `$.` root-name surface (#24) against nothing visible.

**The named cost, stated rather than discovered later: the `distinct numeric`
unit discipline stops at scalars.** Velocity is a `vec2` and a `vec2` has no
unit. This is a real forfeit — the push-n-pull `Units` module that reversed #11's
unit-system call was a *scalar* conversion boundary, and that is precisely the
half that survives here.

## 2. The set is 2/3/4 across the board, square matrices only, floats and integers

Shipped:

- **`$.vec2`, `$.vec3`, `$.vec4`** — `f32` components.
- **`$.mat2`, `$.mat3`, `$.mat4`** — square, `f32`.
- **Integer vectors at the same three widths.**

The rule is one sentence with no gaps: *2, 3 and 4 components; matrices square;
float and integer vectors*. ADR-0008 §6's veto is satisfied — every one of these
is a WGSL primitive.

Three calls inside that:

**Integer vectors ship.** Tile coordinates, grid indices and integer pixel
positions are pervasive in a 2D-first facade, and without a blessed type every
project declares its own `struct { x: int, y: int }` under its own name — #4's
43.7% naming failure manufactured on purpose. They are also GPU-expressible
(`vec2i` and friends), so the veto does not bite.

**`mat2` ships even though `mat3` subsumes it.** A 2×2 rotation-and-scale is
expressible as a `mat3`, so `mat2` is strictly redundant. It ships anyway,
because *2/3/4 across the board* is one rule and *2/3/4 for vectors, 3/4 for
matrices* is a rule plus an exception, and the exception costs more to remember
than the type costs to ship. Shader authors reach for `mat2` in fragment code.

**Non-square matrices and quaternions are rejected, both additive later.**
Non-square matrices are GPU-legal with near-zero gamedev demand. Quaternions are
not a GPU primitive at all, and #40 / #42 made 2D rotation a *scalar* — so a
quaternion has no client anywhere in the mandated facade today, and would bring
`slerp`, normalisation invariants and a normalise-or-not question with it.
Criterion 4 makes adding either later a pure addition, so declining now forecloses
nothing.

## 3. Arithmetic operators exist on the blessed math types, and nowhere else

`a + b`, `a - b`, `v * 2.0`, `m * v` work on the blessed types. No user type ever
gets this, and there is no way to ask for it.

Issue #11 recorded *no overloading* as a Tier 2 cost, so this is a stated
exception and it is argued rather than assumed. The forcing client is the shader
dialect: ADR-0008 observes that shader code is vectors and matrices in almost
every line, and WGSL, HLSL and GLSL all spell it `a + b * 2.0`. An emitter that
required `a.add(b.scale(2.0))` would be a dialect into which nobody's existing
shader knowledge transfers — #4's naming-and-familiarity evidence turned against
us, in the one place where the corpus of prior art is largest.

**Operators-only-inside-shaders was rejected on sight.** One notation on the CPU
and another on the GPU is the dialect variance criterion 4 forbids, and issue #21
already settled that there is no second syntax.

This is the **fourth use of the established privileged-compiler-knowledge shape**
— after #15's derived `Eq`/`Hash`/`Clone`, #25's SoA transform, and ADR-0008 §6's
recognise-by-name — not a new mechanism. It is cheap against #24: the operator
tokens already exist in the grammar, so the spend is a checker rule, not a
production.

Restrictions: `+` and `-` are elementwise on matching types; `*` is vector-scalar,
matrix-scalar, matrix-vector and matrix-matrix; **there is no `/` between two
vectors** and no elementwise `*` between two vectors — both are spelled as named
functions, because *which multiplication is this* must be visible at the call
site.

## 4. The quantities that ship are `Radians`, `Seconds` and `SampleFrames`

`Color` already exists as a `distinct` type from #28 and is unchanged.

**Each shipped quantity appears in a `$.` return type.** That is the test this
section applies, and it is what separates a compiler-enforced boundary from a
type the stdlib defines and never uses.

- **`Radians`** — 2D rotation is a scalar (#40, #42), degrees-versus-radians is a
  recurring real bug, and the conversion function is the visible boundary #11
  wanted.
- **`Seconds`** — `$.time.now` is the one genuine ambient clock #26 admitted.
- **`SampleFrames`** — see §5.

**`PxPerSec` and `PxPerTick` are dropped.** Velocity is a `vec2` in every real
use and §1 just established that a `vec2` carries no unit — so a scalar-only
speed type advertises a discipline it cannot keep at the exact site where it is
wanted. Separately, #26 deleted `dt` and fixed the step at 60Hz, so per-tick is
the *only* rate: there is no second rate for `PxPerTick` to be distinguished
from, and a distinct type that distinguishes nothing is surface without a job.

**A `Ticks` quantity is also rejected**, and named here rather than skipped in
silence, because the symmetry pressure toward it is real. Unlike sample frames,
`Ticks → Seconds` is exact — #26 fixed 60Hz as a compile-time constant. But #26
also made elapsed time a `persist` counter *the user declares*: the stdlib never
hands one out, so there is no signature to attach the type to. It would be a type
the stdlib defines and never uses. Users who want it declare it themselves —
`distinct numeric` is #11's machinery and was never stdlib-only.

## 5. `SampleFrames` ships, and ADR-0007's cursor signature is amended

**Superseded — ADR-0007, "The play cursor":**

```
$.audio.cursor() -> int
```

**Replaced by:**

```
$.audio.cursor() -> SampleFrames
```

The bare `int` was a live inconsistency inside this ticket's scope, not another
ticket's business: #51 owns *which `distinct numeric` quantities ship*, and the
audio facade already had a quantity with no type. Amending a closed ADR by
quoting the superseded line follows #25's correction of #15's stale premise —
recorded, never a silent edit.

The hazard is specific and ADR-0007 states it itself: the cursor is in frames **at
the current device rate, with a discontinuity across a device change**. So
`cursor() / 48000` is a plausible-looking line that is silently wrong the moment
the device changes — and issue #56 found that unsolicited device changes are
unspecified in every browser, so this is not a hypothetical.

Conversion therefore **takes the rate as an argument and never as a constant**:

```
$.audio.seconds(frames: SampleFrames, rate: int) -> Seconds
```

which makes the stale-rate bug unspellable rather than merely documented. This is
the conversion-boundary shape #11 reversed its unit-system call on, in its
clearest instance on the map.

`distinct numeric`'s closure rules fit the use exactly: cursor deltas, loop
points and ring offsets are addition and scalar multiplication, and
`SampleFrames * SampleFrames` correctly does not exist.

## 6. SIMD lowering is not a conformance obligation

**A conforming implementation is not required to lower operations on the blessed
math types to SIMD instructions.**

- **MUST-lower** makes conformance depend on a backend's vectoriser, whose
  failure has no named entity and no source location — criterion 5's textbook
  case. *Which line failed to vectorise* is not a question a diagnostic can
  answer.
- **#19's MUST-MEASURE** was the tempting third option and does not transfer.
  Reload latency measures one observable (save to visible) at three sizes defined
  by shape. SIMD throughput has no equivalent single observable; the spec would
  have to define and freeze a benchmark corpus, which is a far larger authoring
  commitment than the property is worth.

What the spec mandates instead is **everything that makes lowering possible, and
everything that makes it safe**:

1. **Alignment is specified on the blessed types**, via #25's `#align(n)`. `vec4`
   is 16-byte aligned by declaration, not by a backend's good luck — this is the
   part a library type could not previously state for itself and #25 supplied.
2. **The batch shape is `[]T` views**, already true by #15 and #25: a column *is*
   a `[]T`, so `integrate(pos: ![]vec2, vel: []vec2)` is an ordinary function and
   a vectoriser sees contiguous aligned data.
3. **Float evaluation is exactly as written** — no reassociation, no implicit FMA
   contraction, no fast-math, in any mode.

Clause 3 is the one that earns this section. It is not a performance rule, it is a
**determinism** rule: a backend that vectorises with reassociation produces
different floats from one that does not, for identical source. That breaks replay
and lockstep — the same reason #15 specified map iteration order — and it is a
P8-shaped divergence between two builds of the same program. Without it, the
freedom granted in this section would silently cost the property the map has been
protecting since #15.

The blessed types are therefore **a naming convention plus a layout guarantee plus
an evaluation guarantee**. Speed is quality-of-implementation, with the door held
open rather than nailed shut.

## 7. No blessed batch functions

The stdlib ships **no** `$.math.add_all`-shaped batch surface.

§6 mandated the batch *shape*, and that shape is already ordinary functions over
`[]T` — which was the whole win of #15 and #25. A blessed batch list would add
`$.` root names (#24) that buy nothing a user function does not already have, and
would give a loop a second spelling, against criterion 3.

Positional constructors are a separate matter and do ship, as #15 required:
`$.vec3(1, 2, 0)`, because struct literals are named-field only.

## 8. The lenses

**Simplicity — strong, with one honest forfeit.** The type set is one sentence
with no exceptions. Operators mean vector code reads like vector code in every
textbook the user has read, on both CPU and GPU. The forfeit is §1's: a beginner
who expects units on a velocity does not get them, and the reason is a line #11
drew for good reasons that this ticket did not reopen.

**Robustness — moderate, deliberately.** Three compiler-enforced quantities
delete three real confusion classes (degrees/radians, frames/seconds, and the
stale-rate conversion §5 makes unspellable). §6's clause 3 protects determinism
against a whole class of backend-dependent divergence. But §1 concedes that the
largest unit-bearing quantity in a game — velocity — is unprotected, and that
concession is stated rather than papered over.

**Agent-friendliness — strong.** Every math type is a `$.`-rooted name in the
oracle's name table, which is the direct mitigation for #4's 43.7% figure and the
reason integer vectors ship rather than being re-declared per project. Operators
match the largest existing corpus, so shader knowledge transfers unmodified. No
generic element parameter means no `Vec2[?]` inference question at any call site.

## 9. #24 delta

Recorded against the **stdlib root-name companion count**:

- **9 types** — `vec2`, `vec3`, `vec4` (float), three integer vector widths,
  `mat2`, `mat3`, `mat4`.
- **3 quantity types** — `Radians`, `Seconds`, `SampleFrames`.
- **Positional constructors** for each math type, per #15.
- **1 conversion function** — `$.audio.seconds(frames, rate)`.
- **1 amended signature** — `$.audio.cursor()`, no name added.

**Grammar count: unchanged.** §3's operators spend no productions — the tokens
already exist and the exception is a checker rule. This is the second result
#24's budget rewarded, after #25.
