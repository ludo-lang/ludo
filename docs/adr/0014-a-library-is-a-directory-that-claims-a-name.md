---
status: accepted
---

# A library is a directory that claims a name

ADR-0012 §5 permitted third-party wrapper packages explicitly and declined to
constrain them. That was the fourth decision on this map to lean on packages
existing without the map ever having said what one *is*: issue #5 reserved the
`$.` root *no package may occupy*, issue #3 put **"a program is a file you run"**
— no manifest, no `main`, no module scaffolding — on the must-keep shortlist, and
ADR-0006 R3 refused conditional compilation outright, making **a backend a
module** and target selection a build-time module set.

So the module system already had to express something package-shaped, in a
language whose defining constraint is that a program is one file you run.

This ADR answers the language-level half only. **The build system and the package
manager stay out of scope** — no resolution algorithm, no registry, no lockfile,
no versioning policy. That is the same split the map ran for hot reload
(semantics in, machinery out) and for tooling (language-level preconditions in,
implementations out). What this ADR *does* owe that effort is a contract, and §9
states it.

## Decision

### 1. One mechanism, not two

A **file is a module**. A **directory is a namespace node**. A **library is a
directory**.

There is no separate package concept, no second kind of unit, and no manifest.
A backend module (ADR-0006 R3), a third-party library, and the second file of
your own program are the same kind of thing; what differs is who wrote it and how
you reach it.

A declared `library` unit distinct from a plain module was rejected: it spends
grammar against issue #24's budget rule on a distinction the filesystem already
makes, and it re-introduces the manifest issue #3 killed under another name. Two
mechanisms — modules for intra-program structure, packages for distribution — was
rejected as the two-namespaces shape issue #4's 43.7% naming failure punishes.

### 2. The dependency declaration is in the source

A program names a library **in the file that uses it**:

```
use "vec-utils"
```

There is no manifest. "A program is a file you run" survives untouched — a
single-file program contains no `use` line and is unaffected by everything here.

This follows issue #29, which put `extern "SDL3" fn SDL_Init(...)` in the source
and foreclosed linkage-as-a-build-concern for C. Making ludo libraries the *only*
thing that needs an out-of-file declaration would be incoherent. It also keeps
issue #22's guarantee 4 (deterministic, order-independent module resolution)
readable from the file alone.

The named cost: the library name is a resolution input, and resolution policy —
where `"vec-utils"` is found — is the build system's. §9 fixes what the language
demands of it.

### 3. The library declares its own name

The name a call site writes belongs to the **library**, not to the consumer. It
is declared once, in one line at the library root:

```
library vec_utils
```

The `use` string is a lookup key. The bound identifier is the library's own.

Two libraries claiming the same root name is a **hard error with no escape
hatch**. The consumer cannot rename, alias, or re-export around it — issue #5
forbids all three, and a rename is a second spelling for one entity. This is
exactly issue #29's refusal to let an `extern` declaration rename a C symbol, for
exactly its reason: an agent that knows the library emits the library's name, and
a local alias is issue #4's 43.7% naming failure wearing a different hat.

The cost is real and is accepted: **two libraries claiming `math` cannot coexist
in one program**, and the fix is social — rename upstream — not syntactic.

Deriving the binding from the directory name on disk was rejected: whoever checks
the library out can rename the directory, which is aliasing via the filesystem.
Fully-qualified origin names (`github.com/adamico/vec-utils.normalize`) were
rejected because the origin URL is a *distribution* fact leaking into every call
site.

**This line is what makes a directory a library.** A directory without it is a
path in your program. Same files, same layout; the declaration is the whole
difference. It is ludo source, one line, not metadata — §2's "no manifest" holds.

### 4. The standard library is an ordinary module set

`$.` is not compiler-internal. It is ludo files, resolved by the same code path
that resolves any library, and privileged in exactly two ways: **the root name is
reserved** (issue #5 criterion 3, already decided) and **it is in scope with no
`use` line**.

Nothing else. The map has three precedents for privileged compiler knowledge —
issue #15's derived `Eq`/`Hash`/`Clone`, issue #25's SoA transform, issue #33's
shader emitter recognising math types by name — so a baked-in stdlib was a live
option rather than a formality. It was rejected on three grounds:

- A stdlib the compiler knows structurally is a **second name-resolution
  semantics**, and only one of the two would be specified. Issue #22's guarantee 3
  (name origin local and unambiguous) would then hold for only half the names an
  agent writes.
- It creates a **tier of library nobody outside the project can write**: whatever
  powers `$.graphics` uses would be unavailable to `vec-utils`.
- Issue #4 found **45.47% of agent-hallucinated crates are real stdlib module
  names** — agents already treat stdlib and third-party as one namespace. Under
  one mechanism that confusion is a plain resolution error at a source location.
  Under two, the categories genuinely *are* different kinds of thing and no rule
  fixes it.

The named cost: the bootstrap compiler (ADR-0013) must resolve modules before it
can compile `$.` at all. A baked-in stdlib would have let the bootstrap ship with
`$.` hardcoded. That saving is a bootstrap convenience bought with a permanent
split in the language.

### 5. Visibility is a declaration-site marker

Issue #5 bans re-export. That removes the pattern every other language reaches
for — a root module re-exporting the internals it wants public — so privacy must
be a **marker at each declaration** or it does not exist.

It exists. A declaration in a library is public or private, marked by its author.

No privacy at all was rejected: every internal helper becomes permanent public API
the moment anyone imports it, which is ADR-0012 §5's failure mode (shared
vocabulary escaping the count) at library scale. Directory-position privacy —
root files public, files below private — was rejected because it makes *where a
file sits* semantics-bearing: moving a file silently changes the API surface, and
the compiler can attribute the breakage to nothing, which is issue #5 criterion 5
failing.

This is a new keyword. §10 records it against issue #24's count.

### 6. One root name is one library per program

**Within one program, a root name denotes exactly one library.**

This is a language-level invariant, stated here even though *satisfying* it is
build-system work. It is the one place this ADR reaches toward distribution, and
it has to: issue #11 made types nominal and generics monomorphised, so two
instances of `vec-utils` in one program means two distinct nominal types with
identical spellings, and the resulting error message is unreadable.

Leaving it unstated was the dangerous option — the type system's nominal identity
rule would carry an undocumented dependency on a tool the spec does not own.

The named cost: **no in-program version pluralism, ever.** Two dependencies
wanting incompatible `vec-utils` versions is a build failure, not a silently
resolved dual instantiation.

### 7. A path is your code; a name is a library

A program reaches **its own files by relative path** and **a library by its
claimed name**. Two reference forms, and the compiler enforces the boundary:

**A path reference may not cross a library boundary.** You cannot path into a
library. A library cannot path out of itself.

Provenance is therefore legible at every reference site rather than in a
directory listing. This looks like an issue #5 criterion 3 risk and is not:
criterion 3 forbids two spellings for **one entity**; this is one spelling each
for two genuinely different things, and the distinction it encodes — do I own
this code — is the one the criterion exists to protect.

**Location is not load-bearing.** A library can sit in a global cache, in a
`vendor/` directory inside your tree, or anywhere else. It is still a library,
still reached by name, still opaque to paths. Vendoring and caching are both
legal and neither changes one line of ludo source.

The compiler does not search for a directory. It is **given a mapping** from name
to location by the build system. That shape — a mapping supplied, not discovered —
is what keeps issue #22's deterministic, order-independent resolution true no
matter what the tool does.

### 8. `use` is file-scoped and non-transitive

Every file that uses a library says so. One `use` at a library root does not
serve the files below it.

A program cannot name a dependency's dependencies. If your program uses
`physics`, and `physics` uses `vec-utils`, you cannot write `vec_utils.…`
without your own `use`.

Both are forced rather than chosen. File scope follows from issue #22's guarantee
that one file never needs the whole program: under library scope, reading a file
tells you nothing about where its names come from, which is issue #5 criterion 1
(locality R1) failing at the module layer. Non-transitivity is the re-export ban
restated — if `physics` could hand you `vec-utils`'s names without your asking,
`physics` has re-exported them.

The named cost: every file repeats its `use` lines. This is the trade issue #29
already took by putting `extern "SDL3"` in the file rather than in build config,
and it is what makes an agent's file-local view honest.

### 9. What a backend is, and what it now satisfies

ADR-0006 R3 made a backend a module and target selection a build-time module set.
Under §1 that resolves with nothing new: **a backend is a library claiming a root
name**, and target selection is which of the mutually exclusive libraries
claiming that name is in the build. §6 holds — a desktop and a web backend both
claim the name, and are never in one program.

A backend **satisfies a declared nominal interface** (issue #11: explicit nominal
interface satisfaction, static dispatch only). This is the ADR paying for itself:
ADR-0006 R4 — the unavailable-backend error keeps an identical signature on every
target — stops being a rule a reviewer applies and becomes a type error.

This does **not** replace ADR-0006 R2's struct of non-capturing function
pointers. The two are different seams and both stand:

- **R2's struct is the runtime seam** — the thing issue #17's dylib swap
  re-points at a quiescent frame boundary.
- **The nominal interface is the compile-time seam** — it says a candidate
  backend is shaped right before it is ever loaded.

A backend module is a module that satisfies the interface and produces the
struct.

### 10. A library has no identity beyond its name

The `library` declaration carries a name and nothing else. No version, no hash,
no origin.

A vendored, edited copy of `vec-utils` is a **fork**, it *is* `vec-utils` for that
program, and the compiler cannot tell it from a clean checkout. This is the
honest description of a normal thing to want, not a failure mode.

Carrying a version or origin in the declaration was rejected twice over: the
compiler would have to know about distribution to do anything useful with the
field, which is the boundary this ADR refuses to cross; and a version in the
source is a second fact that must agree with the build tool's fact, which is
precisely the failure issue #5 criterion 5 forbids — a skew the compiler cannot
attribute to a named entity at a source location.

### 11. What the language does *not* decide

**Editing a library's files is legal and takes effect.** Nothing stops it,
nothing warns. The boundary of §7 governs **name resolution**, not file
authority.

Two attempts to give "can a developer see that this directory is third-party" a
language-level answer were made and both were wrong. They are recorded so nobody
retries them:

- *Location carries provenance* — third-party code lives outside your source
  tree. Incoherent: it uses a fact the spec declared out of scope (where the
  build tool puts things) to buy an in-scope property.
- *The boundary prevents accidental edits* — false. A vendored library's files
  are ordinary files on disk and edit normally.

The language's contribution is bounded, and stating the bound is the point: it
decides **reference discipline** — a path is your code, a name is a library, and
the two cannot cross. It does not decide file authority and must not pretend to.
Marking third-party directories is a tooling and convention matter (see
Consequences).

### 12. The contract handed to the build system

Acquisition — how a library is fetched, versioned, cached and placed — is out of
scope, but not unconstrained. Whoever builds that tool inherits four obligations
generated here:

1. **The `use` name is the sole input.** No path, no URL, no version at the call
   site.
2. **One root name denotes one library per program** (§6). The tool makes this
   true or fails loudly.
3. **Resolution is deterministic and order-independent** (issue #22, guarantee 4).
4. **The resolved location is never named in source** (§7).

## Consequences

- The module system gains three keywords — `use`, `library`, and the visibility
  marker — which must be lodged as a delta against issue #24's grammar count,
  the third such delta after ADR-0007's `$.audio` and ADR-0009's `$.graphics`.
- Issue #24's **companion count is otherwise unaffected**, confirming ADR-0012
  §5. The count measures the reserved root — names a package may not occupy and
  every program therefore shares. A library occupies a name outside `$.`, is
  opt-in, and shadows nothing, so no library mechanism reintroduces shared
  vocabulary that escapes the count.
- ADR-0006 R4 moves from reviewer discipline to a compiler-checked interface
  obligation (§9).
- The bootstrap compiler (ADR-0013) must implement module resolution before it
  can compile the standard library at all (§4).
- The spec gains a stated invariant the type system silently depended on: one
  root name, one library, one nominal identity (§6).
- **Acquisition is future work**, constrained by §12's four obligations.
- **Third-party directory legibility is out of scope**: the build tool should put
  libraries in one obvious place and an editor or linter may mark them, but
  neither is a spec rule (§11).
