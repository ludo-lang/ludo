---
status: accepted
---

# The canvas is fixed at 1280×720

Every ludo program draws in a 1280×720 logical canvas. It is not declared, not
configurable, and not defaulted — it is a constant of the language, exactly as
DragonRuby's Grid is.

This overturns [ADR-0005](0005-the-drawing-facade-surface.md)'s **caller-declared
logical canvas**, which rejected precisely this on the record:

> DragonRuby's Grid fixes 720p and 16:9 for everyone, which is untenable for a
> language not exclusively for 16:9 games.

That judgement is reversed, not forgotten. It was argued entirely on **game
shapes** and never weighed the cost that turned out to matter.

## 1. The new information: a declared canvas makes the screen un-knowable to libraries

`$.graphics.set_canvas` shipped with **no getter**. All 26 `$.graphics` calls in
the corpus were enumerated — `to_logical`, `measure_text`, `get_pixels` among them
— and none reports the canvas size. The dimensions were **write-only**.

A program could live with that: it typed the literal, so it declares a `const`.
**A library could not.** Under [ADR-0014](0014-a-library-is-a-directory-that-claims-a-name.md)
a library is an ordinary module set, and [ADR-0012](0012-the-wrap-the-facade-idiom-and-the-forwarding-function-guarantee.md)
blesses wrapping the facade — but a third-party HUD, UI, camera, layout or
particle library had **no way to learn the play area's dimensions**. It would take
the size as a caller-supplied parameter on every entry point, forever, and any
library that forgot would be silently wrong on somebody's canvas.

That is what a fixed Grid buys for free, and it is why every DragonRuby library
can lay out against the screen without being told anything. ADR-0005 traded it
away without noticing, because it was weighing 16:9 against vertical shmups, not
libraries against parameters.

The alternative was a canvas-size getter, and it was rejected as strictly worse
than fixing the constant: it costs a call against [#24](https://github.com/adamico/ludo/issues/24),
it leaves every library's layout logic conditional on a value that varies per
program, and it makes *what does the screen look like* a runtime question in a
language whose whole agent story is that such questions have answers you can read
off the spec.

## 2. What the reversal costs, stated in full

**A non-16:9 game letterboxes itself inside the grid.** A 1:1 puzzler draws a
720×720 play area centred in 1280×720 and owns the two side regions; a vertical
shmup does the same with a tall strip. The language will not do it for them, and
those regions are ordinary canvas the program may draw into — unlike the
[ADR-0030](0030-the-fit-is-a-uniform-transform-and-the-bars-are-not-the-programs.md)
§6 bars, which are outside the canvas and unreachable. This is exactly what
DragonRuby games do, and it is a real cost: such a game spends coordinates on
centring arithmetic that a declared canvas would have made free.

**Pixel art is authored scaled.** A 320×180-style game draws 16×16 sprites at
64×64 in the grid — again the DragonRuby workflow. The arithmetic is kinder than
it looks: at 1080p the fit gives `k = 1.5`, so a 4× sprite lands at 6× device
pixels, and at 1440p `k = 2` gives 8× — integer in both cases, without anyone
asking for it. At 1366×768, `k ≈ 1.067` and the multiple is fractional, which
after [ADR-0031](0031-integer-scaling-loses-to-the-priority-ordering.md) is
accepted rather than corrected.

**A game wanting a genuinely different resolution has no recourse.** This is the
sharp edge of the trade and it is not softened here. The bet is that the ceremony
and library costs of a per-program canvas outweigh the flexibility, which is the
bet DragonRuby has been running in production for years.

## 3. What survives unchanged

**ADR-0030 and ADR-0031's fit, entirely.** `k = min(w/W, h/H)` becomes
`k = min(w/1280, h/720)` — the same uniform scale plus centring translation,
aspect preserved, stretch-to-fill forbidden, content clipped, bars opaque black.
The formula gains constants where it had variables and loses nothing. ADR-0031
§4's re-grounding of the anti-stretch clause on simulation stability is
unaffected, and now stronger: with a fixed canvas the world is the same shape
*and the same size* on every machine.

**#19 P13** is unchanged and easier to test — the expected aspect is now a
constant, so the property no longer has to read the program's declaration to know
what to assert.

**ADR-0013 §9** — *the real window size is not exposed* — is untouched and better
motivated. ADR-0011's pointer reporting in logical space is untouched.

## 4. `set_canvas` shrinks to the style token

The size is gone, but `style` still needs a declaration site and still must be
immutable for the process's life — ADR-0013 §5's argument for that (a token that
differs between frames makes every guarantee conditional) is about `style` as much
as about size, and survives intact.

So the call remains, one field lighter. Its `CanvasDesc` descriptor loses its
reason to exist: a single-field descriptor is ceremony, and the call takes the
token directly. ADR-0013 §5's framing of it as #26's first real top-level client
is unaffected.

Whether the call keeps the name `set_canvas` when it no longer sets a canvas is
the one judgement here that could reasonably go the other way. It does not: the
token is a property *of* the canvas, ADR-0007's verb-first rule wants a verb that
says what it acts on, and a rename spends churn across every example in the
corpus for a shade of accuracy.

## 5. One constant ships

`$.graphics.canvas_size: Vec2`, so that no program types `1280`. A `Vec2` rather
than two scalars because [ADR-0016](0016-the-blessed-math-set-concrete-types-scalar-quantities-no-simd-mandate.md)
blessed it and it costs one root name instead of two.

It is a **constant, not a call** — there is nothing to query, and ADR-0021's
const-eval floor folds it at compile time. It never engages
[ADR-0028](0028-the-admission-test-for-a-query-that-reads-backend-state.md)'s
admission test, which governs reads of *backend* state; this reads a number
written in the spec and leaks zero bits about the player's hardware.

## 6. Against issue #24: net negative

- **`CanvasDesc` type: removed** (−1).
- **`$.graphics.canvas_size` constant: added** (+1).
- `set_canvas` survives as a function, one field lighter — no change to the count.

Net zero on names and **one fewer concept**, since a descriptor type is a thing to
learn and a constant is not. A reversal this size paying nothing is worth
recording: the budget counts surface, and this decision removes surface while
removing a decision the programmer had to make.

## 7. How it fares on the three lenses

- **Simplicity.** The strongest result in the corpus so far. The zero-ceremony
  program is now literally zero — no canvas line, no size to choose before writing
  the first `fill_rect`, and no first-run decision a beginner has no basis for
  making. *The screen is 1280×720* is a fact, not a configuration.
- **Robustness.** Positive, in an indirect way. The bug class it deletes is not in
  the compiler but in the ecosystem: a library laid out against an assumed size,
  correct on its author's canvas and wrong on yours, with nothing to catch it.
  That class is now unrepresentable.
- **Agent-friendliness.** The decisive lens, and the one that flips ADR-0005.
  [#5](https://github.com/adamico/ludo/issues/5)'s locality criterion says an
  agent should write correct code without seeing the rest of the codebase — and
  under a declared canvas, *where is the centre of the screen* was unanswerable
  without reading another file. It is now a constant an agent already knows, which
  is the same fix [#4](https://github.com/adamico/ludo/issues/4)'s 43.7%
  naming-failure finding keeps pointing at: give the agent facts it cannot get
  wrong rather than values it must go and find.

## 8. The priority ordering

Neutral. The canvas is a coordinate space, not a rendering path; nothing here
changes throughput or frame pacing, and ADR-0031's demotion of pixel fidelity is
what makes §2's fractional-multiple case acceptable rather than a defect.

## Amendments this ADR makes

- **ADR-0005** — the **caller-declared logical canvas is overturned**; the canvas
  is a language constant. Its 16:9 objection is answered by §2 accepting the cost,
  not by rebutting it.
- **ADR-0013** — §5 loses the canvas size and its `CanvasDesc`; `set_canvas` keeps
  its name and takes the `style` token directly (§4). §5's immutability argument
  survives for `style`. §9 unchanged.
- **ADR-0030 / ADR-0031** — unchanged in substance; `W` and `H` are now constants
  (§3).
