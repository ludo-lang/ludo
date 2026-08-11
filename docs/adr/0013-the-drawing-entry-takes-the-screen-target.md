---
status: accepted
---

# The drawing entry takes the screen target, and the canvas is declared once at top level

Two accepted decisions contradicted each other, and ADR-0011 surfaced it while
spelling `$.input`:

- **ADR-0009, §"The target arrives at the frame entry"**: *"A program does not
  construct or fetch the screen target: the runner passes it into the per-frame
  entry point."* A fetch (`$.graphics.screen()`) is rejected there as "ambient
  state wearing a function, with nothing preventing two calls mid-frame."
- **Issue #26**, resolved: *"the frame entry takes zero parameters"* — the 60Hz
  step is a compile-time constant, so passing it is ceremony carrying no
  information.

A zero-parameter entry cannot receive a `Target`.

## The contradiction was narrower than it looked

#26's argument is about `dt`, not about parameters. Its words: *the step is a
compile-time constant, so passing it every frame is ceremony carrying no
information*. That reaches exactly the values a program could have written down
itself.

A `Target` is not such a value. It carries the real window, runner-owned
resources, and configuration the program cannot obtain any other way. So #26's
conclusion — *zero parameters* — was a correct statement about the only
parameter on the table at the time, not a rule about entry signatures.

ADR-0011 had already drawn the operative line while arguing that ambient input
calls do not reopen this question: input is **immutable for the whole frame**
(the runner latches it before the entry runs), making it an ambient *constant*.
A `Target` is mutable configuration — its transform changes during the frame —
so it does not qualify, and ADR-0011 said so explicitly rather than leaving it
to be discovered here.

## Decision

### 1. The entry takes the screen target as a parameter

`frame(screen: !Target)`. #26's ceremony argument is recorded as scoped to
values carrying no information, not to parameters in general.

**A fetch is rejected**, on ADR-0009's unrepaired objection: nothing prevents two
calls mid-frame and nothing says whether they return the same target. ADR-0005's
first sentence about the target — *"an opaque value, passed and never ambient"* —
already forecloses it.

**A top-level binding is rejected too**, and it is the interesting wrong answer.
#26 makes top-level bindings **immutable constants from the frame's view**, and a
target is mutable. Making it `persist` is worse: the runner owns the window, so
reload and resize would reach into user-declared `persist` state, and #17's
reload set would contain a value the user never authored.

The `!` mark follows from #8: the frame mutates the target's transform.

### 2. The parameter is the screen target, spelled `screen`

`Target` is the type, and ADR-0005's reserved offscreen shape will produce more
of them, so the parameter names *which* target this is. Under #16's
free-functions-plus-UFCS this reads `screen.fill_rect({...})`, and stays honest
once a second target exists.

### 3. The rule that governs the parameter list

> **The entry's parameters are exactly the values only the runner can supply.**

Today that set has one member. An **offscreen target is constructed by the
program** (ADR-0005's reserved constructor), so it is an ordinary value held in
`persist` and never joins the list. The list grows only if the runner acquires a
new exclusive property — rare, and deserving its own ADR.

The rule is a predicate with a fact behind it — *can the program construct this
itself?* — rather than a taste line, so it is decidable when the next candidate
appears.

### 4. The target belongs to the entry that **draws**

#26 recorded the constraint that fixed-timestep-with-interpolation requires two
entries, and routed render-rate decoupling to #28 rather than deciding it blind.

So the target is stated as a parameter of **the entry that draws**, which today
*is* the frame entry. If #28 later splits simulation from rendering, the drawing
half keeps the parameter and the simulation half takes zero parameters — which is
#26's original conclusion surviving in the place its argument actually applies.

The two decisions were never in conflict about substance, only about which entry
the word *frame* named while there was only one.

### 5. The canvas and style are declared once, at top level

This is the crux the contradiction was hiding. ADR-0005 mandates a
**caller-declared** logical canvas and a `style:` token that must hold for a whole
frame — but if the runner constructs the target, it needs both **before the first
frame runs**. ADR-0009's phrasing (*"declared once at that entry"*) was ambiguous
between the entry's declaration and its first call.

```
$.graphics.set_canvas({size = $.vec2(320, 180), style = .crisp})
```

A **top-level statement**. #26 built a top level that runs exactly once before any
frame precisely so setup has a home without `love.load` ceremony; this is its
first real client, and it costs no new grammar.

Rejected: attaching the configuration to the frame entry's *declaration*, which
invents entry-declaration syntax against #24's budget for one call site.

Rejected harder: making canvas and style **mutable per frame**. A logical canvas
that can differ between two frames makes every letterbox and integer-blit
guarantee in ADR-0005 conditional on the program not having changed it — the
*mixing aesthetics inside one frame* failure the `style:` token was introduced to
delete, reintroduced across frames instead of within one.

**Consequence, accepted:** canvas and style are **immutable for the process's
life**. Resolution changes belong to the window, not to the canvas.

Naming: `set_canvas`, not `configure` — ADR-0007's verb-first rule wants a verb
that says what it acts on, and `configure` says nothing. `style` is a field of its
descriptor rather than a second call, because both are frame-invariant and
declared together.

### 6. A fresh target each frame, transform reset to identity

The runner hands the entry a target whose transform is identity at frame start.
The transform does **not** survive into the next frame.

This deletes the *forgot to pop the transform* bug family by construction — the
OpenGL/Cairo failure where a stray camera offset leaks into the following frame
and manifests as slow drift, which is miserable to debug. It also makes each
frame's drawing independent of the previous frame's, which is #4's
non-local-reasoning lens applied at the loop level.

**Cost, and it is small:** a camera that persists across frames is a `persist`
value applied to the target each frame — one line, and it makes the camera
**greppable state** rather than something hidden inside a runner-owned object,
which is #26's call 3 applied consistently.

### 7. Top-level code cannot draw

The target exists only as a parameter of the drawing entry, so there is no target
at top level. A "render one frame and exit" program is therefore a drawing entry
that exits, not a top-level statement.

Recorded as a deliberate consequence rather than left to be discovered. The
alternative — letting top level obtain a target — reintroduces §1's rejected fetch
through a side door, and the case is thin: a one-frame program still wants the
window, the backend and the present, which is the runner's entire job.

The honest cost: ludo's smallest **visual** program is larger than its smallest
program. `$.print("hi")` stays zero-declaration, and the first visual program
costs exactly one entry — the loop concept a beginner meets anyway.

### 8. `to_world` and `to_logical`

ADR-0011 owes `to_world` — the pointer reports in logical-canvas space, and world
space is a property of a `Target`. With the target in scope this is
`screen.to_world(p)` under UFCS, and its ergonomics needed nothing further.

The **inverse ships with it**: `$.graphics.to_logical(target, world_point)`. Not
scope creep — the same call's other direction. A program that reads a pointer in
world space also needs to place a world-space thing at a fixed screen position (an
edge marker, UI tracking an entity), and without the inverse the user reimplements
the transform's inverse by hand — precisely the arithmetic the target's transform
exists to own, and silently wrong when they get it right-ish. The pair is
guessable in a way a lone direction is not.

### 9. The real window size is not exposed

The logical canvas exists to hide it: the facade letterboxes and scales, ADR-0011
reports the pointer in logical space, and ADR-0005 puts the integer-blit guarantee
in logical space. Exposing the window size hands programs a way to become
resolution-dependent, which is the failure the canvas was mandated to prevent —
and once the query exists agents will reach for it, since training data is full of
`getWindowSize()`.

One genuinely motivated case is **not** decided here and goes to the fog:
**choosing a canvas size from the display you are on**, which §5's top-level
declaration site could in principle read. It is an initialisation question tangled
with fullscreen and display enumeration — platform-layer surface this map has not
touched — and it is not this ticket's to settle.

## #24 companion count delta

**+2 functions and +1 type on `$.graphics`**: `set_canvas` and `to_logical`, plus
the `CanvasDesc` descriptor. No new module. The `screen` parameter is not a name
in the reserved root and is not counted.

## Consequences

- ADR-0009's "the runner passes it into the per-frame entry point" is confirmed
  and made precise; issue #26's "zero parameters" is narrowed on the record to the
  simulation entry and to values carrying no information.
- ADR-0005's caller-declared logical canvas gains its declaration site, and gains
  process-lifetime immutability it did not previously state.
- #28 inherits a cleaner split: whichever entry draws takes the target, and the
  simulation entry takes nothing.
- The fog gains one item: choosing a canvas size from the display, which is
  platform-layer initialisation.
