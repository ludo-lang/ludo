---
status: accepted
---

# A string is a literal, and constructed text is bytes

Issue #81 was re-scoped after [#80](https://github.com/adamico/ludo/issues/80) / ADR-0042 on
the observation that strings were not wholly unspecified: [#15](https://github.com/adamico/ludo/issues/15)
Q11 had already fixed them as **immutable UTF-8, not an indexable aggregate**, and
[#17](https://github.com/adamico/ludo/issues/17) §4 had already fixed **program constants as
reload-stable**. What was left was ownership, construction and spelling — and the ownership
half turned out to be a live contradiction between those same two decisions.

## 1. The contradiction the ticket resolves

- **#17 §4** treats a string as a **view into a runner-held blob**, which is what makes
  `persist name = "boss"` survive a reload.
- **#15 Q10** fixes `[]T` as a **transient non-escaping view**: parameter position only, never
  stored in a struct, never returned.

A stored view is exactly what #15 forbids, so a single view-shaped `string` cannot both be
#15-legal and satisfy #17 §4. Three shapes were weighed: waive #15's rule for strings, ship a
second owned `String` carrying an allocator, or narrow what a `string` may point at.

**The resolution narrows the type: `string` is a view into the runner's constant blob, and
nothing else.** Its escape permission is then *derived rather than granted* — a `string` may
be stored, `persist`ed and used as a `Key` because its referent provably outlives every ludo
frame, reload and image swap. No lifetimes, no second owned type, no borrow checker, and no
carve-out in #15.

**Rejected: an owned `String` carrying an allocator.** It is a second concept in a beginner's
first line of text handling, and it puts `clone`-shaped ownership where Lua has none.

**Rejected: waiving #15's rule for a single view-shaped `string`.** Constructed strings would
inherit an escape permission that only literals earn, and nothing in the type distinguishes
the two — the failure is a dangling view with no diagnostic, which is criterion 5's
prohibition.

## 2. The widening is one-way, and that is the whole design

A `string` widens implicitly to `[]u8` at parameter position. **There is no conversion in the
other direction and there cannot be one**, because a `string`'s meaning *is* the memory it
points at. No runtime bytes can become a `string`.

Text therefore flows one way — literals into byte-land, never back — and the consequences are
stated rather than discovered:

- **No runtime-constructed `Map[string, V]` key.** #15 admits strings to `Key`; that admission
  now covers literal keys only.
- **No `persist` field typed `string` holding anything a player typed.** Player text is stored
  as bytes (§5).
- **`string` is a slightly misleading name for what this is**, and the name is kept anyway
  (§3).

The payoff is that a text-entry program never mentions the type at all: `$.input.text()` is
`[]u8`, a stored name is `[]u8`, `TextDesc.text` is `[]u8`, and the player's bytes reach the
screen with no conversion step anywhere in the path.

## 3. Spelling: `string`, and #17 §4 is stamped

The corpus split, and §1's narrowing moved the count. ADR-0009 and ADR-0010's
`TextDesc { text: string }` and ADR-0011's `$.input.text() -> string` all describe **non-literal**
text, so they widen to `[]u8` (§11) and stop being sites for this name. The one surviving
genuine use — a literal bound to a name — is #17 §4's `persist name: str = "boss"`, which
spells it **`str`**. The majority spelling lost its sites; the minority spelling owned the one
that remained.

**`string` wins on recognisability, not on edit cost** — the site count is one either way.
[#4](https://github.com/adamico/ludo/issues/4) found that agents fail at *naming things that
exist* (43.7%), and `string` is overwhelmingly the token the pretraining corpus emits, while
`str` reads as an abbreviation to a beginner.

**The counter-argument is recorded rather than dismissed:** `str` is honest about being a view
rather than a container, which is exactly what §1 made it. It loses because a beginner writing
`name: string = "boss"` never needs to know, and an agent reaching for `string` finds it.

## 4. One checked `format`, and no concatenation

**`format(dst: ![]u8, "Score: {}", n) -> []u8`** writes into the buffer it is handed and
returns a view of exactly what it wrote.

The set was arrived at by elimination. A `format_int` / `format_float` pair was proposed
first, and `concat` cut on ADR-0017's rule that *a higher-order stdlib function must
contribute an algorithm, never just a loop shape*. **That cut was wrong and is recorded
because it was instructive**: with no way to place a literal into the buffer, `"Score: " + n`
became unbuildable, and the most common HUD line in games had no spelling. `format` restores
it by subsuming all three.

**It is not a macro.** ADR-0021 makes macros a stated non-goal. This is a stdlib call with a
compiler-known signature, the same narrow named exception #15 used for `Eq`/`Hash`/`Clone`:
the format string is a **literal the compiler checks**, so hole count and argument type
mismatches are **compile errors** rather than garbage bytes at runtime. That is the robustness
lens, and it is what decides between this and a primitive set.

**The hole grammar is `{}` and nothing else.** No width, no precision, no alignment — those
are a text library's job on ADR-0005 §180's line.

**Hole types are a closed, spec-owned list: integers, floats, `bool`, `char`, `string`,
`[]u8`.** No reflection exists (ADR-0021), so this is a list rather than an interface a user
type can satisfy. **The math types are deliberately excluded**: `Vec2` has no single obvious
text form — `(1, 2)` versus `1, 2` versus `[1 2]` is a house style the spec would be minting
arbitrarily — and a caller writes `format(buf, "({}, {})", v.x, v.y)` at no loss.

**Overflow truncates at a scalar boundary and returns what was written.** Never a
[#18](https://github.com/adamico/ludo/issues/18) fault and never a fallible return, on the same
reasoning as §8: the bytes are usually the player's, and a player must not be able to fault a
shipped game by holding a key down. Truncating at a scalar boundary rather than mid-sequence
keeps the destination valid UTF-8, so it never draws a stray U+FFFD at the end of a name.

**Slicing is free.** #15 already gives `s[2..<5]` on views, and §2's widening makes it apply to
`string` with no new surface.

**Rejected: a `Scratch`-destination form.** `format(scratch!, ...)` passes §9's rule cleanly,
but a local `[32]u8` is a stack value that costs no allocator and covers the transient case
already — so the second spelling buys only a saved declaration line, against
[#5](https://github.com/adamico/ludo/issues/5)'s *one spelling per entity*, a Tier 1 criterion.
Keeping `format` allocator-free also keeps it usable in **core headless conformance**, where no
entry and no `Scratch` exist.

## 5. `TextBuf[N]`, because appending is not formatting

The buffer-and-cursor idiom was written out in full during the grilling and rejected on sight:

```ludo
written := format(name![name_len..<32], "{}", typed)
name_len += written.len
```

**One call was doing two jobs.** `format(label_buf!, "Score: {}", score)` reads fine; the line
above does not, because the reader must decode four things at once — slice the tail, format
into it, recover what was written, hand-advance a cursor. That is `snprintf` plus offset
arithmetic, and it was the one line in the whole text story a Lua user would stop at.

**A fixed-capacity text buffer ships:**

```
TextBuf[N] = struct { bytes: [N]u8, len: usize }

buf!.append(text: []u8)
buf!.format("...", ...)
buf.view() -> []u8
buf!.clear()
```

It is a plain value with no pointer, so it lives in `persist` and survives reload by the same
rule as a bare `[N]u8`. It deletes the manual cursor, the truncation bookkeeping, and the
unspecified `!`-on-a-sliced-place spelling that the manual version needed (§10). UFCS
([#16](https://github.com/adamico/ludo/issues/16)) makes `append` a free function, so no method
system is involved.

**`append` returns nothing and truncates silently.** A returned-but-unread view would fire
[#8](https://github.com/adamico/ludo/issues/8)'s must-use at every call site on the
most-written text function. Silent truncation is also the correct behaviour for a
capacity-bounded name field: a player holding a key down at 32 characters should see nothing
happen, not handle an error. `len` and remaining capacity are readable for a caller who cares,
and `format`'s own return-what-was-written rule (§4) survives underneath.

**`format` is kept as the primitive.** One-shot formatting into a bare buffer, and core
headless conformance, both work with no builder in sight.

The whole text-entry program is then:

```ludo
persist name:  TextBuf[32]
persist score: i64 = 0

fn draw(screen: !Target, scratch: !Scratch)
  name!.append($.input.text())

  $.graphics.fill_text(screen!, {
    text      = name.view(),
    font      = $.graphics.default_font,
    font_size = 24.0,
    position  = { x = 40.0, y = 40.0 },
    anchor    = { x = 0.0, y = 0.0 },
    paint     = { r = 1, g = 1, b = 1, a = 1 }
  })

  label_buf: [32]u8 = {}
  label := format(label_buf!, "Score: {}", score)
  -- ... fill_text with `text = label`
end
```

**`TextBuf[N]` depends on a language feature #11 never settled: an integer generic parameter.**
#15 blesses `[N]T`, but that is a builtin; whether a *user-declared* generic may take a
compile-time integer is open. The alternatives are dead ends — a `TextBuf` holding a `![]u8`
is forbidden outright by #15's ban on views in struct fields, and a single fixed capacity is
the spec choosing every game's name-field length. **This decision is therefore conditional**:
integer generic parameters, scoped to array lengths, are split to their own ticket (§10). If
they are refused, `TextBuf` is withdrawn and the manual cursor above returns, disturbing
nothing else here.

## 6. `$.input.text()` returns `[]u8` with a frame lifetime

ADR-0011 §294 writes `$.input.text() -> string`. Under §1 it cannot be one — the text composed
this frame is not a literal and does not live in the constant blob.

**It returns `[]u8` viewing runner-owned memory, valid until the frame entry returns.**
ADR-0006 already establishes runner-owned buffers the program reads without owning, and #15's
store ban means keeping the text is already an explicit, visible copy — which `TextBuf.append`
is.

**Rejected: `$.input.text(buf: ![]u8) -> []u8`.** It forces every caller to size a buffer for a
quantity the runner knows and they do not.

## 7. A literal carries a hidden NUL; nothing else does

[#29](https://github.com/adamico/ludo/issues/29) freezes an ABI-stable subset, C strings are
NUL-terminated, and a `[]u8` carries no terminator.

**String literals are emitted NUL-terminated in the runner's constant blob.** The NUL sits
outside `len` and is invisible to ludo, so passing a literal to an `extern` is free. Nearly
every real `extern` call takes one — `"SDL3"`, a window title, a shader entry point — and
paying a copy for a constant is a cost with no beneficiary.

**Everything else pays an explicit conversion** into memory the caller owns. The guarantee
attaches to the **blob**, not to the type: a `[]u8` never acquires it, so there is no rule an
agent can misapply.

## 8. Iteration: `chars()` survives, `bytes()` does not

#15 Q11 mandated iteration via `s.bytes()` or `s.chars()`, written when `string` was assumed
opaque. Under §2 it is not: **a `string` already *is* its bytes**, so `bytes()` is the identity
function and is **deleted**. `chars()` is kept — it decodes UTF-8, which no view operation
does.

Both are defined on `[]u8`, and `string` inherits them through the widening. Defining them on
`[]u8` is forced: iteration must work on constructed text, or §1 is useless. #15's pairing
existed to force the byte-versus-scalar choice into the open, and the widening now does that
at the type level — iterating as bytes means writing `[]u8`, visibly.

**`[]u8` is not validated UTF-8, and an invalid sequence yields U+FFFD.** Never a fault, never
a fallible return. The bytes usually arrive from `$.input.text()` or a storage slot, so a
malformed sequence is not programmer error, and #18's fault is for programmer error —
ADR-0042 §3's own reasoning. Faulting would let a player crash a shipped game by pasting into
a name field.

## 9. The narrow view-return clause, and the rule it exposes

`format` returns a view, which #15 Q10 forbids outright. **The clause this decision states is
narrow: a view returned by a string-construction call is derived from a buffer the caller
passed in, so it names memory the caller already owns and is not an escape.**

The general question is bigger than strings and is **split out** (§10), because **#15's rule is
already overrun by three mandated signatures** that predate this ticket: ADR-0042's
`alloc(size, align) -> ?[]u8` and `grow(old, size) -> ?[]u8`, and ADR-0022's
`get_pixels -> []u32`. The re-cut spans the memory model and the graphics facade, and deciding
it inside a string ticket would bury it.

## 10. What is split out, and what is left unspecified

**Two tickets are filed rather than decided here:**

1. **The general view-return re-cut** — when may a function return a `[]T`, given `alloc`,
   `grow` and `get_pixels` already do. #15 and ADR-0042/ADR-0022 territory.
2. **Integer generic parameters** — may a user-declared generic take a compile-time integer,
   scoped to array lengths. #11 territory, and §5's stated condition.

**One spelling is left unspecified and named so it is not assumed:** where the `!` marker sits
when taking a writable sub-view of a place — `name![a..<b]` or `name[a..<b]!`. The candidate-B
prototype marks whole places postfix (`pool!.add(e)`) and never slices one. It is #15's marker
grammar, not this ticket's; §5's `TextBuf` removes this decision's only need for it.

## 11. The stamp list

- **ADR-0009 §190** — `TextDesc { text: string, ... }` → `text: []u8`.
- **ADR-0010 §269** — same descriptor, same change.
- **ADR-0011 §294, §355** — `$.input.text() -> string` → `-> []u8`, with §6's frame-lifetime
  clause.
- **#17 §4** — `persist name: str = "boss"` → `string`.
- **#15 Q11** — `bytes()` deleted; `chars()` retained and relocated to `[]u8`.

`CONTEXT.md` gains **String** and **Text buffer** entries.

## 12. Costs, properties and the three lenses

**#24: +1 type and +5 names** (`TextBuf`, `append`, `format`, `view`, `clear`), against
`bytes()` deleted. **This ends five consecutive zero-delta tickets and is stated plainly
rather than buried.** It is charged to the stdlib root-name companion count, not to core
grammar — nothing here is a keyword or an operator. The spend was weighed twice: `format`
alone subsumed three proposed calls for +1, and `TextBuf`'s +4 bought back the one un-Lua-like
line in the text story (§5).

**No new #19 property, and the reason is recorded** per ADR-0030 §8: every clause here is
ordinary normative library semantics that the conformance corpus tests directly. Nothing is
implementation-visible in #19's sense — there is no backend behaviour to observe, no readback,
and no timing. **P1–P15 stands.**

**Simplicity — strong, one named cost.** A beginner writes `name: string = "boss"`,
`format(buf, "Score: {}", score)` and `name!.append(...)`, all of which read as themselves.
The cost is `TextBuf[32]`'s capacity number in a first text declaration — though `[32]u8`
carries it too, so what is charged is the type, not the number.

**Robustness — decisive, and at compile time.** A wrong argument type or hole count in
`format` is a compile error rather than garbage bytes. A dangling text view is impossible by
construction, because the only escaping text type points at memory that outlives everything.
`Map[string, V]` cannot be keyed by a freed buffer, because it cannot be keyed by a
constructed one at all.

**Agent-friendliness — strong, with the conflict named.** `format` with `{}` holes is the most
recognisable text idiom in existence, so an agent writes it correctly without reading another
file (#4). The conflict, per the map's standing instruction: **`string` is the recognisable
name and the inaccurate one.** An agent that reaches for it to hold constructed text finds a
type that cannot hold it — a compile error, not a silent wrong answer, which is why the trade
is taken rather than resolved the other way.
