# What actually makes Lua feel simple

**Research ticket:** [adamico/ludo#3](https://github.com/adamico/ludo/issues/3) · **Serves:** [#1](https://github.com/adamico/ludo/issues/1) · **Sibling:** [`02-language-survey.md`](./02-language-survey.md) · **Date of evidence:** 2026-07-24

The claim "Lua-simple" is only useful if it names mechanisms rather than a feeling. This document takes each candidate mechanism, measures it where measurement is possible, and asks two questions: is it **load-bearing** for the felt simplicity, and does it **survive contact with static typing**? The empirical instruments are Teal and Luau — two independent attempts to bolt static types onto Lua, one compiling to Lua and one embedded in a VM at Roblox scale. Where they both gave something up, that thing did not survive. Where they gave up different things, the difference tells you which constraint was doing the work.

Three findings frame everything below, and two of them are uncomfortable.

**First: Lua's authors do not claim their language is easy.** The closing paragraph of *A Look at the Design of Lua* (Ierusalimschy, de Figueiredo, Celes, CACM 61(11), 2018) is a warning, not a boast:

> "The lack of built-in complex constructions and minimalist standard libraries (for portability and small size) make Lua a language that is **not as good as other scripting languages for writing 'quick-and-dirty' programs**. Many programs in Lua need an initial phase for programmers to set up the language, as a minimal infrastructure for object-oriented programming. … **Lua's economy of concepts demands from programmers a deeper understanding of what they are doing**, as most constructions are explicit in the code. This explicitness also allows such deeper understanding. We trust this is a blessing, not a curse." ([cacm2018.pdf](https://www.lua.org/doc/cacm2018.pdf))

"Economy of concepts" is a property of the *language definition*, not of the *user's first hour*. These are separable, and the destination in #1 needs both. Anything in this document that claims a mechanism is load-bearing for felt simplicity is claiming it about the language definition unless it says otherwise.

**Second: the authors regret three of the eight candidate mechanisms.** In §7 "Retrospect" of *The Evolution of Lua* (HOPL III, 2007), they write that `nil`-as-false was probably a mistake, that having two false values was a mistake, and that string→number coercion "could have been omitted" ([hopl.pdf](https://www.lua.org/doc/hopl.pdf)). The Lua 5.4 manual itself disowns coercion: *"These coercions exist mainly for compatibility and may be removed in future versions of the language"* ([manual §3.4.3](https://www.lua.org/manual/5.4/manual.html#3.4.3)). Three of the mechanisms this ticket asks about are ones the designers would drop given a clean slate. A greenfield language *has* a clean slate.

**Third: the load-bearing mechanisms and the bug-producing mechanisms are not the same mechanisms.** This is the good news and the whole reason the project is viable. The measurements below separate them cleanly.

---

## The measurements

Taken 2026-07-24. Counting method stated so it can be re-run.

**Keyword counts** (`goto` puts Lua 5.4 one ahead of 5.1):

| Language | Reserved words | Source |
|---|---:|---|
| **Lua 5.1** | **21** | [manual §2.1](https://www.lua.org/manual/5.1/manual.html#2.1) |
| **Lua 5.4** | **22** | [manual §3.1](https://www.lua.org/manual/5.4/manual.html#3.1) |
| Go | 25 | [spec, Keywords](https://go.dev/ref/spec#Keywords) |
| C (C89) | 32 | ISO/IEC 9899 |
| Python 3.14 | 35 hard + 4 soft | `keyword.kwlist`, `keyword.softkwlist` |
| Odin | 41 | [`core/odin/tokenizer/token.odin`](https://github.com/odin-lang/Odin/blob/master/core/odin/tokenizer/token.odin), `B_Keyword_Begin`…`B_Keyword_End` |
| Zig | 46 | [`lib/std/zig/tokenizer.zig`](https://github.com/ziglang/zig/blob/master/lib/std/zig/tokenizer.zig), `keywords` map |
| Rust | 39 strict + 13 reserved + 5 weak = **57** | [reference, Keywords](https://doc.rust-lang.org/reference/keywords.html) |

**Grammar size** — nonterminals with a `::=` definition, counted from each project's own published EBNF:

| Grammar | Nonterminals | Of which type sublanguage | Source |
|---|---:|---:|---|
| **Lua 5.4** | **25** | 0 | [manual §9](https://www.lua.org/manual/5.4/manual.html#9) — 43 non-blank EBNF lines, one page |
| **Teal** | **43** | 18 | [`docs/src/grammar.md`](https://github.com/teal-language/tl/blob/master/docs/src/grammar.md) |
| **Luau** | **57** | 21 | [luau.org/grammar](https://luau.org/grammar) |

**Documentation size** — words after HTML tag stripping:

| Document | Words |
|---|---:|
| Lua 5.4 manual, **language only** (§1–3: lexis, semantics, metatables, GC, coroutines) | ~12,950 |
| Odin, official Overview (entire language doc) | 27,017 |
| Lua 5.4 manual, **complete** (language + C API + standard libraries) | 45,240 |
| Zig langref (master) | 64,473 |
| Rust Reference (single-page print build) | 157,950 |

The CACM paper's own figure — *"The Lua reference manual is small, with approximately 100 pages covering the language, its standard libraries, and the API with C"* — is consistent with the 45k-word count.

The headline number is not the keyword count. It is that **the entire semantics of Lua fits in ~13,000 words and the entire syntax fits on one page**, and that the Rust Reference is 12× the former. That ratio, not the keyword table, is what a reader means by "Lua-simple".

The second headline number is the grammar row: **adding static types to Lua roughly doubles the grammar (25 → 43 or 57 nonterminals), but essentially all of the growth is a self-contained type sublanguage.** In Teal, the only change to the *expression* grammar is two new operators, `as` and `is`; statement forms grow only by new *declaration* kinds. Luau's core grew more (25 → 36) but from features unrelated to types — compound assignment, string interpolation, if-else expressions, attributes. **The one-page grammar does not survive typing; a one-page imperative grammar plus a one-page type grammar does.** That is a design constraint ludo can adopt deliberately.

---

## Mechanism 1 — Small grammar and tiny keyword set

**What it is.** 22 reserved words, 25 grammar productions, 43 lines of EBNF you can print on one page, no preprocessor, no attribute syntax, no macro layer.

**Is it load-bearing?** Partly, and less than folklore says. Keyword count correlates badly with felt complexity: Go has 25 keywords and nobody calls Go "Lua-simple"; Zig has 46 and its own documented usability problems are about `comptime` and `anytype`, not keyword count ([Athaydes](https://renato.athaydes.com/posts/comptime-programming), and open proposals [ziglang/zig#17198](https://github.com/ziglang/zig/issues/17198)). What is load-bearing is the *derived* property: a grammar this small is fully readable in one sitting, and there is exactly one syntactic form per concept. Lua's own authors tie simplicity to embeddability rather than to learnability — *"Lua tries to avoid or reduce the use of special syntax for a new mechanism, as syntax is not accessible through an API"* (CACM 2018). The small grammar is partly a **side effect of the C API constraint**, not a usability goal. Worth knowing, because ludo has no such constraint and therefore does not automatically inherit the pressure that produced it.

**What Teal and Luau did.** Both roughly doubled it, both confined the growth to a type sublanguage (18 and 21 nonterminals respectively). Teal added contextual words — `record`, `interface`, `enum`, `type`, `global`, `macroexp`, `is`, `as`, `where`, `userdata`, `metamethod`. Neither dialect made the imperative core meaningfully harder to read.

**Verdict: load-bearing, `survives-typing` with a stated budget.** The realistic target for ludo is *one page of statement/expression grammar plus one page of type grammar*, roughly 45–55 productions total. Claiming Lua's 25 while having static types would be dishonest; landing at 100+ (Rust, C++) forfeits the wedge.

---

## Mechanism 2 — One aggregate data structure (the table)

**What it is.** *"Tables are the sole data-structuring mechanism in Lua; they can be used to represent ordinary arrays, lists, symbol tables, sets, records, graphs, trees, etc."* ([manual §2.1](https://www.lua.org/manual/5.4/manual.html#2.1)). Modules are tables. Objects are tables. The global environment is a table. Metatables are tables.

**Is it load-bearing?** Yes — this is the strongest candidate in the list, and the authors single it out: *"One thing that has worked really well was the early decision (made in Lua 1.0) to have tables as the sole data-structuring mechanism in Lua … We have resisted user pressure to include other data structures, mainly 'real' arrays and tuples, first by being stubborn, but also by providing tables with an efficient implementation and a flexible design"* (HOPL §7). The felt effect is that a Lua user never chooses a container. There is no `Vec` vs `array` vs `slice` vs `HashMap` vs `BTreeMap` decision, and no import to reach one.

**But it already leaks in untyped Lua.** The `#` operator is defined over "borders", and the manual is explicit: *"The table `{10, 20, 30, nil, 50}` has two borders (3 and 5), and therefore it is not a sequence. … When `t` is not a sequence, `#t` can return any of its borders. (The exact one depends on details of the internal representation of the table, which in turn can depend on how the table was populated and the memory addresses of its non-numeric keys.)"* ([manual §3.4.7](https://www.lua.org/manual/5.4/manual.html#3.4.7)). One data structure with a length operator whose result depends on memory addresses is a bug class, not a simplicity.

**What Teal did — the sharpest data point in this document.** Teal splits the one table into **six** static shapes: `record`, `interface`, array `{T}`, tuple `{T, U}`, map `{K:V}`, and the "arrayrecord" (a record with an array interface, `local record Node is {Node}`). Plus `enum`, plus userdata records. The docs then have to police the boundaries by hand: *"records and maps are distinct types with distinct uses in Teal. Even though both are implemented as Lua tables, they are not interchangeable"* ([functions.md](https://github.com/teal-language/tl/blob/master/docs/src/functions.md)). Inference across the boundary is heuristic and documented as such: *"A table will be inferred as an array if all of its elements are the same type, and as a tuple if any of its types aren't the same"* ([tuples.md](https://github.com/teal-language/tl/blob/master/docs/src/tuples.md)) — so `{1, 2, "hi"}` and `{1, 2, 3}` get structurally different types, and if you wanted an array of a union you must annotate. There is an open request for yet another shape, ["Add an 'arraymap' type"](https://github.com/teal-language/tl/issues/118), and another for ["tuplerecords"](https://github.com/teal-language/tl/issues/497). **One runtime data structure became six static ones and users are asking for an eighth.**

**What Luau did.** Kept one syntactic table type, but gave it **three internal states** — unsealed, sealed, generic — with *opposite* subtyping rules: *"Unsealed tables are *exact* in that any property of the table must be named by the type"* versus *"Sealed tables are *inexact* in that the table may have properties which are not mentioned in the type. As a result, sealed tables support *width subtyping*"* ([types/tables](https://luau.org/types/tables)). A table seals when annotated, when returned from a function, or when its scope exits. This is the mechanism behind the commonest Luau confusion — the same code type-checks in one position and not another — and the [unsealed-table-literals RFC](https://rfcs.luau.org/unsealed-table-literals.html) is candid that loosening it costs robustness: *"It does mean that some spelling mistakes will not be caught."*

The friction is concrete and open. [luau-lang/luau#745](https://github.com/luau-lang/luau/issues/745), still open, 8 upvotes: `for a, b in v do` over `{x = 1, y = 2}` fails with *"Cannot iterate over a table without indexer"*. A maintainer's answer: *"Using string fields resolve into a table type with no indexer. … It is a bit opinionated from the typechecker."* The community workaround is `for a, b in v::any do`. Users in-thread: *"This is a really annoying bug. I keep encountering it in my projects"*; *"fix this please i hate it"*. **A completely ordinary Lua idiom — iterate a record-shaped table — is rejected, and the sanctioned escape is `any`.**

**Verdict: load-bearing, `needs-a-replacement`.** "One data structure" cannot survive typing — no type system can give useful guarantees over a container that is simultaneously a struct, a list, a hash map and a namespace. What is actually load-bearing is not "one structure" but **"no container decision at the point of use, and one literal syntax"**. That is preservable: a compiled language can keep `{...}` as the single literal form and let the *annotation* (or an inferred nominal type) decide the layout, with `struct`, array and map as the only three underlying shapes, not six. The specific things Teal proves you must avoid are (a) inference that silently picks a different shape based on element homogeneity, and (b) a separate spelling for each shape at construction sites.

---

## Mechanism 3 — No type annotations anywhere

**What it is.** No declared types on variables, parameters, returns, or fields. `local x = 1` and `function f(a, b) return a + b end`.

**Is it load-bearing?** Yes for the first hour, and Roblox has the hardest number in either direction. Their telemetry paper over 1.5M records from 340,000 Roblox Studio sessions reports *"opt-in gradual types are unpopular: there is a 100x gap between the number of untyped Luau sessions and the number of typed ones"* ([arXiv:2403.02409](https://arxiv.org/pdf/2403.02409)). The Luau team's own design constraint states the audience assumption directly, in the local-type-inference RFC: *"It's not viable for us to require users to write type annotations. Many of our users do not yet know what types are but we are nevertheless committed to providing them a tool that is helpful to them"* ([local-type-inference.md](https://rfcs.luau.org/local-type-inference.html)).

But read the 100× number carefully. It measures adoption of an **optional** feature that produces **no benefit** if ignored — the counterfactual "would they annotate if annotations were required and inference were good?" is not measured by it. The honest reading is narrower than "people hate annotations": **opt-in typing does not get adopted**, which is an argument for making types mandatory, not for having none.

**Where inference actually gives out.** Three documented cliffs, all worth stealing as requirements:

1. **Untyped `local` with no initialiser.** Teal simply forbids it: *"`local x` — Error! What is this?"* ([local_variables.md](https://github.com/teal-language/tl/blob/master/docs/src/local_variables.md)). Lua's zero-ceremony declaration form is gone.
2. **Ordering sensitivity.** Luau's own HATRA'23 paper: *"Currently the type inference system uses **greedy inference**, which is very sensitive to order of declarations, and can easily result in false positives"* ([hatra23.pdf](https://asaj.org/papers/hatra23.pdf)). The new solver's local type inference exists to fix exactly this — the motivating example in the RFC is `index_of`, where the old solver saw `return i` first and refused `return nil`.
3. **`self` across methods.** Covered under Mechanism 6; Luau's docs currently mark the `self` annotation *"a **required** annotation."*

**Verdict: NOT load-bearing in the form stated; `dies-under-typing` and should.** What is load-bearing is the *absence of ceremony at the point of use* — not the absence of types. The evidence for that reframing is that Lua users overwhelmingly *do* write type information, just in comments: LuaCATS annotations in `lua-language-server` are widespread, and one Teal user reports preferring them (see Mechanism 5). Local inference for `local` bindings plus mandatory annotations on **module boundaries only** captures the felt property. This also happens to be the agent-friendly choice: a mandatory signature at the module boundary is exactly the context an LLM needs in order to write a caller without reading the callee, and the sibling survey's central finding — in-context idioms are worth 24 points to Sonnet 4.5 on an unseen language ([arXiv:2605.15607](https://arxiv.org/html/2605.15607v1)) — argues that signatures are load-bearing *documentation*, not overhead.

---

## Mechanism 4 — Uniform function syntax, functions as values, multiple returns

**What it is.** One `function` keyword for named, anonymous, local, method and field definitions. Functions are ordinary values. Calls return any number of values, adjusted silently at the use site.

**Is it load-bearing?** The uniformity, yes. The multiple returns, yes — HOPL records the rationale, and it is a *simplicity* argument: *"From CLU we took multiple assignment and multiple returns from function calls. We regarded multiple returns as a simpler alternative to reference parameters used in Pascal and Modula and to in-out parameters used in Ada; we also wanted to avoid explicit pointers (used in C)"* (HOPL §4). Multiple returns are the mechanism that lets Lua have no out-parameters, no pointers, no tuple type, and no `Result` type, all at once. That is a lot of concepts avoided by one feature, which is precisely the "economy of concepts" pattern. Odin and Go independently confirm the ergonomics.

**The colon is the crack in the uniformity, and the authors say so.** CACM 2018: *"the colon syntax can be somewhat confusing, mainly for programmers who are new to Lua but have some experience with another object-oriented language. Lua needs that syntax because of its economy of concepts that avoids introducing the concept of method when the existing concept of function will suffice."* An explicit admission that a *usability* cost was paid for an *economy-of-concepts* gain.

**What typing costs.** Multiple returns survive — but they force a second kind of type variable into the type system, and the complexity is severe. Luau needed **type packs**: `T` and `T...` are distinct kinds, generics range over both, and the [type-alias type packs RFC](https://rfcs.luau.org/syntax-type-alias-type-packs.html) enumerates the resulting arity rules — `type E<S...> = Y<S..., number, string>` is *"error, type parameters are not allowed after a type pack"*; `type G<S...> = Z<S...>` is *"error, not enough regular type arguments, can't split the front of `S...` into `T`"*; `X<(number)>` and `X<number>` mean the same thing but `Y<(number), (string)>` is *"error: too many type pack parameters"*. The same RFC works through `Car`, `Cdr` and `Cons` type-level list operations. This is in a language whose own team says its users "do not yet know what types are". Teal's version is lighter (`...: number`, return `number...`, and casts like `table.unpack(s) as (number, string)`) but has an open request for [parameter packs](https://github.com/teal-language/tl/issues/670), which is the same complexity arriving later. Luau's own typed-variadics RFC concedes the wart: *"The mismatch between the type of `...` in function declaration (`number`) and type declaration (`...number`) is a bit awkward. This also gets more complicated when we introduce generic variadic packs."*

**Verdict: load-bearing, `survives-typing` — with the type-system cost paid up front.** Multiple returns are worth keeping, but the lesson from Luau is that **variadic packs are where the cost lives, not multiple returns per se**. A fixed-arity multiple return (`-> (int, string)`) is cheap and Odin-proven. Generic variadic packs are where the type algebra explodes. A defensible ludo position: fixed-arity multiple returns yes; generic variadics no, or restricted to homogeneous `...T` with no generic pack variables. Also: **drop the colon.** It buys one concept and costs the uniformity that made the mechanism load-bearing in the first place, and Luau is currently unwinding it ([shared-self-types RFC](https://rfcs.luau.org/shared-self-types.html)).

---

## Mechanism 5 — `nil` as a universal absent value that is also falsy

**What it is.** One value means uninitialised variable, absent table field, missing argument, "no more results", and error-return sentinel. It is also falsy, so `if x then` tests presence.

**Is it load-bearing?** For terseness, yes — `x = x or default`, `if t.field then`, and `for k in next, t` all depend on it. For simplicity of the *language definition*, also yes: one value replaces `Option`, `undefined`, default arguments, and an absent-key protocol.

**The authors regret it, twice, in writing.** HOPL §6.1: *"In retrospect, it would probably have been better if **nil raised an error in boolean expressions**, as it does in other expressions. This would be more consistent with its role as proxy for undefined values. However, such a change would probably break many existing programs."* And §7: *"Not introducing booleans from the start had a few unfortunate side-effects. One is that we now have **two false values**: `nil` and `false`. Another is that a common protocol used by Lua functions to signal errors to their callers is to return `nil` followed by an error message. It would have been better if `false` had been used instead of `nil` in that case, **with `nil` being reserved for its primary role of signaling the absence of any useful value**."*

**Teal: total capitulation.** *"Every type in Teal accepts `nil` as a valid value, even if, like in Lua, attempting to use it with some operations would cause a runtime error, so be aware!"* ([local_variables.md](https://github.com/teal-language/tl/blob/master/docs/src/local_variables.md)). Teal has **no nil safety at all**. The top-voted **open** issue on the tracker is [#598, "Every type contains nil"](https://github.com/teal-language/tl/issues/598) (14 👍), opened with:

> ```
> local x: number
> x = nil
> print(x + 1)
> ```
> "is valid according to `tl`, yet it crashes at runtime."

Real user reactions in that thread, which are the most direct friction evidence in this document:

- *"My gut feeling is that this is a deal-breaker of the projects I'm working on."* (Feuermurmel)
- *"`nil` errors are constant source of pain for me when using Teal, which is a pain I no longer have with normal Lua!"* (lewis6991, comparing against `lua-language-server`'s `T?` annotations)
- *"we should fix this issue, don't want to continue the billion dollar mistake"* (w1kee)

Read the second one again. **A user reports that a typed dialect of Lua gives worse nil safety than untyped Lua plus a language server.** That is the sharpest possible statement of what "simplicity paid for by robustness" costs when you inherit it rather than design it out.

Hisham Muhammad's own position, from the linked discussion ["Much ado about nil"](https://github.com/teal-language/tl/discussions/339#discussioncomment-2523409), names the three obstacles precisely — and they are exactly the three problems ludo owes an answer to:

1. *"how to deal with arrays without having to nil-check every `a[i]` access: go Typescript-like and make arrays unsound or some other solution"*
2. *"how to deal with another common pattern: constructing tables that have 'required' fields starting from `{}`"*
3. *"how more complex will the implementation be once people start expecting smarter and smarter flow typing"*

**Luau: bought it, at the cost of a refinement engine.** Luau has `T?` optional types and a documented refinement system — truthiness tests, `type()` guards, equality/singleton narrowing, `assert`, composed with `and`/`or`/`not` ([types/type-refinements](https://luau.org/types/type-refinements)). This works, and it is the right answer. But it is not free, and the tracker records exactly where it hurt: [#1038, "Using 'or' should refine optional types"](https://github.com/luau-lang/luau/issues/1038) — `num = num or 1` on a `number?`, *the* canonical Lua default-argument idiom, failed to narrow, and the reporter's workaround list is telling: *"You can work around this with type casts but it's kind of annoying and then you can't use compound assignment operators like `+=`. You can also just redefine the variable as `local num = num or 1` but then you get a shadowed variable warning."* Closed only when the new solver's typestates landed. Related open/closed friction: [#356](https://github.com/luau-lang/luau/issues/356) (`if not x then return end` should exclude falsy types afterwards), [#913](https://github.com/luau-lang/luau/issues/913) (guard clauses using `continue`/`break` should refine), [#2236](https://github.com/luau-lang/luau/issues/2236) (iterating a table of optionals doesn't strip `nil`). **Every one of those is a case where flow analysis was weaker than an ordinary reader's inference, and the user paid in casts.**

**Verdict: `nil`-as-universal-absent is load-bearing; `nil`-as-falsy is not, and it is the bug class. `needs-a-replacement`.**

The replacement is well-established and the evidence supports it:

- **`T?` as the only optional form**, non-nullable by default. Luau proves the syntax is Lua-legible; Teal proves omitting it is fatal to the value proposition.
- **Refinement/narrowing is mandatory, not optional.** Without it, `T?` is unusable and users cast. The specific patterns that *must* narrow on day one, from the Luau tracker: `x = x or default`; `if not x then return/continue/break end` (early-exit narrowing for the rest of the block); `assert(x)`; iteration over `{T?}`.
- **`nil` must not be falsy.** Both regret quotes point here, and it is the one change that costs Lua compatibility and costs ludo nothing. `if x then` should require `x : bool`. This is the largest single terseness loss in the whole document and it should be taken deliberately, not by accident.
- **Answer Hisham's three questions in the spec.** Array indexing (`a[i] : T?` and pay for it, or a checked `a[i]` that traps, or both via distinct operators); required-field construction (a literal must be complete, or a designated builder state); a stated ceiling on flow-typing ambition so the implementation stays finite.

---

## Mechanism 6 — No inheritance in the language proper; metatables as an opt-in escape hatch

**What it is.** Lua has no `class`. Prototype-based OOP is assembled from tables, `setmetatable`, `__index`, and the colon. CACM 2018 defends this on economy-of-concepts grounds and notes the payoff: *"Because method selection and the variable `self` are independent, Lua does not need additional mechanisms to call methods from other classes (such as 'super')."*

**Is it load-bearing?** For the *language*, yes — it deletes an entire subsystem. For the *user*, it is where the "not good for quick-and-dirty programs" admission bites hardest, and the authors name the cost: *"the do-it-yourself approach to classes and objects leads to proliferation of different, often incompatible, systems"* (CACM 2018). Every Lua codebase ships its own `class.lua`. That is a real tax on beginners and a severe one on AI agents, which must infer which of a dozen incompatible class idioms is in play from surrounding code — precisely the context an agent is supposed not to need.

**What Teal did — it reintroduced the hierarchy.** Teal has `interface`, records inheriting interfaces via `is`, multiple interface inheritance, a `self` type that resolves to the child record, `where` discriminator clauses, and nominal typing with explicit aliasing rules ([interfaces.md](https://github.com/teal-language/tl/blob/master/docs/src/interfaces.md), [aliasing.md](https://github.com/teal-language/tl/blob/master/docs/src/aliasing.md)). It stops short of implementation inheritance and says why: *"records cannot inherit from other records … you need to ensure that the actual implementations of these functions are resolved at runtime the same way as they would do in Lua, most likely using metatables. **Teal does not implement a class/object model of its own, as it aims to be compatible with the multiple class/object models that exist in the Lua ecosystem.**"* So Teal pays for the subtyping *and* still makes you write the metatable plumbing. Worst of both.

**What Luau did — and its own docs are the indictment.** The page is titled ["Adding types for faux object oriented programs"](https://luau.org/types/object-oriented-programs). It concedes Luau *"cannot pin down on the types of `self` when it spans multiple methods"*, that `Account.new` infers as `<a,b>(name: a, balance: b) -> {...}` so *"you are allowed to call `Account.new(5, "hello")` as well as `Account.new({}, {})`"*, and that the fix requires:

```luau
export type Account = typeof(setmetatable({} :: AccountData, Account))
```

with, on every method, *"this annotation on `self` is currently a **required** annotation."* And the fix converts `function Account:deposit(credit)` into `function Account.deposit(self: Account, credit: number)` — **the colon sugar, the one syntactic concession Lua made to make objects feel native, has to be abandoned at the definition site.** Luau has an RFC in flight to undo this ([shared-self-types](https://rfcs.luau.org/shared-self-types.html) — proposed, not marked implemented as of 2026-07-24) precisely because the status quo is untenable, and the docs say the change is being made *"based on feedback"*.

**Verdict: load-bearing for the language definition, actively harmful for users and agents. `dies-under-typing`, and it should die.** Two independent typed dialects both had to reintroduce nominal types and a `self` story, and neither ended up simpler than just having a `struct` with methods. `typeof(setmetatable({} :: AccountData, Account))` is not something a Lua user recognises; it is an incantation. The mechanism to preserve is not "no classes" but **"no inheritance hierarchy"** — struct + methods + interfaces-as-constraints, no subclassing, no `super`, no vtables by default. That keeps the concept Lua actually deleted (the hierarchy) while deleting the thing Lua's absence of classes actually produced (a dozen incompatible userland class systems, and an untypeable `self`).

---

## Mechanism 7 — Minimal ceremony to a running program

**What it is.** `print("hi")` in a file, `lua hi.lua`. No `main`, no imports, no build step, no manifest, no project directory. A chunk *is* a function body; top-level code just runs.

**Is it load-bearing?** Yes, and this is the mechanism most under-credited relative to its effect. It is also the one Lua's authors did *not* claim as a virtue — see the "not as good for quick-and-dirty programs" admission — because they were thinking about the *embedded* case, where setup is the host's job. For a standalone language it is the whole first impression. The sibling survey's DragonRuby section is the corroboration: the layering philosophy is explicitly along the **ceremony** axis, not the beginner/expert axis.

**What Teal and Luau did.** Teal keeps `tl run add.tl` for a single file, which is genuinely close to Lua. But the type system forces ceremony *into the source*: `local x` is illegal without a type; every global must be declared (`global n: number`); every function must be `local` or `global` (*"Unlike Lua, bare `function` declarations are not global by default"*); anything non-trivial needs `tlconfig.lua`; and using untyped Lua libraries needs hand-written `.d.tl` files, of which the community repo covers ~44 libraries against LuaRocks' thousands. Notably, Teal also has a **type-checking-optional runtime path** — loading a `.tl` module from Lua via `tl.loader()` gives you *"no type checking!"* ([your_first_teal_program.md](https://github.com/teal-language/tl/blob/master/docs/src/your_first_teal_program.md)) — another hole where the guarantee leaks. Luau in Roblox Studio has no ceremony at all, but that is the IDE's doing, not the language's; standalone Luau needs `.luaurc` for anything with modules.

**Verdict: load-bearing, `survives-typing` but only if defended as an explicit requirement.** Nothing about static typing requires a manifest, a `main`, or an import block for the first program. Odin manages `package main` + `import "core:fmt"` and that is already more than Lua. The specific commitments worth writing into the spec: single-file programs run with one command and no config file; top-level statements execute; the compiler infers project structure from the filesystem rather than a manifest; and there is no untyped escape path like `tl.loader()` that silently disables the checker.

**Conflict to name, not resolve.** Zero ceremony is unambiguously good for a human beginner. For an **AI agent** it is mildly negative: an explicit module header is free context that tells the model what is in scope, and Lua's "any bare name is a global" (Mechanism 8) means an agent's typo compiles. The resolution is probably *ceremony-free but not context-free* — no manifest, but a mandatory explicit import list, which costs the human one line and buys the agent a closed name-space.

---

## Mechanism 8 — Forgiving semantics: coercions, missing-key reads, loose arity

Three separate mechanisms usually bundled together. They have different verdicts and should be separated.

### 8a — Missing-key reads return `nil`

*"Any key associated to the value `nil` is not considered part of the table. Conversely, any key that is not part of a table has an associated value `nil`"* ([manual §2.1](https://www.lua.org/manual/5.4/manual.html#2.1)). Elegant as a definition — presence and absence are the same predicate — and it is what makes `t.optional_field` need no ceremony.

It is also the single largest bug source measured in the Luau telemetry: `UnknownSymbol` and `UnknownProperty` dominate the error mix, and the authors conclude *"most errors in the data are likely due to typos during edits"* ([arXiv:2403.02409](https://arxiv.org/pdf/2403.02409)).

**What Luau did, and it is the most useful design finding here.** Luau cannot change the runtime, so the [new non-strict mode RFC](https://rfcs.luau.org/new-nonstrict.html) reclassifies the *result* as a defect:

> "Luau tables do not error when a missing property is accessed … `local x = t.Fop` won't produce a run-time error, but is more likely than not a programmer error. … **For this reason, we consider it a code defect to use a value that the type system guarantees is of type `nil`.**"

And on the write side — `t.Fop = 7` where `Fop` is never read — they concede defeat: *"The only way to detect that `Fop` was never read would be whole-program analysis, which is prohibitively expensive."* Luau's partial answer is read-only and write-only properties, making it an error to create a write-only one.

**Verdict: simplicity paid for by robustness. `needs-a-replacement`.** The replacement is straightforward once fields and keys are distinguished (Mechanism 2): **a struct field access is either present or a compile error — there is no `nil` outcome**; a *map* lookup returns `T?` and must be narrowed. That is the entire fix, it costs no syntax, and it is what makes the `?` in Mechanism 5 tractable rather than viral. Luau's "guaranteed-`nil` expressions are defects" rule is a good secondary lint to keep even in a language that got this right, because it also catches `if x.field == nil` where `field` is non-optional.

### 8b — Loose arity

*"At call time they accept a variable number of arguments: excess arguments are discarded and missing arguments are given the value `nil`"* (HOPL §6.5). `CountMismatch` is among the top error kinds in the Roblox telemetry.

**Teal's history is the clean experiment.** Teal was arity-lenient through 0.15.x, then added arity checking — and had to ship a `--#pragma arity off` mechanism because existing code and existing `.d.tl` files depended on the leniency. Their own rationale is exactly right: *"More often than not, this is not the case, and a missing argument does not mean that the argument was optional, but rather that the programmer forgot about it (this is common when adding new arguments during a code refactor)"* ([pragmas.md](https://github.com/teal-language/tl/blob/master/docs/src/pragmas.md)). The corresponding request, [tl#71 "Required / Optional function arguments?"](https://github.com/teal-language/tl/issues/71), is in fact the most-upvoted issue on the whole Teal tracker (15 👍, closed).

Note the residual hole Teal documents: *"arity checks are about the number of _expressions_ used as arguments … it does not check whether the _values_ are `nil` or not. … you could still write `greet(nil, nil)` and that would be accepted by the compiler as valid, even though it would crash at runtime. Explicit checking for `nil` is a separate feature, which may be added in a future version."*

**Verdict: not load-bearing, pure bug class. `dies-under-typing`, and Teal already killed it.** Strict arity plus explicit optional parameters (`name?: string`) with defaults. No cost to felt simplicity — nobody's mental model of "call a function" includes silent argument dropping.

### 8c — Implicit coercions

The authors' retrospect: *"Automatic coercion of strings to numbers in arithmetic operations, which we took from Awk, could have been omitted."* The 5.4 manual: *"It is always a good practice not to rely on the implicit coercions from strings to numbers, as they are not always applied; in particular, `"1"==1` is false and `"1"<1` raises an error … These coercions exist mainly for compatibility and may be removed in future versions of the language."* Luau's new non-strict RFC classifies them as defects outright: *"Luau supports various implicit coercions, such as allowing `math.abs("-12")`. These should be reported as defects."*

**Verdict: not load-bearing, disowned by the authors and by both dialects. `dies-under-typing`.** Delete. Note the one coercion the authors *did* keep: number→string in concatenation is *"convenient and less troublesome"*, and `..` exists as a separate operator precisely so that `+` could stay unambiguous (HOPL §4). A separate concatenation operator is a cheap, Lua-recognisable way to keep the ergonomics without the ambiguity — keep `..`, and consider whether it should still auto-stringify numbers or require an explicit conversion. That is a genuine open question, not a settled one.

---

## Mechanisms the research surfaced

### 9 — Only two kinds of variable

*"Lua offers only two kinds of variables: local variables and table fields. Syntactically, Lua also offers global variables: any name not bound to a local declaration is considered global. Semantically, these unbound names refer to fields in a particular table associated with the enclosing function"* ([Passing a Language through the Eye of a Needle](https://www.lua.org/doc/cacm2011.pdf), CACM 54(7), 2011).

This is a genuine and under-discussed economy: there are no module-level variables, no statics, no class variables, no namespaces — all of them are table fields. It is why Lua needs no `import` statement and no visibility keywords: *"any file of Lua code can declare local variables that are visible only inside the file."*

**Its dark twin is global-by-default**, and it is the clearest simplicity-for-a-human / hazard-for-an-agent case in this document. Luau's linter documentation states it plainly: *"By default, variables in Luau are global (this is inherited from Lua 5.x and can't be changed because of backwards compatibility). This means that **typos in identifiers are invisible to the parser, and often break at runtime**."* And the lint is only half a fix: *"The `UnknownGlobal` lint can catch typos in globals that are read, but **can't catch them in globals that are assigned to**"* ([lint reference](https://luau.org/lint)). Teal simply reversed the default and says why: *"global variables in Teal need to be declared … It also allows the compiler to catch typos in variable names, because an invalid name will not be assumed to be some unknown global that happens to be nil."*

**Verdict: the two-kinds-of-variable economy is load-bearing and `survives-typing`. Global-by-default is a pure bug class, `dies-under-typing`, and Teal already killed it.** Keep "everything is a local or a field"; make declaration mandatory; make the module's exported surface an explicit list.

### 10 — "Mechanisms, not policies"

*"Our motto in the design of Lua has always been 'mechanisms instead of policies'"* (CACM 2018). Simplicity by *deferral*: the language supplies tables and functions and lets userland build modules, classes, and error handling.

The authors log its cost twice — the incompatible class systems, and: *"Despite our 'mechanisms, not policy' rule — which we have found valuable in guiding the evolution of Lua — we should have provided a precise set of policies for modules and packages earlier"* (HOPL §7).

**Verdict: load-bearing for the language definition, and the sharpest lens/lens conflict in the ticket.** Deferral makes the *language* small and the *ecosystem* fragmented. For an AI agent this is close to worst-case: the sibling survey's PyLang result is that models pick the right algorithm 80% of the time and fail on implementation fidelity, so **every additional userland-defined idiom is another way to be subtly wrong**, and by construction the agent cannot infer which one is in play without reading the rest of the codebase — the exact failure the destination's agent-friendliness lens is meant to prevent. This is where "Lua-simple" and "agent-friendly" pull hardest against each other. **Name it; do not resolve it silently.** The defensible position for ludo is to invert the motto exactly once, for the things Lua's authors themselves said they got wrong: ship *policy* for modules, for object construction, and for error handling, and keep *mechanism* everywhere else.

---

## Ranking

Ordered by how load-bearing the mechanism is for the felt simplicity of Lua.

| # | Mechanism | Load-bearing? | Verdict | Paid for by robustness? |
|---:|---|---|---|---|
| 1 | **Minimal ceremony to a running program** (M7) | **High** — the entire first impression | `survives-typing` (defend explicitly) | No |
| 2 | **One aggregate data structure** (M2) | **High** — no container decision, ever | `needs-a-replacement` | Partly — `#` on non-sequences is unspecified |
| 3 | **~13k-word semantics / one-page grammar** (M1) | **High** — but as a *derived* property, not the keyword count | `survives-typing` with a doubled budget | No |
| 4 | **Two kinds of variable** (M9) | **High** — deletes modules, statics, namespaces, visibility | `survives-typing` | No (but global-by-default is a pure bug class) |
| 5 | **Uniform function syntax + multiple returns** (M4) | **High** — deletes out-params, pointers, tuples, `Result` | `survives-typing`; drop the colon; cap variadics | No |
| 6 | **`nil` as universal absent value** (M5) | **Medium-high** — terseness of `x or d`, `if t.f then` | `needs-a-replacement` (`T?` + mandatory narrowing) | **Yes — the largest single bug class** |
| 7 | **No type annotations anywhere** (M3) | **Medium** — real for hour one; 100× telemetry gap is about *opt-in*, not annotations | `dies-under-typing`; replace with boundary-only annotations | **Yes** |
| 8 | **"Mechanisms, not policies"** (M10) | **Medium** — small language, fragmented ecosystem | `survives-typing`, but invert it for modules/objects/errors | **Yes — indirectly, via idiom fragmentation** |
| 9 | **Missing-key reads return `nil`** (M8a) | **Medium** — one predicate for presence and absence | `needs-a-replacement` (fields error; map lookups return `T?`) | **Yes — top error class in Roblox telemetry** |
| 10 | **No inheritance; metatables as escape hatch** (M6) | **Low for users, high for the language** | `dies-under-typing` — replace with struct + methods + interfaces | **Yes — untypeable `self`, incompatible class systems** |
| 11 | **Loose arity** (M8b) | **None** | `dies-under-typing` — Teal already killed it | **Yes — `CountMismatch` in telemetry** |
| 12 | **Implicit coercions** (M8c) | **None** — disowned by authors and manual | `dies-under-typing` | **Yes** |
| 13 | **`nil` is falsy** (M5, split out) | **Low** — saves keystrokes in `if x then` | `dies-under-typing` — authors say they'd change it | **Yes — the regret they name first** |

The pattern to read off the table: **the top five are free.** Nothing about static typing forces you to give up zero-ceremony startup, one literal syntax, a small grammar, two kinds of variable, or multiple returns. Everything from #6 down is either paid for by robustness or was already disowned by Lua's own designers. The tension the destination worries about is real but it is concentrated in the bottom half of the list, and the bottom four items cost almost nothing to delete.

---

## Autopsy: what typing Lua actually cost

Teal and Luau made opposite bets and surrendered different things. Comparing the two is more informative than either alone.

**What both lost.**

- **The one-page grammar.** 25 → 43 (Teal) / 57 (Luau) nonterminals. Both confined the growth to a type sublanguage; neither made the imperative core harder.
- **The one data structure.** Teal split it into six named shapes; Luau kept one shape with three states and opposite subtyping rules in two of them. Both produce "why does this work here and not there" as their signature confusion.
- **`self`.** Both had to reintroduce nominal types and a story for method receivers. Luau still calls its own OOP page "faux object oriented programs" and requires `typeof(setmetatable({} :: AccountData, Account))`.
- **Ecosystem coverage.** Teal's `.d.tl` coverage is ~44 libraries against LuaRocks' thousands; Luau's `any` casts are the sanctioned route around embeddings and cyclic requires, per its own docs.

**What Teal surrendered that Luau did not: nil safety.** *"Every type in Teal accepts `nil` as a valid value."* The consequence is measurable in user sentiment on its own tracker — a deal-breaker for one user, and worse than untyped Lua plus a language server for another. Teal also surrendered union expressiveness for a structural reason worth restating: `is` compiles to Lua's `type()`, so unions are limited to what a Lua runtime tag can discriminate, and `where` clauses are unverified — *"Teal has no way of proving at compile time that the set of `where` clauses in the union is actually disjoint … that is up to you"* ([current_limitations_on_union_types.md](https://github.com/teal-language/tl/blob/master/docs/src/current_limitations_on_union_types.md)). **A natively-compiled language pays neither tax.**

**What Luau surrendered that Teal did not: soundness and inference predictability.** Luau is sound only over the `any`-free fragment by its own HATRA'23 paper, and `any` suppresses errors transitively by design ([type-error-suppression RFC](https://rfcs.luau.org/type-error-suppression.html)). Old-solver inference was greedy and order-sensitive by their own admission. Luau bought `T?` and refinements, and then spent years closing refinement holes (`x or default`, early-exit narrowing, guard clauses, optional iteration) that a reader would consider obvious.

**What both had to keep, and what it cost.** Backward compatibility with Lua's runtime is the single constraint that produced almost every wound above. `nil` cannot stop being falsy. Missing keys cannot start erroring. Globals cannot stop being globals (*"can't be changed because of backwards compatibility"*). Arity cannot tighten without a pragma. Metatables must remain the object mechanism. **Every one of those constraints is absent for a language that compiles natively and has no legacy corpus.** The autopsy's real finding is that the Teal/Luau ceiling is a compatibility ceiling, not a type-theory ceiling — which is exactly the gap the sibling survey identified.

**The escape valve, and what it costs.** Teal's `any` is honest and annoying: *"there isn't much you can do with it, besides comparing for equality and against nil, and casting it into other values using the `as` operator"*, and it exists as a *"last resort"* for ecosystem interop. Luau's `any` is convenient and corrosive: it suppresses errors transitively, and the official docs recommend it for breaking cyclic module dependencies. The design lesson: **an escape hatch that must be re-stated at each use site (Teal's `as`) is strictly better for a language that wants real guarantees than one that infects a whole dataflow (Luau's `any`).** If ludo has an escape hatch at all, it should be per-expression, syntactically loud, and greppable.

---

## The shortlist

**To earn the phrase "Lua-simple", ludo must keep these five. They are all free — none of them conflicts with static typing.**

1. **A program is a file you run.** `ludo run hello.ludo` with no manifest, no `main`, no import block, no project directory. Top-level statements execute. No path exists that runs code with checking disabled.
2. **One literal syntax for aggregates, and no container decision at the point of use.** `{...}` constructs everything. At most three underlying shapes (struct, array, map), chosen by the annotation or the nominal type, never by inference heuristics over element homogeneity.
3. **A budget of roughly one page of statement/expression grammar plus one page of type grammar** — call it 50 productions and under 30 keywords. Teal and Luau both prove the type sublanguage can be quarantined so the imperative core stays Lua-shaped.
4. **Two kinds of variable: locals and fields.** No statics, no class variables, no namespace construct. Declaration mandatory; the module's exported surface is an explicit list.
5. **Uniform function syntax, functions as plain values, fixed-arity multiple returns.** One `function` form. No colon. No out-parameters, no `Result` type — multiple returns do that job, as they were designed to.

**And it owes these six replacements for the mechanisms that die.**

| Dies | Owed replacement |
|---|---|
| `nil` everywhere, `nil` is falsy | Non-nullable by default; `T?` as the only optional form; `if` requires `bool`. **Narrowing is a shipped feature, not a nice-to-have**: `x = x or default`, `if not x then return/continue/break end`, `assert(x)`, and iteration over `{T?}` must all narrow on day one — those are the four cases Luau's tracker shows users hitting. |
| Missing-key reads returning `nil` | Struct field access is present-or-compile-error, never `nil`. Map lookup returns `T?`. Answer Hisham's array question explicitly in the spec: what does `a[i]` return, and is it checked. |
| Required fields defaulting to absent | A struct literal must be complete, or there is a designated incremental-construction form with a defined completion point. Teal's `{}`-satisfies-any-record hole ([tl#721](https://github.com/teal-language/tl/issues/721)) is the failure to avoid. |
| Loose arity | Strict arity; explicit `name?: T` optionals with defaults. Already proven by Teal, including the migration pain that proves it was needed. |
| Implicit coercions | None, except a decision to make about whether `..` still auto-stringifies numbers. Keep `..` as a distinct operator — it is why `+` was never ambiguous. |
| No classes / metatable OOP | Struct + methods + interfaces-as-constraints. **No inheritance hierarchy, no `super`, no subclassing** — that is the concept Lua actually deleted, and it is worth keeping deleted. Do not make users invent an object system; the fragmentation cost falls hardest on AI agents. |

**The two lens conflicts, named rather than resolved.**

- **Zero ceremony (human beginner) vs. closed name-space (agent).** No manifest is good for both; global-by-default and implicit scope are good for the human and bad for the agent, since a typo compiles and Luau's linter cannot even catch the assignment case. Provisional resolution: ceremony-free but not context-free — no config file, but a mandatory explicit import list.
- **"Mechanisms, not policies" (small language) vs. one obvious way (agent).** Lua's authors documented both sides of this themselves: the motto kept the language small, and it produced *"proliferation of different, often incompatible"* class systems and a modules policy they wish they had shipped earlier. Every userland-defined idiom is a way for a model to be subtly wrong about a codebase it cannot see. Provisional resolution: invert the motto exactly three times — modules, object construction, error handling — and keep it everywhere else.

---

## Evidence quality — what is weak

Stated plainly.

- **The Teal friction quotes are a handful of named individuals on one GitHub thread**, not a survey. They are unusually direct and one is from a well-known Lua-ecosystem contributor, but N is small. The 15-upvote count on [tl#598](https://github.com/teal-language/tl/issues/598) is real but the whole tracker is small.
- **No LLM evaluation of Teal or Luau exists**, and none of Lua-vs-typed-Lua. The MultiPL-E figures for Lua (StarCoderBase-15B pass@1 26.6, vs JavaScript 31.7) are 2023-era open models on HumanEval-derived toy problems and measure corpus size more than language design. Every agent-friendliness claim in this document is reasoning from the sibling survey's PyLang and RustEvo² results plus documented failure modes, not from a measurement of Lua.
- **The 100× typed/untyped telemetry gap measures opt-in adoption, not annotation burden.** It does not tell you what happens when types are mandatory and inference is good. I have used it only to argue against optional typing, which is what it supports.
- **"Load-bearing for felt simplicity" is not directly measurable** and I have not pretended otherwise. Where I claim it, the evidence is either the designers' own stated rationale, or the fact that both typed dialects independently had to surrender the same thing. Mechanism 7 (ceremony) is the one where I am relying most on inference rather than evidence; the supporting material is DragonRuby's written layering philosophy in the sibling survey, not anything about Lua.
- **Grammar nonterminal counts are not perfectly comparable across projects** — each writes its EBNF in its own style, and Luau's core-grammar growth (25 → 36) is partly stylistic and partly from non-type features. The type-sublanguage counts (18 and 21) are the reliable part of that comparison.
- **The Odin/Zig/Rust word counts measure different kinds of document** (Odin's "Overview" is a tutorial-flavoured single page; the Rust Reference is a normative spec). They establish an order of magnitude, not a precise ratio.
- **`docs/agents/domain.md` was not consulted** for terminology alignment; if this document's vocabulary conflicts with `CONTEXT.md`, that file wins.
