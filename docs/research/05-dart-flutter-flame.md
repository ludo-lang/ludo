# Dart, Flutter and Flame: the closest thing to what this language wants to be

**Research ticket:** _(none — ad-hoc)_ · **Serves:** [#12](https://github.com/adamico/ludo/issues/12), [#17](https://github.com/adamico/ludo/issues/17), [#9](https://github.com/adamico/ludo/issues/9) · **Date of evidence:** 2026-07-25

Survey [02](02-language-survey.md) named the unoccupied position as *"a natively-compiled, statically-typed, non-lifetime-managed game language whose robustness comes entirely from the cheap features, whose iteration loop is DragonRuby's, and whose compiler is designed as a machine-readable verifier for an agent loop"*. It also named DragonRuby as the only thing that had the loop, and nothing in the compiled-static column as having it at all.

That survey did not look at Dart. It should have. Dart is a statically-typed, soundly-null-safe, AOT-compilable language with a generational GC, sum types with compiler-enforced exhaustiveness, stateful hot reload that preserves live object state, an error model that replaces the failing subtree rather than killing the process, an official MCP server that lets an agent drive and hot-reload a running program, and 2.04 million public GitHub repositories. It is backed by Google. It has a first-party UI framework and a well-maintained community game engine.

The uncomfortable framing is this: **Dart occupies roughly four of ludo's five legs already, and the one it does not occupy — being a systems language with manual memory control and gamedev-native data layout — is the leg ludo has least evidence anyone is asking for.** This document is an attempt to establish exactly which ground is taken and exactly where the seams are.

Two measurements frame it. Public repository counts from the GitHub search API, measured 2026-07-25 (`gh api search/repositories -f q='language:X'`; Linguist-classification dependent, order-of-magnitude only):

| Language | Repos |
|---|---:|
| TypeScript | 17,222,602 |
| **Dart** | **2,036,174** |
| Rust | 1,268,331 |
| Lua | 707,437 |

Dart's corpus is **1.6× Rust's and 2.9× Lua's**. And on pub.dev, `Results 74124 packages` as reported by the package index header ([pub.dev/packages](https://pub.dev/packages), measured 2026-07-25).

Set against that: Dart is 5.9% of respondents and **19th** in the [2025 Stack Overflow survey](https://survey.stackoverflow.co/2025/technology). Corpus and mindshare are not the same number.

---

## Dart the language

### Simplicity vs Lua — Dart is not the simplicity model

Dart is C-family/Java-shaped: classes, interfaces, `new`-less constructors, generics, `async`/`await`, cascades. A Lua user does not recognise it. But the sharper point for ludo is **concept count**, because survey 02 indicted C3 for having *"five-plus operators and forms for a single concept"* (optionals/faults).

Dart's null handling, from the official operators page ([dart.dev/language/operators](https://dart.dev/language/operators)) plus the null-safety guide, is:

- `?` — nullable type suffix
- `?.` — *"Like `.`, but the leftmost operand can be null"*
- `?..` — *"a null-shorting cascade… guarantees that none of the cascade operations are attempted on that null object"*
- `??` — *"If expr1 is non-null, returns its value; otherwise, evaluates and returns the value of expr2"*
- `??=` — *"Assign value to b if b is null; otherwise, b stays the same"*
- `!` — *"Casts an expression to its underlying non-nullable type, throwing a runtime exception if the cast fails"*
- `?[]` — conditional subscript access
- `...?` — null-shorting spread
- `late` — *"enforce this variable's constraints at runtime instead of at compile time"* ([understanding null safety](https://dart.dev/null-safety/understanding-null-safety))
- `required` — named-parameter modifier

**That is ten distinct syntactic forms for the single concept "this might be absent."** Dart is a worse offender than C3 on exactly the metric survey 02 used to condemn C3. It reached that state not through bad taste but through retrofitting: each form exists to make some previously-legal untyped pattern expressible after the fact. This is the strongest single argument in this document for ludo deciding the fate of `nil` **before** there is a corpus, not after (issue [#9](https://github.com/adamico/ludo/issues/9)).

Dart 3 added a second concept cluster on top: five class modifiers (`base`, `interface`, `final`, `sealed`, plus `mixin`) with legal combinations ([dart.dev/language/class-modifiers](https://dart.dev/language/class-modifiers)), records, and patterns. The language grew, not shrank, in exactly the period it also became sound.

### Sound null safety — the design, and what it actually cost

The definition is unusually precise, and worth quoting because most languages that claim null safety cannot say this ([understanding null safety](https://dart.dev/null-safety/understanding-null-safety)):

> "Soundness in the context of static checking means different things to different people. For us, in the context of null safety, that means that if an expression has a static type that does not permit null, then no possible execution of that expression can ever evaluate to null."

The payoff is stated in performance terms, not safety terms:

> "When the language makes hard guarantees about semantic properties of a program, it means that the compiler can perform optimizations that assume those properties are true. When it comes to null, it means we can generate smaller code that eliminates unneeded null checks, and faster code that doesn't need to verify a receiver is non-null before calling methods on it."

**That is the argument ludo should reuse.** Non-nullability paid for by codegen quality is a much stronger case than non-nullability paid for by developer virtue — and it is the case Nim's `not nil` cannot make, because being control-flow-insensitive and exception-blind ([manual_experimental](https://nim-lang.org/docs/manual_experimental.html), per survey 02), it cannot be relied on for optimisation either.

The mechanism: all types non-nullable by default (*"We've made all types non-nullable by default. If you have a variable of type String, it will always contain a string"*), plus flow analysis extended from Dart's existing type promotion, plus definite-assignment analysis (*"a local variable must be definitely assigned before it is used"*).

**The migration is the part ludo should study hardest, because it is the fully-documented cost of retrofitting.** The timeline, from primary sources:

- **2021-03-03** — Dart 2.12 ships sound null safety, explicitly opt-in ([announcing Dart 2.12](https://dart.dev/blog/announcing-dart-2-12)): *"Because null safety is such a fundamental change to our typing system, it would be extremely disruptive if we insisted on forced adoption. So that you decide when the time is right, null safety is an opt-in feature."* At launch: *"98% of the top-100 most popular packages, 78% of the top-250, and 57% of the top-500 already support null safety."* Note what that means — nearly half the top 500 did not.
- The SDK's own migration-tracking page recorded, mid-migration, **8,208 packages with null safety versus 13,066 without**, out of **21,274 packages in total** with Dart 2.12 support, with a "blocking score" ranking of which unmigrated packages were holding up the most transitive dependents ([dart-lang/sdk docs/Null-safety-migration-status.md](https://github.com/dart-lang/sdk/blob/main/docs/Null-safety-migration-status.md)). Google built a **dependency-graph triage system for a type-system change**. That is the true cost signature.
- **2023-05** — Dart 3 removes the opt-out ([dart-3-migration](https://dart.dev/resources/dart-3-migration)): *"In Dart 3, null safety is built in; you cannot turn it off."* Pre-2.12 language-version overrides now hard-error, and `pub get` fails resolution: *"Because pkg1 doesn't support null safety, version solving failed."*
- **2025-08** — Dart **3.9**, four and a half years after 2.12, finally *"assumes null safety when computing type promotion, reachability, and definite assignment"* ([announcing Dart 3.9](https://dart.dev/blog/announcing-dart-3-9)), with the caveat that opting in *"may produce additional dead code warnings."*

**Read that last one carefully.** The analyser could not fully exploit soundness for four and a half years after soundness shipped, because mixed-mode code was still in the graph. Retrofitting null safety onto a live ecosystem cost Google two years of dual-mode language semantics, a migration tool, a blocking-score triage dashboard, an ecosystem-wide flag day at the major version, and another two years before the optimiser could believe its own type system. **ludo gets all of this for free by deciding at design time.** That is the answer to issue #9's framing: the question is not "what should `nil` be", it is "you have exactly one chance to answer this cheaply."

### Type system depth and inference (issue [#11](https://github.com/adamico/ludo/issues/11))

Dart is sound but not *statically* sound — and the docs say so directly ([dart.dev/language/type-system](https://dart.dev/language/type-system)):

> "A sound type system means you can never get into a state where an expression evaluates to a value that doesn't match the expression's static type."

> "Dart's type system, like the type systems in Java and C#, is sound. It enforces that soundness using a combination of static checking (compile-time errors) and **runtime checks**."

**Soundness is bought partly at runtime.** For a language that wants to compile to native code with no runtime tax, this is a design fork ludo has to take deliberately: Dart chose covariant generics (`List<Cat>` is a subtype of `List<Animal>`) for usability and pays for it with checked stores. A language with invariant or declaration-site-variance generics pays nothing at runtime and annoys the user instead.

Inference has a documented floor, also admitted: *"The analyzer can infer types for fields, methods, local variables, and most generic type arguments. When the analyzer doesn't have enough information to infer a specific type, it uses the `dynamic` type."* **`dynamic` is still the escape hatch** — the same hole survey 02 documented in Luau (error-suppressing `any`), Teal (`any` + `as`), and TypeScript. Dart narrowed it far more than those, but did not close it.

**What Dart 3 got right, and ludo should copy wholesale**: sealed types plus exhaustiveness. From [dart.dev/language/class-modifiers](https://dart.dev/language/class-modifiers):

> "To create a known, enumerable set of subtypes, use the `sealed` modifier. This allows you to create a switch over those subtypes that is statically ensured to be exhaustive."

And from [dart.dev/language/branches](https://dart.dev/language/branches), exhaustiveness checking is *"a feature that reports a compile-time error if it's possible for a value to enter a switch but not match any of the cases"*, with the docs explicitly endorsing the functional framing — *"use Dart in a somewhat functional algebraic datatype style"* — and naming the refactoring payoff: adding a subtype later makes every non-exhaustive switch a compile error.

This is precisely the "cheap robustness" bucket survey 02 identified from the Rust split (sum types + exhaustive match survived LogLog Games' departure; the borrow checker did not). Dart got there in 2023 by adding it to a fifteen-year-old OO language. ludo can have it on day one at lower syntactic cost.

**One negative datapoint on type-system ambition, and it is important for issue #17.** Dart cancelled macros — its flagship static-metaprogramming feature — on 2025-01-29, and the stated reason was **hot reload** ([an update on Dart macros & data serialization](https://dart.dev/blog/an-update-on-dart-macros-data-serialization)):

> "Our current implementation regresses both editing (e.g., static analysis and code completion) and incremental compilation (the first step of a hot reload)."

> "Semantic introspection, unfortunately, turned out to introduce large compile-time costs which made it difficult to keep stateful hot reload *hot*."

> "We've concluded we're simply too far away from shipping macros with the developer-time performance we require."

**Google killed a major language feature because it would have made hot reload slower.** If ludo commits to state-preserving reload, that commitment is a permanent constraint on the compile-time metaprogramming layer — the exact layer Jai and Zig build their identity on. This is a hard trade, it is documented by a first-party source with the resources to have solved it if it were solvable, and it should be stated in issue #17 as a constraint rather than discovered later.

### The memory model — GC, and no way around it (issue [#8](https://github.com/adamico/ludo/issues/8))

The Dart VM's GC is documented in the SDK at [runtime/docs/gc.md](https://github.com/dart-lang/sdk/blob/main/runtime/docs/gc.md), and it is a serious piece of engineering, not a naive collector:

> "The Dart VM has a generational garbage collector with two generations. The new generation is collected by a parallel, stop-the-world semispace scavenger. The old generation is collected by concurrent-mark-concurrent-sweep or by concurrent-mark-parallel-compact."

> "The Dart VM's GC is precise and moving."

Concurrent marking exists for exactly the reason a game developer would want it: *"To reduce the time the mutator is paused for old-space GCs, we allow the mutator to continue running during most of the marking work."* There are Smi (tagged small-integer) immediates, write-barrier elimination when the compiler can prove a store is safe, and parallel scavenging with work-stealing.

**But the young generation is still stop-the-world**, and a compacting old-generation collector still moves objects. The honest reading matches survey 02's V8 finding: this is a GC that has been optimised hard against frame budgets and is still a GC. The confirmation comes from Flame's own performance guidance ([Flame performance docs](https://docs.flame-engine.org/latest/flame/other/performance.html)), which reaches for the same mitigation JavaScript game developers reach for:

> "avoid creating unnecessary objects in every frame. Even a seemingly small object can affect the performance if spawned in high volume"

with `ComponentPool` recommended for bullets, particles and enemies. **Object pooling — manually defeating the GC — is the documented idiom in the fastest-moving Dart game engine.** That is the same standing mitigation survey 02 recorded for JavaScript. It is strong evidence that a GC of *any* quality does not remove the allocation-discipline burden from a game programmer; it just moves it from the type system into folklore.

Concurrency: isolates, *"independent workers that are similar to threads but don't share memory, communicating only through messages"* ([dart.dev/overview](https://dart.dev/overview)). No data races by construction; also no shared mutable world state, which is a serious constraint for a game with a large hot entity array. ludo should note that Dart's answer to threading is to forbid the thing games most want.

### Error model — exceptions, unchecked, all the way down (issue [#10](https://github.com/adamico/ludo/issues/10))

From [dart.dev/language/error-handling](https://dart.dev/language/error-handling):

> "In contrast to Java, all of Dart's exceptions are unchecked exceptions. Methods don't declare which exceptions they might throw, and you aren't required to catch any exceptions."

> "Dart programs can throw any non-null object—not just Exception and Error objects—as an exception."

There is no `Result` type in the core library and no error-union machinery. Dart went the full opposite direction from Rust/Zig/Odin/C3: **maximum ergonomics, zero static accounting.** A function's signature tells you nothing about its failure modes, which is a direct hit on the agent-friendliness lens — a model writing Dart cannot know from a signature whether to wrap a call.

Notably, Dart is sound about *null* and unsound about *failure*. It fixed the cheap bug class and left the expensive one entirely to convention. If ludo wants both, it is choosing something neither Dart nor Lua has, and should expect the Zig/C3 concept-count bill.

### AOT vs JIT — what "compiled" means here

From [dart.dev/overview](https://dart.dev/overview):

> "During development, a fast developer cycle is critical for iteration. The Dart VM offers a just-in-time compiler (JIT) with incremental recompilation (enabling hot reload)"

> "When apps are ready to be deployed to production… the Dart ahead-of-time (AOT) compiler can compile to native ARM or x64 machine code."

`dart compile exe` produces *"a standalone, architecture-specific executable file containing the source code compiled to machine code and a small Dart runtime"*; `aot-snapshot` produces the same *"but no Dart runtime"*, requiring `dartaotruntime`. Cross-compilation to Linux ARM/ARM64/RISCV64/x64 landed in Dart 3.8–3.9. Documented limitation: *"No support for `dart:mirrors` and `dart:developer`"* ([dart.dev/tools/dart-compile](https://dart.dev/tools/dart-compile)).

**So Dart is genuinely a compiled language that ships a single binary — and it runs a fundamentally different compiler in development than in production.** That two-compiler split is the price of admission for hot reload, and it is the central fact of the next section.

---

## The iteration story: hot reload, and what it costs the language

This is the section that matters most for issues [#17](https://github.com/adamico/ludo/issues/17) and [#18](https://github.com/adamico/ludo/issues/18), because the Dart VM ships the best-documented state-preserving reload in any production language, and the SDK's own design doc is unusually candid.

### The mechanism, from the source that owns it

[dart-lang/sdk runtime/docs/hot-reload.md](https://github.com/dart-lang/sdk/blob/main/runtime/docs/hot-reload.md) — *"Live program changes in the Dart VM"* — states the model in three principles:

> "The semantics are very close to those of Smalltalk, which doesn't have a name for this feature since in most Smalltalk implementations program changes can only be made in a live programming environment."

> "* The program behaves as if method lookup happens at every call.
>  * The 'atoms' of reload are methods. Methods are never changed, but method dictionaries are updated with new methods.
>  * Fields retain their values."

And the caveat that no marketing page states:

> "It's also important to note that hot reloading only changes the behavior of the program going forward. It does not change the program's state to reflect what would have happened if the new program had been running from the beginning. **The behavior of a hot reloaded program can differ from the behavior of both the old and the new program running without a hot reload.**"

The implementation details are exact and each one is a design constraint:

- **Pervasive late binding.** *"Any call that runs after a reload, even those call sites that have previously run or are pending on the stack, should reflect the result of a lookup in the new program rather than the old program. (In the implementation, the VM aggressively attempts to avoid method lookups by using caches and inlining. To preserve the semantics of hot reload, **inline caches are cleared and inlining is unfolded** at the time of a reload.)"* — reload is purchased with deoptimisation.
- **Immutable methods.** *"Changes to a method declaration create a new method, mutating the class or library's method dictionary. The old method may still exist if has been captured by a closure or stack frame."* Closures and stack frames keep the *old* code. The doc's worked example shows a closure created before reload printing `"before"` twice.
- **State is retained.** *"Hot reload does not reset fields, neither the fields of instances nor those of classes or libraries. Resetting all fields would make a hot reload equivalent to a restart."* A changed top-level initializer does **not** take effect — unless the static was never touched, because *"statics fields in Dart are lazily initialized"*, in which case it does. **The same edit produces different results depending on whether the variable was read yet.** That is the sharpest usability wart in the whole design and it is inherent to lazy statics, not incidental.
- **Instance layout migration is done with `become`.** From [runtime/docs/gc.md](https://github.com/dart-lang/sdk/blob/main/runtime/docs/gc.md): *"Become is an operation that atomically forwards the identity of a set of objects. A heap walk is performed where every pointer to a before object is replaced with a pointer to an after object… In the Dart VM, it is only used during reload to map the old program and instances with the old sizes onto the new program and instances with the new sizes."*

**That last one is the load-bearing requirement and ludo must confront it.** Reshaping live instances when you add a field requires walking the whole heap and rewriting every pointer. That requires a *precise, moving* collector with an object header, handles for foreign references, and safepoints. **A compiled language with raw pointers, arenas, and no object headers cannot do `become`.** Dart's stateful hot reload is not a compiler feature bolted onto a runtime; it is a property of a managed heap. This is the single most important mechanical finding in this document for issue #17: *state-preserving reload of arbitrary object graphs and manual memory management with unmanaged pointers are in direct tension*, and Dart resolves it by having the managed heap.

### The limitations, split into inherent and incidental

The VM doc lists four rejection cases (*"Changes are atomic: they are never partially applied. If a change is rejected, the program continues as if no change had been attempted"*):

| Rejected change | VM's stated reason | Inherent or incidental? |
|---|---|---|
| `class C {}` ⇄ `enum C {}` ⇄ `typedef void C()` | (none given) | **Inherent** — different runtime representations, no migration path |
| Changing arity of class type parameters (`C<T>` ⇄ `C<T,S>`) | *"Schema changes to type arguments are not safe because of the type argument prefix optimization"* | **Incidental** — an optimisation choice, not a semantic one |
| Changing a library with deferred imports | *"This simply hasn't been implemented."* | **Incidental**, self-admitted |
| Changing the number of native fields | *"Changing the shape of native wrappers generally requires corresponding changes to the native code"* | **Inherent** at the FFI boundary |

Plus a fifth, which is not a rejection but a hole: *"The Dart VM also lacks a way to migrate field values during a change. In particular, **there is no way to communicate the intention of a renamed field** to carry the value over from the old name to the new name. The Dart VM only sees addition and removal of unrelated fields."* **Renaming a field silently drops its value.** If ludo wants better, it needs a rename-aware diff between compilations — which is a real, specifiable, unoccupied piece of design work, and probably the most concrete differentiator available on issue #17.

And a "Defects" section listing three cases where the VM *accepts* a change and produces wrong results — all involving frames on the stack at reload time (an unqualified call changing between instance and static; a `super` call whose target moved; a static call gaining or losing a target). The doc's own mitigation is instructive:

> "Note all of these cases involve code pending on the call stack. **In Flutter, reload usually happens at the top of the message loop where there is an empty call stack**, so one is quite unlikely to encounter these cases in Flutter."

**Reload is only sound at a quiescent point.** Flutter gets away with it because a UI framework has a natural one. A game has one too — the top of the frame — which is good news for ludo, and it is exactly the point DragonRuby's `hotload.rb` also chose (survey 02).

### What Flutter layers on top

[docs.flutter.dev/tools/hot-reload](https://docs.flutter.dev/tools/hot-reload) describes the pipeline:

> "When hot reload is invoked, the host machine looks at the edited code since the last compilation. The following libraries are recompiled: Any libraries with changed code; The application's main library; The libraries from the main library leading to affected libraries. The source code from those libraries is compiled into kernel files and sent to the mobile device's Dart VM. The Dart VM re-loads all libraries from the new kernel file. So far no code is re-executed. The hot reload mechanism then causes the Flutter framework to trigger a rebuild/re-layout/repaint of all existing widgets and render objects."

Note the division of labour: **the VM does state-preserving code replacement, and the framework does the re-execution.** The VM alone would change nothing visible — Dart's own doc says "so far no code is re-executed". Flutter supplies the "run the world again from a known root" half. **A ludo runtime needs both halves, and the second half is a framework/engine responsibility, not a language one.** That is a direct input to issue #12: reload is not purely a language feature.

The documented user-facing limitations are the leaks of the mechanism, and they are what a user actually hits:

- *"Only Flutter apps in debug mode can be hot reloaded or hot restarted."*
- *"Global variables and static fields are treated as state, and are therefore not reinitialized during hot reload."*
- Changes to `main()` or `initState()` are invisible — *"Hot reload only re-executes code downstream of the root widget's `build()` method"*, requiring hot restart.
- *"Previous state is combined with new code"*: *"If code changes affect state or dependencies, the data may be inconsistent with what would occur from a fresh start, resulting in different behavior."*
- Enum ⇄ class and generic-arity changes fail (the VM restrictions surfacing).
- Native code changes need a full restart.
- Compilation errors reject the reload with a message.
- One long-lived framework-specific bug: *"Hot reload won't apply changes made to a `builder` of a `CupertinoTabView`"* ([flutter#43574](https://github.com/flutter/flutter/issues/43574)).

And one that matters more for games than for apps: **hot reload does not reach spawned isolates.** [dart-lang/sdk#44640](https://github.com/dart-lang/sdk/issues/44640), "VM service does not hot reload non-main Isolates", opened 2021-01-12, reports `reloadSources` working for the main isolate and not for spawned ones. If a game puts simulation or asset loading on a worker isolate, that code is outside the reload loop. *(The issue is now closed; I could not establish whether it was fixed or closed as stale — see the evidence section.)*

### Errors as a pause — Flutter's answer, which is not a pause (issue #18)

Survey 02 identified DragonRuby's *"errors pause the game; they do not kill it"* as the mechanical basis of its felt joy. Flutter's answer to the same problem is different and arguably better for the specific case of a rendering tree, from [docs.flutter.dev/testing/errors](https://docs.flutter.dev/testing/errors):

> "The Flutter framework catches errors that occur during callbacks triggered by the framework itself, including errors encountered during the build, layout, and paint phases."

> "All errors caught by Flutter are routed to the `FlutterError.onError` handler."

> "When an error occurs during the build phase, the `ErrorWidget.builder` callback is invoked to build the widget that is used instead of the one that failed. By default, in debug mode this shows an error message in red, and in release mode this shows a gray background."

**The failure mode is substitution, not termination and not suspension.** The failing subtree is replaced by an error node; the rest of the app keeps rendering and keeps running; the developer fixes the code; hot reload rebuilds; the error node disappears. The loop closes without the program ever stopping.

That is a genuinely different point in the design space from DragonRuby's pause-plus-console, and it has one property DragonRuby's does not: **the error is spatially localised.** You see *which* part of the scene is broken, in place, while everything around it still works. For a game, "the enemy that threw is drawn as a red box and everything else keeps playing" is arguably a better debugging surface than "the whole game froze."

It also has an obvious failure: it only works because Flutter's unit of work is a pure, re-runnable, side-effect-free `build()` over a tree. A gameplay `update()` that has already mutated the world before throwing cannot be re-run. **Issue #18 should treat "error-as-pause" and "error-as-substitution" as two options with different prerequisites**, and note that Flutter's requires a re-entrant, idempotent per-frame unit — the same architectural property that makes immediate-mode rendering work.

### Under AOT: none of this exists

Release builds are AOT-compiled, with no VM service, no kernel service, no incremental compiler, and therefore no reload, no `become`, no live introspection, and no `ErrorWidget` message (release shows *"a gray background"*). The precise wording is *"Only Flutter apps in debug mode can be hot reloaded or hot restarted."*

**This is the honest shape of the trade and ludo should adopt it consciously rather than pretend otherwise.** Dart does not deliver "a compiled language with hot reload." It delivers *two* execution modes — a JIT-and-managed-heap development mode where reload works, and an AOT production mode where it does not — and accepts that the program you ship is not, mechanically, the program you debugged. Survey 02's issue #17 phrasing ("how a compiled language delivers state-preserving reload") has, on this evidence, exactly one known answer at production scale: **it doesn't; it delivers a second runtime that does.**

---

## Flutter and Flame: does the language need an engine? (issue [#12](https://github.com/adamico/ludo/issues/12))

### The three-layer arrangement, and who owns each layer

| Layer | Owner | Evidence (measured 2026-07-25) |
|---|---|---|
| Dart, the language + VM + AOT compiler | Google | [dart-lang/sdk](https://github.com/dart-lang/sdk) — 11,227 stars, 8,419 open issues |
| Flutter, the UI framework + renderer | Google | [flutter/flutter](https://github.com/flutter/flutter) — 177,923 stars, 12,987 open issues, 114,643 issues filed all-time |
| Flame, the game engine | **Not Google** | [flame-engine/flame](https://github.com/flame-engine/flame) — 10,695 stars, MIT, 88 open issues, 9 open PRs |

**Flame is emphatically not first-party.** [flame-engine.org](https://flame-engine.org/) says it plainly: *"Flame was created and is maintained by independent members of the Flutter community."* The maintaining organisation, Blue Fire, describes itself as a *"Team working on open source packages and plugins for Flutter, including Flame, Audioplayers, Photo View, and more"* and links a **Patreon** as its funding channel ([github.com/bluefireteam](https://github.com/bluefireteam)).

Google's relationship is endorsement plus one-off collaboration, not ownership:

- The **Casual Games Toolkit** ([docs.flutter.dev/resources/games-toolkit](https://docs.flutter.dev/resources/games-toolkit), page last updated 2026-07-21) ships three templates, one of which — the endless-runner — is *"created in partnership with the open source game engine, Flame."* The toolkit's own framing is revealing: Flutter is presented as well-suited to *turn-based* games (board, card, puzzle), and for *real-time* games with *"advanced features like collision detection, camera views, game loops"* it points you at *"an open source game engine like the Flame game engine."* **Google's own documentation says: for real games, use the third-party thing.**
- **I/O Pinball** (Google I/O 2022) was built on Flame and `forge2d` by Very Good Ventures in partnership with Google ([blog.flutter.dev](https://blog.flutter.dev/i-o-pinball-powered-by-flutter-and-firebase-d22423f3f5d)). The repo has since been moved to `flutter-team-archive/pinball` and is **archived** (2,313 stars, verified archived 2026-07-25). A showcase, not a commitment.

**The lesson for issue #12 is not "you need an engine."** It is: *a language plus a general-purpose rendering framework gets a community engine for free, and the language owner never has to build one.* Flame exists because Flutter already solved windowing, input, texture upload, compositing, cross-platform packaging, and a 60/120 Hz render loop. Flame's job was reduced to a game loop, a component tree, sprites, collision and camera. That is a much smaller project than an engine, and it is why a Patreon-funded team of three could ship it.

The corollary is uncomfortable: **that free ride is available only if the language ships a general-purpose renderer.** ludo shipping a language alone gets no Flame. ludo shipping a language plus raylib bindings is Odin's position, which survey 02 already called redundant.

### What Flame gets and what it has to build

Gets from Flutter, for nothing: the render surface, the frame ticker, input events, widget embedding (`GameWidget` can sit anywhere in a Flutter tree), asset bundling, and the entire cross-platform packaging story. Builds itself: the game loop, the Flame Component System, sprites and spritesheets, animations, collision detection, camera, and `ComponentPool`. Delegates: audio to `flame_audio`, physics to `forge2d` (a Dart port of Box2D), and multiplayer entirely — the docs recommend Nakama, Firebase or Supabase and state that netcode is *"outside of the scope of the Flame Engine ecosystem"* ([docs.flame-engine.org](https://docs.flame-engine.org/latest/)).

### Realistic state of Flame

**Maturity signals are genuinely good, and better than most of survey 02's cohort.** Measured 2026-07-25:

- v1.38.0 published 2026-07-19; 147 versions on pub.dev; last push to `main` the same day this was written.
- 226 commits in the trailing 52 weeks. Only 88 open issues and 9 open PRs against 10,695 stars — an unusually clean tracker.
- **No bus-factor cliff**: top contributors are `spydon` (1,053), `luanpotter` (781), `erickzanardo` (764), `st-pasha` (247). Compare survey 02's Beef (`bfiete` 5,066 vs next 149) and C3 (`lerno` 3,779 vs next 94). Flame has three genuine co-maintainers.
- Post-1.0 and versioned properly, unlike Odin, Zig, Jai, C3 and Bevy.

**Platform and console story is the hard limit.** [docs.flame-engine.org/latest/flame/platforms.html](https://docs.flame-engine.org/latest/flame/platforms.html): *"At the moment, Flame supports web, mobile (Android and iOS) and desktop (Windows, macOS and Linux)"* and *"Flame keeps its support on the stable channel. The dev, beta and master channels should work, but we don't support them."* Web carries documented holes — `Flame.device.setOrientation` and `fullScreen` *"can be called, but nothing will happen"*, `flame_audio` pre-caching *"doesn't work"* on web. **No consoles, at all**, because Flutter has no console backends. For a game language whose ambition includes shipped commercial titles, that ceiling is the same one Godot spent years and third-party porting houses climbing.

**Performance: 2D only, GC-aware, and unbenchmarked in public.** Flame is a 2D engine. Flutter's rendering runtime is Impeller, built specifically to eliminate shader-compilation jank — *"Impeller precompiles a smaller, simpler set of shaders at engine-build time so they don't compile at runtime"* — default and sole backend on iOS, default on Android API 29+ with an OpenGL fallback, still Skia on web, experimental on macOS ([docs.flutter.dev/perf/impeller](https://docs.flutter.dev/perf/impeller)). 3D exists only via **Flutter GPU**, which is *"in early preview"* and *"does not guarantee API stability"* ([flutter/engine docs/impeller/Flutter-GPU.md](https://github.com/flutter/engine/blob/main/docs/impeller/Flutter-GPU.md)), plus the community `flutter_scene` package.

I found **no published benchmark comparing Flame to Unity or Godot.** The closest is [filiph/game_benchmarks](https://github.com/filiph/game_benchmarks) — by a member of the Flutter games community — which sets out to compare "Flutter vanilla, Flutter Flame, Unity, Godot" on startup time, max entities before framerate degrades, CPU and memory. It has **13 stars, no results in the README, and no push since 2024-09-26.** The comparison was attempted and abandoned. *Flag this: Flame's performance relative to mainstream engines is unmeasured in public.*

**Shipped games: real, but the evidence is vendor-supplied.** [flame-engine.org](https://flame-engine.org/) claims *"Millions of people have played countless Flame games"* and *"thousands of developers use Flame to ship games every day"* — **marketing claims, uncited**. It names Idle Horizons: Dawn of Heroes, Gunslinger, The Darkblade, Gigabull, Tomb Toad, WalkScape and Watchsteroids. I did not independently verify sales, players or platform availability for any of them; WalkScape's own [press kit](https://walkscape.app/presskit) describes a walking fitness RPG in closed beta, and I could not find a first-party statement confirming its engine. **This is still a better shipping record than Zig, Beef, C3, Nim or Jai from survey 02 — none of which could produce a single named commercial title — but it is worse than Odin's, and it is not corroborated by anyone but the vendor.**

---

## Agent-friendliness

### Corpus: the strongest in this research programme so far

Measured 2026-07-25: **2,036,174 public GitHub repositories classified as Dart**, and **74,124 packages on pub.dev**. Against survey 02's table, Dart sits between Rust (1.27M) and TypeScript (17.2M) — three orders of magnitude above Odin, four above Beef and C3, five above Teal.

This is the datapoint that makes Dart uncomfortable for ludo. Survey 02's central agent-friendliness argument was that a new language starts at zero corpus and PyLang shows a 30-point deficit that fine-tuning does not close ([arXiv:2605.15607](https://arxiv.org/html/2605.15607v1)). **Dart already has the corpus.** It is not hypothetical; it is 1.6× Rust's.

### Measured LLM performance: exists, but thin and I could not reconcile it

Dart is one of the 40 languages in **McEval** ([arXiv:2406.07436](https://arxiv.org/abs/2406.07436)), the only multilingual code benchmark I found that covers it. MultiPL-E — the benchmark survey 02 leaned on for the JS/Rust comparison — does **not** include Dart, so the survey-02 pass@1 table simply cannot be extended to it.

**I could not extract a trustworthy Dart number from McEval and I am flagging it rather than reporting one.** Two separate extractions of the same paper's tables returned GPT-4o Dart code-generation pass@1 as **82.0%** and as **54.9%**, and the accompanying comparison row in the second extraction (Rust 83.0% > Python 76.0%) is implausible enough that I do not trust the table read at all. The mceval.github.io landing page does not carry the numbers, and the [MCEVAL/McEval](https://github.com/MCEVAL/McEval) repo has 48 stars. **Treat "how well do LLMs write Dart" as unmeasured for practical purposes** — which is, notably, the same verdict survey 02 reached for Odin, Nim, Beef, C3 and Teal despite Dart having orders of magnitude more corpus than all of them combined. Corpus size does not automatically produce published evaluation.

The one Dart/Flutter-specific study I found is *Fine-tuning Large Language Models for Turkish Flutter Code Generation*, reporting correct-solution rates rising from ~30–70% to 80–90% after fine-tuning small models on a Turkish Flutter Q&A set. **Not a primary-source-quality result for this purpose** — Turkish-language prompts, small models, similarity metrics (BLEU/ROUGE/CodeBLEU) rather than execution, and I read it only via search snippets. Discount heavily.

### Official LLM-facing documentation: verified, and it is more than a text file

Unlike DragonRuby (where survey 02 found `llms.txt` 404s), Flutter's exists and is real:

- **[docs.flutter.dev/llms.txt](https://docs.flutter.dev/llms.txt) exists**, opening *"# Flutter documentation"* followed by a curated, sectioned index of documentation URLs across 17 areas explicitly framed as a resource for large language models. Verified 2026-07-25.
- **[dart.dev/llms.txt](https://dart.dev/llms.txt) returns HTTP 404.** Flutter publishes one; Dart does not. Verified 2026-07-25.
- Pages are additionally served in markdown form (e.g. `docs.flutter.dev/ai/mcp-server/index.html.md`).

**The bigger artefact is the Dart and Flutter MCP server** ([docs.flutter.dev/ai/mcp-server](https://docs.flutter.dev/ai/mcp-server), source at [dart-lang/ai/pkgs/dart_mcp_server](https://github.com/dart-lang/ai/tree/main/pkgs/dart_mcp_server)), shipped with Dart 3.9 and self-described as **"experimental and likely to evolve quickly."** It exposes to an AI client: analysing and fixing errors, resolving symbols to elements *"to ensure their existence"* and fetching signatures, searching pub.dev, editing `pubspec.yaml`, running tests, formatting, and — the important one — introspecting and interacting with a running application. The docs state you can *"drive a running Flutter app from your AI assistant—take screenshots, tap buttons, enter text, scroll, and hot reload."*

**This is the closest thing that exists to survey 02's fourth leg — "a compiler designed as a machine-readable verifier for an agent loop" — and it is shipping, from Google, today.** It is also the direct answer to issue #18 read as an agent question: the pause/inspect/fix/resume loop that DragonRuby gives a human, Dart gives a model as a tool protocol. Symbol resolution *"to ensure their existence"* is precisely the mitigation for the hallucinated-API failure mode.

Note also what it implies about ludo's design: **that loop requires a live VM service.** Another entry on the "hot reload is a runtime property" ledger.

### Churn — Flutter is the best-behaved API surface in this research programme

Survey 02 established churn as poison, quantified by RustEvo²'s 23.6-point penalty for post-cutoff APIs ([arXiv:2503.16922](https://arxiv.org/abs/2503.16922)), and used Bevy's **165 and 117 breaking-change entries in single releases** as the negative benchmark.

Flutter's equivalent, counted from [docs.flutter.dev/release/breaking-changes](https://docs.flutter.dev/release/breaking-changes) on 2026-07-25: **roughly 150 entries total across 25 releases spanning Flutter 1.17 (2020) to 3.44 (2026)** — 9 in 3.44, 5 in 3.41, 6 in 3.38, 9 in 3.35, 10 in 3.32, 5 in 3.29, 7 in 3.27. **Call it six to ten per release, against Bevy's hundred-plus.** An order of magnitude better, over nine years, on a framework with a vastly larger surface.

The policy explains it ([docs.flutter.dev/release/compatibility-policy](https://docs.flutter.dev/release/compatibility-policy)). Flutter defines breakage empirically rather than by declaration:

> "a change that caused one or more of these submitted tests to require changes"

> "Our commitment is that we won't make any changes that break these tests without working with the developers of those tests to (a) determine if the change is sufficiently valuable, and (b) provide fixes for the code so that the tests continue to pass."

**Users submit tests to a registry; the framework is contractually bound to those tests.** That is a genuinely clever mechanism and it is stealable by a much smaller project: instead of writing a stability promise in prose, accept executable stability promises from your users.

And the deprecation posture has *tightened* over time — in the direction agents need. The older public policy was a fixed lifetime (announced as one year or four stable releases, whichever is longer — *sourced via [blog.flutter.dev](https://blog.flutter.dev/deprecation-lifetime-in-flutter-e4d76ee738ad), which I could not fetch directly; treat the exact wording as second-hand*). The current first-party statements say removal has stopped being routine:

> "The Flutter team doesn't remove deprecated APIs on a scheduled basis." ([compatibility policy](https://docs.flutter.dev/release/compatibility-policy))

> "Removing deprecated APIs from the framework are not currently planned. In the past, deprecations were removed after a set amount of time. This is not currently in practice." ([flutter/flutter docs/contributing/Tree-hygiene.md](https://github.com/flutter/flutter/blob/master/docs/contributing/Tree-hygiene.md))

Deprecations must also carry a `flutter fix` migration where possible, and the annotation format encodes the version: `@Deprecated('[description of how to migrate] [brief motivation…] This feature was deprecated after v[beta version at time of deprecation].')`. **Machine-readable deprecation plus a mechanical fixer plus a policy of not removing things is close to the ideal agent-facing stability regime**, and it is the exact opposite of Zig's.

Dart the language is similarly quiet: Dart 3.11 (2026-02-11) states outright *"No new language updates in this release of Dart"* ([announcing Dart 3.11](https://dart.dev/blog/announcing-dart-3-11)).

The counterweight, in fairness: Flutter has **114,643 issues filed all-time and 12,987 open**, `flutter/engine` was archived on 2025-02-25 in a monorepo consolidation, and repos have moved to a `flutter-team-archive` org. Stability of the public API is not the same as stability of the project's shape.

---

## What to steal / what to avoid

### Steal

1. **The soundness argument framed as codegen, not virtue.** *"we can generate smaller code that eliminates unneeded null checks, and faster code that doesn't need to verify a receiver is non-null"*. Non-nullability that pays for itself in the backend is defensible to a performance-minded audience in a way that non-nullability-as-discipline is not.
2. **Sealed types plus compile-time exhaustiveness, with the refactoring argument attached.** Dart's docs sell it correctly: add a subtype, every incomplete switch becomes an error. This is the survivor from survey 02's Rust split, and Dart shows it works in a non-Rust-shaped language.
3. **The test-registry compatibility policy.** Users submit tests; the project is bound to them. It converts "we promise not to break you" from prose into CI. A one-person language can run this from day one, and it is the cheapest available answer to survey 02's demand to "freeze the surface early".
4. **`@Deprecated` with an encoded version plus an automated fixer (`flutter fix`), and a policy of not removing.** Machine-readable, machine-migratable, non-destructive. Directly counters the RustEvo² churn penalty.
5. **Error-as-substitution as a *second* option alongside error-as-pause** (issue #18). Replacing the failing subtree while everything around it keeps running is spatially localised debugging that a global pause cannot give you. Prerequisite: a re-entrant, idempotent per-frame unit.
6. **Reload at a quiescent point, stated as a rule.** The VM doc's own defects all involve frames on the stack; Flutter dodges them by reloading at the top of the message loop. A game's frame boundary is the same affordance. **Specify it; don't discover it.**
7. **Rename-aware field migration** — the thing Dart explicitly does *not* have (*"there is no way to communicate the intention of a renamed field"*). A small, concrete, genuinely unoccupied piece of design work sitting inside issue #17.
8. **An MCP server as a first-class language artefact**, including symbol resolution *"to ensure their existence"*, error analysis, and driving/reloading a live program. Google shipped this in 2025 and it is the working prototype of survey 02's fourth leg.
9. **Publishing an `llms.txt`** — Flutter has one, Dart doesn't, and it costs nothing.

### Avoid

1. **Ten syntactic forms for absence.** `?`, `?.`, `?..`, `??`, `??=`, `!`, `?[]`, `...?`, `late`, `required`. Dart is a worse offender than C3 on the metric survey 02 used to condemn C3. Every one of them exists because null was retrofitted.
2. **Retrofitting anything into the type system after a corpus exists.** The full bill: opt-in flag, two years of dual-mode semantics, a migration tool, a dependency-blocking-score dashboard tracking 21,274 packages, a major-version flag day, and *four and a half more years* before the analyser could assume its own guarantee. Decide `nil` now (#9).
3. **Soundness paid for with runtime checks.** Dart admits *"a combination of static checking (compile-time errors) and runtime checks"*. Covariant generics are a usability choice with a per-store cost. A compiled language aiming at frame budgets should take the annoying-but-free option.
4. **`dynamic` as the inference fallback.** *"When the analyzer doesn't have enough information to infer a specific type, it uses the `dynamic` type"* — the same hole as Luau's `any` and TypeScript's, just smaller. Inference failure should be an error, not a silent widening.
5. **Unchecked exceptions with nothing in the signature.** *"Methods don't declare which exceptions they might throw, and you aren't required to catch any exceptions."* Fine for apps, actively hostile to an agent trying to write correct code from a signature alone (#10).
6. **Assuming a GC removes allocation discipline.** Dart's collector is genuinely sophisticated — precise, moving, generational, concurrent-marking, with write-barrier elimination — and Flame still tells you to pool your bullets and hoist your `Vector2`s out of the frame. The mitigation is identical to JavaScript's. If ludo takes a GC (#8), it should expect to ship pooling in the standard library and admit why.
7. **Believing hot reload is free of language-design consequences.** Google cancelled macros because *"Semantic introspection… made it difficult to keep stateful hot reload hot."* Committing to reload forecloses parts of the compile-time metaprogramming design space permanently.
8. **Assuming an engine will appear.** Flame exists because Flutter had already solved rendering, windowing, input and packaging. Without a general-purpose renderer in the box, there is no Flame-shaped project waiting to happen.

### Where Dart occupies ground ludo wants — stated plainly

Dart is, on this evidence, the closest existing thing to *"approachable language + batteries-included cross-platform runtime + state-preserving hot reload."* It is statically typed and soundly null-safe. It has sum types with exhaustiveness. It compiles to a standalone native binary and cross-compiles. It has a documented, principled, Smalltalk-derived stateful reload. It has an error model that keeps the program alive. It has an official agent tool protocol that can drive and reload a live process. It has 2.04M repositories, 74k packages, a nine-year framework with ~6–10 breaking changes per release and a policy against removing deprecations, and Google's payroll behind all of it.

**What that implies, said without softening:**

- **"Approachable + hot reload + cross-platform + backed by a large company" is taken.** ludo cannot win on that combination. If the pitch collapses to that sentence, the correct response is "use Flutter."
- **ludo's remaining differentiation is narrow and specific**: (a) Lua-grade *syntactic* simplicity, which Dart conclusively does not have — ten null forms and five class modifiers say so; (b) manual memory control and data-oriented layout, where Dart offers a managed heap and message-passing isolates and nothing else; (c) an error model with static accounting, which Dart declined entirely; (d) gamedev-native rather than UI-native semantics, where Google's own docs redirect real-time games to a Patreon-funded third party; and (e) consoles, which Flutter cannot reach at all.
- **The hot-reload leg is now contested, not empty.** Survey 02 said nobody in the compiled-static column had the DragonRuby loop. That was correct only if you insist on a single execution mode. Dart has the loop, in production, at Google scale — by having two runtimes. ludo's genuine open question for #17 is therefore sharper than it was: *can a single natively-compiled runtime do this, or must ludo also ship a development VM?* Dart's evidence, plus the `become`/precise-moving-GC dependency, says the honest answer is probably **no, and yes** — and the spec should say so rather than promise otherwise.
- **The one place ludo can beat Dart on Dart's own turf is field-rename migration and reload-at-frame-boundary as specified semantics**, both of which Dart leaves as documented gaps.
- **And the corpus argument cuts against ludo harder here than anywhere in survey 02.** Against Odin or Zig, ludo's zero corpus is a small relative disadvantage. Against Dart it is a two-million-repo gap, with a shipping MCP server and an `llms.txt` on the other side. Whatever agent-friendliness ludo claims has to be *mechanical* — structured diagnostics, a frozen surface, an idiom corpus shipped as a language artefact — because it will never be *statistical*.

---

## Evidence quality — what is weak

- **McEval Dart numbers are unusable as extracted.** Two reads of the same paper's tables gave GPT-4o Dart code-generation pass@1 as 82.0% and 54.9%, and a companion row (Rust 83.0% above Python 76.0%) is implausible enough to discredit the whole table read. The `mceval.github.io` landing page carries no numbers and the repo has 48 stars. **Treat LLM performance on Dart as unmeasured.** MultiPL-E does not cover Dart at all, so survey 02's pass@1 table cannot be extended.
- **The Turkish Flutter fine-tuning study** (30–70% → 80–90%) was read via search snippets only, uses similarity metrics rather than execution, and targets small models with Turkish prompts. Not load-bearing.
- **Flame's shipped-game record is entirely vendor-supplied.** *"Millions of people have played countless Flame games"* and *"thousands of developers use Flame to ship games every day"* are uncited marketing claims on flame-engine.org. I verified none of the seven named titles' engines, sales or player counts independently. WalkScape's press kit does not state its engine.
- **Flame's performance relative to Unity/Godot is unmeasured in public.** The only attempt I found, [filiph/game_benchmarks](https://github.com/filiph/game_benchmarks), has 13 stars, no results in its README, and no commits since 2024-09-26.
- **Medium-hosted Flutter blog posts are effectively unfetchable.** `blog.flutter.dev` 307-redirects into a `medium.com/m/global-identity-2` loop. This blocked verbatim quotation from *"Deprecation Lifetime in Flutter"* (the one-year / four-release policy) and *"Perspectives from early adopters of Flutter as a game development tool"* — the latter would have been the best available first-party source on what game developers actually complain about, and I could not read it. **The early-adopter complaint set is a real gap in this document.**
- **The one-year/four-release deprecation policy is second-hand.** It is asserted in search results attributed to a Flutter announcement; the current first-party pages ([compatibility policy](https://docs.flutter.dev/release/compatibility-policy), [Tree-hygiene.md](https://github.com/flutter/flutter/blob/master/docs/contributing/Tree-hygiene.md)) say the opposite — that removal is not scheduled and *"not currently in practice"*. Trust the first-party pages; treat the one-year figure as historical and unverified.
- **The Flutter breaking-change count (~150 across 25 releases) is a page-structure read**, not a hand audit. Per-release counts (9 in 3.44, 5 in 3.41, …) come from the same read. Order-of-magnitude reliable; do not quote as exact.
- **pub.dev's 74,124 figure** comes from the search-page header on 2026-07-25; the `api/package-names` endpoint errored and `sitemap.txt` returned only 39,002 package URLs (evidently truncated). The two disagree by roughly 2×; I report the header figure and flag the discrepancy.
- **GitHub repo counts** depend on Linguist classification, exactly as in survey 02. Dart's 2.04M is plausibly inflated by generated Flutter scaffolding, which is boilerplate-heavy — the effective *distinct-idiom* corpus is likely much smaller than the raw number suggests, though I have no way to quantify that.
- **`dart-lang/sdk` wiki pages have been folded into the repo and are stale-marked.** `docs/Null-safety-migration-status.md` carries a banner: *"This page was copied from https://github.com/dart-lang/sdk/wiki and needs review."* The 8,208 / 13,066 / 21,274 package figures are a snapshot from during the migration with no date on the page. Directionally sound, temporally unanchored.
- **`dart-lang/sdk#44640` (hot reload does not reach non-main isolates) is closed**, and I could not find a maintainer comment establishing whether it was fixed or closed as stale. The claim is reported as of the 2021 issue text and corroborated only by search summaries. **Not confirmed for 2026.**
- **Stack Overflow 2025 Flutter numbers could not be read.** Flutter appears only in the survey's web-framework write-ins at 0%, which is a survey-instrument artefact rather than a measurement. Only the Dart language figure (5.9%, 19th) is usable.
- **No first-party statement about Google's funding relationship to Flame was found** in either direction. "Flame is not first-party" rests on flame-engine.org's own *"maintained by independent members of the Flutter community"* and Blue Fire's Patreon, which is good evidence, but the absence of a Google statement is an absence, not a denial.
- **The Dart VM `hot-reload.md` and `gc.md` docs are undated** and were read from `main` on 2026-07-25. They are the authoritative first-party description of the mechanism, but I could not establish when they were last substantively revised, so "the VM currently rejects exactly these four changes" should be read as "the doc currently says so".
