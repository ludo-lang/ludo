# LÖVE2D and DragonRuby: where batteries-included frameworks draw the line

**Research ticket:** [#27](https://github.com/adamico/ludo/issues/27) · **Serves:** [#1](https://github.com/adamico/ludo/issues/1) (cover-or-delegate) · **Date of evidence:** 2026-08-08

Issue #1's open "cover-or-delegate" question is whether ludo's standard library should cover the platform layer (graphics/audio/input) itself, or delegate it to something else. This ticket does not decide that. It gathers evidence from the two chosen peer comparisons — LÖVE2D and DragonRuby Game Toolkit (GTK), both batteries-included game frameworks rather than editor/scene-graph products like Godot — on how each actually drew that line, what it cost them, and where their users hit the edges.

---

## 1. Where is the framework/user line drawn?

### LÖVE's module surface

LÖVE's native modules live one-to-one under [`src/modules`](https://github.com/love2d/love/tree/main/src/modules) in the `love2d/love` source tree, each backed by C++ and exposed as a `love.*` Lua table:

`love.audio`, `love.data`, `love.event`, `love.filesystem`, `love.font`, `love.graphics`, `love.image`, `love.joystick`, `love.keyboard`, `love.math`, `love.mouse`, `love.physics`, `love.sensor`, `love.sound`, `love.system`, `love.thread`, `love.timer`, `love.touch`, `love.video`, `love.window` — plus the `love` core module itself that owns the boot/callback lifecycle ([love2d/love `src/modules`](https://github.com/love2d/love/tree/main/src/modules)).

Two things are conspicuously **not** in that list:

- **Networking.** LÖVE has no `love.net`. The [LÖVE wiki](https://love2d.org/wiki/love) documents no networking module at all; the community fills the gap with third-party Lua libraries — `luasocket` (bundled as a usable dependency but not a `love.*` module) and `lua-https` for HTTPS, both maintained outside the core repo. A user who wants multiplayer brings their own library.
- **UI toolkits.** There is no `love.ui`. Immediate-mode or retained UI (buttons, panels, text fields) is entirely community-supplied (SUIT, Slab, and similar), listed on [awesome-love2d](https://github.com/love2d-community/awesome-love2d) rather than shipped.

`love.physics` is the interesting middle case: it is **shipped**, but as a thin Lua binding over a vendored, unmodified **Box2D** (the wiki page [`love.physics`](https://love2d.org/wiki/love.physics) documents it as a near-direct wrap of Box2D's C++ API — bodies, fixtures, joints, contacts map almost 1:1 to Box2D's own vocabulary). LÖVE did not write a physics engine; it packaged someone else's and shipped the binding as if it were native. That is a third pattern beyond "ship" and "delegate": **vendor-and-bind**.

### DragonRuby's surface

DragonRuby's guest-facing API is much smaller and flatter, centered on the object handed into `tick`, documented in `docs.dragonruby.org` / the static docs bundle:

- `args.outputs` — the only way pixels reach the screen: `.sprites`, `.solids`, `.borders`, `.lines`, `.labels`, `.primitives`, plus `.sounds` for audio playback, all populated by `<<`-ing onto arrays each tick.
- `args.inputs` — keyboard, mouse, controller, touch, unified per tick.
- `args.state` — an open, persistent, hot-reload-surviving OpenStruct-like bag; there is no framework-imposed entity/component model, unlike Flame's ECS-ish component tree (see doc [05](05-dart-flutter-flame.md)).
- `args.grid`, `args.gtk` (the runtime object, also globally reachable as `$gtk`), `args.geometry`, `args.audio`, `args.easing` — small numeric/geometry helpers.
- `$gtk.dlopen` — the escape hatch to native code, discussed under Q4.

There is **no bundled Box2D-equivalent physics engine**. DragonRuby ships primitive collision helpers (`intersect_rect?` and similar geometry utilities) but rigid-body/joint physics is not in the box; users reach for community wrappers (e.g. Ruby ports/bindings community members have built around Box2D or write their own simple AABB resolution). This is the sharpest divergence from LÖVE on Q1: **LÖVE vendors a physics engine and calls it native; DragonRuby does not vendor one at all.**

Sample apps ship in bulk rather than as a stdlib surface: the [`dragonruby-game-toolkit-contrib`](https://github.com/DragonRuby/dragonruby-game-toolkit-contrib) repo and the in-download `samples/` tree (mirrored at [samples.dragonruby.org](https://samples.dragonruby.org)) are organized by numbered topic directories (e.g. `03_rendering_sprites`, `12_c_extensions`) — teaching by runnable example is DragonRuby's substitute for exhaustive API reference, which is itself a data point about what the framework considers "in the box": working code, not a module catalogue.

**Delegated by name, in DragonRuby's own docs:** networking is left to the user exactly as in LÖVE — no `args.net` exists. Native/C-level access is an explicit, named feature (`12_c_extensions` sample, `README.md` at [DragonRuby/dragonruby-game-toolkit-contrib](https://github.com/DragonRuby/dragonruby-game-toolkit-contrib/blob/main/samples/12_c_extensions/01_basics/README.md)) rather than an afterthought, discussed further under Q4.

---

## 2. What's in the box on first run?

### LÖVE: three callbacks, ~10 lines for a moving sprite

A LÖVE project is a directory with a `main.lua`; the runtime calls three well-known global callback functions each frame. The minimal moving-sprite program (assembled from the pattern documented across the LÖVE wiki's `love.load`/`love.update`/`love.draw` pages and forum examples, e.g. [Love2D Interactive Hello World](https://gist.github.com/004f9bf873fd5e19c502)):

```lua
function love.load()
  player = { x = 100, y = 100, speed = 200, img = love.graphics.newImage("player.png") }
end

function love.update(dt)
  if love.keyboard.isDown("right") then player.x = player.x + player.speed * dt end
  if love.keyboard.isDown("left")  then player.x = player.x - player.speed * dt end
end

function love.draw()
  love.graphics.draw(player.img, player.x, player.y)
end
```

That is **9 lines of actual logic** across the three callbacks. For that, "hello world" (`love.graphics.print("Hello World!", 300, 400)` inside `love.draw`) gets, for free: a resizable window, a fixed/variable timestep game loop already running, delta-time already computed and passed into `update`, image loading and GPU texture upload via `love.graphics.newImage`, and keyboard polling via `love.keyboard.isDown`. No project file, no build step — `love .` or dragging the folder onto the LÖVE executable runs it.

### DragonRuby: one method, comparable line count

DragonRuby's unit is a single `tick` method called 60 times a second, documented across the `docs.dragonruby.org` static docs and demonstrated in the `samples/03_rendering_sprites` and `02_player-movement`-style samples (mirrored in community write-ups such as [Building Games with DragonRuby, ch. 2](https://book.dragonriders.community/02-player-movement.html)):

```ruby
def tick args
  args.state.player_x ||= 100
  args.state.player_y ||= 100

  args.state.player_x += 5 if args.inputs.keyboard.right
  args.state.player_x -= 5 if args.inputs.keyboard.left

  args.outputs.sprites << { x: args.state.player_x, y: args.state.player_y, w: 100, h: 80, path: 'sprites/misc/dragon-0.png' }
end
```

**6 lines of logic**, slightly shorter than LÖVE's because state initialization, input, and output are all reachable off the single `args` parameter rather than three separate globals, and there's no explicit `dt` multiplication needed for a simple discrete-step move (DragonRuby's fixed 60 Hz tick makes `dt` implicit in most beginner code, at the cost of no built-in variable-timestep story for `tick` itself). Hello world is one line: `def tick args; args.outputs.labels << { x: 100, y: 100, text: "Hello World" }; end`. No file beyond `mygame/app/main.rb`; `dragonruby mygame` runs it, no compile step, and `args.state` persists across a hot-reload edit without extra code — the free-on-first-run list also includes hot reload itself, which LÖVE does not have built in (community `lurker`/`lume`-style reload libraries exist but are not first-party).

**Read together**, both frameworks converge on roughly the same minimal-code budget (single digits of lines) for a moving sprite, but the free list differs: LÖVE gives you an image/texture pipeline and a manual timestep; DragonRuby gives you a fixed tick, an auto-persisting state bag, and hot reload, and treats `dt`-based movement as something you opt into rather than something the callback hands you.

---

## 3. How is the platform layer bound?

### LÖVE: C++ core, thin Lua wrapper, LuaJIT host

LÖVE is a C++ application embedding LuaJIT, not a Lua library. The module tree under `src/modules` ([love2d/love](https://github.com/love2d/love/tree/main/src/modules)) is C++ implementing the real work (SDL2 for windowing/input, OpenGL/Vulkan/Metal for rendering depending on version, `stb_image`/FreeType for images and fonts, Box2D for physics, `libmodplug`/`ogg`/`mpg123` for audio decoding), with a wrapping layer (`wrap_*.cpp` files per module, per a maintainer's comment in [GitHub issue #1640, "C API"](https://github.com/love2d/love/issues/1640): *"Ideally all of the existing Lua wrapper code would call the new C APIs, to keep consistency"* — confirming the wrap layer is a distinct code layer between the C++ engine objects and the Lua-visible functions) that exposes each engine method as a Lua C function via the standard Lua C API, not primarily via LuaJIT's FFI. A forum thread on the topic ([Why is the FFI enabled?](https://love2d.org/forums/viewtopic.php?t=77617)) states plainly: *"LÖVE itself doesn't rely on the FFI either right now, though LuaJIT comes with the FFI unless you explicitly disable it when compiling LuaJIT."* So the binding surface is: **C++ engine → hand-written C-API wrapper functions → Lua**, with FFI available to user code as an *escape hatch*, not as LÖVE's own binding mechanism. Error propagation crosses this boundary as C++ exceptions caught and translated to Lua errors at the wrapper layer (per the same issue thread). There is no published exact C++-vs-Lua line-count split in the primary sources found; the architectural fact that is sourced is the layering itself, not a ratio.

### DragonRuby: mruby fork as guest, C "levels" as host

DragonRuby does not run standard CRuby; it runs a **fork of mruby**, the lightweight embeddable Ruby implementation. Per the community FAQ compiled from maintainer statements ([DragonRuby FAQ gist](https://gist.github.com/amirrajan/de261c81a8ba6d15374e299eb8aab429)) and a public architecture description, the runtime is explicitly layered:

- **Level 1** — a substantial portion of stock mruby.
- **Level 2** — DragonRuby's own optimizations to mruby, per target platform.
- **Level 3** — portable C libraries plus their Ruby C-extension bindings (this is the layer that talks to SDL2, audio decoders, and platform backends, structurally analogous to LÖVE's `wrap_*` layer but built on mruby's C-extension ABI rather than Lua's C API).

This has a direct, documented consequence for compatibility: because DragonRuby is not CRuby, *"many popular Ruby libraries are actually incompatible with the engine"* ([Thoughts on DragonRuby Game Toolkit](https://silverhammermba.github.io/blog/2022/02/19/dragonruby)) — the guest language looks like Ruby syntactically but is a different, smaller implementation underneath, so the RubyGems ecosystem is not available the way LuaRocks-style community libraries are broadly available to LÖVE (LÖVE runs on LuaJIT, which is much closer to reference Lua semantics than mruby is to CRuby). The host/guest boundary is therefore narrower in practical terms than LÖVE's: LÖVE's guest (Lua via LuaJIT) is close enough to stock Lua that most pure-Lua libraries "just work"; DragonRuby's guest (mruby fork) is a deliberately reduced Ruby that breaks compatibility with the wider gem ecosystem by design, trading ecosystem breadth for embeddability and startup/footprint characteristics suited to console targets.

---

## 4. What do users hit at the edges?

### LÖVE

- **Shaders are the sanctioned escape hatch for custom rendering effects**, via `love.graphics.newShader`, which compiles GLSL directly ([`love.graphics.newShader` wiki](https://love2d.org/wiki/love.graphics.newShader)). Friction is real and recurring on the forums: porting shaders from other engines/Shadertoy requires manually reimplementing runtime-provided uniforms and boilerplate the other tool supplied for free (forum thread ["Shaders: having trouble porting existing shaders"](https://love2d.org/forums/viewtopic.php?t=86345) — Shadertoy-style variables like buffers and `iTime` are not automatically wired up and users must hand-implement the equivalents; ["changing the GLSL Version"](https://love2d.org/forums/viewtopic.php?t=77428) — GLSL version selection requires an explicit `#pragma language glsl3` directive at the top of the shader, an easy-to-miss requirement that shows up as opaque compile failures for newcomers). A separate mobile-specific thread, ["Shader compilation issue on mobile"](https://love2d.org/forums/viewtopic.php?t=82838), documents shader portability breaking specifically on mobile GL drivers — the escape hatch itself is not uniformly portable across the platforms LÖVE otherwise abstracts away.
- **Native code / C++ integration** is possible but explicitly off the beaten path: a forum thread titled ["Can I use C++ code in my game?"](https://www.love2d.org/forums/viewtopic.php?t=78613) exists because the answer isn't in the manual — the sanctioned routes are LuaJIT's FFI (for calling into existing shared libraries) or building a custom fork/patch of LÖVE itself; there is no first-party plugin/native-extension ABI comparable to DragonRuby's `dlopen` convention. A companion thread, ["Automatic FFI binding generation from C headers"](https://love2d.org/forums/viewtopic.php?t=89488), exists precisely because hand-writing FFI declarations for a nontrivial C API is tedious enough that users built tooling to generate it.
- **Networking and UI**, as noted in Q1, are entirely off-framework; the recurring pattern in the ecosystem (`awesome-love2d`) is "install one of several community libraries," which means no single documented, stable API — friction is diffused across whichever library a given user picked, not concentrated on one official gap.

### DragonRuby

- **The escape hatch is named and first-party**: `$gtk.dlopen` loads a platform-specific shared library the user builds themselves, with the sample directory `12_c_extensions/01_basics` ([DragonRuby/dragonruby-game-toolkit-contrib](https://github.com/DragonRuby/dragonruby-game-toolkit-contrib/blob/main/samples/12_c_extensions/01_basics/README.md)) documenting the convention: DragonRuby looks for `mygame/native/$PLATFORM/ext.$PLATFORM_DLL_EXTENSION`, and everything the extension exposes becomes reachable under an `FFI::CExt` module. This is a **documented, supported feature**, not a forum workaround — a structurally different posture from LÖVE's FFI-as-tolerated-side-door.
- **Web/WASM is explicitly named as a target for this same mechanism**: per a search-derived summary of the docs, C extensions "can be used on WASM/Web builds, with `$gtk.dlopen` looking for assemblies under `mygame/native/emscripten-wasm`" — i.e. the escape hatch is asked to work identically (same API surface, different binary format) across desktop and browser targets, which is a nontrivial promise given how differently native code loading works in a browser sandbox versus a desktop OS. This document could not independently verify how robust that promise is in practice from primary sources beyond the docs' own description; treat it as a documented intent rather than a battle-tested one.
- **No bundled physics** (Q1) means the most common "edge" a DragonRuby user hits is not a missing escape hatch but a missing *module* — the standard complaint pattern in the community is "how do I get proper physics," answered by rolling your own AABB/impulse resolution or porting a small physics library into mruby-compatible Ruby, because CRuby gems (including existing Ruby Box2D wrappers) are not reliably mruby-compatible per the Q3 finding.

**Cross-cutting pattern**: both frameworks treat shader/GPU-level customization as "in the box, but leaky at the edges" (GLSL portability issues surface in both love.js/mobile and, by extension, any WASM target), and both push networking and UI entirely outside the frame. The meaningful difference is in *how* the native-code escape hatch is presented: LÖVE's is informal (FFI available because LuaJIT ships it, no first-party naming convention for a C extension), DragonRuby's is a named, documented, sample-backed feature (`dlopen` + `FFI::CExt` + a fixed native/ directory convention) that also claims to extend to the web target.

---

## 5. Versioning/stability cost

### LÖVE

LÖVE's graphics/audio surface has broken repeatedly across major versions, each time documented in first-party wiki version pages and `changes.txt`:

- **0.9 → 0.10**: `Canvas:clear` was removed/changed, breaking existing canvas-clearing code; forum threads (["\[solved\] Canvas:clear in 0.10.0"](https://love2d.org/forums/viewtopic.php?t=81702), ["What has Canvas:clear been replaced with in 0.10.0?"](https://love2d.org/forums/viewtopic.php?t=81401&start=10)) show users hitting this live, mid-project.
- **→ 11.0** ([wiki `11.0`](https://love2d.org/wiki/11.0)): a cluster of simultaneous breaking changes to the graphics API — `Shader:send`'s matrix variant flipped from column-major to row-major interpretation by default; `Canvas:newImageData` gained required `slice`/`mipmap` parameters; several `love.graphics`/`love.window`/`love.event` calls were changed to *error* if a Canvas was active where they previously didn't; stencil operations with an active Canvas now require an explicit `stencil=true` flag; `Mesh:setDrawRange` changed its parameter names/semantics from `min`/`max` to `start`/`count`; and, the single most invasive one, **all colour values changed range from 0–255 to 0–1** — a change that silently corrupts every existing `setColor`/`Canvas` colour call in a ported project rather than erroring, the worst kind of breaking change for an agent or human doing a mechanical migration.
- **→ 12.0** ([wiki `12.0`](https://love2d.org/wiki/12.0)): shader language default moved to GLSL3 (GLSL 3.30 / GLSL ES 3.00); `love.graphics.points` now requires `love_PointSize` to be set explicitly in custom vertex shaders; shape-drawing functions (`rectangle`, `polygon`, etc.) changed to supply texture coordinates to shaders where they previously didn't, which can change shader output for anyone relying on the old (undefined-texcoord) behavior; `setCanvas` now always clears auto-generated temporary depth/stencil buffers; `love.graphics.clear` stopped respecting the active scissor/color-mask state; and the **minimum runtime requirement was raised** to OpenGL 3.3+/OpenGL ES 3.0+/Vulkan 1.0+/Metal — a hardware-support floor change, not just an API change, meaning some machines that ran 11.x cannot run 12.x at all.

**Pattern**: LÖVE's major versions repeatedly break the graphics API in ways that require **manual code inspection to catch**, not just a compiler error — the colour-range change in particular can silently produce wrong-but-valid output. Maintaining `love.graphics`/`love.audio` as first-party surface has committed the LÖVE team to renegotiating fundamental representational choices (colour range, matrix majorness, shader language version) roughly once per major version across a fifteen-plus-year project.

### DragonRuby

Primary-source detail on graphics/audio-specific breaking changes was harder to establish independently (see Evidence quality below), but the documented shape of DragonRuby's versioning commits the maintainers to a comparable cost in a different place: **runtime-level churn rather than pure API churn**. Version 3.0 updated the "Level 1" runtime to mruby 3.0 and is described as supporting new Ruby-3.0-level language features with API performance gains of "30% to 200%" faster, while stating exceptions from breaking changes "should be fairly straightforward to fix" (per search-derived summary of DragonRuby's own devlog/roadmap material — this document could not fetch `docs.dragonruby.org/changelog.txt` directly due to tooling access limits; treat the specific percentages as reported, not independently verified). Because DragonRuby's guest language is itself a moving target (a maintained fork of mruby, upgraded to track newer mruby/Ruby-language versions), a DragonRuby major version can break user code at the **language semantics** level, not only the API level — a cost LÖVE does not carry, because LuaJIT's Lua semantics have been comparatively stable across LÖVE's own major versions. This is the mirror image of Q3's finding: DragonRuby's narrower, more controlled guest/host boundary (Section 3) is also the thing that ties DragonRuby's breaking-change surface to upstream mruby's own evolution, an axis of churn LÖVE does not share.

**What maintaining a platform surface commits to, stated plainly from both cases**: shipping `love.graphics`/`love.audio` as first-party means the maintainers own graphics-API design decisions (colour ranges, matrix conventions, shader language versions, minimum hardware floors) indefinitely, and each major-version renegotiation of those decisions is a mass migration event for every existing project — exactly the "flag day" cost pattern doc [05](05-dart-flutter-flame.md) found in Dart's null-safety migration, but recurring roughly once per major LÖVE release rather than once ever.

---

## 6. wasm/web export

### LÖVE: love.js, a direct Emscripten port, explicitly experimental

LÖVE's web path is **not** an official first-party deliverable of the `love2d/love` project; it is a third-party Emscripten port, `love.js` ([TannerRogalsky/love.js](https://github.com/TannerRogalsky/love.js/), with an actively updated fork at [Davidobot/love.js](https://github.com/Davidobot/love.js/) tracking LÖVE 11.5, and a separate `love-web-builder` project targeting LÖVE 12.0). Per the project's own framing (search-derived from the readme and forum thread ["Love.js - A Direct Emscripten Port"](https://love2d.org/forums/viewtopic.php?t=81736)), it *"differs from Motor or Punchdrunk in that it is not a reimplementation but a direct port of the existing LÖVE... code with very few code modifications"* — i.e. the strategy is compiling the same C++ engine to WASM via Emscripten and letting Emscripten's runtime shim the browser's callback-driven event loop underneath LÖVE's normal blocking-style main loop, rather than restructuring LÖVE's architecture around `requestAnimationFrame` natively. Documented consequences of that "compile the same loop, shim underneath" strategy:
- Threading (`pthreads`) is **disabled by default on itch.io embeds** with only experimental support available, because browser `pthread` support depends on `SharedArrayBuffer`/cross-origin-isolation headers many hosts don't set, and browsers have restricted it for security reasons.
- Memory sizing is a known rough edge: the port *"still needs a large-enough initial memory until figuring out how to properly wait for the memory to be sized-up before initialising all the file-system stuff"* — i.e. Emscripten's linear-memory model doesn't grow gracefully mid-boot the way native memory allocation does.
- Shaders require **stricter type-checking on the web** than desktop, because WebGL's GLSL ES is type-safe with no implicit conversions, surfacing a compatibility gap in exactly the customization layer flagged as friction-prone in Q4.
- Mouse grab/relative-mode requires an explicit user click to "lock" the mouse pointer — a browser security requirement (the Pointer Lock API needs a user gesture) that has no equivalent on desktop and must be handled specially in web builds.

**LÖVE's web export is therefore evidence that the browser's host-driven event-loop model is survivable by brute-force emulation** (Emscripten reimplements a blocking main loop on top of the browser's non-blocking one) **but leaks at every subsystem that assumed a native OS underneath it** — threads, memory growth, shader type strictness, and input-permission gestures all needed special-casing, and the port remains a third-party project with its own versioning lag behind mainline LÖVE rather than a first-party, continuously-supported target.

### DragonRuby: HTML5/WASM as a first-party, documented supported target

DragonRuby's own publishing tool produces an HTML5 build as one of its standard outputs alongside native platform binaries (per the search-derived docs summary: *"the dragonruby-publish tool outputs the HTML5 version along with files for other platforms"*), and — per Q4 — the native-extension mechanism (`$gtk.dlopen`) is documented as extending to `emscripten-wasm` specifically, meaning the web target is treated as a first-class member of the same platform list as desktop and console, not a separate experimental side-project. This is architecturally consistent with DragonRuby's Level 1/2/3 layering (Q3): because the mruby-based guest runtime and its C-level host are already built to be cross-compiled to multiple targets (the documented target list spans PC, Mac, Linux, Raspberry Pi, WASM, iOS, Android, and multiple consoles), adding WASM as one more Level-3 backend is a smaller architectural leap than it is for LÖVE, whose engine was designed around SDL2's native desktop assumptions before a browser target was retrofitted by a third party.

**What this forces on internal architecture, read across both**: neither primary source describes DragonRuby's tick loop being restructured specifically because of `requestAnimationFrame`'s host-driven callback model, but the structural precondition for a clean fit is visible in both cases — a runtime whose "loop" is already expressed as "call this function once per external tick" (DragonRuby's `tick(args)`, called by the host runtime each frame) maps directly onto `requestAnimationFrame`'s callback model with no impedance mismatch, whereas a runtime whose loop is `while true do update(); draw() end` (LÖVE's native `love.run`, documented on the [LÖVE wiki `love.run`](https://love2d.org/wiki/love.run) as an overridable blocking loop function) requires Emscripten's `emscripten_set_main_loop`-style trampoline to convert a blocking loop into a sequence of host-driven callback invocations. **The architectural lesson for ludo: a framework whose control-flow primitive is already "the host calls you once per frame" ports to the browser without restructuring; a framework whose control-flow primitive is "you call the host in a loop you own" needs an emulation/trampoline layer to reach the browser at all**, and LÖVE's experience — third-party port, disabled threading, memory-growth workarounds — is the visible cost of that mismatch.

---

## What this means for the cover-or-delegate decision

This section names costs and tradeoffs surfaced by the evidence above. It does not decide #1.

- **Shipping a graphics/audio platform surface commits to indefinite API-design ownership, and the two peers show that ownership has a recurring, not one-time, migration cost.** LÖVE renegotiated fundamental representational choices — colour range (0–255 → 0–1), matrix majorness, shader-language version, minimum hardware floor — at *each* major version (0.10, 11.0, 12.0; Q5), and at least one of those changes (colour range) is silently-wrong rather than compile-error-loud, the worst failure mode for anyone doing a mechanical port. A stdlib that covers the screen is signing up for this cadence indefinitely.
- **Vendoring a third-party engine and presenting it as native (LÖVE's `love.physics`/Box2D pattern) is a third option between "cover" and "delegate," and it inherits the vendored project's own versioning and bug surface** — the wiki page for `love.physics` documents an API that mirrors Box2D's own vocabulary closely enough that LÖVE's physics stability is partly hostage to upstream Box2D's stability, a dependency cost that is easy to overlook when the binding feels first-party.
- **Delegating (LÖVE's stance on networking and UI, DragonRuby's stance on physics) costs a documented escape-hatch story, and the two frameworks show it can be done informally or formally.** LÖVE's native-code escape hatch (FFI, or a full engine fork for anything deeper — Q4) is real but undocumented as a first-party convention; forum threads exist precisely because "can I use C++?" isn't answered in the manual. DragonRuby's (`$gtk.dlopen` + `FFI::CExt` + a fixed `native/$PLATFORM` directory convention, with a numbered sample and README) is a designed, supported feature. **If ludo delegates, the DragonRuby shape — a named, documented, sample-backed native-extension convention — is the cheaper cost to pay than the LÖVE shape**, where the escape hatch exists but the path to it is folklore.
- **The guest/host binding choice has a second-order effect on ecosystem availability that neither "cover" nor "delegate" fully escapes.** LÖVE's LuaJIT-hosted Lua is close enough to reference Lua that the broader Lua library ecosystem mostly works, which is *what makes delegation viable* for LÖVE (there's a UI library ecosystem to delegate to). DragonRuby's mruby fork is deliberately non-CRuby-compatible, which *narrows* what delegation can draw on (most RubyGems don't run), pushing DragonRuby's users toward writing things themselves or reaching for DragonRuby's own native-extension path more often. **A "delegate" decision is only as good as the guest language's compatibility with an existing library ecosystem** — a lesson directly relevant if ludo's own language design narrows compatibility with any existing corpus the way mruby narrowed Ruby's.
- **The browser target rewards an architecture that was already "host calls you" over one that was "you call the host."** LÖVE's `love.run`-owns-the-loop design needed a third-party Emscripten trampoline, with disabled threading and memory-growth workarounds surfacing as a direct consequence (Q6); DragonRuby's `tick(args)`-per-frame design maps onto `requestAnimationFrame` natively, and its publishing tool treats HTML5 as a first-party output, not a third-party port. **If ludo wants a credible wasm/browser story regardless of how it resolves cover-or-delegate, the control-flow primitive of whatever it ships (language runtime or delegated framework) should be "callback per frame," decided before the fact — the same "decide at design time, not after a corpus exists" lesson doc [05](05-dart-flutter-flame.md) drew from Dart's null-safety retrofit.**
- **The line itself is drawn differently by each peer, and neither line is "obviously correct":** LÖVE ships more native surface (20 modules, including a vendored physics engine) with a shakier, less-designed escape hatch; DragonRuby ships less native surface (no bundled physics, a much flatter `args` API) with a more deliberately designed escape hatch and a first-party web target. The evidence suggests these are correlated choices, not independent ones — the framework that covers less can afford to invest more design effort in making the edges leave cleanly, because there are fewer, more consistently-shaped edges to design for.

---

## Evidence quality — what is weak

- **DragonRuby's official docs site (`docs.dragonruby.org`) returned HTTP 403/404 to direct fetches during this research** (the root page 403'd, `/static/docs.html` 404'd). All DragonRuby claims sourced from "docs.dragonruby.org" in this document were obtained via web-search-engine summaries of that site's content, or via secondary/community sources (community book chapters, devlogs, GitHub-hosted samples/READMEs, a maintainer FAQ gist) rather than by directly reading the primary page. This is a materially weaker sourcing chain than the LÖVE material, most of which was read from `love2d.org/wiki`, GitHub, and the LÖVE forums directly or via search-engine extraction of those same primary URLs.
- **DragonRuby's `changelog.txt` could not be fetched directly** (`docs.dragonruby.org/changelog.txt` was identified as a search result but not independently opened), so the version-3.0 "30–200% faster" and mruby-3.0 upgrade claims in Q5 are reported as found via search-summary, not verified against the primary changelog text. The DragonRuby side of Q5 is therefore weaker evidence than the LÖVE side, which is sourced to first-party wiki version pages (`11.0`, `12.0`) and `changes.txt` on GitHub.
- **No exact LÖVE C++-vs-Lua line-count ratio was found in any primary source.** The claim in Q3 is architectural (a distinct C++ engine layer, a distinct hand-written wrapper layer, Lua as the guest) rather than quantitative; I did not run a line-counting tool against the `love2d/love` repository to produce a number, and no maintainer-published ratio was found in the sources searched.
- **DragonRuby's WASM/`emscripten-wasm` native-extension claim rests on a single search-summarized docs excerpt** (\"C Extensions can be used on WASM/Web builds, with `$gtk.dlopen` looking for assemblies under `mygame/native/emscripten-wasm`\") and was not corroborated by a second independent source or a working example. Treat as documented intent, not confirmed-in-practice.
- **The "requestAnimationFrame forces a callback-per-frame architecture" mechanical claim for DragonRuby is inferred, not sourced to a maintainer statement.** No primary source found explicitly says DragonRuby's `tick`-per-frame design was chosen *because of* or *validated against* the browser's event-loop model; the inference is drawn from the structural fit between the two, which is sound as architecture but not confirmed as the maintainers' stated reasoning.
- **LÖVE's web export sources are a mix of a project fork (`Davidobot/love.js`) and forum-thread search summaries**, not a single canonical up-to-date readme; `love.js` itself has forked lineages (`TannerRogalsky`, `Davidobot`, and the separate `love-web-builder` for 12.0), and I did not verify which fork is the current de facto standard recommended by the LÖVE project itself, if any is officially endorsed at all.
- **Forum-thread evidence throughout (Q4, Q5) is illustrative of documented friction patterns, not a systematic or statistically representative survey of complaint frequency.** Threads were found via targeted search queries designed to surface friction; this document makes no claim about what fraction of LÖVE or DragonRuby users hit these specific issues.
