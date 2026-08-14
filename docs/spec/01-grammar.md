# Chapter 1 — Lexical structure and grammar

## 0. About this chapter

**0.1** This chapter is **normative**. It states the rules a conforming
implementation MUST follow, and nothing else. Rationale lives in the ADR or
issue each clause cites; where a reader wants to know *why*, the citation is the
answer and this chapter does not repeat it. (ADR-0044 §5.)

**0.2** The key words MUST, MUST NOT, SHALL, SHOULD and MAY are used in their
RFC 2119 sense. Where this chapter says a construct is a **compile error**, a
conforming implementation MUST reject the program and MUST report the error
under chapter 7's envelope (ADR-0018).

**0.3** [`grammar.ebnf`](grammar.ebnf) is normative and is the authority on
syntax. This chapter cites its rule names in `Fixed` type. Where prose here
disagrees with a production there, **the production wins and the prose is a
defect** in the prose.

**0.4** This chapter owns **syntax**: what a conforming implementation accepts
as well-formed source, and what tokens the language spends. It does not own the
meaning of any construct. Where a form is spelled here and its semantics live
elsewhere, the clause names the chapter that owns them.

**0.5** Every clause cites its source. A clause with no citation is a defect.

---

## 1. Source text

**1.1** A ludo source file MUST be encoded as UTF-8. An implementation MUST
reject a file that is not well-formed UTF-8. (Follows from ADR-0043 §7, which
fixes string literals as UTF-8 in the runner's constant blob.)

**1.2** Whitespace separates tokens and carries no other meaning. **A newline
does not terminate a statement and indentation is not significant.** Blocks are
delimited by keywords (`Block`, and the `end`-terminated forms throughout
`grammar.ebnf` §1.4). (#7, candidate B: the keyword feel and low punctuation
density are what the winning candidate preserved.)

**1.3** There is no statement separator. Semicolons are not part of the
language.

**1.4** A comment begins with `--` and runs to the end of the line
(`Comment`). **There is no block-comment form.** (#7 candidate B; the
prototype's own source uses only the line form.)

---

## 2. Identifiers, keywords, and reserved tokens

**2.1** An identifier is a letter or `_` followed by letters, digits and `_`
(`Identifier`). Identifiers are case-sensitive.

**2.2** `_` alone is **not** an identifier. It is the **discard token**, legal
in a binding target (`BindingName`), in a pattern (`Pattern`) and as a
standalone expression when discarding a must-use value. Discarding costs
exactly one greppable token, `_ = f()`. (#10, propagation and handling; #15 Q7,
`_` as the one greppable discard.)

**2.3** The keywords of the **core grammar** are reserved in every position and
are exactly (`Keyword`):

```
and  as  break  const  continue  defer  do  else  elseif  end  extern  false
fn  for  frame  if  in  library  match  not  or  persist  pub  rescue  return
then  true  type  unless  unsafe  use  while
```

**2.4** The keywords of the **type sublanguage** are reserved in every position
and are exactly (`TypeKeyword`):

```
distinct  enum  numeric  struct
```

They are listed apart because they are budgeted apart (§13.3, #24 call 3).

**2.5** `$` is a **reserved token**, not an identifier. It denotes the standard
library root and MUST be followed by `.` (`PrimaryExpr`, `Suffix`). A program
MUST NOT declare, rebind or shadow it. (#6's Option C, surviving contact in #7;
ADR-0014 §4 makes the standard library an ordinary module set reached through
this root.)

**2.6** Primitive type names (`bool`, `usize`, the sized integer and float
names, `string`) are **prelude identifiers, not keywords**. They are ordinary
names resolved in the prelude and MAY be shadowed by a declaration, and they do
not count against §13. (ADR-0043 §6 spells `string` as a name; ADR-0045 §5
states `usize` is "an existing type name" and charges zero keywords for it.)

**2.7** `none` and `some` are **prelude identifiers, not keywords**. `?T` is
sugar over an ordinary user-expressible sum type, so its constructors MUST be
nameable by the same rules any other sum type's are. (#9: `?T` is "sugar over a
user-expressible sum type, not a compiler built-in with no user-level
equivalent".)

**2.8** `heap` is a **prelude identifier**. (ADR-0042 §8, which respells
`$.mem.heap` as a bare `heap`.)

---

## 3. Literals

**3.1** Integer literals are decimal, hexadecimal (`0x`) or binary (`0b`), and
MAY contain `_` as a digit separator (`IntegerLiteral`). Float literals are
decimal with a fractional part, an exponent, or both (`FloatLiteral`).

**3.2** A numeric literal's type is defaulted from its context. **Where the
context does not determine it, the program is a compile error naming the
ambiguity — never a silent pick.** (#11, inference: "numeric literals default,
and the ambiguous case is a named compile error, never a silent pick.")

**3.3** Float literals denote the value written. An implementation MUST NOT
reassociate or otherwise re-evaluate a float expression: float evaluation is
exactly as written. (ADR-0016 §7.)

**3.4** A string literal is delimited by `"`, MUST NOT span a line, and admits
the escapes in `Escape` (`StringLiteral`). **There is no character literal and
no raw-string form.**

**3.5** A string literal has type `string`, and `string` is the type of literals
and nothing else. Constructed text is `[]u8`. (ADR-0043 §1, §3.)

**3.6** String literals are emitted NUL-terminated in the runner's constant
blob. The NUL sits outside `len` and is invisible to ludo. **The guarantee
attaches to the literal, not to the type**: no other value carries a
terminator. (ADR-0043 §7.)

**3.7** The boolean literals are `true` and `false` (`BoolLiteral`). **There is
no `nil` literal and no null of any spelling.** (#9.)

**3.8** The aggregate literal is `{...}` (`AggregateLiteral`), and it is the one
literal form for every aggregate. Its items are positional, named (`x = 1`), or
computed-key (`[expr] = v`); the forms MAY be mixed only as chapter 3 permits.
(#15 Q2, Q3, Q13, Q22 — one target-typed literal, `{[expr] = v}` being Lua's own
spelling.)

**3.9** A `{...}` literal MAY be prefixed by a type name
(`NamedType AggregateLiteral`). The prefixed and bare forms produce the same
AST; §11.3 entry 2 requires the prefixed form in a module marked `#explicit`.
(ADR-0029 §4 entry 2.)

**3.10** Chapter 3 owns what a `{...}` literal means, which types it may target,
and the compile error a bare literal with no expected type produces (#15 Q3).
This chapter fixes only its spelling.

---

## 4. Modules and top-level items

**4.1** A file is a module and a module is a file. (ADR-0014 §1.)

**4.2** A module is `ExplicitMarker? TopLevelItem*` (`Module`). A top-level item
is a `library` declaration, a `use` declaration, a declaration, or a statement
(`TopLevelItem`).

**4.3** The grammar admits statements at the top level of any file. **Which
files may carry them is not a syntactic rule**: top-level statements run once,
in the entry file only, and an imported module carries declarations only.
Chapter 5 owns that restriction and the diagnostic for violating it. (#26 calls
1 and 2.)

**4.4** `library <name>` (`LibraryDecl`) declares a library's own root name. It
carries a name and nothing else — no version, no hash, no origin. Chapter 4
owns where it may appear and what it means. (ADR-0014 §3, §10.)

**4.5** `use "<key>"` (`UseDecl`) is the one import form. It takes a string
key and binds the name the target claims for itself; **there is no aliasing
form.** (ADR-0014 §3: "the bound identifier is the library's own", which an
alias would contradict, and #5 criterion 3, which forbids two spellings for one
entity.) Chapter 4 owns resolution, file scope and non-transitivity (ADR-0014
§8, ADR-0027).

**4.6** `use` is file-scoped: every file that uses a library says so.
(ADR-0014 §8.) This chapter records it because it is what makes §4.5's single
form sufficient.

---

## 5. Declarations

**5.1** A declaration MAY carry the visibility marker `pub` (`Visibility`).
Visibility is a **declaration-site marker**; there is no re-export and no
directory-position rule. (ADR-0014 §5.) The spelling `pub` is authored by this
chapter — see §14.1.

**5.2** `type <Name> = <body>` (`TypeDecl`) declares a named type. The body is a
struct, an enum, a distinct type, or a type expression (`TypeBody`). All four
bodies are `end`-terminated where they open a block.

**5.3** A struct body is `struct` followed by field declarations and `end`
(`StructBody`). A field is `name: Type` with an optional default expression
(`FieldDecl`). **Field defaults are permitted**; a literal may omit only
defaulted fields. (#15 Q21.)

**5.4** An enum body is `enum` followed by variant declarations and `end`
(`EnumBody`). A variant MAY carry a positional payload type list
(`VariantDecl`). Sum types are mandatory in this language, so this form is not
optional depth. (#9, which makes them mandatory; #10, which requires them for
error sets; #11 Q10.) The spelling is authored by this chapter — see §14.2.

**5.5** A distinct body is `distinct <Type>` or `distinct numeric <Type>`
(`DistinctBody`). The two are different types with different rules and chapter 2
owns the difference. (#11 Q5–Q7.)

**5.6** `const <NAME> = <expr>` (`ConstDecl`) declares a module constant. `const`
is a keyword rather than an inferred property of a binding's position.
(ADR-0021 §4.) The expression MUST fold under const-eval; chapter 2 owns the
floor (ADR-0021 §1) and the compile error an ill-defined fold produces
(ADR-0021 §5).

**5.7** `fn <name>(<params>) -> <returns>` followed by a block and `end`
(`FnDecl`). **Every function is top-level and named.** There is no anonymous
function literal, no closure and no nested `fn`. (#52 / ADR-0017.)

**5.8** A signature is fully annotated: every parameter carries its type and
every return type is written. **A return type is never inferred.** (#11 Q4.)

**5.9** A parameter MAY carry a default expression (`Param`). (#6's
in-signature defaults; ADR-0029 §4 permits omitting one outside `#explicit` and
entry 3 forbids it inside.)

**5.10** `ReturnType` admits a comma-separated list, which is how fixed-arity
multiple returns are spelled. **The returned group has no type**: return arity
is grammar, there is no tuple type to name, and a multi-return call is legal
only in a destructuring binding or a `return` tail. Chapter 2 owns that
restriction. (#15 Q7.)

**5.11** `frame(<params>)` followed by a block and `end` (`FrameDecl`) declares
the per-frame entry. The entry is a **marked declaration, never a magic name**.
(#12; #26.) The grammar admits a parameter list; **chapter 5 fixes it as exactly
`screen: !Target, scratch: !Scratch`** (ADR-0013 as amended by ADR-0042 §6).

**5.12** `persist <name>: <Type> = <expr>` (`PersistDecl`) declares
reload-surviving state. Chapter 5 owns its scope (entry file only), its
initialisation ordering (cold start only) and its reload semantics. (#26 call 4
and #26's amendment from #17; ADR-0041 §5.)

**5.13** The grammar makes `PersistDecl`'s initialiser optional. **Whether an
initialiser may be omitted is chapter 5's call, not this chapter's**, and the
corpus is in tension: #9 requires every binding to initialise at its
declaration, while ADR-0045 §1 and §8 write `persist name: TextBuf[32]` with no
initialiser. The grammar is permissive so that chapter 5 can decide either way
without a grammar change. Recorded rather than resolved here — see §14.5.

**5.14** `extern` introduces a declaration whose referent lives outside ludo
source (`ExternDecl`). Two shapes:

- **`extern "<library>" fn <name>(...)`** (`ExternFn`) names library and symbol
  verbatim and **MAY NOT rename**. (#29 / ADR-0019.)
- **`extern <class> <name>` or `extern <class> <name> = "<path>"`**
  (`ExternItem`) declares an item the runner resolves. The classes are `image`,
  `font`, `shader` and `storage`. (ADR-0015; ADR-0026; ADR-0008 §8.)

**5.15** The extern classes are **ordinary identifiers in that position, not
reserved words.** This is what makes ADR-0015's, ADR-0026's and ADR-0008 §11's
zero-keyword claims true, and it is why they do not appear in §2.3.

**5.16** `extern shader <name> = "<path>"` is the spelling of ADR-0008 §8's
extern shader declaration. The spelling is authored by this chapter — see
§14.3.

---

## 6. Statements

**6.1** A block is a sequence of statements (`Block`). **A block holds
statements only**: there is no local `type`, `const`, `fn`, `persist` or
`frame`. This is what makes #52's "every function is top-level and named" a
grammatical fact rather than a rule to enforce. (#52 / ADR-0017; #26's
amendment from #17 for `persist`.)

**6.2** There are two binding forms and exactly two (`LetStatement`):

- `x := <expr>` — type inferred from the initialiser.
- `x: T = <expr>` — type annotated.

**One binding form per spelling, and no third.** Candidate C was rejected in
part for offering three (`::`, `:=`, `=`). (#7.)

**6.3** `x := <expr>` MAY bind several names at once (`BindingTarget`), which is
how a multi-return call is destructured. **Destructuring MUST be total**; `_`
discards a component. (#15 Q7.)

**6.4** `<place> = <expr>` (`AssignStatement`) assigns to an existing place.

**6.5** The compound assignments are `+=`, `-=`, `*=`, `/=` and `%=`
(`CompoundAssign`). **Compound assignment is a statement, not an expression** —
`x := (a += 1)` is a parse error — and it is pure sugar over `a = a + b` with a
single AST and no new semantics. **There is no `++` or `--`.** (#7, surface call
2.)

**6.6** `return` MAY carry a comma-separated expression list (`ReturnStatement`),
matching §5.10.

**6.7** **Implicit tail return is in, with one rule: only an expression in tail
position returns.** `if`, `for`, `while` and `match` are statements, so a
function ending in a branch writes `return` in each arm. A checker MUST NOT walk
into a nested block hunting for the live value. (#7, surface call 3.)

**6.7.1** The rule is structural in the grammar, not a side condition on it:
`TailExpr` hangs off `FnDecl` and **not** off `Block`, so a nested block has no
tail position to walk into. (#7, surface call 3.)

**6.8** `break` and `continue` exit and advance the innermost enclosing loop.
**There is no labelled break and no loop label.** (#24, first application of
its payment rule: rejected as semantics-bearing with no failure class left to
name.)

**6.9** `defer <statement>` (`DeferStatement`) registers a statement to run at
scope exit, in LIFO order. It is a registerable statement and MAY appear inside
an `if` or a loop. **There is no `errdefer`** and there are no destructors.
(#8 call 5; #10 Q8, Q17.)

**6.10** `if` / `elseif` / `else` / `end` and `while` / `do` / `end` are as in
`IfStatement` and `WhileStatement`. `for <targets> in <expr> do ... end`
(`ForStatement`) is the one iteration construct over every aggregate. (#15 Q6,
Q19, Q24.)

**6.11** A condition is either an expression or a **binding condition**
`e := <expr>` (`Condition`, `BindingCondition`). The binding condition is its
own production, not a bool coercion: **the check is the bind**. It is legal in
`if` and in `while`, and **it is not legal in `for`**, which already binds from
an iterator. (#9, handling forms.)

**6.12** **Conditions are `bool` only.** There is no truthiness and no
coercion; `if target then` on a `?Entity` is a compile error naming the type.
(#9, truthiness.)

**6.13** A simple statement MAY carry a trailing statement modifier,
`... if <cond>` or `... unless <cond>` (`SimpleStatement`). The modifier binds
into the **enclosing** scope. **There is no block-form `unless` and never an
`else` on a modifier.** (#9, handling forms.)

**6.14** Where a modifier's condition is a binding condition, the modified
statement MUST diverge — `return`, `break`, `continue`, or a call that panics.
A conforming implementation MUST report a compile error naming the
non-diverging statement. With a plain `bool` condition there is nothing to bind
and any modifiable statement is legal. (#9.)

**6.15** `unsafe do ... end` (`UnsafeBlock`) is the escape hatch, and it is
**per-site, loud and greppable**. Chapter 3 owns what it permits. (#8 call 6.)
The `do ... end` delimiting is authored by this chapter — see §14.4.

---

## 7. Expressions

**7.1** The precedence levels, loosest to tightest, are the productions
`RescueExpr`, `OrExpr`, `AndExpr`, `CompareExpr`, `BitOrExpr`, `BitXorExpr`,
`BitAndExpr`, `ShiftExpr`, `RangeExpr`, `AddExpr`, `MulExpr`, `CastExpr`,
`UnaryExpr`, `PostfixExpr`. Every binary level is left-associative. The ordering
follows Lua's where the operators coincide, because recognition is the property
#7 selected candidate B for.

**7.2** The arithmetic operators are `+ - * / %`. **There is no exponentiation
operator and no floor-division operator**; both are stdlib functions where they
exist at all.

**7.3** The comparison operators are `== != < <= > >=`. **`!=` is the
inequality spelling**, not Lua's `~=`, because `~` is the bitwise xor operator
(§7.4) and one glyph MUST NOT denote two entities. (#10's rejection of infix
`!` for exactly this reason, applied to `~`.)

**7.4** The bitwise operators are `& | ~ << >>`, with `~` binary xor and unary
complement. (ADR-0021 §1 lists bitwise operations in the const-eval floor,
which requires them to exist.)

**7.5** The logical operators are the words `and`, `or` and `not`. (#7,
candidate B: Lua's own spellings.)

**7.6** **`or` is also unwrap-or.** Where its left operand is `?T`, `a or b`
yields the contained value or `b`. The word is `or` and not `??`: it is already
Lua's spelling and, with truthiness dead, the left operand's static type selects
one meaning with no ambiguity. (#9, handling forms.)

**7.7** `..<` is the half-open range operator and doubles as the slicing
operator (`RangeExpr`, and `xs[2..<5]` through `Suffix`). **There is no
inclusive range operator**: with 0-based indexing `0..<xs.len` is the
always-correct shape. Ranges are ordinary `Iter[int]` values, not grammar.
(#15 Q24.)

**7.8** Indexing is 0-based. (#15 Q4, the first deliberate break with Lua.)

**7.9** The postfix suffixes are field access `.`, optional chain `?.`, call
`(...)`, index `[...]` and the mutation mark `!` (`Suffix`).

**7.10** **`!` marks mutation at the place**: `pool!.add(e)`, `w!.frame += 1`,
`w.rocks!`. The same glyph marks a mutable binding or parameter in type position
(§9.2). `!Pool[Entity]` and `Pool[Entity]` are the same type, differently
accessed. (#7; #11 Q3.)

**7.11** **`?.` short-circuits**, yielding `?C` for the whole chain, and it works
for UFCS calls as well as field access (`a?.f(b)`). (#9.)

**7.12** `a.f(b)` is **UFCS sugar for `f(a, b)`**. This chapter fixes only that
the spelling exists; chapter 2 owns the resolution rule — the module declaring
the receiver's type, and nowhere else. (#16; #11 Q8.)

**7.13** `x as T` (`CastExpr`) is the cast form. (ADR-0021 §1 lists casts in
the const-eval floor, which requires a spelling.) The spelling is authored by
this chapter — see §14.6.

**7.14** `<expr> rescue <tail>` (`RescueExpr`) is the failure-handling form,
with three tails (`RescueTail`): a diverging statement (`rescue return`), a
default value (`rescue 0`), and a binding handler
(`rescue err then ... end`). `rescue` is purely local with no stack
involvement. (#10, propagation and handling.)

**7.15** `or` handles **absence** and `rescue` handles **failure**. They are not
duplicates: the word at the call site tells a reader which type they are looking
at without consulting the signature. (#10.)

**7.16** **There is no postfix `?` propagation operator** and no `try` prefix.
(#10, rejected spellings.)

**7.17** Propagation is **explicit at every call site**. (#10.)

---

## 8. Patterns and `match`

**8.1** `match <expr> do <arms> end` (`MatchStatement`) is the general handling
form for sum types. Each arm is `<pattern> then <block> end` (`MatchArm`). The
arm spelling is authored by this chapter — see §14.7.

**8.2** The pattern language is: variants, bindings, nested patterns, struct
destructuring and literals (`Pattern`). (#11 Q10.)

**8.3** **There are no guards.** A condition goes in an `if` inside the arm. A
guard makes exhaustiveness undecidable, and the wildcard arm it forces is what
silently swallows the variant added later. (#11 Q10.)

**8.4** **There are no or-patterns and no range patterns.** (#11 Q10.)

**8.5** A bare identifier pattern (`IdentPattern`) is one parse node denoting
either a fresh binding or a nullary variant of the scrutinee's type. **The
distinction is a checking question, not a parsing one**, so #22 guarantee 1 —
parseable without semantic information — holds.

**8.6** Chapter 2 owns exhaustiveness and the compile error a non-exhaustive
`match` produces. (#11 Q10.)

---

## 9. The type sublanguage

**9.1** The type sublanguage is **fenced from the expression grammar** and the
core grammar descends into it at exactly three points — `TypeBody`, `TypeExpr`
and `GenericArgs`. This quarantine is the property #7 rejected candidate A for
breaking, and §13.3 is what keeps it auditable. (#3, types quarantined; #7.)

**9.1.1** The fence is one-way for the type grammar and not for values: a field
default, a parameter default and an array length are expressions. Those are the
only descents back into the core grammar and each is named at its production in
`grammar.ebnf`.

**9.1.2** `StructBody`, `EnumBody` and `DistinctBody` are **type-sublanguage
productions**, reached from the core grammar's `TypeDecl` through `TypeBody`.
They are budgeted on the type side (§13.6), which is what makes §13.3's
quarantine claim true of `struct`, `enum`, `distinct` and `numeric` rather than
only of type expressions.

**9.2** `!T` (`TypeExpr`) marks a mutable place in a declaration or a parameter.
It is not a type constructor: `!T` and `T` are the same type. (#11 Q3.)

**9.3** `?T` (`PrefixType`) is the optional type and the **only** absence
concept in the language. Chapter 2 owns its semantics, including the rule that
`?(?T)` does not auto-flatten. (#9.)

**9.4** `[N]T` is the fixed array, with the length in the type; `[]T` is the
view (`PrefixType`). Chapter 3 owns what each means and where a view may
appear. (#15 Q1, Q10, Q23.)

**9.5** The array length position holds a const expression. It MUST fold under
ADR-0021's floor, and a generic value parameter MAY appear in it **only bare**
(§10.4). Both are static rules over a production the grammar states without
restriction. (ADR-0021 §1; ADR-0045 §1 restriction 2.)

**9.6** `fn(T, U) -> R` (`FnType`) is the function-pointer type. It is
non-capturing, and chapter 2 owns the blessed rule by which it satisfies a
single-function interface. (#52 / ADR-0017.)

**9.7** `T rescue E` (`FallibleType`) is the fallible type; `T rescue` with no
set named elides to the enclosing file's declared error set. **Infix `!` was
rejected** for this position: `!` is already the mutation mark, and one glyph
MUST NOT denote two entities. (#10, representation.)

**9.8** `Name[Args]` (`NamedType`, `GenericArgs`) applies generic arguments.
**The type sublanguage owns `[...]` for this purpose and nothing else in the
expression grammar changed to allow it.** (#7; #15 Q9 — prefix `[` is an array,
postfix `[` after a name is a generic argument list, and the two are
distinguishable without semantic information.)

**9.9** There is **no transparent type synonym**. `type X = <TypeExpr>` declares
a nominal type, not an alias. (#11, nominal newtypes only.)

---

## 10. Generic parameters

**10.1** A generic parameter list is `[...]` after the declared name
(`GenericParams`). Arguments at the use site are positional. (#11; ADR-0045 §3.)

**10.2** A **type parameter** MAY carry a constraint (`TypeParam`,
`ConstraintList`). Generics are monomorphised and constraint-bounded, and a
generic body is type-checked once against its constraints, never per
instantiation. (#11 Q1, Q11.)

**10.3** A **value parameter** is declared `N: usize` (`ValueParam`). `usize` is
the only admitted value type. (ADR-0045 §1 restriction 1.)

**10.4** A value parameter MAY appear **only as an array length, bare, or as an
argument to another generic's integer parameter**. `[N]u8` is legal;
`[N*2]u8` and `[N+1]u8` are compile errors. (ADR-0045 §1 restriction 2.)

**10.5** A value parameter is **not a value in expression position**. It cannot
be read, compared, printed or passed. (ADR-0045 §1 restriction 3.)

**10.6** The annotation is **mandatory at the declaration and absent at the use
site**: `TextBuf[N: usize]` declares, `TextBuf[32]` uses. There is no inference
of a parameter's kind from how the body uses it. (ADR-0045 §3.)

**10.7** For a generic **function**, a value parameter is inferred at the call
site from the argument's type and is never written. (ADR-0045 §4.)

**10.8** **Value parameters are not constraint-bounded** — a narrow, stated
exception to §10.2. A value parameter promises nothing, so its `usize`
annotation is its whole bound. (ADR-0045 §4.)

**10.9** An argument that is not const-evaluable, is not a `usize`, or that
overflows is a **compile error**, never a wrapped value and never a chapter 5
fault. `N = 0` is legal and has no special case. (ADR-0045 §7.)

---

## 11. `#explicit`

**11.1** `#explicit` is **one line at the top of a file** (`ExplicitMarker`) and
marks a module. There is no declaration-level form, no directory- or
library-level form, and no compiler flag. (ADR-0029 §2.)

**11.2** A layer choice **never crosses a library boundary** in any direction: a
library cannot force `#explicit` on its consumers, cannot forbid it, and cannot
see it. There is no `#explicit`-requiring signature and no query. (ADR-0029 §3.)

**11.3** **`#explicit` forbids a spelling that omits a name. It forbids nothing
else.** The list is **closed**; a future ticket adds to it only by amending
ADR-0029. The four rejected spellings are:

1. **A binding without its type.** `x := ...` MUST be written `x: T = ...`.
2. **An aggregate literal without its type name.** `{...}` MUST be written
   `SpriteDesc{...}` (§3.9).
3. **A call that omits a defaulted argument.** The argument MUST be written at
   the call site.
4. **A UFCS call.** `a.f(b)` MUST be written as the qualified call.

(ADR-0029 §4.)

**11.4** Explicitly permitted in an `#explicit` module, recorded so it is not
re-litigated: **descriptor field defaults**, **`+=`**, **the implicit tail
return**, and **`?T`**. None omits a name. (ADR-0029 §4.)

**11.5** **No `#explicit` check needs a whole-program view.** Every edge —
a generic declared elsewhere, an in-signature default declared elsewhere, a
UFCS call resolving into another module — resolves at the **call site**, so the
checker stays per-file. (ADR-0029 §5.)

**11.6** A violation is an **error**, under four spec-assigned codes with
severity `error`. There is no way to silence one, and the word "lint" does not
appear in this spec. (ADR-0029 §6; ADR-0023 §1, §4.)

**11.7** `#explicit` changes **zero semantics**. The set of programs that
compile depends on the program's own source, never on a flag or a vendor.
(ADR-0029 §6.)

---

## 12. Attributes

**12.1** An attribute is `#name` or `#name(args)` (`Attribute`). The mechanism
is one production and the attribute names cost nothing against §13.
(ADR-0008 §4; ADR-0029 §8.)

**12.2** The attribute set is closed and is exactly:

| Attribute | Marks | Owner |
|---|---|---|
| `#explicit` | a module | §11, ADR-0029 |
| `#align(n)` | a type | chapter 3, ADR-0025 as corrected by ADR-0029 §7 |
| `#vertex` | a function | chapter 6, ADR-0008 §7 |
| `#fragment` | a function | chapter 6, ADR-0008 §7 |

**12.3** There is **no field-level `#align`**. (ADR-0024 §6, as clarified by
ADR-0029 §7: the ADR declined the field-level form permanently and the
type-level form ships.)

---

## 13. The grammar budget

**13.1** The spec binds a grammar budget **as a process plus a published,
reproducible count, never as a hard cap**. (#24 call 1.)

**13.2** The binding unit is **keywords plus operators**. A production count
rides along as the machine-checkable companion, published with an explicit note
that it is sensitive to how the grammar is factored. (#24 call 2.)

**13.3** **Core grammar and type sublanguage are counted separately.** A single
merged number would hide exactly the leak candidate A was rejected for — type
parameters borrowing the expression grammar — so the fence is made visible as a
number rather than left to judgement. (#24 call 3.)

**13.4** **`#explicit` is in neither count**, and this section asserts that as a
checkable property. (#24 call 3; ADR-0029 §8.)

**13.5 Counting rules.** The count is computed from
[`grammar.ebnf`](grammar.ebnf), which is what makes it reproducible by anyone.
(#24 call 7.)

1. The count is over `grammar.ebnf` **§1 and §2 only**. §3 is lexical
   structure, and its terminals (`0x`, `\u{`, quotes, digit ranges) spell the
   shape of a token rather than being tokens.
2. A **keyword** is a member of `Keyword` or `TypeKeyword`.
3. An **operator** is any other quoted terminal, counted once per distinct
   spelling. Opening and closing delimiters count separately, matching the Lua
   reference manual's own "other tokens" list, so §13.7 compares like with like.
4. A terminal appearing in **both** sections is counted once, in the section
   that introduces it, and noted in the other.
5. Attribute names are **not** counted, and this includes `ExplicitMarker`'s
   `#explicit` terminal — the sole mechanism cost is `#` (§13.4, ADR-0029 §8).
6. Primitive type names and prelude identifiers are **not** counted: they are
   identifiers, not tokens. This covers `ValueParam`'s `usize` terminal, which
   the grammar quotes only because ADR-0045 §1 admits no other value type.
7. Extern class words (`image`, `font`, `shader`, `storage`) are **not**
   counted: §5.15 makes them identifiers.

**13.6 The counts.**

| Section | Keywords | Operators | Total |
|---|---:|---:|---:|
| Core grammar | 32 | 39 | **71** |
| Type sublanguage | 4 | 1 | **5** |

Core keywords are §2.3's list. Core operators are:

```
+  -  *  /  %                        (5)
+=  -=  *=  /=  %=                   (5)
==  !=  <  <=  >  >=                 (6)
&  |  ~  <<  >>                      (5)
=  :=  !  ?.  ..<                    (5)
.  ,  :  ->                          (4)
(  )  {  }  [  ]                     (6)
_  $  #                              (3)
```

Type-sublanguage keywords are §2.4's list. Its one uncounted-elsewhere operator
is `?`, the optional marker. `!`, `[`, `]`, `:`, `,`, `->` and `.` appear in
type position and are counted in the core section under rule 3.

**13.7 The comparison table.** The target is **within ~30% of Lua's core
grammar**, stated as a comparison and never as a cap. (#24 call 4.)

| Language | Keywords | Other tokens | Total | vs. Lua |
|---|---:|---:|---:|---:|
| Lua 5.4 | 22 | 33 | **55** | — |
| **ludo, core** | 32 | 39 | **71** | **+29.1%** |
| ludo, type sublanguage | 4 | 1 | 5 | — |
| Odin | — | — | — | not yet counted |
| Go | — | — | — | not yet counted |

Lua's figures are its reference manual §3.1: 22 keywords and 33 other tokens.
**ludo's core grammar is inside the ~30% target.** The type sublanguage is
budgeted apart and has no Lua counterpart to compare against.

**The Odin and Go rows are a stated gap.** #24 call 4 asks for all three in one
table; counting two more languages to §13.5's rules is work this chapter did not
do, and the rows are left empty rather than filled with a number nobody could
reproduce. The target is defined against Lua alone, so the gap does not block
it. Filling them is a task, not a decision.

**13.8 The payment rule, tiered.** (#24 call 5.)

1. **Sugar is strict one-in-one-out**: a new sugar MUST delete an existing one.
2. **A semantics-bearing production** need only name, permanently and in this
   spec, the failure class it deletes.
3. The tier line is machine-checkable and is not a judgement call: **anything
   `#explicit` rejects is sugar** (§11.3).

**13.9** Exceeding the target requires a **recorded overrule on the map**, not a
silent increment. (#24 call 5's third consequence.)

**13.10** On an implementation the budget is not a budget at all but **no vendor
syntax extensions**, which chapter 8 carries as a conformance property rather
than as a second mechanism here. (#24 call 6; #19 P9.)

**13.11** The **stdlib root-name companion count** is reported with no target,
because relocating a feature into the stdlib satisfies a grammar budget by pure
relocation and must therefore show up. It is a stdlib figure, not a grammar
figure, and **chapter 8 publishes it**. (#24 call 5's second consequence.)

**13.12 Production count**, the machine-checkable companion required by §13.2:

| Section | Productions |
|---|---:|
| `grammar.ebnf` §1, core grammar | 71 |
| `grammar.ebnf` §2, type sublanguage | 21 |
| `grammar.ebnf` §3, lexical structure | 20 |

**This number is sensitive to how the grammar is factored** and is published
with that note so that nobody games it by inlining rules. The binding unit
remains §13.2's.

---

## 14. Spellings authored by this chapter

Recorded under ADR-0044 §6: **a hole is repaired in the spec text, and an ADR is
written only for a reversal.** Each entry below is a spelling the corpus
required but never wrote. None reverses a decision, so none takes an ADR.

**14.1 `pub`.** ADR-0014 §5 fixes that visibility is a declaration-site marker
and says "this is a new keyword" without naming it. The word is `pub`, on #4's
naming-failure grounds: it is the highest-corpus spelling for exactly this
marker. **Private is the default**; `pub` marks public, so a declaration becomes
API only by an author's act.

**14.2 `type X = enum ... end`.** #9, #10 and #11 make sum types mandatory and
name their variants (`Some`, `None`, error sets) without ever spelling the
declaration. The form mirrors §5.3's struct body exactly — same `type` header,
same `end` termination — so it costs one keyword and no new shape.

**14.3 `extern shader <name> = "<path>"`.** ADR-0008 §8 fixes that a `.wgsl`
file is named by an extern shader declaration "the identical shape to #29's
`extern "SDL3" fn`" and never writes it. The form mirrors ADR-0015's
`extern image player = "art/player.png"`, which is the shape that names a file.

**14.4 `unsafe do ... end`.** #8 call 6 fixes `unsafe` as per-site, loud and
greppable, and never delimits it. `do ... end` reuses the block delimiters
`while` and `for` already spend, costing no new token.

**14.5 `persist` without an initialiser — recorded, not resolved.** #9 requires
every binding to initialise at its declaration; ADR-0045 §1 and §8 write
`persist name: TextBuf[32]` with no initialiser. The grammar is permissive
(§5.13) and **chapter 5 MUST resolve this**, either by requiring `= {}` at that
declaration or by stating the exemption. Flagged so that chapter 5 finds it
rather than inheriting it silently.

**14.6 `x as T` for casts.** ADR-0021 §1 puts casts in the const-eval floor,
which requires a spelling, and none was written. `as` also disambiguates nothing
else: §4.5 declines the import-alias form, so the keyword denotes exactly one
entity.

**14.7 `<pattern> then <block> end` match arms.** #11 Q10 fixes the pattern
language and never spells an arm. The `then`/`end` shape is the one every other
branching form in the language already uses, and an explicit arm terminator is
what keeps the grammar parseable with bounded lookahead for the ADR-0020
bootstrap.

**14.8 `!=` rather than Lua's `~=`.** #7 preserved Lua's look, but ADR-0021 §1
requires bitwise operators, and Lua spends `~` on both xor and inequality. One
glyph MUST NOT denote two entities (#10's ground for rejecting infix `!`), so
`~` keeps the bitwise job and inequality takes the corpus-majority spelling.

**14.9 No import alias.** #26's amendment shows `use "world" as World`. That
predates ADR-0014 §3 (the bound identifier is the library's own) and ADR-0014 §7
(own files by relative path, libraries by claimed name). §4.5 admits the single
form; the older example is stale prose.

**14.10 `type TextBuf[N: usize] = struct ... end`.** ADR-0045 §1 renders the
declaration inline as `TextBuf[N: usize] = struct { bytes: [N]u8, len: usize }`.
That is an informal one-line rendering, not a second declaration form: the
normative spelling is §5.2's, with the `type` header and the `end` termination
every other type declaration carries. The decision ADR-0045 makes — the
parameter, its annotation, and the three restrictions — is unaffected.

---

## 15. What this chapter does not decide

Recorded so the boundary is legible, and so a later chapter is not read as
having inherited a silence:

- **Meaning of every construct spelled here.** Chapters 2–7 own it, per the
  clause-by-clause pointers above.
- **The entry's parameter list.** §5.11 admits a list; chapter 5 fixes it.
- **`persist` without an initialiser.** §5.13 and §14.5; chapter 5's call.
- **The stdlib root-name companion count.** §13.11; chapter 8 publishes it.
- **Vendor-extension conformance.** §13.10; chapter 8 tests it.
