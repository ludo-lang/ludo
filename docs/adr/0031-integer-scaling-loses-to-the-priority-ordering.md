---
status: accepted
---

> **Amended by [ADR-0036](0036-the-window-opens-maximal-and-belongs-to-the-player.md):**
> §2's deletion of the too-small-window case is restated as a positive clause — no floor, no
> minimum window, no refusal to launch — and the cost §3 leaves implicit is named: at `k < 1`
> a `crisp` target drops texels rather than blurring them.
>
> **Amended by [ADR-0038](0038-the-escape-hatch-is-a-player-set-render-scale.md):** §3's
> rule that a style token may not mean different things on different monitors is
> **satisfied by, and does not decide,** the render scale's upscale filter — any fixed
> answer satisfies it. Stability decides it: the resample from `s · k` to `k` is **linear on
> both tokens**, because nearest at a non-integer ratio shimmers in motion, which is the
> same argument §2 used to delete integer scaling. Content sampling is untouched.
>
> **Amended by [ADR-0039](0039-stretch-to-fill-is-refused-and-no-player-exception-is-added.md):**
> §4's *recorded as open, and deliberately not decided here* is **closed** — stretch as a
> player preference is **refused**, along with a zoom-and-crop variant §4 never named. §4's
> three passing tests all hold; what refuses it is §4's own similarity-transform grounding,
> which §4 stated and did not apply to the player case.

> **Absorbed by [spec ch5](../spec/05-runner.md):** the chapter is normative; this ADR
> keeps the argument — the single `k` and unconditional `crisp` are transcribed there.

# Integer scaling loses to the priority ordering, and the fit is one formula

[ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md)
shipped the fit with a two-branch scale rule: real `k` on a `smooth` target,
`max(1, floor(k))` on a `crisp` one, to avoid uneven pixel sizes. Within an hour
of it landing, the map gained a standing priority ordering that decides the
question the other way:

> **Performance and frame stability above rendering fidelity.** Where a fidelity
> guarantee costs throughput, frame pacing, or stability on a major platform, the
> guarantee loses. **1:1 pixel fidelity is not a design target**, and *k = 1* is
> not a state to engineer toward.

This ADR applies it. The record shows the fit shipping with integer scaling and
losing it to a principle stated afterwards, because *that this was thought and
then corrected* is the finding — ADR-0030 §5 was argued carefully and was still
answering to the wrong priority.

## 1. What integer scaling actually cost

Not stated plainly enough in ADR-0030, and the arithmetic is decisive. A
`1280×720` canvas in a `1920×1080` window:

| rule | `k` | image | bars |
| --- | --- | --- | --- |
| real | `1.5` | `1920×1080` | **none** |
| ADR-0030 §5 integer | `1` | `1280×720` | 320px left and right, 180px top and bottom |

**The bars were never a consequence of letterboxing.** Identical aspects
letterbox to nothing. They were a consequence of integer scaling, which existed
solely to keep pixel sizes even — a rendering-fidelity guarantee costing a third
of the screen in the single most common desktop configuration. Under the
ordering that is not a close call.

ADR-0030 §5 recorded the 1366×768 case as the cost and treated 1080p as the
comfortable one. That was backwards: 1080p is where the rule is most expensive,
because it is where real `k` is exactly `1.5` and integer `k` throws away half a
screen to round it down.

## 2. The fit is one formula, with no `style` branch

ADR-0030 §4 and §5 are replaced by:

> Given a logical canvas of `(W, H)` and a window of `(w, h)` in device pixels,
> `k = min(w/W, h/H)`. The canvas image occupies `(kW, kH)` device pixels,
> centred at `origin = (round((w - kW)/2), round((h - kH)/2))`. The same `k`
> applies on both axes.

Everything else in ADR-0030 §4 stands unchanged: the mapping is a uniform scale
followed by a translation and nothing else, **aspect ratio is preserved, and no
conforming implementation may scale the axes independently, shear, or stretch to
fill**; the device pixels outside the rect are the bars.

Three special cases disappear with the branch. There is no `max(1, ...)` floor;
no `style` term in a geometric formula; and no *window smaller than the canvas*
case, which ADR-0030 §5 handled by clipping to the canvas centre and which now
simply downscales — the better behaviour, arrived at by deleting rather than
adding. **ADR-0030 §6's clip rule is unaffected** and still stands on its own
grounds: it stops program content leaking into the bars, which is independent of
how `k` is computed.

## 3. `crisp` keeps nearest sampling unconditionally

The question deleting §5 forces: at `k = 1.5` a `crisp` target samples nearest at
a fractional scale, giving uneven pixel sizes *and* shimmer on motion — plausibly
worse-looking than linear at the same scale. So does `crisp` degrade to linear
off integer scales?

**No.** ADR-0005 chose one `style` token over two independent fields precisely so
conformance rules could be sentences about one named thing, and a token whose
meaning depends on the window size — which ADR-0013 §9 forbids the program from
seeing — is the worst available option: an aesthetic that varies per player,
invisibly, which is the class of divergence the logical canvas exists to prevent.

The ordering is silent here, since neither reading costs performance. Where it is
silent the lenses decide, and both simplicity and agent-friendliness want the
unconditional reading: *`crisp` is nearest sampling* is a fact an agent can apply
without knowing anything about the player's monitor.

**The cost is stated rather than hidden: pixel art at fractional `k` looks worse
than it did under ADR-0030 §5.** That is the fidelity being deliberately
demoted, and this is the sentence that says so.

## 4. Why stretch-to-fill does not follow, and is re-grounded rather than weakened

Demoting fidelity might look like it reopens ADR-0030 §4's ban on stretching. It
does the opposite: the ban's justification improves.

A uniform scale plus a translation is a **similarity transform** — distances,
angles and circles survive it, so the program's world is the same *shape* on
every machine. Non-uniform scale is not, and makes the visual output a function
of the display. That is the **simulation-stability** half of the ordering, not
the fidelity half, so the clause now rests on the same principle that deleted
integer scaling instead of standing against it.

**A program-facing `fit:` field stays rejected** on ADR-0030 §9's grounds, which
the ordering does not touch.

**Recorded as open, and deliberately not decided here: stretch as a *player*
preference**, in ADR-0025's fullscreen mould — the player choosing to fill their
panel. It passes the tests that killed the program-facing version: it shows **no
more world**, so #60's hardware-as-advantage argument does not reach it; it is
symmetric across players; and pointer input mapping back through the same
transform keeps the game self-consistent, so aiming still works and only shapes
look wrong. It is map fog, not a ticket, because it hangs on ADR-0025's
preference machinery and nothing presses on it.

## 5. Conformance: P13 restated, unchanged in substance

ADR-0030 §8's P13 referenced the two-branch `k`. Restated:

> **P13 — the canvas is fitted, not stretched.** *Precondition:* a program
> declaring a logical canvas of a known aspect. *Action:* run it in windows of
> several aspects, including one narrower and one wider than the canvas, and read
> back the presented framebuffer. *Observable:* the canvas image occupies a
> centred rect whose aspect equals the declared aspect, at `k = min(w/W, h/H)`,
> with every pixel outside it opaque black and no program content among them.

Still **full conformance only**, and still the first #19 property core
conformance cannot run. The test is strictly easier to write than ADR-0030's
version, which needed the target's `style` to predict `k`.

## How it fares on the three lenses

- **Simplicity.** A clear win, unusually. One formula replaces two, a floor
  disappears, a special case disappears, and the surviving sentence has no
  conditional in it. The thing a beginner had to know — *`crisp` means thicker
  bars* — stops being true.
- **Robustness.** Neutral. No guarantee is lost that anything depended on:
  ADR-0030 §5 was one day old and no other clause referenced its floor. The
  uniform-transform guarantee, which is the load-bearing one, is untouched and
  better justified (§4).
- **Agent-friendliness.** Positive. `k = min(w/W, h/H)` is checkable arithmetic
  with no branch on a token, and `crisp` means one thing everywhere (§3) — both
  are properties an agent can apply without reading the rest of the corpus, which
  is #5's locality criterion.

## Against issue #24: zero delta

No keyword, operator, facade call or stdlib root name. This ADR deletes clause
text and adds none.

## Amendments this ADR makes

- **ADR-0030** — §5 is **deleted**; §4's formula is restated without the `style`
  branch, the floor and the too-small-window case (§2); §8's P13 is restated
  against the new `k` (§5). §3, §6, §7 and §9 are unchanged.
- **ADR-0005** — the `style` token gains an explicit statement that `crisp` is
  nearest sampling at every scale, not only integer ones (§3).
