# Chapter 4 — Modules and libraries

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where this chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under chapter 7's envelope (ADR-0018).

**0.3** This chapter owns **the unit of code and how one unit reaches another**:
the module, the library, visibility, `use` and its scope, name resolution and
the mapping the compiler is handed, the `libs/` default, the standard-library
root, backends as libraries, the `#explicit` module marker, and const
evaluation. [Chapter 1](01-grammar.md) owns the spelling of every form named
here, and [`grammar.ebnf`](grammar.ebnf) remains the authority on syntax.

**0.4** This chapter does not own the **entry file, top-level execution order,
`persist` and reload** (chapter 5), the **C boundary** (#29), the **facade
surfaces** reached through `$.` (chapter 6), the **diagnostic envelope**
(chapter 7), or **conformance** (chapter 8). Where a clause reaches one of them,
it names the routing.

**0.5** Every clause cites its source. A clause with no citation is a defect.

**0.6 Const evaluation lives here.** Chapter 1 §5.6 and §9.5 routed the
const-eval floor to chapter 2, and chapter 2 §15 transcribed the floor because
the typing of a constant position needs it. **ADR-0021 is this chapter's
source** and §12 below is the whole of it; chapter 2 §15 is a restatement of
§12.2–§12.3 for the type-facing reader and is not a second rule. The routing
conflict recorded at [`coverage/02-types.md`](coverage/02-types.md) §3 item 6 is
resolved this way — by ownership, not by deletion — because the positions
requiring a constant are a type question and the evaluable subset is not.

---

## 1. The unit of code

**1.1** A **file is a module** and a module is a file. A **directory is a
namespace node**. A **library is a directory**. (ADR-0014 §1.)

**1.2** There is **no package concept, no second kind of compilation unit, and
no manifest**. A backend module, a third-party library and the second file of a
program are the same kind of thing. (ADR-0014 §1, §2.)

**1.3** A **single-file program contains no `use` line** and is unaffected by
everything in this chapter. (ADR-0014 §2; #3's *a program is a file you run*.)

**1.4** An implementation MUST NOT require any out-of-source declaration of a
dependency. A program names a library **in the file that uses it**. (ADR-0014
§2.)

---

## 2. A library is a directory that claims a name

**2.1** A directory is a library **if and only if** its root carries a
`library <name>` declaration (chapter 1 §4.4). A directory without that line is
a path in the program that contains it. Same files, same layout; the
declaration is the whole difference. (ADR-0014 §3.)

**2.2** The `library` declaration carries **a name and nothing else** — no
version, no hash, no origin. (ADR-0014 §3, §10.)

**2.3** The name a call site writes belongs to the **library**, not to the
consumer. The `use` string is a lookup key; the **bound identifier is the name
the library claims**. (ADR-0014 §3.)

**2.4** A consumer MUST NOT rename, alias or re-export a library's name. There
is no import-alias form (chapter 1 §4.5, §14.9). (ADR-0014 §3; #5 criterion 3.)

**2.5** An implementation MUST NOT derive a library's bound name from the
directory name on disk. (ADR-0014 §3.)

**2.6** **A vendored, edited copy of a library is that library.** An
implementation MUST NOT distinguish a fork from a clean checkout, because
nothing in the source or the declaration carries the distinction. (ADR-0014
§10.)

**2.7** **Editing a library's files is legal and takes effect.** No rule of this
chapter governs file authority; §3 governs name resolution only. An
implementation MUST NOT reject or diagnose an edit to a library's source.
(ADR-0014 §11.)

---

## 3. A path is your code; a name is a library

**3.1** A program reaches **its own files by relative path** and **a library by
its claimed name**. These are the only two reference forms. (ADR-0014 §7.)

**3.2** **A path reference MUST NOT cross a library boundary.** A program cannot
path into a library, and a library cannot path out of itself. Either is a
compile error. (ADR-0014 §7.)

**3.3** **Location is not load-bearing.** A library may sit in a global cache,
in a directory inside the consumer's tree, or anywhere else; it is reached by
name in every case, and no ludo source names its location. Vendoring and caching
change no line of source. (ADR-0014 §7, §12 obligation 4; ADR-0027 §3.)

**3.4 The path form is spelled `use "./<path>"`.** A `use` key that begins with
`./` or `../` is a **path reference** to a file in the referring program; every
other key is a **library name** (§4). The path is relative to the **referring
file's directory**, names the target file including its `.ludo` extension, and
binds the target's file **stem** as the module's identifier. The stem MUST be a
valid identifier (chapter 1 §2.1) or the reference is a compile error. This
spelling is **authored by this chapter** — see §15.1. (ADR-0014 §7, which fixes
the rule and never spells it.)

**3.5** A path reference MUST resolve to a file that exists. There is no search,
no extension inference and no directory-index file: the key names one file and
an absent file is a compile error. (ADR-0027 §1, whose no-search rule is stated
for the mapping and holds a fortiori for a literal path.)

**3.6** An asset path likewise MUST NOT cross a library boundary, so a library
carries its own content. (ADR-0015, which derives this from §3.2; chapter 5 owns
asset declarations.)

---

## 4. `use` is file-scoped and non-transitive

**4.1** **Every file that uses a library says so.** A `use` in one file of a
library or program does not serve any other file, including files below it in
the directory tree. (ADR-0014 §8.)

**4.2** **A program cannot name a dependency's dependencies.** If a program uses
`physics` and `physics` uses `vec-utils`, a file of that program MUST NOT write
`vec_utils.…` without its own `use`. (ADR-0014 §8; #5's re-export ban restated.)

**4.3** A name being **resolvable** is not a name being **visible**. §7.3's flat
program-wide mapping makes every transitively required library resolvable;
§4.2's visibility rule is untouched by that. (ADR-0027 §4.)

**4.4** `use` names a library and binds one identifier (§2.3). There is no
selective-import form, no wildcard form and no alias form. (Chapter 1 §4.5;
ADR-0014 §3.)

---

## 5. Visibility

**5.1** Visibility is a **declaration-site marker**. A declaration in a library
is public or private, marked by its author. (ADR-0014 §5.)

**5.2** **Private is the default**; `pub` marks public (chapter 1 §5.1, §14.1).
A declaration becomes API by an author's act. (ADR-0014 §5; chapter 1 §14.1,
which authored the spelling.)

**5.3** An implementation MUST NOT derive visibility from a file's position in
the directory tree. Moving a file MUST NOT change the API surface. (ADR-0014
§5.)

**5.4** There is **no re-export**. A module MUST NOT make another module's
declaration part of its own surface. (ADR-0014 §5, §8; #5.)

**5.5** A reference from **outside a library** to a declaration that is not
`pub` is a compile error. Whether `pub` also gates a reference between two
modules **inside** one library, or between two files of a plain program, is not
decided by this chapter — see §16 and
[#111](https://github.com/ludo-lang/ludo/issues/111). (ADR-0014 §5, which argues
the marker across the library boundary and states no other; the diagnostic is
chapter 7's.)

---

## 6. One root name is one library per program

**6.1** **Within one program, a root name denotes exactly one library.**
(ADR-0014 §6.)

**6.2** Two libraries claiming the same root name is a **hard error with no
escape hatch**. The consumer cannot rename, alias or re-export around it
(§2.4). (ADR-0014 §3, §6.)

**6.3** The consequence is stated rather than left to a tool: **two libraries
claiming `math` cannot coexist in one program**, and there is **no in-program
version pluralism**. Two dependencies requiring incompatible versions of one
library is a build failure, not a dual instantiation. (ADR-0014 §3, §6.)

**6.4** §6.1 is what makes chapter 2's nominal identity rule sound: two
instances of one library in one program would be two distinct nominal types with
identical spellings. (ADR-0014 §6; #11, nominal types and monomorphised
generics.)

**6.5** The duplicate-claim diagnostic is **two-place** — it names both claiming
declarations. (ADR-0018 §6, which makes this its canonical two-place case.)

---

## 7. Resolution: the compiler is handed a mapping

**7.1** **Compilation takes a name → location mapping as a required input,
complete before compilation begins.** (ADR-0027 §1.)

**7.2** The compiler performs **no search**: no environment variable, no ambient
directory walk, no parent-directory traversal, no network, and no fallback of
any kind. `use "vector-math"` is a **lookup, not a search**; if the key is
absent that is a compile error and there is no second place to try. (ADR-0027
§1.)

**7.3** There is **one flat, program-wide mapping**, not a tree of nested ones.
A library's own dependencies are keys in the same table. (ADR-0027 §4, forced by
§6.1.)

**7.4** A mapping carrying **two entries for one root name MUST be rejected**,
loudly, at the point it is supplied. (ADR-0027 §1, which makes §6.1 checkable.)

**7.5** Resolution is therefore a **total function over a fixed table**:
deterministic and order-independent by construction, not by a tool's good
behaviour. (ADR-0027 §1; #22 guarantee 4.)

**7.6** The mapping's **values are source directories the compiler parses**.
There is no pre-compiled artifact and no interface file. (ADR-0027 §5.)

**7.7** **Separate compilation is not mandated.** An implementation MAY cache
compiled output however it likes; that caching is invisible to every rule in
this chapter. (ADR-0027 §5.)

**7.8** A diagnostic for an unresolvable name MUST NOT suggest a name the
compiler has not confirmed exists. (ADR-0027 §1; ADR-0018's never-guess clause.)

**7.9** **The set of resolvable root names is the mapping's key set**, and an
implementation MUST expose it through the compiler-as-oracle interface. An agent
asks the compiler what names exist rather than inferring them from a directory
listing. (ADR-0027 §7; #22; #4.)

**7.10** The spec fixes **no file format, no CLI flag, no registry, no version
policy and no lockfile**. The mapping is a required input, not a syntax.
(ADR-0027 §1.)

---

## 8. The `libs/` default

**8.1** **A conforming runner, given no mapping, MUST build one by scanning
`libs/` beside the entry file.** (ADR-0027 §3.)

**8.2** Each **immediate subdirectory** of `libs/` is a candidate. The runner
reads that subdirectory's `library <name>` declaration and **that claim is the
key**. One level, **no recursion**. (ADR-0027 §3.)

**8.3** **The folder name is irrelevant.** `libs/vm-fork-2/` claiming
`vector-math` resolves exactly as a clean checkout would. (ADR-0027 §3, which is
what keeps §3.3 intact.)

**8.4** Two subdirectories claiming one name is the loud error of §7.4.
(ADR-0027 §3.)

**8.5** The directory is named **literally `libs/`**, because a runner
obligation reading "a conventional directory" is not conformance-testable.
(ADR-0027 §3; #19.)

**8.6** **No ludo source file ever contains the string `libs/`.** This is runner
behaviour, not language semantics. (ADR-0027 §3.)

**8.7** Under §8.1 **`libs/` holds the transitive set, flat**: `physics` and
`vec-utils` sit side by side even though the program asked only for `physics`.
(ADR-0027 §4.)

**8.8** **Supplied replaces scanned, never merges.** When a mapping is supplied,
the runner MUST NOT also scan `libs/`. A merge would require a precedence rule,
and a precedence rule is order-dependence. (ADR-0027 §6.)

---

## 9. The standard library root `$.`

**9.1** `$.` is **not compiler-internal**. It is ludo files, resolved by the same
code path that resolves any library. (ADR-0014 §4.)

**9.2** `$.` is privileged in exactly two ways: **the root name is reserved**,
and **it is in scope with no `use` line**. Nothing else. (ADR-0014 §4; #5
criterion 3.)

**9.3** **`$.` is always present, never a key in the mapping, and not
shadowable.** A build tool MUST NOT supply, replace or override it. (ADR-0027
§2, which clarifies ADR-0014 §4.)

**9.4** An implementation MUST NOT give `$.` a second name-resolution semantics.
Every name an agent writes resolves under one set of rules. (ADR-0014 §4; #22
guarantee 3.)

**9.5** There is no tier of library that only the standard library may use: any
mechanism `$.graphics` uses is available to a third-party library. (ADR-0014
§4.)

**9.6** `$` is a reserved token, not an identifier, and denotes the standard
library root (chapter 1 §2.5), and the
facades it carries are chapter 6's.

---

## 10. Backends are libraries

**10.1** A **backend is a library claiming a root name**, and target selection
is **which of the mutually exclusive libraries claiming that name is in the
build**. (ADR-0014 §9; ADR-0006 R3.)

**10.2** A desktop and a web backend both claim the name and are **never in one
program**, by §6.1. (ADR-0014 §9.)

**10.3** A backend **satisfies a declared nominal interface**, so a divergent
signature is a **type error** rather than reviewer discipline. (ADR-0014 §9,
amending ADR-0006 R4; #11's explicit nominal satisfaction.)

**10.4** **The nominal interface is necessary and insufficient.** A claimant of
a spec-defined root owes the spec's *behaviour*, not merely its signature; the
type error of §10.3 is not the end of the obligation. (ADR-0019 §1.)

**10.5** A conformance obligation is stated **on one claimant, never on a
pair**. Agreement between two claimants is a consequence of both conforming and
is never a thing anyone checks. (ADR-0019 §1.)

**10.6** Mutually exclusive claimants stay **legal and unblessed** for third
parties. Nothing in this spec implies the language supports interchangeable
implementations. (ADR-0019 §4.)

**10.7** §10.3's compile-time seam does **not** replace the runtime seam — the
struct of non-capturing function pointers the runner re-points at a quiescent
frame boundary. Both stand; a backend module is a module that satisfies the
interface and produces the struct. (ADR-0014 §9; ADR-0006 R2; ADR-0024 §5, which
owns the quiescence predicate that re-point requires.)

---

## 11. `#explicit` and the module boundary

**11.1** **ADR-0029's rule is transcribed in [chapter 1 §11](01-grammar.md),**
which owns it: the marker's one-line-per-file form, the rule that `#explicit`
forbids a spelling that omits a name and nothing else, its closed list of four,
what is explicitly permitted, the per-file check, and the error severity. This
chapter does not restate them. Recorded here because #88 assigns ADR-0029 to
chapter 4 and a reader looking for it in this chapter must find the routing.
(ADR-0029; chapter 1 §11.1–§11.7.)

**11.2** What is **this** chapter's subject is the boundary: `#explicit` is a
property of a **module**, which is a file (§1.1), and a **layer choice never
crosses a library boundary** in any of the three directions — a library cannot
force it on a consumer, cannot forbid it, and cannot observe it (chapter 1
§11.2). Two consequences follow for the module system and are stated here:

**11.3** A library author who wants explicit source marks **their own** modules.
That is a statement about their source and about nobody else's. (ADR-0029 §3.)

**11.4** A library that changes its own layer **can never break a consumer's
build**, because no `#explicit` check reads a file other than the one being
checked (chapter 1 §11.5). (ADR-0029 §3, §5.)

**11.5** `#explicit` is the one attribute in the language that marks a **module**
rather than a declaration; `#align(n)` marks a type, `#vertex` and `#fragment`
mark functions (chapter 1 §12.2). The asymmetry is deliberate. (ADR-0029 §2.)

---

## 12. Const evaluation

**12.1** **Const evaluation folds an expression. It never runs a program.**
(ADR-0021 §1.)

**12.2 The floor.** The evaluable subset is **literals, other constants,
arithmetic, comparison and bitwise operators, and casts**. Nothing else. In
particular there are **no calls** — not even to a function the compiler could
prove pure — **no loops, no recursion, no `if` or `match` selection**, no
allocation, no I/O, and no access to anything the running program would have.
(ADR-0021 §1.)

**12.3** With no calls there is no recursion, so **non-termination is impossible
by construction**. An implementation MUST NOT require a step limit, a fuel
counter or a timeout to evaluate a constant expression. (ADR-0021 §1; #5
criterion 5, discharged by shape.)

**12.4 The non-goal, stated so it is not re-litigated.** ludo has **no macro
system, no comptime blocks, no compile-time code generation, no compile-time
reflection, and no compile-time function execution**. It folds constant
expressions; that is all it does. (ADR-0021 §2.)

**12.5** The **computed module constant is ruled out**, and the language owes a
library none of its three replacements: write the literal, generate the source
file with a script, or build the value at run time in the entry file's top level
and hold it in `persist` (chapter 5). (ADR-0021 §2; #26 call 2.)

**12.6 `const` is a keyword** (chapter 1 §5.6). A `const` binding's initialiser
MUST be const-evaluable under §12.2, and the binding is usable in any
constant-required position. One meaning everywhere. (ADR-0021 §4.)

**12.7** Const-ness is **not inferred from a binding's position**. The entry
file's runtime top-level bindings keep the plain form; a `const` means the same
thing in every file. (ADR-0021 §4; #5 R1.)

**12.8** The constant-required positions are `const` declarations, array
lengths, and generic value arguments (chapter 2 §15.1; chapter 1 §5.6, §9.5,
§10.9). A generic value argument is additionally constrained by chapter 2 to a
`usize` length. (ADR-0045 §1, which reuses this floor verbatim.)

**12.9 Failure is a compile error.** Division by zero, integer overflow, a
negative or over-wide array length, and a lossy cast each **fail the build**, as
a `compile`-producer diagnostic with a source location. None is a fault — there
is no running program to fault — and none wraps or saturates silently.
(ADR-0021 §5; ADR-0018.)

**12.10 The rule that generalises §12.9:** **const evaluation never produces a
value the same expression would not produce at run time, and never produces one
at all when the operation is ill-defined.** (ADR-0021 §5; #51's *float
evaluation exactly as written*.)

---

## 13. What a module's top level may hold

**13.1** **An imported module's top level is declarations only.** No statement
of an imported module runs, ever. (#26 call 2, the firmest call in that ticket;
#22's order-independent resolution; #5 R1.)

**13.2** There is therefore **no module initialisation order** to reason about,
and an implementation MUST NOT introduce one. (#26 call 2.)

**13.3** **A constant declaration is not a load-time side effect.** A library
MAY declare constants, and its consumers MAY use them in constant-required
positions; the compiler folds the declaration and substitutes the value, so
nothing runs when the module is imported. (ADR-0021 §3, which separates
declaration from execution in #26 call 2.)

**13.4** **A module MUST NOT declare `persist`.** A `persist` declaration may
appear only in the entry file. (#26's amendment from #17; chapter 5 owns
`persist` and reload.)

**13.5** A library therefore **cannot own private cross-frame state**. It
exports a type and an initialising function; the consumer declares the state and
passes it. (#26's amendment from #17; #8's explicit-allocator discipline applied
to state.)

**13.6** The entry file's top level, its execution order against frames, and the
diagnostic for a statement in an imported module are **chapter 5's** (#26 calls
1, 3 and 4; chapter 1 §4.3).

---

## 14. What the language hands the build system

**14.1** **Acquisition — how a library is fetched, versioned, cached and placed
— is out of scope**, and nothing in this chapter fetches anything. (ADR-0014
§12; ADR-0027 §9.)

**14.2** A tool that supplies a mapping inherits four obligations, and they
bind it whether or not the tool exists:

1. **The `use` name is the sole input.** No path, no URL and no version at a
   library call site. (ADR-0014 §12.1; §3.4 admits a path only for the
   referring program's own files, never for a library.)
2. **One root name denotes one library per program** (§6.1). The tool makes this
   true or fails loudly. (ADR-0014 §12.2; ADR-0027 §1 makes it checkable at the
   point of supply, §7.4.)
3. **Resolution is deterministic and order-independent** (§7.5). (ADR-0014
   §12.3; #22 guarantee 4.)
4. **The resolved location is never named in source** (§3.3). (ADR-0014 §12.4.)

**14.3** Marking third-party directories for a human reader is **tooling and
convention, not a spec rule**. (ADR-0014 §11, Consequences.)

---

## 15. Spellings authored by this chapter

Recorded under ADR-0044 §6: **a hole is repaired in the spec text, and an ADR is
written only for a reversal.** The entry below is a spelling the corpus required
and never wrote. It reverses nothing, so it takes no ADR.

**15.1 `use "./<path>.ludo"` — the path reference (§3.4).** ADR-0014 §7 fixes
that a program reaches its own files by relative path and a library by its
claimed name, and enforces the boundary between them; chapter 1 §14.9 repeats
the rule. **Neither writes the path form.** The grammar admits it already —
`UseDecl ::= "use" StringLiteral` — so the key's *content* discriminates: a key
beginning `./` or `../` is a path, and every other key is a library name. Three
properties decided the shape:

- **Zero delta against #24.** No keyword, no operator, no production. This is
  what separates it from the two spellings recent chapters filed as tickets
  rather than authored ([#100](https://github.com/ludo-lang/ludo/issues/100),
  [#104](https://github.com/ludo-lang/ludo/issues/104)); both of those moved a
  published count and this moves nothing.
- **The bound name is the file stem**, which is name-from-filesystem — the
  derivation ADR-0014 §3 *rejected* for libraries. It is admissible here for the
  reason §3 gave against it there: a library's directory is renamed by whoever
  checks it out, and a program's own filename is the program author's. The
  rejection is about a third party controlling your identifiers, not about
  filenames as such.
- **The extension is written**, so a key names exactly one file and §3.5 needs
  no search, no inference and no index-file rule.

---

## 16. What this chapter does not decide

- **What a consumer reaches when it writes `<library>.<name>`, and which
  boundary `pub` gates.** ADR-0014 §1 makes a directory a namespace node and §3
  puts the `library` line "at the library root", without saying whether a
  multi-file library's surface is its root file, a flat namespace over the
  directory, or a nested one; and §5's declaration-site marker is argued
  entirely across the library boundary, so §5.5 states the rule only there.
  Whether `pub` also gates a sibling module **inside** one library, or a second
  file of a plain program, is unstated. Filed as
  [#111](https://github.com/ludo-lang/ludo/issues/111). Everything §2, §3, §4
  and §5 do state is unaffected, and #111 may not reverse any of it.
- **The entry file and top-level execution.** #26 calls 1, 3 and 4; chapter 5.
- **`persist`, reload and the reload set.** #17; chapter 5.
- **Asset declarations**, which live in any module (ADR-0015) but whose form and
  loading are chapter 5's. §3.6 records only the boundary rule.
- **The facade surfaces under `$.`** — chapter 6. §9 fixes the root's status and
  nothing about its contents.
- **The diagnostic envelope and the four `#explicit` codes' text.** ADR-0018;
  chapter 7.
- **What a claimant must do to conform**, including the reviewer-facing order.
  ADR-0019; chapter 8.
- **Acquisition, registries, versioning and lockfiles.** §14.1; permanently out
  of scope for the spec (ADR-0027 §9).
- **The C boundary.** #29; a later chapter, with `extern` as the only door below
  `$.` (ADR-0019 §2).
