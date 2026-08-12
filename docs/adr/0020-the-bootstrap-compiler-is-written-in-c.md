---
status: accepted
---

# The bootstrap compiler is written in C, and ludo self-hosts after v1

ADR-0001 recorded that the platform layer is written in ludo and therefore
cannot exist before the compiler does. It did not say what the compiler is
written in. It is written in **C**, ludo **self-hosts after v1 ships**, and the C
compiler is kept structurally portable into ludo so that the self-hosting
rewrite is a translation rather than a redesign.

## The four decisions this rests on

The host language is the last question in this chain, not the first. Four
earlier decisions cut the field before any language was named.

- **Self-hosting is a policy, not a prediction.** ludo self-hosts *after v1*.
  The bar for self-hosting is far lower than the bar for v1 — a self-hosted
  compiler needs a frontend, one codegen target and file I/O, and none of
  ADR-0001's eighteen-backend platform layer. What gates it is spec freeze, not
  calendar time.
- **One host language, two artifacts.** A throwaway frontend-only prototype
  (parse, typecheck, tree-walking interpreter, no codegen) proves the spec; the
  keeper follows. Same host for both, so the prototype's frontend carries
  forward instead of being genuinely discarded.
- **We write our own codegen backend**, with LLVM or Cranelift deferred behind a
  seam as an optional release-mode backend. #19's reload contract is the thing
  that cannot be bought from a third party — the same argument ADR-0001 makes
  one tier up — and ADR-0003 already commits us to hand-writing SPIR-V, DXIL and
  MSL emitters, so backend work is unavoidable regardless. Emitting C was
  rejected outright: it puts a C toolchain in the box, which is the position
  ADR-0003 declined.
- **The compiler ships as a single self-contained binary**, cross-compiling to
  every tier-1 target from any tier-1 host with no platform SDK installed. This
  is a consequence of ADR-0001's runtime dynamic loading, not a new decision,
  and it eliminates any host on a VM or requiring a runtime.

Not needing LLVM bindings is what re-opened the field: it is the requirement
that would otherwise have forced C++, Rust or Zig on a technicality.

## Why the self-hosting port is a selection criterion

The cost of the eventual rewrite is dominated by how many host constructs have
*no counterpart* in ludo. Each one is not a translation but a redesign — you
re-derive what the code was accomplishing and find another way to accomplish it.

ludo's recorded design is explicit allocators (#8), monomorphisation (#11),
errors in the return type (#10) and tagged sums. C matches all four: allocation
is explicit and caller-supplied, errors are return values, tagged unions are
struct-plus-enum, and there is no hidden control flow — no destructors, no
implicit conversions, no ambient allocation. Rust does not match any of them;
ownership, borrowing, lifetimes and traits are load-bearing in idiomatic Rust
and have no ludo counterpart, so a Rust compiler would have to be redesigned
rather than ported.

The non-obvious finding is that this criterion does **not** force a thin
ecosystem. Zig and Odin match ludo's model, but so does C — and C has the
largest corpus of any language, which is exactly the tax Zig and Odin were
supposed to make us pay.

## Why C rather than Zig

Three of these come from decisions already in this repo rather than from taste.

- **C is already a required dependency.** ADR-0001 binds Win32, Cocoa, Vulkan,
  Metal, D3D12, WASAPI and CoreAudio through the C ABI (#12), and explicitly
  admits a hand-written C/Objective-C shim where blocks, ARC or Objective-C
  exceptions force it. Every other host adds a *second* systems language to a
  project that needs C regardless.
- **The bootstrap chain stays reproducible.** Once ludo self-hosts, building
  from source runs `C compiler → ludo₀ → ludo₁` forever, on any machine in any
  decade. A pre-1.0 Zig bootstrap pins that chain to a specific compiler version
  that will be unavailable long before ludo matures. This cost lands on us
  precisely *because* we committed to self-hosting.
- **Zero migration tax on non-ludo work.** Zig breaks across pre-1.0 releases;
  Odin is likewise pre-1.0 and, per `docs/research/02-language-survey.md`, has
  deferred its own self-hosting past 1.0. Riding either means repeated compiler-
  migration work during the years that should go to designing a language. The
  survey's central finding is that these projects die from spending a decade
  not-shipping.
- **Zero impedance with the surface being designed.** We are specifying `extern`
  declarations against C headers (ADR-0006, ADR-0008). When the host *is* C, the
  header is the spec and no binding-generator sits between the design and its
  validation.
- **The closest reference implementation is C.** ludo is Lua-simple by
  intention, and Lua is ~30k lines of readable C — the best available reference
  for this exact artifact.
- **The robustness toolchain is complete.** ASan/UBSan/MSan, Valgrind, AFL++ and
  libFuzzer, all proven on compilers specifically. We do not get safety by
  construction, so we buy the best available detection instead.
- **The browser is the most trodden path.** C→WASM via clang/Emscripten is how
  CPython, SQLite, Lua and Clang itself reached browsers.

## Considered options

- **Zig.** The best fit on the port criterion, with sum types, exhaustiveness
  checking, slices, `defer` and cross-compilation in the box. Rejected on the
  bootstrap chain and the pre-1.0 churn tax, both of which bite specifically
  because of decisions already made here.
- **Odin.** Same model as Zig, gamedev-native, smaller ecosystem, pre-1.0, and
  its own compiler is still C++ with self-hosting deferred — betting on a
  toolchain whose maintainers have not yet made that bet themselves.
- **Rust.** Best tooling, safety by construction, largest modern corpus.
  Rejected because it loses the port: the structure of a well-written Rust
  compiler is organised around features ludo does not have.
- **Emitting C as the codegen backend** (as distinct from writing the compiler
  in C). Rejected: it puts a C toolchain in every user's box, contradicting
  ADR-0003.
- **A deliberately minimal throwaway bootstrap**, the Rust-from-OCaml move.
  Rejected because it fights the decision to carry the prototype's frontend into
  the keeper.

## Consequences

- **Hand-rolled containers and no exhaustiveness checking.** AST and IR node
  dispatch is the dominant pattern in a compiler, and a missed case is a silent
  bug rather than a compile error. Mitigation is a house style — tagged unions
  with a canonical switch helper, and a warning configuration that makes
  non-exhaustive switches an error — not vigilance.
- **Manual cleanup discipline.** No `defer`. Arena-per-compilation-unit
  allocation, freed wholesale, is the answer for almost everything.
- **The review burden shifts, it does not shrink.** Agents write C fluently and
  confidently, including confident undefined behaviour. The failure mode moves
  from hallucinated APIs to subtle UB that passes tests — arguably the worse of
  the two, and the reason sanitizers and a parser fuzzer run in CI from the
  first commit rather than being adopted later.
- **`zig cc` is the build and cross-compilation tool.** This satisfies the
  single-binary, zero-install requirement that C alone does not answer. It is a
  build-time dependency that can be swapped, not a codebase we are married to —
  a deliberately different commitment from writing the compiler in Zig.
- **A written C subset.** C11, with the sharp edges banned in the coding
  standard rather than left to judgement.
- **The compiler frontend is a library, not a binary's internals.** Parser,
  typechecker and diagnostics are embeddable, with no ambient I/O, error-
  tolerant parsing and stable source spans; the CLI is one consumer among
  several. This is what makes the playground, an LSP and a formatter buildable
  at all, and retrofitting it is a frontend rewrite.
- **The browser is a shaping constraint on the toolchain**, exactly as ADR-0001
  makes it one on the platform-layer API: no ambient I/O and no assumed threads
  in the compiler, with the `wasm32` target itself deferred.
- **Writing the compiler in C is a continuous test of ludo's own decisions.** If
  explicit allocators (#8) and errors-in-the-return-type (#10) are painful to
  write a compiler in, we find that out now, on a program we control, rather
  than after mandating them in the spec.
- **Nothing here is built yet.** As with ADR-0001, this is recorded because it
  is a shaping constraint on work that has not started, not because work starts.
