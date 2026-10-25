# Parser and tree

The per-file API is in `include/ludo_frontend.h`; concrete nodes and the accessor
hedge fixed by [The frontend seam has no API, only a boundary](https://github.com/ludo-lang/ludo/issues/130)
are in `include/ludo_ast.h`. Consumers use `ludo_ast_get`/`ludo_ast_child`, rather
than keeping pointers into a parser builder. The keeper can replace the concrete
representation there. This is a syntax tree: no type facts or resolved names
are written onto it.

`ludo_session_create` owns one arena of file records and a separate arena for
each file's tokens, nodes and diagnostics. `ludo_parse_file` borrows source and
display-name buffers until session destruction; it does not read paths or copy
source text. Published files are immutable and remain stable when another file
is added. Source ids are session-minted. A node id is dense **within one file**;
side tables for types and execution use `(file id, node id)`. Id 0 is absent.
No cross-file pointer is embedded in a tree. Module mappings/checking arrive
with their consumers; this API is the per-file layer underneath them.

Nodes have an ordered child chain and one parent. The span is `(file, byte
offset, byte length)` and includes syntactic delimiters, but not outer trivia.
The module covers the whole buffer. Error nodes may point at the token a
missing delimiter was expected before; on recovered trees they need not fit
inside their parent's consumed range. The `token` index records the name,
literal or operator; otherwise it is a syntactic anchor, not an extra child.
All tokens, including comments, whitespace, lex errors and EOF, are retained.

## Child roles

Absent optional clauses have no child. Their kind distinguishes them, so a
consumer does not infer optional positions from a child count. Repeated children
keep source order. Attributes precede a declaration's other children.

| Node | Children |
|---|---|
| module | explicit marker, then top-level items; lex error nodes appended |
| type-decl | attributes, generic-params if present, body |
| const / persist | type if annotated, initializer if present |
| fn | attributes, generic-params if present, params, returns if present, block; interface signatures omit block |
| extern-fn | library string, params, returns if present |
| extern-item | class identifier, path literal if present |
| frame | params, block |
| impl | named-type, constraints |
| generic-params / constraints / params / returns / generic-args | ordered members |
| generic-param | constraints if present; `usize` remains the annotation deciding value versus type parameter |
| param / field | type, default expression if present |
| struct / enum / interface | fields / variants / function signatures |
| variant | payload types |
| distinct | underlying type; `NUMERIC` flag records `numeric` |
| named-type | root if present, qualified-name components, generic-args if present |
| optional-type / pointer-type / view-type / mutable-type | inner type |
| array-type | length expression, element type |
| fallible-type | success type, named error set if present |
| fn-type | params containing types, result type if present |
| block | statements; a function's final expression has a tail wrapper |
| let | bindings, annotated type if present, initializer |
| bindings | identifiers or discard nodes |
| assign / compound-assign | left place, right expression |
| expr-stmt / tail / defer | expression / expression / modifiable statement |
| return | returned expressions; a rescue propagation return has none |
| modifier | modified statement, condition; token distinguishes if/unless |
| if | branches; a branch has condition + block, or just block for else |
| while / for | condition + block / bindings + iterable + block |
| bind-condition | expression; token is the bound name |
| unsafe | block |
| match / arm | scrutinee + arms / pattern + block |
| pattern-variant / pattern-struct / pattern-field | payload patterns / field patterns / pattern |
| unary / cast / binary | operand / operand + type / left + right; token records operator |
| member / optional-member | receiver; token records member name |
| call | callee, then arguments |
| bracket | receiver, generic-args list; **not yet classified as indexing or generic instantiation** |
| mark / deref / group | operand |
| aggregate | optional prefix expression (`PREFIXED` flag), then items |
| named-item / keyed-item | value (token names field) / key + value |
| rescue / handler | fallible expression + tail / handler block (token names error binding) |

Names, literals, break/continue, markers, root and discard have no children.
`IdentPattern` deliberately stays unresolved between binding and nullary variant.
A bracket argument that can be both a name/type and an expression retains the
expression shape; type checking interprets it in the role the receiver requires.
A bare `T rescue` argument retains a fallible-type node over that neutral name
shape, rather than inventing a missing expression handler.
The unresolved boundary between an elided fallible result type and a first
identifier-led body expression is tracked by
[Where an elided fallible result type ends before the function body](https://github.com/ludo-lang/ludo/issues/152).
The current return-type-only heuristic is provisional, not a typechecking rule.

`ExplicitMarker` is the touching pair `#` `explicit` (ch1 §11.1.1); a spaced
`# explicit` becomes one error node and is not read as an attribute.

A named aggregate retains its prefix for `#explicit`, with the same aggregate
node kind as a bare literal. Trailing calls become function tails; frame and
nested statement blocks never acquire tail position.

## Recovery, resources and diagnostics

Missing required tokens insert explicit error nodes. A container's recovery
loop consumes a token when its child made no progress; block terminators remain
with their owner. A declaration inside a block is also a boundary for a missing
`end`, so a bad function does not swallow the next declaration. Lex errors,
including ones inside trivia, produce root error nodes and diagnostics.

`OK` means a file was published, **not** that it is well formed. Program errors
travel exclusively through diagnostics and error nodes. Allocation failure or
the bounded native parse stack's exhaustion publishes no partial file and
returns a host status. The stack budget is 256 active recursive parser entries,
not a language nesting rule; `RESOURCE_LIMIT` is distinct from syntax failure.
This adds an explicit host resource limit to the seam decision's allocation
failure rather than pretending deeply nested valid source is malformed.

Ch7 §5.7 leaves the spec's own codes to the follow-on toolchain effort, and
§5.6.4 forbids a vendor to emit a `ludo:` code meanwhile. The prototype is a
vendor, so its codes live under the owner `bootstrap`. They are provisional and
give way to spec-assigned codes when those exist:

| Code | Meaning |
|---|---|
| `bootstrap:parse-syntax` | input does not match a required grammar shape |
| `bootstrap:lex-invalid-utf8` | ill-formed UTF-8 |
| `bootstrap:lex-unexpected-character` | character begins no token |
| `bootstrap:lex-unterminated-string` | string reaches newline or EOF |
| `bootstrap:lex-bad-escape` | unsupported escape or invalid scalar escape |
| `bootstrap:lex-malformed-number` | numeric token has invalid digits or glued-on text |

The checker's codes (#143) follow the same rule, under `bootstrap:check-`:
`ambiguous-literal`, `ambiguous-none`, `literal-range`, `mismatch`,
`operator`, `condition`, `arity`, `unknown-name`, `unknown-type`,
`unknown-variant`, `not-a-type`, `not-a-value`, `not-callable`,
`generic-value`, `uninferred`, `optional-as-value`, `immutable`,
`unmarked-mutation`, `not-a-place`, `non-exhaustive`, `missing-return`,
`not-constant`, `extern-class`, `duplicate` and `no-loop`. Each message ends in
the name it is about. `src/frontend/tests/test_check.c` is the reject side for
each rule band 1 introduced.

`ludo_diagnostic_json` writes one version-1 compile/error envelope to the
caller's buffer, with no newline, no stream, no invented entity and no guessed
suggestion. `ludo ast FILE` sends that NDJSON stream to stderr and the tree to
stdout; exit 0 means syntactically valid, 1 means source diagnostics, 2 means a
host failure. The tree dump omits positions and ids so whitespace edits do not
churn it. `make ast` regenerates the committed reference dump and `make check`
diffs it alongside tokens. This does not typecheck or execute the tree.

## Grammar coverage

| EBNF region | Implementation | Regression cases |
|---|---|---|
| Module, declarations, attributes, signatures | module, declaration, attributes, signature | test_surface; declarations corpus; reference and its three companion files |
| Statements and conditions | statement, simple_statement, block, condition | test_surface/test_shape; control corpus; modifier/compound-if boundary |
| Match and patterns | pattern, match branch in statement | literal, variant, struct, discard, nested arms; empty variant/struct pattern rejection |
| Every expression precedence level and postfix form | binary, unary, postfix, expression, aggregate | precedence/associativity shape assertions; neutral brackets; rescue propagation/handler/default |
| Type bodies and expressions | type_body, type_expr, prefix_type, named_type, generic_args | all five bodies; function, pointer, optional, view, array, mutable and fallible types |
| Generic constraints/value parameters | generic_params, constraints | constrained/unconstrained/value parameters, interface/impl, multiple/type-only bracket arguments |
| Malformed input | expect, error, progress; lexer-error scan | test_recovery and parser corpus; preserved later declarations |
| Arbitrary bytes and arena growth | fuzz_parser | tree ownership, bounds, diagnostics/error-node agreement; corpus replay in every check |

`make fuzz-parser` searches with libFuzzer. Kept findings belong in
`tests/corpus/parser/`; nightly searches lexer and parser independently. A
matching reference snapshot proves parsing and shape, not type correctness or
construct execution, which remain the following band tickets' work.
