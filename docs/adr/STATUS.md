# ADR status

Derived index of the ADR corpus: **given a source you are about to cite, is it still the
authority?** Answering that question by hand means proving a negative across the whole corpus,
which is the re-derivation cost
[ADR-0051](0051-the-spec-is-the-only-normative-surface-and-an-absorbed-adr-is-stamped.md) §2 names
as the cause of queue growth.

Sibling of [`SOURCES.md`](SOURCES.md), which covers issues that are still the sole authority for
something. This file covers ADRs.

## Two properties, both load-bearing

**Derived, not authored.** Every row restates an edge the tree already carries — an amendment
stamp, a supersession line, a `coverage/` entry. Never write a row that is true only here. If a
row and the ADR disagree, the ADR wins and the row is the bug.

**Temporary.** Per ADR-0051 §3 a landed spec chapter is the only normative surface, and each
chapter that lands converts its sources to `absorbed-by-ch-N`. When the last chapter lands this
file says only that the spec is normative, and is deleted.

## The columns

`ADR | status | one-clause gist | declared edges`

`status` is one of `live`, `amended` (it carries stamps of its own and is still authoritative for
everything else), `superseded-by-ADR-N`, or `absorbed-by-ch-N`. Note that `amended` does **not**
mean "do not cite" — it means read the stamps first.

Assembled by the one-off bootstrap sweep of 2026-08-15 (seven shard passes plus per-pair
verification of every suspected unstamped amendment). Per ADR-0051 §6 the sweep does not recur;
new rows are added by the author who lands the ADR.

## Rows

```
ADR-0001 | live | own and write the platform layer against OS/GPU APIs, not bind SDL/GLFW | -
ADR-0002 | amended | renderer API adopts the WebGPU model, implemented ourselves over D3D12/Metal/Vulkan (web deferred) | amended-by:0037
ADR-0003 | amended | two shader authoring paths (shader ludo dialect, .wgsl asset) lower into one shared shader IR | amended-by:0008
ADR-0004 | amended | spec mandates a thin ~nine-call drawing facade; renderer and platform layer stay delegated/unmandated | amended-by:0006, amended-by:0007, amended-by:0009, amended-by:0010, amended-by:0022, amended-by:0033, amended-by:0035, amended-by:0037
ADR-0005 | absorbed-by-ch-6 | drawing facade surface fixed: top-left Y-down origin, straight alpha/sRGB, an opaque target carrying style/transform/canvas | amended-by:0007, amended-by:0009, amended-by:0010, amended-by:0013, amended-by:0022, amended-by:0030, amended-by:0031, amended-by:0032, amended-by:0034
ADR-0006 | absorbed-by-ch-6 | eight forbidding/requiring rules (R1-R8) constrain the platform-layer API for browser/reload compatibility, and mandate a voice-based audio addition | amended-by:0007, amended-by:0014, amended-by:0033, amended-by:0034
ADR-0007 | absorbed-by-ch-6 | audio facade surface spelled: playing voices (not render-and-sum), no voice cap, mono-in/stereo-out constant-power pan, naming rules, ten functions/five types, ramping, envelopes, clip as procedural path, push/free_space escape hatch, cursor, tolerances, noise/filter formulas | amended-by:0009, amended-by:0015, amended-by:0016, amended-by:0022, amended-by:0033
ADR-0008 | absorbed-by-ch-6 | shader ludo is a strict subset reached by an extern shader declaration, never an ingested asset | amends:0003, amended-by:0037
ADR-0009 | absorbed-by-ch-6 | fixes the drawing facade's call spellings under `$.graphics` — verb-first, three verbs (fill_/stroke_/draw_), a descriptor per call | amended-by:0010, amended-by:0013, amended-by:0015, amended-by:0033, amended-by:0043, amended-by:0047
ADR-0010 | absorbed-by-ch-6 | `fill_` takes a `Paint` not a `Color`, collapsing the three verbs to two and deleting `draw_` | amends:0009, amended-by:0015, amended-by:0022, amended-by:0030, amended-by:0033, amended-by:0034, amended-by:0043
ADR-0011 | absorbed-by-ch-6 | the input facade is a virtual controller over ordinal buttons, with raw keyboard/mouse/pad as the escape hatch | amended-by:0013, amended-by:0030, amended-by:0033, amended-by:0043
ADR-0012 | absorbed-by-ch-6 | the wrap-the-facade idiom is blessed and made free by a normative forwarding-function guarantee | amended-by:0033, amended-by:0034
ADR-0013 | absorbed-by-ch-5 | the drawing entry takes `screen: !Target`, and the logical canvas/style are declared once at top level via `set_canvas` | amends:0009, amended-by:0030, amended-by:0032, amended-by:0033, amended-by:0035, amended-by:0036, amended-by:0042
ADR-0014 | amended | a library is a directory that declares its own name via a `library` line; `use` is file-scoped and non-transitive | amends:0006, amended-by:0027
ADR-0015 | absorbed-by-ch-5 | assets are declared at compile time via `extern` and resolved/decoded before top-level code runs, with no program-visible loading state | amends:0007, amended-by:0033, amended-by:0042
ADR-0016 | absorbed-by-ch-6 | blessed math set ships as concrete f32 vec2/3/4 and square mat2/3/4 plus Radians/Seconds/SampleFrames quantities, with no SIMD lowering mandate | supersedes:0007 (cursor signature, `int` to `SampleFrames`)
ADR-0017 | live | the ceiling above a bare function pointer is interface-bounded generics only; closures, anonymous function literals and UFCS on function values are all rejected | -
ADR-0018 | absorbed-by-ch-7 | compiler and faulted-program diagnostics share one structured stream (mandatory newline-delimited JSON) with normative fields, stable spec-owned codes and closed severity | amended-by:0023, amended-by:0041
ADR-0019 | live | each claimant of a spec-defined root conforms to the spec alone, never to agreement with another claimant, and `extern` remains the only program-visible door below `$.` | -
ADR-0020 | live | the bootstrap compiler is written in C for the self-hosting port and reproducible-bootstrap-chain reasons, and ludo self-hosts only after v1 ships | -
ADR-0021 | live | const evaluation folds only literals/constants/arithmetic/comparison/bitwise/casts, forbids calls and loops so non-termination is impossible, and introduces the `const` keyword | -
ADR-0022 | absorbed-by-ch-6 | spec promises only what is derivable from a backend's own specs, so falsified graphics-exactness claims are deleted or restated with implementation-defined boundaries | amends:0004, amends:0005, amends:0007, amends:0009(unchanged), amends:0010, amended-by:0034, amended-by:0037, amended-by:0047
ADR-0023 | absorbed-by-ch-7 | the spec never mandates a warning-severity diagnostic, so warnings live only in vendor space with no promotion-to-error surface and no in-source silencing | amends:0018(§4 pointer, stamped retroactively by ch7)
ADR-0024 | absorbed-by-ch-5 | concurrency is a stated non-goal and quiescence is defined as a state predicate over stacks, extern calls and persist/asset state, with coroutines rejected in favour of a persist state machine | amended-by:0026, amended-by:0041
ADR-0025 | absorbed-by-ch-5 | fullscreen is a player preference the runner and the program may both read and set, split out from resize (a permission) and bordered/borderless (runner discretion) | amended-by:0028, amended-by:0030, amended-by:0031, amended-by:0033, amended-by:0036, amended-by:0038, amended-by:0040
ADR-0026 | absorbed-by-ch-6 | a program gets a minimal byte-sink surface, extern storage slots plus $.storage.read/write, since runner-owned persistence cannot carry program-defined meaning | amends:0024(quiescence predicate gains resolved storage contents), amended-by:0041
ADR-0027 | live | the compiler is handed a complete name-to-location mapping and performs no search, with the runner building a default mapping by one-level scanning of libs/ when none is supplied | amends:0014(§4 clarifying line on $. never being a mapping key)
ADR-0028 | amended | the admission test (reformulate, guard the return against hardware magnitude, then require strict spec-mandated necessity) is the perimeter for any future $. query reading backend state | supersedes:0025(§8, operative half), amended-by:0030, amended-by:0032, amended-by:0038, amended-by:0040
ADR-0029 | amended | `#explicit` marks a module and forbids only spellings that omit a name, per a closed list of four | amended-by:0033
ADR-0030 | absorbed-by-ch-5 | the fit is a uniform-scale-plus-translation coordinate mapping (not an intermediate framebuffer), with bars opaque and content clipped | amended-by:0031, amended-by:0032, amended-by:0034, amended-by:0038, amended-by:0039
ADR-0031 | absorbed-by-ch-5 | integer scaling on `crisp` targets is deleted under the performance-above-fidelity ordering, leaving one real-`k` fit formula | amends:0030, amended-by:0036, amended-by:0038, amended-by:0039
ADR-0032 | absorbed-by-ch-5 | the logical canvas is fixed at 1280x720 as a language constant, overturning the caller-declared canvas | amends:0005, amends:0013, amended-by:0033
ADR-0033 | live | reconciles stale stamps and count deltas across the corpus (stamps are amendable content; companion count fixed at 55 functions) | amends:0004, amends:0006, amends:0007, amends:0009, amends:0010, amends:0011, amends:0012, amends:0013, amends:0015, amends:0025, amends:0029, amends:0032
ADR-0034 | absorbed-by-ch-6 | withdraws the byte-exact blit and ADR-0022 §4's copy-not-quad condition, replacing them with a texel-selection rule under nearest sampling | amends:0005, amends:0006, amends:0010, amends:0012, amends:0022
ADR-0035 | absorbed-by-ch-5 | accepts a permanent 60Hz render ceiling with no render/simulation split, and mandates exactly one presented image per frame entry | amended-by:0036(§5's "list ends at P13", found by ch5)
ADR-0036 | absorbed-by-ch-5 | first launch opens a maximal 16:9 window sized to the work area, and its size (not position) persists across relaunch | amended-by:0038, amended-by:0040, amends:0025, amends:0031, amends:0013, amends:0035
ADR-0037 | live | backend admissibility has no enumerated roster, only the normative surface as floor, and shader authoring becomes a third conformance rung above core/full | amends:0002, amends:0004, amends:0022, amends:0008, discharges:0033§10
ADR-0038 | absorbed-by-ch-6 | ships a player-set render scale s∈[0.25,1.0] as the spec's one performance escape hatch, rasterising at s·k and upscaling to k | amends:0030, amends:0025, discharges:0033§9, amends:0028, amends:0031, discharges:0036§8
ADR-0039 | absorbed-by-ch-5 | refuses stretch-to-fill and zoom-and-crop outright, with no player-preference exception to the uniform-fit rule | amends:0030, amends:0031
ADR-0040 | absorbed-by-ch-5 | a program reopens on the display it last occupied, runner-tracked with no ordinal and no program-facing surface | amends:0025, discharges:0028§7, amends:0036§4(splits)
ADR-0041 | absorbed-by-ch-5 | corrects six process-assuming statements (flush bound, abnormal-termination observable, call-chain cost, persist location) for non-process hosts, and adds a data-segment-overlap MUST plus frame-task re-entrancy | amends:0026, amends:0024, amends:0018, amends:issue17, amends:issue18, amends:issue19
ADR-0042 | amended | an allocator is a plain struct of non-capturing function pointers, split into storable Allocator and non-escaping Scratch, with mandated implementations kept runner-owned so reload stays safe | amended-by:0047, amended-by:0048
ADR-0043 | amended | a string is a literal view into the runner's constant blob and constructed text is bytes, not a string | amended-by:0045, amended-by:0047
ADR-0044 | amended | the map's destination becomes an artifact criterion (a prototype runs the reference program) and the corpus is consolidated into a chaptered spec | amended-by:0046, amended-by:0049, amended-by:0051
ADR-0045 | amended | a user-declared generic parameter may be a compile-time usize integer, fenced to bare array-length use with no arithmetic in type position | amends:0043(§5, declaration respelled TextBuf[N: usize]), amended-by:spec-ch5(§8's line gains `= {}`)
ADR-0046 | live | the repository is transferred to ludo-lang/ludo as one public Zlib-licensed monorepo, and the split-repo sync workflow is deleted | amends:0044(§5 topology corrected)
ADR-0047 | amended | a function may return a []T iff the view derives from the call's receiver or the constant blob, and mutating or moving the origin with ! kills the view | amended-by:0052(§5's open marker-grammar note is stale — closed by #101 at ch1 §7.10a), reverses:#15(Q10 "never returned"), collapses:0043(§9 into general rule), restamps:ADR-0009§111/ADR-0022(get_pixels(image: !Image)), clarifies:ADR-0042(§4)
ADR-0048 | amended | every allocating call is fallible (push/insert/clone/from), the mandated container set is List+Map, and a persist initialiser's allocation failure faults instead of rescuing | extended-by:0050, narrows:ADR-0042(§3, persist position only), discharges:0047(§9 hand-off)
ADR-0049 | live | a ticket that closes a spec chapter's marked gap is not done until it repairs the chapter text, coverage rows and gap list in the same commit | amends:0044(§8 marked-gap bullet extended), edits:docs/agents/domain.md(directly, third stamp sibling)
ADR-0050 | live | List removal ships as remove/swap_remove/pop/truncate, Map.remove tombstones preserving insertion order, and a descending range operator >.. with reified Range/RevRange types is added | extends:0048(§10 departure surface), reaffirms:0047(§3), reverses:#15(Q24 / chapter1 §7.7 "no inclusive range" and "ranges are not grammar")
ADR-0051 | live | a landed spec chapter is the only normative surface, and the ADR it absorbs is stamped as history in the chapter's same commit | amends:0044(per 0044's own amendment stamp)
ADR-0052 | live | the origin of a view is the container its expression names, disjointness is never reasoned about in any dimension, and pool.each() is deleted as a call the language does not have | reverses:#25(§9 column exclusivity, in full), generalises:0047(§3 to both range and field dimensions), amends:0047(§5 stale marker-grammar note), upholds:#101(ch1 §7.10a fixes the field spelling too)
ADR-0053 | live | the raw pointer is ^T with no arithmetic, no many-pointer and no null, dereference is the suffix p^ and address-of is prefix &x over places only, costing one core operator as ch1 §13.9.1 crossing 3 | spells:#8(call 6's distinct greppable type, and its &x), closes:ch3(§16.5 decline, §20 bullet), upholds:#9(no null carve-out, ch2 §9.1.1), upholds:#11(Q3, ! stays a property of the place), defers:#29(view representation, ch3 §16.2.3), splits-out:#118(uninitialised memory)
```
