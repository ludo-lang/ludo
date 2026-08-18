---
status: accepted
---

> **Absorbed by [spec ch4](../spec/04-modules.md):** the chapter is normative; this ADR
> keeps the argument — the supplied name-to-location mapping, the no-search rule and the runner's default one-level scan are transcribed there.

# The compiler is handed a name-to-location mapping and never searches

Issue #63 asked whether the spec discharges ADR-0014 §12's four obligations with
a resolution rule, or rules the mechanism out of scope. The map lists *"Build
system and package manager"* under Out of scope, so the scoping collision had to
be settled explicitly rather than by silence.

It settles on a line that is not the one the ticket anticipated: **acquisition is
out of scope; consumption of the result is in.** Fetching, versioning, caching
and placing a library remain a follow-on toolchain effort. What compilation
*consumes* is spec territory, because the compiler is the thing consuming it.

## 1. The compiler does not search

ADR-0014 §7 already contained the sentence this decision makes normative: *the
compiler does not search for a directory, it is given a mapping.* Stated as an
obligation:

**Compilation takes a name → location mapping as a required input, complete
before compilation begins.** The compiler performs **no search** — no environment
variable, no ambient directory walk, no parent-directory traversal, no network,
no fallback of any kind.

Consequently `use vector-math` is a **lookup, not a search**. If `vector-math` is
not a key, that is a compile error, and there is no second place to try. The
never-guess clause (ADR-0018) applies: the diagnostic may not suggest a name it
has not confirmed exists.

Two of ADR-0014 §12's obligations become **checkable** rather than merely
inherited:

- **Obligation 2** (one root name, one library): a mapping carrying two entries
  for one root name is **rejected**, loudly, at the point it is supplied. §6's
  invariant stops being hopefully true.
- **Obligation 3** (deterministic, order-independent): resolution is a total
  function over a fixed table, so determinism follows from the shape rather than
  from a tool's good behaviour.

What the spec does **not** fix: any file format, any CLI flag, any registry, any
version policy, any lockfile. The mapping is a required input, not a syntax.

## 2. `$.` is privileged and never in the mapping

ADR-0014 §4 called the standard library "an ordinary module set". That was a
statement about its **shape** — no magic in how you call it — not about its
provenance.

**`$.` is always present, never a key in the mapping, and not shadowable.** #24
counts `$.` as the reserved root every program shares, and ADR-0019 has claimants
conforming to the spec alone; a `$.` a build tool could swap would turn
conformance into a supply-chain question. §4 gains a clarifying line.

## 3. The runner builds a default mapping by scanning `libs/`

"The build system supplies it" is not an answer when the map has ruled the build
system out of scope, and #19 mandates single-command run while ADR-0012 makes the
bare command the whole toolchain. A beginner's first library must not require a
config file.

**A conforming runner, given no mapping, builds one by scanning `libs/` beside
the entry file.** Each immediate subdirectory is a candidate; the runner reads
its `library <name>` declaration and **that claim is the key**. One level, no
recursion. Two subdirectories claiming one name is a loud error (§1).

**The folder name is irrelevant** — this is what keeps ADR-0014 §7 intact. The
library names itself (§3 there), so source still names no location, location is
still not load-bearing, and `libs/vm-fork-2/` claiming `vector-math` resolves
exactly as a clean checkout would. Vendoring and forking survive untouched.

The directory is **named literally** because a runner obligation that says
"a conventional directory" is not conformance-testable under #19. No ludo source
file ever contains the string `libs/`; this is runner behaviour, the same species
as ADR-0025 §3's mandated fullscreen affordance.

## 4. The mapping is flat, and §6 forced that

A library has its own dependencies: `physics` contains `use vec-utils`. There is
**one flat program-wide mapping**, not a tree of nested ones.

ADR-0014 §6 already decided this without naming it: *one root name denotes one
library per program* is precisely the statement that there is one table. Nested
mappings would permit two `vec-utils` instances, and #11's nominal,
monomorphised types cannot survive that — it is the unreadable-error case §6 was
written to prevent.

The zero-config consequence, stated plainly: **`libs/` holds the transitive set,
flat.** `physics` and `vec-utils` sit side by side even though the program only
asked for `physics`.

**§8's non-transitivity is untouched.** A file still cannot write `vec_utils.…`
without its own `use`. A name being *resolvable* is not the same as a name being
*visible*, and only visibility was ever at stake there.

## 5. Values are source directories

The mapping's values point at **source directories the compiler parses**. No
pre-compiled artifact, no interface file.

ADR-0020's bootstrap compiler and #22's incremental-analysis guarantee both want
source, and a binary artifact format is an entire distribution surface the map has
ruled out. **Separate compilation is not mandated**; an implementation may cache
compiled output however it likes, and that is invisible to everything here.

## 6. Supplied replaces scanned, never merges

When a build tool supplies a mapping, the runner does **not** also scan `libs/`.
Either the mapping was built by scanning or it was handed over — never both.

A merge would need a precedence rule, and a precedence rule is order-dependence,
which kills obligation 3. This is the cheapest possible way to keep it alive.

## 7. The resolvable names are queryable through the oracle

#4 found agents fail hardest at **naming things that exist**, which makes "how
does anyone learn what names are available" load-bearing rather than a nicety.

The set of resolvable root names **is** the mapping's key set, and the compiler
already holds it, so exposing it through the #22 compiler-as-oracle costs
nothing and introduces no concept. An agent asks the compiler what names exist
rather than inferring from a directory listing — which is what the oracle is for.

## 8. The three lenses, and the #24 delta

- **Simplicity**: a Lua user's `require` searches a path and this does not, which
  is the one thing to unlearn; in exchange `use vector-math` has exactly one
  possible meaning, and dropping a library into `libs/` is the whole workflow.
- **Robustness**: no search means no shadowing, no accidental pickup, no
  environment-dependent build; duplicate claims fail loudly instead of resolving
  arbitrarily; flatness makes #11's nominal identity rule sound.
- **Agent-friendliness**: Tier 1 clean. Resolution is local to a table an agent
  can query (§7), there is no ambient state to reason about, and there is no
  second spelling for a name.

**#24 delta: zero on every count.** No keywords (`use` and `library` were charged
by ADR-0014), no stdlib root names, no source-visible surface whatsoever. This
decision spends nothing, which is itself an argument for it.

## 9. What stays out of scope

**Acquisition** — how a library is fetched, versioned, cached and placed into
`libs/` — remains the follow-on toolchain effort the map already names. Nothing
here fetches anything. The obligations ADR-0014 §12 handed that tool still bind
it; this decision only fixes what the tool must hand back, and what the runner
does when no tool has run at all.
