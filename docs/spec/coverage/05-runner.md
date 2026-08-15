# Coverage: chapter 5 — the program and the runner

**Non-normative.** This file is the closing test for
[`../05-runner.md`](../05-runner.md), required by ADR-0044 §5:

> A chapter closes only when **every clause of every ADR it covers has been
> transcribed or explicitly dropped, with a citation both ways** — spec clause →
> source, and a checklist of sources → where each landed.

The spec clause → source direction is discharged inside the chapter: §0.5 makes
a clause without a citation a defect. This file is the other direction.

A source clause that cannot be located is a **phantom clause**: the citations
pointing at it are deleted, or the rule is authored, and §16 below says which
happened.

**Chapter 5's boundary is what a program is at run time and what the runner owes
it.** That is why the fixed canvas, the fit, the window and the display are here
and not in a chapter about drawing: none of them is a name under `$.` (chapter
6's boundary), and all of them bind the runner. The two exceptions —
`set_canvas` and `canvas_size` — are `$.`-rooted names chapter 6 routed here
with its §4.2.7, because neither means anything without the canvas this chapter
fixes.

---

## 0. Source check (#72, #73)

Run over **this chapter's sources only** — ADR-0013, ADR-0015, ADR-0024,
ADR-0025, ADR-0030, ADR-0031, ADR-0032, ADR-0035, ADR-0036, ADR-0039, ADR-0040,
ADR-0041, and the clauses of ADR-0042, ADR-0045 and ADR-0048 this chapter reaches
— before transcription, per `docs/agents/domain.md`.

`python3 tools/check-stamps.py` reports **zero findings over the whole corpus**
both before and after this chapter. §0.1's one finding is of the silent kind the
checker is blind to by construction: an ADR that amends without declaring it.

### 0.1 Direction 1 — stamps owed and missing

Applied retroactively in this chapter's commit; stamps are additive metadata,
not prose.

| Record stamped | What moved it, and what moved |
|---|---|
| **ADR-0035** | **ADR-0036 §9.** ADR-0035 §5 states *the contract's property list therefore ends at P13*, and deliberately declines to write the numeral `P14` on the ground that a bare `P14` in prose is indistinguishable, to the grep that audits this corpus, from a property that exists. ADR-0036 §9 then **mints P14** for the window geometry. The sentence is false from that day and was never stamped. ADR-0035 §5's actual decision — that its own §3 presentation clause warrants no property — is untouched. |

The finding is worth its own sentence, because it is the failure mode ADR-0035
§5 predicted and then walked into from the other side: the ADR that guarded a
numeral against drift is the one the drift happened to, one document later.

### 0.2 Direction 2 — stamps **not** owed, recorded so they are not re-derived

- **ADR-0043 §14, ADR-0044, ADR-0047 and ADR-0048** each cite **ADR-0030 §8** as
  the precedent for *record why no property is added*. Citation as precedent,
  never a change. No stamp.
- **ADR-0044** cites **ADR-0013** for the entry's shape while arguing what the
  reference program must contain. It restates; it does not move. No stamp.
- **ADR-0040 §2**'s primary-display clause on first launch is **new**, not an
  amendment of ADR-0036 §2, whose subject is the window's *size*. ADR-0040's
  declared amendment of ADR-0036 §4 (the position split) is stamped and is the
  whole of what it moved.
- **ADR-0039** adds no clause of its own: its normative content is that
  ADR-0031 §2's existing sentence gains **no player exception** (§6). Its stamps
  on ADR-0030 and ADR-0031 are declared and present.

### 0.3 Issue sources against `SOURCES.md` (#73)

- **#9** — listed as *authoritative, contradicted*, with the contradiction named
  as unresolved by design. **This chapter resolves it** (ch5 §4.3), so the row is
  amended in this commit.
- **#26** — listed with three named changes, all absorbed (ADR-0021 §3's
  constant-declaration narrowing is chapter 4's; the `heap` respelling is ch5
  §4.5.8; the `use ... as` stale prose is ch1 §14.9). One change was **not**
  listed and is added in this commit: **ADR-0048 §7** makes allocation failure in
  a `persist` initialiser a fault, which is a rule about call 4's position that no
  stamp can carry because the target is an issue.
- **#17** — listed as *authoritative, respelled*. Both respellings absorbed (ch5
  §4.5.8, §4.5.9). ADR-0041's two corrections are recorded as amendment comments
  on the issue and are transcribed at ch5 §5.2 and §5.9.
- **#18** — listed as authoritative, with the warning not to conflate it with
  ADR-0018. Heeded: ch5 §6 cites #18 for semantics and ADR-0018 only at §6.5.3
  and §6.3.3.
- **#12** — not in `SOURCES.md` §1, because §3 lists it as originating ADR-0001
  and ADR-0003. **Both are about the platform layer and shaders, and neither
  carries #12's runner decisions** (calls 2, 3 and 4), which have no ADR at all.
  §3 gains a parenthesis in this commit so the next author does not read the
  §3 listing as covering the whole ticket.

### 0.4 A note on the checker's floor

§0.1's finding was found by reading every ADR that cites this chapter's sources,
which is the human half `docs/agents/domain.md` assigns to the chapter covering
the ADR. The checker cannot see it: ADR-0036 declares three amendments and
ADR-0035 is not among them, so there is nothing for the script to resolve. This
is the same blind spot chapter 6 §0.4 recorded, reached by a different route —
there the amendment was undeclared because the author did not notice the target,
here because the target is a *sentence about the corpus* rather than a rule.

---

## 1. #12 — the standalone shape and the runner

| Source clause | Where it landed |
|---|---|
| Call 1, standalone + C FFI | §1.1 (the FFI half is #29's, routed at §12) |
| Call 2, the bare command is the runner; `ludo build` drops it; two modes of one language | §1.2, §1.3, §1.4, §1.5, §1.6 |
| Call 3, the runner drives a callback entry rather than a user-written loop | §3.1, §10.2 (the loop's shape) |
| Call 4, the entry is a marked declaration, never a magic name; a typo'd entry and a duplicate entry are errors; the entry is greppable | §3.1, §3.2, §3.3 |
| Call 5, *why not an engine?* is spec front-matter | **Dropped** — front-matter for the spec as a whole, not a rule a program or an implementation can violate. §16 |
| The runner holds `persist` across a rebuild and holds a faulted program paused | §5.1, §6.3.2 |
| The host/guest boundary is dissolved | §1.1 |
| The comparison set is LÖVE2D and DragonRuby, not Godot | **Dropped** — positioning. §16 |
| The three questions it deliberately did not decide | Answered by #26, #28/#29 and their ADRs; nothing owed here |

## 2. #26 — top-level execution and the frame loop

| Source clause | Where it landed |
|---|---|
| Ordering rule steps 1–3 | §2.4 (with ADR-0015 §3's asset step prepended as step 1) |
| Call 1, top level executes; a file without an entry is a valid program | §2.2, §2.5 |
| Call 2, imported modules do not run top-level code | §2.3 — the rule itself is **chapter 4's** (§13.1–§13.3); this chapter carries only the diagnostic ch4 §13.6 routed |
| Call 2's named cost, no computed module constants | Chapter 4's, narrowed by ADR-0021 §3 (ch4 §13.3) |
| Call 3, `persist` is the only mutable cross-frame channel; top-level bindings are immutable constants from the frame's view | §4.1.1, §2.6 |
| Call 4, `persist x: T = <expr>` initialises at its declaration site, cold start only | §4.3.1, §4.4.1, §4.4.2 |
| The entry takes zero parameters | **Narrowed on the record by ADR-0013 §1** and transcribed as §3.11 (no time parameter) plus §3.4 (the two runner-supplied values) |
| Simulation fixed at 60Hz; the step is a compile-time constant | §10.1 |
| No ambient simulation clock; elapsed time is a `persist` counter; wall-clock is the named exception | §10.6 |
| The framerate-dependent-physics bug class is deleted | **Dropped** — a consequence of §10.1, not a separate obligation. §16 |
| A frame that overruns runs late; catch-up is rejected | §10.4 |
| Render-rate decoupling routed to #28; the accumulator/interpolation mechanism | Decided by **ADR-0035 §1** and transcribed as §10.2. The mechanism itself is rationale. §16 |
| Amendment from #17: `persist` is entry-file only; the aggregate idiom; the field-path interaction | §4.2.1, §4.2.3, §4.2.4 |
| The idiom's `use "world" as World` | **Stale prose**, closed at ch1 §14.9. Not transcribed |
| `$.mem.heap` → bare `heap`, confined to `persist` initialisers in the entry file | §4.5.8 |

## 3. #17 — state-preserving reload

| Source clause | Where it landed |
|---|---|
| §1 dylib in dev, one static binary in release | §5.1, §1.6 |
| §1 `persist` memory belongs to the runner's process | **Corrected by ADR-0041 §5** and transcribed as §5.2 |
| §1 rejected: two execution tiers; unspecified mechanism | **Dropped** — rejected alternatives. §16 |
| §2 the reload set is `persist`; `persist` is entry-file only; multiple declarations are fine | §4.1.2, §4.2.1, §4.2.2 |
| §2 a library cannot own private cross-frame state | §4.2.3 (and ch4 §13.5) |
| §2 the one-named-root idiom is guidance, not a rule | §4.2.4 |
| §3 reload-safety is computed transitively and rejects a reachable image pointer; the error names the field path | §4.5.1 |
| §3 it is a diagnostic, not a type property | §4.5.2 |
| §3 the check is on in release too | §4.5.5 |
| §3 the three hazard kinds, two of which are already impossible | **Dropped** — the argument for the check, not the check. §16 |
| §4 program constants are reload-stable; the runner holds the blob | §4.5.9 |
| §4 a `persist` string still holds the old text after a reload | §4.4.4 |
| §5 a `persist` declaration may not reach a function value; no re-resolution by name | §4.5.3, §4.5.4 |
| §5 the substitute is a sum type plus exhaustive `match` | **Dropped** — guidance. §16 |
| §6 `unsafe` lifts the check per declaration; no second spelling | §4.5.6 |
| §7 the swap happens between frames, never mid-frame | §5.4 |
| §8 field-path reset with defaults from the re-evaluated initialiser, and its five bullets | §5.7 |
| §8 the two stated costs (drift leak, rename reads as delete-plus-add) | §5.8 |
| §8 the reset report's wording | Chapter 7's envelope; the operation is §6.4.2. **Routed**, §15 |
| §8 rejected: different `persist` structure in dev and release | §5.11 |
| §9 a reload re-runs top level, skipping `persist` initialisers; side effects repeat | §5.5 |
| §9 `persist` means the same thing in release | §4.1.3 |
| §9 a failed build never touches `persist` | §5.6 |
| The hand-off list to #19, #37, #11, #26 | Chapter 8's and the template's. **Routed**, §15 |
| ADR-0041 §6's passive-segment MUST, added where the swap is named | §5.9 |

## 4. #18 — what a faulted program is

| Source clause | Where it landed |
|---|---|
| §1 a fault is a detected bug and nothing else; the bug list | §6.1.1 |
| §1 an unhandled error value cannot reach the runtime; no unwinding, no handler stack | §6.1.2 |
| §1 a stale handle and NaN/infinity are not faults | §6.1.3 |
| §2 the faulting frame is abandoned; the entry is called again from the top; the two rejected resume points | §6.2.1, §6.2.2 |
| §2 the between-frames boundary is reused, not invented | §5.4, §7.3 |
| §3 every runtime-detected bug is pausable and nothing else is; the line is detection, not severity | §6.2.4 |
| §3 a corrupted arena is not a fault at all; the guarantee is void under `unsafe` | §6.2.5 |
| §4 fault semantics are identical in dev and release | §6.3.1 |
| §4 dev keeps the process alive; release exits non-zero | §6.3.2, §6.3.3 — the release half **restated by ADR-0041 §3** as abnormal termination by the host's convention |
| §5 `persist` is trusted as-is; no rollback; no marking it suspect | §6.2.3 |
| §5 the world is logically inconsistent, never memory-corrupt | §6.2.3 |
| §6 named operations, never evaluation; the four operations | §6.4.1, §6.4.2 |
| §7 `persist` is readable recursively by a structural walk running no game code | §6.4.3, §6.4.4 |
| §7 the three exclusions (`unsafe`, pool slots, depth/size limits) | §6.4.5 |
| §7 what is given up versus DragonRuby | **Dropped** — a consequence of §6.4.1, stated as a cost. §16 |
| §8 the six report contents, including *always concrete values* | §6.5.1 |
| §8 the seam: contents here, format and static half elsewhere | §6.5.3 |
| §9 the call chain is guaranteed in both modes; rejected dev-only chains | §6.5.2 |
| §9's cost sentence | **Amended by ADR-0041 §4** and transcribed with both target classes at §6.5.2 |
| §10 a fault before any frame runs; the simulation never starts; prior `persist` untouched | §6.2.6 |
| Testability list | Chapter 8's. **Routed**, §15 |

## 5. ADR-0013 — the drawing entry takes the screen target

| Source clause | Where it landed |
|---|---|
| §1 `frame(screen: !Target)`; a fetch is rejected; a top-level binding and `persist` are rejected; the `!` follows from #8 | §3.4, §3.6, §3.7 |
| §2 the parameter is spelled `screen`, because a second target will exist | §3.4 |
| §3 the rule: the entry's parameters are exactly the values only the runner can supply | §3.5 |
| §4 the target belongs to the entry that draws; the future-tense split | **Closed by ADR-0035 §1** — there is no second entry (§10.2). The counterfactual is not transcribed. §16 |
| §5 canvas and style are declared once at top level, immutable for the process's life | §9.3, §9.4 — the canvas *size* half **overturned by ADR-0032 §5** |
| §5's `CanvasDesc` and its `size` field | **Removed by ADR-0032 §5**. Not transcribed |
| §6 a fresh target each frame, transform reset to identity; the camera cost | §3.8, §3.9 |
| §7 top-level code cannot draw; the smallest visual program costs one entry | §2.7 |
| §8 `to_world` / `to_logical` ship as a pair | Chapter 6 §4.2.8 fixes the module and this chapter fixes the transform they invert: §9.16. **Routed** |
| §9 the real window size is not exposed | §9.15 |
| §9's fog item, choosing a canvas from the display | **Closed** by ADR-0032 §1: there is no canvas to choose. Not transcribed |
| ADR-0042 §6's amendment: `scratch: !Scratch` joins the list; the rule gains a member | §3.4, §3.5, §3.10 |
| #24 delta | Chapter 8's ledger. **Routed**, §15 |

## 6. ADR-0015 — assets are declared, not loaded

| Source clause | Where it landed |
|---|---|
| §1 an asset is a declaration whose name is fixed at compile time; no load call | §8.1 |
| §1 the path is relative to the declaring file; it may not cross a library boundary | §8.2 |
| §1 a declaration may appear in any module, and why #17's confinement does not transfer | §8.2 |
| §1 rejected: a load call; the compiler embedding the bytes | **Dropped** — rejected alternatives, except the operative half (*no load call*), which is §8.1. §16 |
| §2 one mechanism for every content type; the shader declaration is the odd one out | §8.5 |
| §3 every declared asset is resolved and decoded before top-level code runs | §8.3, §2.4 step 1 |
| §3 no program-visible loading state at all | §8.3 |
| §3 the stated cost: no progress bar for the declared set | **Dropped** — a consequence of §8.3, stated in the ADR as a cost. §16 |
| §4 a missing or undecodable declared file is a fault at top level | §8.4 |
| §4 asset resolution failure is a spec-named fault kind carrying location and path | §8.4, §6.1.4 |
| §4 rejected: substitute the debug pattern and carry on | §8.4's MUST NOT |
| §4 no new diagnostic machinery; no logging concept enters the language | Chapter 7's. **Routed**, §15 |
| §5 the decode-from-bytes calls; an allocator is a parameter; there is no decode-from-path | Chapter 6 §4.9.2. **Routed** with the *no path* half stated at §8.9 |
| §5 the declared set is runner-owned memory the program never names | §8.8 |
| §6 the checkerboard is demoted to a mandated value reached explicitly | Chapter 6 §4.9.10. **Routed**, §15 |
| §7 there is no debug clip, and ADR-0007's reasons are corrected | Chapter 6's. **Routed**, §15 |
| §8 editing a declared asset reloads it live at the next frame boundary; mandated | §8.6 |
| §8 a failed re-decode retains the last-good asset and does not fault | §8.7 |
| §8 the four reasons this is not #17 §8's merge problem | **Dropped** — rationale. §16 |
| §8 the new full-conformance property | Chapter 8's. **Routed**, §15 |
| §9 `Font` needs nothing `Image` does not | §8.5's one mechanism |
| §10 the #24 delta | Chapter 8's ledger. **Routed**, §15 |

## 7. ADR-0024 — concurrency is a non-goal, quiescence is a predicate

| Source clause | Where it landed |
|---|---|
| §1 no concurrency surface; the non-goal is stated; `parallel_for` is rejected on the same rule | §7.1 |
| §2 coroutines are rejected; the `persist` state machine replaces them; the cost is accepted | §7.6 (the rejection's operative half is §7.1) |
| §3 the quiescence predicate, verbatim | §7.2, with ADR-0026 §8's storage clause folded in |
| §3 where the predicate holds; the two badly-stated boundaries | §7.3 |
| §3 a faulted program is quiescent forever; reload can rescue it | §7.4, §6.3.4 |
| §3 the three citers of quiescence | §7.5 |
| §4 an implementation may use threads; the observable must be as if single-threaded | §7.7 |
| §5 `extern` is the escape hatch; ludo code may only be entered from the frame thread | §7.8, **widened to *task* by ADR-0041 §7** |
| §6 field-level `#align` is declined permanently | Chapter 3's layout section. **Routed**, §15 |
| §7 no keywords are reserved | §7.9 |
| Consequences list (#29 discharged, #17's set stays exact, #25's parked item closed) | **Dropped** — corpus bookkeeping. §16 |

## 8. ADR-0025 / ADR-0036 / ADR-0040 — the window and the display

| Source clause | Where it landed |
|---|---|
| 0025 §1 resize is a permission, always granted, not a setting | §11.6 |
| 0025 §1 borderless is runner discretion and the spec is silent | §11.13 |
| 0025 §2 fullscreen belongs to the player | §11.9 |
| 0025 §3 two mutators; the runner MUST provide an affordance; no key or chrome is fixed | §11.9 |
| 0025 §3 the mode is not the program's to cache | §11.9 |
| 0025 §4 the program can read the mode; #60's clause is overturned; zero bits about the hardware | §11.9; the *zero bits* guard is ADR-0028's and is chapter 6's admission-test territory. **Routed**, §15 |
| 0025 §5 ADR-0019 §3 is not triggered | **Dropped** — a finding about another ADR's applicability. §16 |
| 0025 §6 the mode survives relaunch; the spec names no file, format or location; the runner keys it per program | §11.10, §11.12 |
| 0025 §7 first launch is windowed; the developer gets no say; the cost is one toggle per lifetime | §11.2 |
| 0025 §7 *at the runner's default size* | **Phantom, already repaired** by ADR-0036 §2. §16 |
| 0025 §8 the blanket ban on reading backend state is withdrawn; no perimeter replaces it | Chapter 6's (ADR-0028 superseded the operative half). **Routed**, §15 |
| 0036 §1 half the question dissolved with the fixed canvas | **Dropped** — history. §16 |
| 0036 §2 the window opens maximal at the canvas aspect, within the work area, MUST NOT exceed it | §11.3 |
| 0036 §2 the four candidates and the dropped margin | **Dropped** — rejected alternatives. §16 |
| 0036 §3 the player-behaviour evidence | **Dropped** — evidence. §16 |
| 0036 §4 it is a size, not a maximized state; runner discretion how it is reached | §11.4 |
| 0036 §4 the size survives relaunch; a stale size clamps silently and persists clamped; §2 fires once; leaving fullscreen returns to the last windowed size | §11.5 |
| 0036 §4 position is not persisted; a runner may anyway | §11.7, **split by ADR-0040 §3** |
| 0036 §5 resize is free of aspect; the runner MUST NOT constrain the drag; 16:9 is the initial shape | §11.6 |
| 0036 §6 a window smaller than the canvas is ordinary; no floor, no minimum, no refusal to launch; `crisp` drops texels at `k < 1` | §9.8, §9.9 |
| 0036 §7 nothing is program-visible; no setter either; the audit of candidate uses | §11.1, §9.15 |
| 0036 §7 the resolution row dissolves | §11.14 |
| 0036 §8 resolution-for-performance is a real question and goes to the render scale | §11.15 |
| 0036 §9 P14 | Chapter 8's. **Routed**, §15. (This is §0.1's unstamped amendment of ADR-0035 §5.) |
| 0040 §1 the motivating case | **Dropped** — motivation. §16 |
| 0040 §2 a program reopens on the display it last occupied; primary on first launch; primary silently if detached; no ordinal | §11.8, §11.12 |
| 0040 §3 position within a display stays unpersisted; the split is not an overturn | §11.7 |
| 0040 §4 the two-mutator rule does not carry; no new MUST; a MUST is earned by the failure it prevents | §11.11 |
| 0040 §5 no call ships; the admission test is not run; the three refused shapes | §11.1; the refused shapes are **dropped** as rejected alternatives. §16 |

## 9. ADR-0030 / ADR-0031 / ADR-0032 / ADR-0039 — the canvas and the fit

| Source clause | Where it landed |
|---|---|
| 0030 §1 the gap is worse than unwritten; ADR-0011's pointer clause presupposes bars | §9.10 and §9.11 supply the precondition; the finding itself is **dropped** as corpus history. §16 |
| 0030 §2 *integer scale* was never a clause | **Phantom, already repaired.** §16 |
| 0030 §3 the canvas is a coordinate mapping, not an intermediate framebuffer; content rasterises at device resolution | §9.14 |
| 0030 §3 the accepted consequence for ADR-0010's blit clause | **Withdrawn by ADR-0034**, chapter 6 §4.11.7. Not transcribed |
| 0030 §4 the fit as arithmetic; aspect preserved; no independent axes, shear or stretch; the bars defined as a region | §9.5, §9.6, §9.10 — the formula **as restated by ADR-0031 §2** |
| 0030 §4 under fullscreen the window is the display | §9.13 |
| 0030 §5 integer scale on `crisp` | **Deleted by ADR-0031 §2.** Not transcribed |
| 0030 §6 content is clipped to the canvas rect; the bars are opaque black; program-drawable bars rejected | §9.11, §9.10 |
| 0030 §7 what it does not decide (the byte-exact blit) | Chapter 6 §4.11.7. **Routed**, §15 |
| 0030 §8 P13 | Chapter 8's. **Routed**, §15 |
| 0030 §9 zero delta; the `fit:` field and the player preference are rejected | §9.7; the delta is chapter 8's |
| 0031 §1 what integer scaling cost | **Dropped** — the argument for §2. §16 |
| 0031 §2 one formula, no `style` branch, no floor, no too-small-window case; ADR-0030 §6's clip is unaffected | §9.5, §9.8, §9.11 |
| 0031 §3 `crisp` keeps nearest sampling unconditionally; a token may not mean different things on different monitors; the stated fidelity cost | §9.9 |
| 0031 §4 stretch-to-fill does not follow; the similarity-transform grounding; `fit:` stays rejected | §9.6, §9.7 |
| 0031 §4 stretch as a player preference, recorded as open | **Closed and refused by ADR-0039.** §9.7 |
| 0031 §5 P13 restated | Chapter 8's. **Routed**, §15 |
| 0032 §1 a declared canvas makes the screen un-knowable to libraries | **Dropped** — the argument for §1's reversal. §16 |
| 0032 §2 why 1280×720 and not 1920×1080 | **Dropped** — argument for a transcribed number. §16 |
| 0032 §3 a non-16:9 game letterboxes itself inside the grid, and those regions are ordinary canvas | §9.12 |
| 0032 §3 pixel art is authored scaled; a game wanting another resolution has no recourse | **Dropped** — costs and workflow. §16 |
| 0032 §4 the fit survives with constants; ADR-0013 §9 untouched | §9.5, §9.15 |
| 0032 §5 `set_canvas` shrinks to the style token; the descriptor is removed; the name is kept | §9.3 |
| 0032 §6 `$.graphics.canvas_size` ships as a constant, not a call; it never engages the admission test | §9.2 |
| 0032 §7 the #24 delta (net) | Chapter 8's ledger. **Routed**, §15 |
| 0039 §1 what is refused: anisotropic stretch and zoom-and-crop; the fit remains one formula | §9.7 |
| 0039 §2–§5 the record correction, the tests, the arithmetic, *nobody is asking* | **Dropped** — argument and evidence. §16 |
| 0039 §6 the refusal is normative and binds the presentation, whoever asked for it | §9.7 |
| 0039 §7 no reopening trigger; a future proposal argues from scratch | §9.7 |
| 0039 §8 P13 already tests it; no new property | Chapter 8's. **Routed**, §15 |

## 10. ADR-0035 — the render ceiling and one present per entry

| Source clause | Where it landed |
|---|---|
| §1 the render rate equals the simulation rate, fixed at 60Hz, permanently; one call, one step, one image; no second entry, no `alpha`, no interpolation | §10.2 |
| §1 the ordering argument and the stated judder cost | **Dropped** — argument and a cost paid by players, not an obligation. §16 |
| §2 the three rejected alternatives | **Dropped** — rejected alternatives. §16 |
| §3 exactly one image is presented per returned frame entry; no double present, no skip | §10.3 |
| §4 pacing is runner discretion; vsync is not mandated; the environment/implementation line | §10.5 |
| §5 no #19 property, and the test that decided it | Chapter 8's. **Routed**, §15 |
| §5 *the contract's property list therefore ends at P13* | **Superseded by ADR-0036 §9** — see §0.1. Not transcribed |
| §6 P7 is strengthened; an overrunning frame yields one call and one late image | §10.4 |
| §7 the audio-ramp argument for the ceiling | **Dropped** — an independent argument for a transcribed rule; the ramp itself is chapter 6's. §16 |
| §8 the three sites of the stale future tense | **Dropped** — corpus bookkeeping, discharged in the corpus. §16 |
| §9 `frame` collapses to one referent | §10.7 |

## 11. ADR-0041 — the record stops assuming a process

| Source clause | Where it landed |
|---|---|
| §1 the pattern: the corpus wrote a host and called it the world | §1.7 |
| §2 the flush bound is the host's last guaranteed moment | Chapter 6 §8's storage surface (ch6 §1326). **Routed**, §15 |
| §3 the fault observable reads as abnormal termination by the host's convention; the browser's convention is fixed; the third site | §6.3.3 |
| §4 the call chain's price is per call at runtime on a shadow-stack target; the guarantee is untouched | §6.5.2 |
| §5 `persist` lives in runner-controlled memory that outlives the swapped code | §5.2 |
| §6 a build whose initialised data would overlap live `persist` is non-conforming; the passive-segment discharge | §5.9 |
| §7 re-entrancy is by task, not thread; the IndexedDB composition | §7.8; the composition is **dropped** as the argument for the rule. §16 |
| §8 tab discard defeats the rescue in practice; a scope sentence | §6.3.4 |
| §9 no new property, and why | Chapter 8's. **Routed**, §15 |

## 12. The clauses of ADR-0042, ADR-0045 and ADR-0048 this chapter reaches

Each of these ADRs belongs to another chapter. Only the clauses that bind the
runner, the entry or `persist` are transcribed here; the rest is that chapter's.

| Source clause | Where it landed |
|---|---|
| ADR-0042 §6 `Scratch` is the entry's second parameter; the rule gains a member; `!` is required; the arena's lifetime is one entry | §3.4, §3.5, §3.10 |
| ADR-0042 §6 rejected: a program-owned arena | **Dropped** — rejected alternative. §16 |
| ADR-0042 §7 the mandated allocator implementations are runner-owned, which is what keeps #17 §3's check passing; a user-written allocator in `persist` is rejected; vacuous in release | §4.5.7 |
| ADR-0042 §8 `heap` is legal only in a `persist` initialiser in the entry file | §4.5.8 |
| ADR-0042 §8 the prelude/facade categorisation and the `$.mem` respelling | Chapter 3's. **Routed**, §15 |
| ADR-0045 §1 the `persist name: TextBuf[32]` rendering | §4.3.2 — the contradiction's other side |
| ADR-0045 §8 the reference-program line | §4.3.3 — **amended to carry `= {}`**, and the ADR is stamped |
| ADR-0048 §7 allocation failure in a `persist` initialiser is a fault; the report names the declaration; the carve-out is by position | §4.6.1, §4.6.2, §4.6.3 |
| ADR-0048 §6 every allocating call is fallible | Chapter 3's. **Routed**, §15 |

## 13. #9 and the contradiction chapter 1 deferred

| Source clause | Where it landed |
|---|---|
| #9, every binding initialises at its declaration; no uninitialised memory in the safe layer | §4.3.1, resolving the site chapter 1 §14.5 and chapter 2 §9.6 deferred |
| #9, zero values are rejected | §4.3.2's second ground (ch2 §9.3) |
| ADR-0045 §1/§8's initialiser-free rendering | Not adopted; §4.3.2 records why, and §4.3.3 amends the line |

The repairs this resolution owes, all in this chapter's commit per ADR-0049:
chapter 1 §5.13, §14.5 and §15; chapter 2 §9.6, §17.6 and §21; this file and
`coverage/01-grammar.md`, `coverage/02-types.md`; `SOURCES.md`'s #9 row;
ADR-0045's stamp; and the reference program.

## 14. What chapter 6, 7 and 8 own and this chapter routes

Named here so the boundary is checkable rather than assumed.

- **Chapter 6** — the fullscreen pair, the render scale, `to_world`/`to_logical`,
  the decode calls and the debug value, the storage surface and the flush bound,
  wall-clock time, the audio ramp, and the admission test that guards any future
  query. This chapter spells only `set_canvas` and `canvas_size`, and only
  because ch6 §4.2.7 routed them here.
- **Chapter 7** — the envelope, codes, severity and sink of every fault and
  compile error this chapter names, including the reset report's wording.
- **Chapter 8** — every conformance property this chapter's sources mint (P13,
  P14, P7's strengthening, the asset-reload property, and the properties ADR-0035
  §5, ADR-0039 §8 and ADR-0041 §9 *declined*), the testability list of #18, and
  every `#24` delta.
- **Chapter 3** — `Scratch`, `Allocator`, `heap`, field-level `#align`, and the
  container rules `persist` leans on.
- **Chapter 4** — module top level, the asset path's library boundary, and the
  backend re-point that cites quiescence.

## 15. Routed, not dropped

Every row above marked **Routed** names a clause that is normative somewhere
else in this spec, not a clause nobody owes. The distinction matters because a
dropped clause is a decision and a routed one is a boundary.

## 16. Phantom clauses, and what was explicitly dropped

**Two phantom clauses were met, and both were already resolved before this
chapter began.** No new phantom was found, and **one clause was authored** —
§4.3.1, which is the resolution chapter 1 §14.5 required rather than a rule
invented here.

1. **Integer scale.** Cited by ADR-0025 §2 and by #60 as something ADR-0005
   mandated; ADR-0005 mandates no such thing, and the phrase occurs nowhere as a
   clause. ADR-0030 §2 found it and ADR-0031 deleted the concept outright. This
   chapter carries the single real `k` of §9.5 and no citation of the belief.
2. **The runner's default window size.** ADR-0025 §7 promised a size that
   existed nowhere in the spec — *a phrase written to finish a sentence about
   fullscreen*, in ADR-0036's own words. ADR-0036 §2 supplied it, and §11.3
   transcribes the supplied rule, never the promise.

A third, ADR-0011's pointer-in-bars clause, is not a phantom but the same family:
an operative clause whose precondition nothing stated. Chapter 6 §6.4.9–§6.4.10
carries the clause and this chapter's §9.10 and §9.11 supply the precondition, so
the two chapters agree by construction rather than by assumption.

**Explicitly dropped, by class:**

- **Rationale about a rejected alternative** — the bulk of every *dropped* row
  above. A rejection is transcribed **only** where it produces a rule a program
  or an implementation can violate: §3.6 (no fetch), §3.7 (no stored target),
  §5.11 (no dev/release `persist` split), §6.4.1 (no evaluation), §8.4 (no
  substitution), §9.7 (no stretch, no crop, no player exception), §10.4 (no
  catch-up), §10.5 (no vsync mandate), §11.14 (no resolution row).
- **Arguments for a number or a rule that is itself transcribed** — ADR-0032
  §2's three grounds for 720p, ADR-0031 §1's arithmetic, ADR-0035 §1's ordering
  argument and §7's audio-ramp argument, ADR-0036 §3's evidence, ADR-0039 §5's
  table. The rule is normative; the derivation is not.
- **Costs paid by a player or a developer rather than obligations on an
  implementation** — ADR-0035 §1's judder, ADR-0032 §3's authoring workflow,
  ADR-0015 §3's absent progress bar, ADR-0024 §2's coroutine loss. Named in the
  chapter only where the cost *is* a rule (§9.9's dropped texels, §5.8's drift
  leak).
- **Corpus and map bookkeeping** — ADR-0035 §8's three sites, ADR-0030 §1's and
  §2's findings about the corpus, ADR-0024's consequences list, ADR-0041 §1's
  pattern beyond the one sentence §1.7 needs, and every `#24 delta` and
  amendment list.
- **Positioning and front-matter** — #12 call 5 and its comparison set.

## 17. The reference program

ADR-0044 §7's last step. [`../reference/reference.ludo`](../reference/reference.ludo)
gains, in this chapter's commit:

- **`$.graphics.set_canvas(.crisp)` at top level** — the only legal site, once,
  immutable for the process's life (ch5 §9.3, §9.4).
- **`$.graphics.canvas_size`** replacing the `1280.0, 720.0` literal the program
  typed at chapter 6 (ch5 §9.2).
- **`extern image rock = "art/rock.png"`**, drawn — a declared asset with no load
  call, no readiness check and no path at any call site (ch5 §8.1–§8.3).
- **`persist name: TextBuf[32] = {}`** — ADR-0045 §8's line, carrying the
  initialiser §4.3.1 requires, with the comment naming why `= {}` is complete for
  this type and not yet writable for a user-declared one (#116).
- **`scratch` used** for the first time: a `List` built from the frame arena, not
  stored, not reset by the program (ch5 §3.10).
- **A header note** explaining what the top level and the frame entry *are*,
  which the program had been spelling since chapter 1 without a chapter that
  owned them.

Not exercised, deliberately: a fault, a reload, and the window. All three are
runner behaviour with no program-visible spelling — there is no line a program
can write that demonstrates them, which is itself the point of §11.1.

## 18. Induced holes opened by this chapter

Per `docs/agents/domain.md`, one line per hole opened by **writing a repair**
rather than by reading.

- **No spelling for a zero-filled fixed array** — opened by §4.3.1's requirement
  that every `persist` declaration carry an initialiser, which is the repair
  chapter 1 §14.5 asked for. It lands in **chapter 3** (the literal's arity
  against `[N]T`), is recorded as ch5 §13.1, and is filed as
  [#116](https://github.com/ludo-lang/ludo/issues/116).
