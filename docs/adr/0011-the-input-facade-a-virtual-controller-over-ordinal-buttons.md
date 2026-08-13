---
status: accepted
---

> **Amended by [ADR-0013](0013-the-drawing-entry-takes-the-screen-target.md):** the
> ADR-0009 / #26 frame-entry conflict this ADR raises and leaves open is **resolved** —
> the drawing entry takes `screen: !Target`.
>
> **Amended by [ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md):**
> unchanged in text — the pointer-in-bars clause gains the graphics precondition it was
> always written against, since nothing before it required bars to exist.
>
> **Amended by [ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md):**
> `to_world`, owed by this ADR and shipped by ADR-0013 §8, is counted for the first time.

# The input facade: a virtual controller over ordinal buttons, with the raw devices beneath

ADR-0004 mandated "input query" in the same breath as the drawing calls, and
ADR-0009 separated it out: input shares nothing with drawing except a backend, so
it is its own facade area with its own module. Unlike drawing, input had no
settled-semantics ADR to apply names to — ADR-0005 did that job for drawing and
nothing had done it for input — so this ADR owes both halves, the semantics and
the spelling.

It inherits ADR-0007's six naming rules unchanged (one module per facade area
under `$.`, verb-first, `snake_case`, no abbreviations at all, a descriptor struct
past two parameters, no overloads) and ADR-0009's additions (`PascalCase` types,
`snake_case` enum members, `Vec2` over scalar pairs).

## The complaint that re-rooted this decision

The obvious design — a keyboard, a mouse and a gamepad, each with its own
queries — was drafted and abandoned. It fails a developer who does not use a
QWERTY keyboard, and it fails them **as a developer, not as a player**: writing
`Key.w` for "forward" requires knowing where `W` sits on a US board, which on an
AZERTY board is under the `Z` cap. Every physical-key spelling of movement taxes
that developer on every line.

Two intermediate answers were tried and both were wrong:

- **A physical key enum named by the US legend** (SDL scancode, `KeyboardEvent.code`).
  It fixes the *player's* AZERTY problem — WASD keeps its shape — and leaves the
  developer's untouched.
- **A layout-mapped key enum** (`Key.w` is whatever key prints `w`). It breaks
  WASD's shape on AZERTY, and on a Cyrillic or Greek layout there is no key that
  prints `w` at all, so a closed enum would carry members that cannot exist.

The decisive observation is that **the axis is not physical-versus-layout, it is
role**. Positional keys (WASD, QE) mean the *shape* on the board; mnemonic keys
(M for map, I for inventory) mean the *letter*. The two roles want opposite
answers, so no single key enum can serve both — and the positional role should
not be spelled as keys at all.

## The mandated surface is a virtual controller

The primary surface is an **abstract controller** which the implementation maps
onto real hardware, following PICO-8 (`btn(i, p)` over six indices and eight
players) and Usagi (`pressed`/`held`/`released` over named actions). The raw
keyboard, mouse and pad queries remain in the facade, but as the escape hatch.

This answers the complaint at the root rather than patching it: **a game written
against the virtual controller never names a key**, so there is no US legend to
remember and no layout question anywhere in the gameplay path. The layout-mapped
key enum survives only for dev hotkeys and keyboard-only games.

The cost is stated plainly: ludo mandates a **normative default binding table**
(below), which is the first time this spec mandates a *convention* rather than a
mechanism. It will be wrong for someone.

**Rebindable actions are rejected**, not deferred. A remapping layer needs
persistence and a UI, which is the engine boundary ADR-0004 and #12 both drew.
Godot and Unity are the corpus evidence for the other road: everyone who ships
semantic action names makes them user-defined, and that is a subsystem.

## Buttons are ordinals, and hardware nouns live only in the table

The set was first drafted with controller nouns — `Button.south`,
`Button.shoulder_left`, `Button.start` — following SDL3, which renamed A/B/X/Y to
south/east/west/north precisely because Xbox and Nintendo swap them. **That is
rejected**, because it puts hardware in the language: a keyboard-only game writing
`Button.shoulder_left` is an anatomy lesson about a device it never touches.

The conflict is structural, not cosmetic. The set's size is fixed by a principle —
**a control is in the set iff every conforming platform's pad has it** — and that
principle admits controls which exist *only* on a pad, which can only be named
after the pad. One of the two had to give.

The resolution is **ordinals**, which dissolves the conflict rather than choosing
a loser:

```
Button = one | two | three | four | five | six
       | seven | eight | nine | ten | eleven | twelve
```

Fixed normatively by prominence, so the number means the same thing on every pad
and a model can predict it:

| Ordinal | Pad control |
|---|---|
| `one` … `four` | Face buttons, in SDL3's **south, east, west, north** order |
| `five`, `six` | Left and right shoulders |
| `seven`, `eight` | Left and right triggers |
| `nine`, `ten` | Start, select |
| `eleven`, `twelve` | Left and right stick clicks |

"Shoulder" survives as a row in a table, which is where a hardware noun belongs —
a fact about a mapping, not a concept in the language.

**South is `one`** because it is the confirm/jump position on every platform's
convention. Word-spelled members rather than `button_1` follow rule 3 and
ADR-0009's `snake_case` member convention.

### What this deletes, recorded honestly

Ordinals cost the readability that DragonRuby's docs explicitly recommend — its
inputs API tells the reader to prefer `:north`/`:south`/`:east`/`:west` over
`a`/`b`/`x`/`y` because cardinal names are controller-agnostic. That advice is
sound *for DragonRuby*, whose cardinal names sit under a `controller_one.`
namespace that genuinely is a controller. Here the same names would sit in a
module a keyboard-only game also uses.

**A consequence of the ordinal choice: the Switch A/B confirm swap is not
remapped.** DragonRuby ships `accept`/`cancel` and Usagi remaps `BTN1` to A on
Switch, because both have a *semantic* layer where "confirm" is a name the runtime
can bind. This facade has no such layer, so **positions are positions**: `one` is
always the physically-south button, and a game wanting Switch-native confirm
branches on the platform itself. Recorded because an earlier draft of this
decision carried a Switch remap clause inherited from Usagi, and with ordinals
that clause is incoherent.

**Numbered indices with a bitmask are rejected** — PICO-8's `btn()` returning a
bitfield of all states is a 1980s memory compaction. An `int` index has no
compile-time catch (`btn(7)` is a silent wrong answer where `Button.thirteen` is a
named error at a source location), and a bitmask is an opaque integer whose bits
are documented in prose, which is the inverse of what #22's oracle can complete.
Rule 4 also kills `btn` for `button`.

## Direction is a `Vec2`, not four buttons

PICO-8 and Usagi both make LEFT/RIGHT/UP/DOWN ordinary buttons. This facade does
not, because a bool set cannot carry a stick's magnitude and walk-versus-run is
the one thing a stick has that keys do not.

```
$.input.direction(side: Side = .left, player: int = 0) -> Vec2
Side = left | right
```

`Side` rather than a priority word: DragonRuby names its sticks `left_analog` and
`right_analog`, and it is right that the axis is side, not rank. An earlier draft
called this `direction_secondary`, which was a bad name because it was the wrong
concept.

Normative behaviour:

- **Keys synthesise magnitude 1.0**, and a keyboard diagonal is unit length. A
  diagonal that is 1.41× faster than a cardinal is a bug class, not a taste.
- **A stick's true magnitude passes through**, clamped to 1.0 — real sticks
  over-range in the corners, and a game must not move faster diagonally on a pad
  either.
- **The dead zone is fixed by the spec and already applied**: the returned vector
  is zero inside the zone and **rescaled so it reaches 1.0 at the rim**.
- `direction(.left)` unions the d-pad, the left stick, and the keyboard's bound
  direction keys.
- **`direction(.right)` falls back to the face buttons** where no right stick
  exists, following DragonRuby's `directional_vector_right`. Twin-stick survives
  on a keyboard.

### Why the dead zone is not a knob, when DragonRuby's is

DragonRuby exposes `analog_dead_zone` (default 3600) and
`left_analog_active?(threshold_raw:, threshold_perc:)`. That is real evidence
against a fixed zone, and it is answered rather than ignored: DragonRuby needs the
knob because its `raw` and `perc` values expose the dead zone as a **cliff the
caller must handle**, so of course the cliff's position becomes a parameter. Under
the rescaling above the caller has nothing left to tune — the knob answers a
question the return type no longer asks.

**Cost, stated:** a fighting game wanting a tighter zone for precise inputs cannot
get it and drops to the delegated tier.

### One number, not three

DragonRuby ships `left_analog_x_raw`, `_perc` and `_angle`, plus six variants of
`left_right`. This facade ships a `Vec2` and nothing else; an angle is a `Vec2`
question the math library answers. ADR-0009 already committed the spec to `Vec2`
over scalar pairs, so this costs nothing and avoids the redundancy rule 6 deletes.

## Analog triggers are not exposed

Triggers are ordinals `seven` and `eight` — bools — which is DragonRuby's call
(`l1`, `r1`, `l2`, `r2` are all buttons there, with no analog trigger value in the
API at all).

The reason is that a trigger is the only control in the set with **no keyboard
degradation that preserves information**. An `f32` that is always exactly 0.0 or
1.0 on half the platforms is a value whose type lies. A racing game wanting real
trigger pressure drops to the delegated tier — a narrow, nameable class, unlike
the broad class that dropping shoulders entirely would have created.

## Edge queries, and no auto-repeat

`_down` (level), `_pressed` and `_released` (edge, exactly this frame) are the
triple, for buttons, keys and pointer buttons alike. **The edge state is the
runner's**, computed against the previous frame, so nobody hand-rolls a
previous-frame `persist`.

**Auto-repeat is not mandated.** PICO-8's `btnp` repeats after 15 frames then
every 4, tunable by poking `0x5F5C`/`0x5F5D` — two magic addresses, which is
ADR-0004's deleted-policy-class thesis demonstrated by counter-example. Usagi's
`pressed` does not repeat.

**The cost is real and is the weakest point in this surface:** menu navigation
wants discrete, edge-triggered, repeating up/down, and a `Vec2` gives none of the
three. A menu in ludo needs a threshold, a latch and a timer in `persist` — more
work than in PICO-8. If a future ADR makes one concession here it belongs as a
**separate named call**, never as a flag on `pressed`.

## Players are slots, not devices

Every controller query takes `player: int = 0`, with a **fixed maximum of four**,
so single-player never writes it. A `Controller` handle was rejected: it can
dangle across a hot-unplug.

- **An absent device is present and idle** — no buttons down, zero direction —
  never an error and never a missing symbol. A call site conditional on a device
  is a branch that never gets tested.
- **Unplugging pad 2 leaves player 2 present and idle**; players 3 and 4 do not
  renumber downward, which would swap two humans' characters mid-match.
- **The keyboard is player 0** and is never auto-assigned elsewhere. Two players
  on one keyboard — which PICO-8 mandates — is **not** covered by the default
  table and drops to raw keys. Stated as a cost; it is the classic couch case.

`$.input.connected(player)` exists despite the idle rule, for one job idleness
cannot do: "Player 2, press Start to join".

## The pointer

**Position is reported in logical-canvas space** — ADR-0005's letterboxing
removed, the `Target`'s camera transform *not* applied. World space is a property
of a `Target`, not of the pointer, and ADR-0005 reserves offscreen targets, so
"the world position" is not well-defined without naming one. A world-space
hit-test writes one visible conversion; a UI hit-test, which is the commonest
pointer use and lives in screen space, needs none.

**A pointer in the letterbox bars reports a position outside the canvas rect.**
Clamping would make the canvas edge unhittable but not obviously so.
`$.input.pointer_over()` is what asks the question, and the letterbox rule is what
makes it necessary rather than convenient.

**Pointer lock is a request that can fail**, never a setter: on the web it is
gesture-gated and the user can exit it at any time, so a call that pretended to
succeed would be lying. It is mandated because mouselook cannot be built from
absolute positions once the cursor reaches a screen edge — relative motion comes
with it and is the reason it is in the set.

Scroll is a `Vec2` delta; horizontal scroll exists and is free.

## The raw layer

**Keys are identified by what they print. Keys that print nothing are identified
by position** — and for those two, the answer never differs: Escape is Escape on
every layout. This is one rule, not two, and it is what makes `Key.m` give an
AZERTY developer their M key. It is also `KeyboardEvent.key` and `SDL_Keycode`,
so it is the portable path rather than the exotic one.

`Key` is a **closed enum, and a key it does not name is dropped** — invisible to
the program. The international keys (`IntlBackslash`, `IntlRo`, `IntlYen`) are
named in specifically, because dropping them makes the extra key on a non-US
keyboard unusable, which is the AZERTY unfairness in a second place. F13–F24,
media and vendor keys are out.

**An `unknown(u32)` escape variant is rejected.** A variant carrying a
platform-specific integer is a portability hole with a type-shaped invitation:
the value means different things per backend, so any code branching on it is
unportable by construction, and #11's static-only sum types make it trivially
reachable. A key this facade cannot name portably is a key the facade does not
have.

**Modifiers are ordinary `Key` members, left and right distinguished.** A
`Modifiers` bitset would be a second way to ask a question `key_down` already
answers — rule 6 — and the two can disagree at the edges (a chord held across a
focus loss). Left and right are physically distinct keys, so collapsing them
would be the one place the enum stops being honest. `key_down(.left_shift) or
key_down(.right_shift)` is one line. Cost: chorded shortcuts are more verbose here
than in any engine shipping a modifier mask.

Raw key queries carry Usagi's advisory — dev hotkeys and keyboard-only games —
because they honour neither the binding table nor the pad.

### `text()` is not an escape hatch

`$.input.text()` returns the text composed this frame. **Stamped by ADR-0043**: the
return type is `[]u8`, not `string` — a `string` is a view into the runner's
constant blob and this text is not a literal — and the view is runner-owned
memory valid until the frame entry returns. It is promoted out of the
raw layer for two reasons: it is the **only** way to spell text entry at all —
IME, dead keys and repeat cannot be reconstructed from key state — and it is the
mandated idiom for the prompt problem below.

**No general event queue.** Polling is the whole input model; a queue is ordering
the caller must not get wrong, and #26 deleted ambient ordering everywhere else.
`text()` is the one sequence, and it is scoped to one frame.

### `key_label` is rejected by name

A game showing "Press **W**", or a rebind screen, must display something for a
key. `$.input.key_label(k) -> string` is **rejected and named here so it is not
re-proposed**: asking *ahead of time* what letter is printed on a physical key
requires `navigator.keyboard.getLayoutMap()`, which is Chromium-only and absent in
Safari and Firefox. It is the one call in this facade that would be
unimplementable in a conforming browser, which ADR-0006's negative web obligation
forbids.

**The mandated idiom instead:** a rebind screen reads `key_pressed` for the
binding and `text()` for the label **in the same frame**. The label is knowable at
the moment the user presses the key, which is exactly when a rebind screen needs
it.

`mapping_for(action)` — Usagi's glyph query — is likewise **not** mandated. The
binding table is normative and static, so a game renders its own glyphs from
`last_source()` plus a `Button` value, at authoring time.

## The mandated set

Twenty-two functions in one module, `$.input`. Every call takes two parameters or
fewer, so **no descriptor struct is needed anywhere** — the first facade in this
spec for which rule 5 does not fire.

```
# Virtual controller — the primary surface
$.input.direction(side: Side = .left, player: int = 0) -> Vec2
$.input.button_down(b: Button, player: int = 0) -> bool
$.input.button_pressed(b: Button, player: int = 0) -> bool
$.input.button_released(b: Button, player: int = 0) -> bool
$.input.connected(player: int = 0) -> bool
$.input.last_source() -> Source

# Pointer
$.input.pointer_position() -> Vec2
$.input.pointer_motion() -> Vec2
$.input.pointer_over() -> bool
$.input.pointer_down(b: PointerButton) -> bool
$.input.pointer_pressed(b: PointerButton) -> bool
$.input.pointer_released(b: PointerButton) -> bool
$.input.scroll() -> Vec2
$.input.set_cursor_visible(visible: bool)
$.input.cursor_visible() -> bool
$.input.request_pointer_lock()
$.input.release_pointer_lock()
$.input.pointer_locked() -> bool

# Raw keyboard — the escape hatch, plus text
$.input.key_down(k: Key) -> bool
$.input.key_pressed(k: Key) -> bool
$.input.key_released(k: Key) -> bool
$.input.text() -> []u8
```

Five types: `Key`, `Button`, `Side`, `PointerButton`, `Source`.

```
Side           = left | right
PointerButton  = left | middle | right
Source         = keyboard | mouse | gamepad
```

`last_source()` is what a game reads to switch between keyboard and pad glyph art
the instant the player switches devices. A controller **`name`** is deliberately
absent: it is a vendor string that differs across backends for the same physical
pad, so any code branching on it is unportable — the `unknown(u32)` argument in
string form.

**`to_world` lives in `$.graphics`, not here.** Inverting a `Target`'s transform is
a `Target` operation that happens to be useful to the pointer, and putting it in
`$.input` would give the same computation two owners.

## The normative default binding table

Fully normative, not advisory. An advisory table means the same program controls
differently on two conforming implementations, which is exactly the silent
divergence #19's experience contract exists to delete.

**The table names keyboard keys by physical position** (US legend), while the
`Key` enum is layout-mapped. That is not an inconsistency: a binding is a fact
about where a control sits, and the table is the one place in this design where
physical addressing is allowed to exist. It never surfaces as a `Key` value.

| Control | Pad | Keyboard (player 0) |
|---|---|---|
| `direction(.left)` | D-pad + left stick | `WASD` + arrow keys |
| `direction(.right)` | Right stick | Face-button keys, per the fallback rule |
| `one` … `four` | South, east, west, north | `Z`, `X`, `C`, `V` |
| `five`, `six` | Left, right shoulder | `Q`, `E` |
| `seven`, `eight` | Left, right trigger | `1`, `3` |
| `nine`, `ten` | Start, select | `Enter`, `Tab` |
| `eleven`, `twelve` | Left, right stick click | `Left Shift`, `Right Shift` |

Players 1–3 have **no keyboard bindings**, per the slot rule above.

## How input reaches the frame, and an inconsistency this raises

Input is read by **ambient calls** — `$.input.button_down(.one)` — not delivered
as a value at the frame entry.

ADR-0009 rejected `$.graphics.screen()` as "ambient state wearing a function,
with nothing preventing two calls mid-frame". **Input survives that objection
where `Target` did not.** The objection bites for mutable configuration like the
style token and the logical canvas; input is **immutable for the whole frame** by
construction, since #26's fixed step means the runner latches it before the entry
runs. Two calls cannot disagree. There is no ambient *state*, only an ambient
*constant* — which is the argument #26 itself used to justify a zero-parameter
entry.

That reasoning does **not** rescue `Target`, and it exposes a conflict this ADR
does not settle: ADR-0009 says the runner *passes the target into the per-frame
entry point*, and #26 resolved that the frame entry takes **zero parameters**.
Both cannot hold. Flagged here, ticketed on the map, and deliberately left open —
input's spelling does not depend on which way it goes.

## Conformance

- Input is **latched once per frame**, before the frame entry runs. Every query
  within one frame observes the same snapshot.
- Edge state (`_pressed`, `_released`) is computed by the runner against the
  previous frame's snapshot.
- **`connected(player)` means "has produced input"**, not "is physically
  attached". The browser Gamepad API returns nothing until the user presses a
  button, so a browser pad that has not been touched is a pad that is present and
  idle — which is the idle rule above, needing no new concept.

### The ADR-0006 negative web obligation, discharged

No mandated call is unimplementable in a browser, and the two degradations are
stated rather than discovered:

- **Pointer lock** is gesture-gated and revocable — already a request in the type.
- **Gamepad visibility** is gated on first input — covered by the `connected`
  definition above.
- **Keyboard layout** — nothing mandated depends on it; `key_label` is rejected
  by name.
- **Analog triggers** — not exposed at all, so their absence cannot be observed.

## How it fares on the three lenses

- **Simplicity.** Strong on the gameplay path: a movement-and-jump game reads two
  calls and never meets a key, a layout or a device. Weaker on first contact with
  `Button.five`, which is meaningless without the table — the ordinals buy
  layer-neutrality by spending readability, and that is the trade.
- **Robustness.** Three bug classes deleted by construction: the fast diagonal,
  the hand-rolled previous-frame latch, and the device-conditional branch that
  never gets tested. The dead-zone rescaling deletes a fourth at the type level.
- **Agent-friendliness.** Closed enums throughout mean a wrong control name is a
  compile error at a named source location rather than a silent wrong answer —
  #4's 43.7% failure mode, caught. No integer indices, no bitmask, no vendor
  strings, no descriptor structs to get field-nesting wrong in.

## Consequences

- **#24 companion count delta**: one new module, **22 functions, 5 types**. This
  is `$.graphics`-sized and larger than the surface's concept count suggests; the
  honest comparison is against what it deletes at the call site, since the virtual
  controller replaces the per-device union boilerplate every game in the corpus
  writes by hand. **Trim candidates, in order, if a later ADR needs them:** the
  pointer-lock trio, `scroll`, `connected`.
- **ADR-0009's owed input delta is discharged**, and nothing in `$.graphics`
  changes.
- **`to_world` is owed by `$.graphics`** as a consequence of the logical-space
  pointer rule, and is not counted here.
- **A rebindable action layer, `key_label`, `mapping_for`, analog triggers,
  auto-repeat, a modifier bitset, controller names, an `unknown(u32)` key variant,
  a bitmask, touch and gestures are each rejected by name**, so none returns as a
  natural extension.
- **The ADR-0009 / #26 frame-entry conflict is raised, not resolved.**
