# Which library the delegated platform layer uses

Research for [#132](https://github.com/ludo-lang/ludo/issues/132).

[ADR-0056](../adr/0056-the-platform-layer-is-delegated-to-a-third-party-library.md)
reversed [ADR-0001](../adr/0001-own-the-platform-layer.md) and deliberately did
not name the library. This document names candidates and runs them against the
test the ticket sets: **chapter 6 §10.2–§10.12, the eight rules**, plus the three
things ADR-0056 §3 rests on — the runner keeps the frame loop, the mandated
facade sits on top unchanged, and
[#12](https://github.com/ludo-lang/ludo/issues/12)'s no-bundled-engine ruling
still binds.

Vocabulary is `CONTEXT.md`'s. A **platform layer** is the triple window/input,
renderer, audio device. A **backend** is one platform's implementation of that
triple. An **engine** — frame loop, component model, sprites, animation,
collision, camera — is the tier above, and stays out.

## Summary of verdicts

| Rule (ch6 §10) | SDL3 | raylib | sokol |
| --- | --- | --- | --- |
| R1 nothing may block (§10.2) | **Pass, with care** | **Fail, three ways** | Pass, with care |
| R1 loading before any frame (§10.3) | Pass | Pass | Pass |
| R2 C ABI seam of function pointers (§10.4) | Pass | Pass | Pass |
| R3 backend is a module, no `#if` (§10.5) | Ours, not the library's | Ours, not the library's | Ours, not the library's |
| R4 identical unavailable-backend signature (§10.6) | Ours | Ours | Ours |
| R5 push-only audio, nothing calls back (§10.7) | **Pass** | **Fail** | Pass |
| R6 copies, reports, never demands (§10.9) | **Partial** — no play cursor | **Fail** — no play cursor, no free-space count in frames | **Partial** |
| R7 request-and-poll file IO (§10.11) | **Pass** — `SDL_AsyncIO` | **Fail** — `fopen`/`fread` | Not supplied |
| R8 runner-owned ring, drain-then-silence (§10.12) | Ours, reachable | Not reachable | Ours, reachable |

Three rules — R3, R4, R8 — are obligations on **our** code and no library
supplies or blocks them; they are marked "ours" rather than scored. What a
library can do is put them out of reach, and §4.4 records the one case where that
happens.

## The shape of the answer

**SDL3.** raylib fails R1 in its default configuration at three separate points,
fails R5 by design (its own header comments describe audio callbacks as running
"on audio threads"), and supplies no play cursor for R6. Those are not incidental
— they follow from raylib being one rung higher than the platform layer, which is
also why it collides with the mandated facade (§5) and why it carries engine-tier
API that #12 excludes (§5.3).

SDL3's cost is real but bounded and lands in one place: **the WebGPU-model
adoption of ADR-0002 survives only in part** (§7), and **SDL_GPU does not reach
the browser today** (§8), so the deferred web backend runs over SDL's 2D render
API rather than the same GPU path as desktop.

---

## 1. Candidates considered

- **SDL3** — <https://wiki.libsdl.org/SDL3/FrontPage>. Window/input, audio, a 2D
  render API, and since SDL 3.x a GPU API.
- **raylib** — <https://www.raylib.com>, source at
  <https://github.com/raysan5/raylib>. Version inspected: `src/raylib.h`
  self-identifying as `raylib v6.1-dev`.
- **sokol** — <https://github.com/floooh/sokol>. Header-only; `sokol_app.h`,
  `sokol_gfx.h`, `sokol_audio.h` as separable pieces.
- **GLFW + miniaudio** — considered only as a component set; treated in §9.
- **wgpu-native** — treated in §7.3 and §10; it is a renderer component, not a
  platform triple, so it cannot be the answer to this ticket alone.

---

## 2. R1 — nothing in the platform-layer API may block

Ch6 §10.2: *"Nothing in the platform-layer API may block. Every wait is a poll or
a host-driven callback. A blocking read is not permitted in loading either."*

R1 is checked at four sites: the present path, event pumping, audio, and file IO.

### 2.1 SDL3 — the present path blocks by default, and has a documented non-blocking form

`SDL_AcquireGPUSwapchainTexture`
(<https://wiki.libsdl.org/SDL3/SDL_AcquireGPUSwapchainTexture>) states: *"In
VSYNC present mode (which is the default) this function may block on vblank."*
The same page documents the non-blocking outcome: when too many frames are in
flight the function *"will fill the swapchain texture handle with NULL and return
true. This is not an error."*

That is exactly R1's required shape — a poll that reports "not yet" rather than a
wait. The page also carries a counter-recommendation: *"You should use
`SDL_WaitAndAcquireGPUSwapchainTexture()` unless you know what you are doing with
timing,"* on the ground that accumulating command buffers grows memory. R1 means
we are in the "know what you are doing" case: the runner must take the polling
form and manage its own in-flight count.

**Verdict: pass, conditionally.** The blocking call is the default and the
documented recommendation; the non-blocking call exists and is normatively
described. Nothing is unreachable. This must be written down as a constraint on
the runner, because the natural code path is the forbidden one.

### 2.2 raylib — blocks in `EndDrawing`, unconditionally in the default build

`src/rcore.c`, `EndDrawing()`:

```c
    // Wait for some milliseconds...
    if (CORE.Time.frame < CORE.Time.target)
    {
        WaitTime(CORE.Time.target - CORE.Time.frame);
```

and `WaitTime()` in the same file:

```c
    #if defined(_WIN32)
        Sleep((unsigned long)(sleepSeconds*1000.0));
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
        ...
        while (nanosleep(&req, &req) == -1) continue;
    #endif
    #if defined(__APPLE__)
        usleep(sleepSeconds*1000000.0);
    #endif
```

The header's own comment for the escape hatch states the coupling plainly
(`src/raylib.h`, above `SwapScreenBuffer`): *"By default EndDrawing() does this
job: draws everything + SwapScreenBuffer() + manage frame timing +
PollInputEvents()."*

There is a compile-time escape, `SUPPORT_CUSTOM_FRAME_CONTROL` in `src/config.h`:

```c
#ifndef SUPPORT_CUSTOM_FRAME_CONTROL
    // Support custom frame control, only for advanced users
    // By default EndDrawing() does this job: draws everything + SwapScreenBuffer() + manage frame timing + PollInputEvents()
    // Enabling this flag allows manual control of the frame processes, use at your own risk
    #define SUPPORT_CUSTOM_FRAME_CONTROL    0       // Disabled by default
#endif
```

Setting it exposes `SwapScreenBuffer()`, `PollInputEvents()` and `WaitTime()` as
separate calls and strips the wait out of `EndDrawing`. So the block is
**avoidable**, but only by building raylib in a non-default configuration
described by its own source as *"only for advanced users… use at your own risk."*
That is a fork-flavoured dependency, not a supported API mode.

### 2.3 raylib — file loading is `fopen`/`fread`

`src/rcore.c`, `LoadFileData()`:

```c
        FILE *file = fopen(fileName, "rb");
        ...
                size_t count = fread(data, sizeof(unsigned char), size, file);
```

`LoadTexture`, `LoadImage`, `LoadWave`, `LoadMusicStream` all sit on this. Ch6
§10.3 lets a preload at entry-file top level be the runner's problem, so the
default path survives R1; **§10.11 (R7) does not survive**, because streaming is
in-frame by definition and raylib has no request-and-poll IO. See §2.6.

raylib does expose `SetLoadFileDataCallback` (`src/raylib.h`), which lets us
substitute our own loader. That is a mitigation for R7 only in the sense that we
would write the async IO ourselves — i.e. the library supplies nothing here.

### 2.4 SDL3 — audio does not require a callback

`SDL_OpenAudioDeviceStream` (<https://wiki.libsdl.org/SDL3/SDL_OpenAudioDeviceStream>)
documents the callback parameter as: *"Can be NULL, in which case the app will
need to call `SDL_PutAudioStreamData` or `SDL_GetAudioStreamData` as necessary."*
With NULL, *"the app is expected to queue audio data for playback."*

The category overview (<https://wiki.libsdl.org/SDL3/CategoryAudio>) describes the
push model as primary: *"An app opens an audio device and binds any number of
audio streams to it, feeding more data to the streams as available,"* with the
callback form described as the optional alternative that *"maps pretty closely to
the SDL2 audio model."*

Push from the frame entry, no callback, no thread. This is R5 (§10.7) satisfied
by the library's own default model, not by working around it.

### 2.5 raylib — audio callbacks run on the audio thread, by the library's own account

`src/raudio.c`, the `AudioBuffer` struct field:

```c
    AudioCallback callback;         // Audio buffer callback for buffer filling on audio threads
```

and the miniaudio device callback that drives it:

```c
static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount)
{
    ...
    // Using a mutex here for thread-safety which makes things not real-time
    // This is unlikely to be necessary for this project, but it can be reconsidered
    ma_mutex_lock(&AUDIO.System.lock);
```

raylib mixes on miniaudio's device thread, under a mutex, walking its own
`AudioBuffer` list. `SetAudioStreamCallback` installs a callback invoked from
there.

There is a push path — `UpdateAudioStream(stream, data, frameCount)` plus
`IsAudioStreamProcessed(stream)` — and a ludo runner that used only those would
never be called back. But **the mix is still raylib's, on raylib's thread**. Ch6
§10.7 requires *"mixing happens in the frame entry, on the main thread, in
ordinary ludo code"*; §10.12 (R8) requires a **runner-owned** ring that survives
reload, and drain-then-silence after a fault. With raylib the ring is
`AudioBuffer`'s double sub-buffer inside `raudio.c` and the mix is raylib's, so
neither R8 guarantee is ours to make. **R5 fails and R8 becomes unreachable
through the supported API.**

### 2.6 File IO: SDL3 supplies async, raylib does not

SDL3's async IO category (<https://wiki.libsdl.org/SDL3/CategoryAsyncIO>) states:
*"the functions that request I/O never block while the request is fulfilled."*
The three pieces are `SDL_LoadFileAsync`, `SDL_AsyncIOQueue`, and
`SDL_GetAsyncIOResult`, the last documented as checking *"if any task is finished
without blocking."*

That is R7 (§10.11) — *"File IO is request-and-poll, never a blocking read"* —
supplied verbatim by the library.

**Could not verify:** whether `SDL_AsyncIO` is implemented on the Emscripten
target, and if so what it maps to. The wiki category page does not enumerate
per-platform backends and I did not read `src/file/async/`. This matters because
the browser has no synchronous read at all; SDL3's async surface is the right
*shape* for the web whether or not the current implementation covers it.

raylib has no async IO surface. `SetLoadFileDataCallback` is a hook for us to
supply one.

---

## 3. R2, R3, R4 — the seam, the module, the signature

Ch6 §10.4 requires the backend seam be *"one struct of non-capturing function
pointers"*; §10.5 forbids conditional compilation and makes a backend a module
selected at build; §10.6 requires the unavailable-backend error to have the same
signature on every target.

**All three are properties of ludo's own binding layer, not of the library.**
Every candidate here is a C library with a C ABI, which is what
[#12](https://github.com/ludo-lang/ludo/issues/12)'s binding-first decision
already assumed and what ADR-0056 §4 confirms. A struct of function pointers
filled from SDL3 symbols and a struct filled from raylib symbols are the same
struct.

One asymmetry worth recording. R3 says target selection happens by which modules
the build includes. raylib's own portability story is `#if defined(PLATFORM_WEB)`
inside the user's `main.c` (§6.2), and its config is a header of `#define`s
(§2.2). None of that reaches ludo source — it is C-side build configuration — but
it means the raylib path multiplies **build configurations**, where the SDL3 path
has a runtime-selected backend behind one API.

**Could not verify:** whether SDL3 exposes any symbol whose *signature* differs by
target, which would bite R4 at the binding layer. I checked no such case and did
not audit the headers for it.

---

## 4. Who owns the frame loop

ADR-0056 §3's whole argument is *"the frame loop is not the library's… the runner
still decides when to step, when to stop stepping, when to re-present the last
image and when to keep pumping events."* This section checks that claim
concretely.

### 4.1 SDL3 — a plain `main()` works, and callbacks are available everywhere

`README-main-functions` (<https://wiki.libsdl.org/SDL3/README-main-functions>) on
the `SDL_AppInit`/`SDL_AppIterate`/`SDL_AppEvent`/`SDL_AppQuit` entry points:
*"This is completely optional and you can ignore it if you're happy using a
standard 'main' function."*

And on the web: *"Emscripten (programs that run on a web page) absolutely
requires this to function at all."* iOS is described the same way. The document
resolves the divergence in SDL's favour: *"Using the callback entry points works
on every platform, because on platforms that don't require them, we can fake them
with a simple loop in an internal implementation of the usual SDL_main."*

This is the best available answer for ludo. Ch6 §10.2 permits *"a poll or a
host-driven callback"*; `SDL_AppIterate` **is** the host-driven callback, and it
is the same shape on desktop and in a tab. R3's no-`#if` rule is satisfied at the
ludo level without a per-target loop structure, because there is only one loop
structure.

The runner therefore owns the *contents* of the frame — when to step the
simulation, when to stop stepping, when to redraw — while SDL owns only the
cadence. That is the split ADR-0056 §3 assumed.

### 4.2 SDL3 — re-presenting without re-running user draw code

Supplied by construction. The runner renders into its own texture (SDL3's
`SDL_CreateGPUTexture` / render-target texture) and blits that to the swapchain
each frame; a paused frame skips the user's draw and blits the same texture
again. Nothing about SDL3 forces the swapchain image to be produced by user code
on that frame.

**Could not verify** from a normative sentence that SDL3 guarantees swapchain
contents persist across frames without a redraw — I found no such statement and
would not rely on one; the render-to-texture route makes the question moot.

### 4.3 raylib — the loop is the user's, but `EndDrawing` is a bundle

`WindowShouldClose()` / `BeginDrawing()` / `EndDrawing()` is a user-driven `while`
loop, so raylib does not own the loop in the sense ADR-0056 means. But
`EndDrawing()` bundles draw-flush, buffer swap, frame pacing (with the blocking
wait of §2.2) and `PollInputEvents()` into one call, so "keep pumping events while
not stepping" and "do not block" cannot both be had without
`SUPPORT_CUSTOM_FRAME_CONTROL`.

Re-presenting a prior frame is available: `LoadRenderTexture` /
`BeginTextureMode` gives a render target, and drawing that texture is a normal
draw call. Usagi does exactly this (§6.2).

### 4.4 raylib on the web forces the callback loop, with no unified form

The raylib wiki page *Working for Web (HTML5)*
(<https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)>) states the
browser constraint: *"the browser needs to control the executed process and just
allow a single Update-Draw execution in a time-frame, so execution could be
controlled and locked when required (i.e. when the tab is not active or browser is
minimized)."*

It offers two routes: refactor the body of the loop into an `UpdateDrawFrame()`
handed to `emscripten_set_main_loop`, or compile with `-s ASYNCIFY` so emscripten
*"can detect synchronous code and run it asynchronously"* — the page notes *"a
small performance penalization."* Web linking also needs `-s USE_GLFW=3`, and
`-s MAX_WEBGL_VERSION=2` for ES3.

Unlike SDL3, raylib offers **no unified entry point**: the desktop build has a
`while` loop and the web build has a callback, which is the `#if`-shaped
divergence R3 exists to prevent, pushed down into the C host rather than into ludo
but present all the same.

### 4.5 sokol — callback-only, and it takes `main()`

`sokol_app.h` inverts control by default: the user writes `sokol_main()`
returning an `sapp_desc` with `.frame_cb`, and the implementation calls it
(`_sapp.desc.frame_cb();`). `SOKOL_NO_ENTRY` is offered to *"define this if
sokol_app.h shouldn't 'hijack' the main() function."* A host-driven callback is
permitted by §10.2, so this is not disqualifying; it is simply less flexible than
SDL3, which offers both forms.

---

## 5. Facade-altitude collision

ludo mandates its own drawing facade: ten verbs in fill_/stroke_ pairs over rect,
ellipse, triangle, line and sprite, each taking a descriptor carrying
`paint: Paint`, where `Paint = Color | Texture` and `Texture` carries a
`Mapping { stretch, tile }` (spec ch6 §4.3, §4.4; ADR-0010).

### 5.1 SDL3 sits below it

SDL3's 2D render API (<https://wiki.libsdl.org/SDL3/CategoryRender>) offers
`SDL_RenderPoint`, `SDL_RenderLine`, `SDL_RenderRect`, `SDL_RenderFillRect`,
`SDL_RenderTexture` (with rotation, affine, tiled and 9-grid variants) and
`SDL_RenderGeometry`. **No ellipse, no circle, no filled triangle primitive.** Its
own introduction positions it as a floor rather than a facade: *"This API is
designed to accelerate simple 2D operations. You may want more functionality such
as 3D polygons and particle effects, and in that case you should use SDL's
OpenGL/Direct3D support, the SDL3 GPU API, or one of the many good 3D engines."*

The gap is real work — ellipse and triangle tessellation, stroke geometry,
`Mapping.tile` — but it is work with no competing abstraction to reconcile
against. Collision risk: **low**.

### 5.2 raylib sits at it

From `src/raylib.h`: `DrawRectangle`, `DrawRectangleRec`, `DrawRectanglePro`,
`DrawRectangleLines`, `DrawRectangleLinesEx`, `DrawRectangleRounded`,
`DrawRectangleGradientV/H/Ex`, `DrawCircle`, `DrawCircleV`, `DrawTriangle`,
`DrawTriangleLines`, `DrawTriangleFan`, `DrawTriangleStrip`, `DrawLine`,
`DrawLineEx`, `DrawLineStrip`, `DrawLineBezier`, `DrawLineDashed`, plus the
`DrawTexture*` family.

This is ludo's facade with a different spelling and a wider surface. Two
consequences.

First, **every divergence becomes a decision with a tempting wrong answer.** Where
the spec's semantics differ from raylib's — stroke alignment, `Mapping.tile`,
`crisp`/`smooth` coverage, sprite texel rules — the cheap move is to inherit
raylib's behaviour into the mandated facade. That is how a normative surface
acquires an undocumented dependency on one implementation.

Second, **there is no clean lower seam.** raylib does expose `rlgl` as a
standalone OpenGL abstraction module (per `src/raylib.h`'s feature list: *"Custom
OpenGL abstraction layer (usable as standalone module): [rlgl]"*), so building
the facade on `rlgl` and ignoring the `Draw*` verbs is technically possible — but
then the argument for raylib over SDL3 has evaporated, since what remains is an
OpenGL wrapper.

Collision risk: **high**.

### 5.3 raylib carries engine-tier API, which #12 excludes

`CONTEXT.md` defines an engine as *"frame loop, component model, sprites,
animation, collision, camera."* `src/raylib.h` ships `Camera2D`/`Camera3D` with
`BeginMode2D`/`BeginMode3D`, `GetWorldToScreen2D`/`GetScreenToWorld2D`,
`GetCameraMatrix`, a `CheckCollision*` family, model loading, `UpdateModelAnimation`
and billboards. The header's own feature list names *"Flexible Materials system"*,
*"Animated 3D models supported (skeletal bones animation)"* and *"VR stereo
rendering"*.

ADR-0056 §4 keeps #12's ruling: *"Delegating the platform layer does not bundle an
engine; the tiers are distinct and the higher one stays out."* Linking raylib
links an engine's worth of API under the facade. It would be unreachable from ludo
— we bind what we choose — but the ruling is about what the runtime *is*, not only
about what is spelled in the stdlib, and the honest statement is that raylib is
not a platform layer.

### 5.4 sokol

`sokol_gfx.h` is a GPU abstraction with no 2D drawing verbs at all; collision
risk is nil, and the facade work is the same as SDL3's or larger.

---

## 6. The precedents

### 6.1 DragonRuby Game Toolkit

**Verified.** Source read: the MIT-licensed
<https://github.com/DragonRuby/dragonruby-game-toolkit-contrib>, which publishes
the Ruby-side runtime (`dragon/runtime.rb`, `dragon/outputs.rb`,
`dragon/remote_hotload_client.rb`). The C host is closed, so everything below is
inferred from the Ruby side plus first-party devlogs, and where it is inference I
say so.

**Which SDL.** Currently **SDL2**. Primary evidence in-repo:
`dragon/docs.rb:707` documents keycodes by linking
`https://wiki.libsdl.org/SDL2/SDLKeycodeLookup`. First-party statement of the
move: the DragonRuby devlog *DragonRuby's Seventh Year — Where We Started and
Where We're Going*
(<https://dragonruby.itch.io/dragonruby-gtk/devlog/1497015/>) says *"The SDL3
upgrade of DragonRuby is near completion,"* and lists what it buys: *"true
cross-platform, post effect shaders (which will look the same everywhere with a
single code-base)"*, *"the ability to render raw geometries"*, *"fine-grained
control of textures via affine transforms"*, *"improved HD support and font
scaling (specifically Windows)"*, *"pixel perfect rendering provided directly by
SDL3"*, and *"No. Breaking. Changes."*

Read against §5.1: three of the five named gains are things a language runtime
needs from the tier *below* its facade, and DragonRuby is getting them by moving
SDL2 → SDL3 without changing the surface its users see. That is the delegated-tier
boundary holding under a library version change, which is the strongest available
evidence for ADR-0004's split.

**Who owns the frame loop.** The C host calls into Ruby once per frame; the Ruby
side's `calc_wrapper` is that entry, and it is a three-part sequence
(`dragon/runtime.rb`):

```ruby
      tick_gtk_engine_before
      tick_usr_engine
      tick_gtk_engine_after
```

The runner decides whether user code steps at all:

```ruby
    def skip_tick_usr_engine?
      @paused ||
      quit_after_startup_eval? ||
      @controller_config.should_tick? ||
      @slowmo_factor_debounce ||
      @load_status != :ready  ||
      @is_reloading ||
      @reload_debounce > 0
    end
```

`tick_usr_engine` begins `return if skip_tick_usr_engine?`, while
`tick_gtk_engine_before`/`_after` still run — clearing draw passes, pumping input,
ticking the console. **This is error-as-pause and reload-quiescence in the exact
shape ADR-0056 §3 describes**: stop stepping user code, keep pumping, keep
presenting. Note also that the tick counter advances when
`@load_status == :main_rb_load_error_shown`, with the comment *"we have to
increment `Kernel.global_tick_count` because the console relies on this for
interaction/'ticking'"* — the runner keeps running while the game does not.

**Hot reload.** File-mtime-driven, applied at a tick boundary:
`reload_requested_ruby_files_synchronously` is called from `__require_sync__`, the
runtime tracks `@reload_list_history`, `@files_reloaded`, `@reloaded_files`,
`@is_reloading` and `@reload_debounce`, and `dragon/remote_hotload_client.rb`
plus `tick_remote_hotload_client` add reload over the network. The reload is
Ruby-VM-level; SDL is not involved and does not need to be.

**What the library made hard.** Not directly evidenced in the open repo. The
devlog's list of what SDL3 unlocks is indirect evidence that SDL2 made
cross-platform shaders, raw geometry and pixel-perfect scaling hard.

**Did the boundary hold.** Yes. `dragon/outputs.rb` defines DragonRuby's own
primitive set; the only SDL leakage found in the open Ruby source is a keycode
documentation link and an error message mentioning *"SDL's FFI"*
(`dragon/outputs.rb:278`).

### 6.2 Usagi

**Verified — the project exists.** <https://github.com/brettchalupa/usagi>
(mirror; development at <https://codeberg.org/brettchalupa/usagi>), by Brett
Chalupa, licensed **Unlicense**. Cloned and read at `version = "1.4.0-dev"`.
`Cargo.toml` confirms the stack the ticket describes:

```toml
mlua = { version = "0.11", features = ["lua55", "vendored", "serde"] }
sola-raylib = { version = "6.3", features = ["noscreenshot"] }
```

— Lua 5.5 via `mlua`, raylib via the `sola-raylib` Rust bindings. The package
description reads *"Simple 2D game engine for rapid prototyping with Lua,
featuring live reload; powered by Rust and sola-raylib."*

**Who owns the frame loop.** Usagi does, and it went to deliberate trouble to keep
it. `src/session.rs` module doc:

> State lives on a `Session` struct so we can drive frames identically on
> native (a `while session.frame() {}` loop) and on emscripten (handing
> the struct to `emscripten_set_main_loop_arg`, which yields to the
> browser between frames). Avoiding a blocking native loop on emscripten
> is what lets us drop ASYNCIFY entirely.

One `Session::frame(&mut self) -> bool`, two drivers: native
`while session.frame() {}`, web an `extern "C" fn frame_callback` handed to
`emscripten_set_main_loop_arg(frame_callback, session_ptr, 0, 1)` with the comment
*"0 = drive at requestAnimationFrame rate (matches refresh)"*.

This is §4.4's problem solved by hand. Usagi pays the cost SDL3's main callbacks
would have absorbed, and pays it in `#[cfg(target_os = "emscripten")]` — the
divergence R3 (§10.5) forbids in ludo source.

**What raylib made hard.** Three items, all from Usagi's own comments:

- Frame pacing on the web (`src/session.rs`): *"Don't call `set_target_fps`:
  raylib's implementation uses `emscripten_sleep` for the pacing wait, which
  requires ASYNCIFY (we deliberately don't link with it)."* The call is therefore
  `#[cfg(not(target_os = "emscripten"))] rl.set_target_fps(60);`. This is §2.2's
  blocking wait, biting a real downstream project on a real target.
- Fullscreen on the web: an `extern` declaration for `usagi_fullscreen_toggle`,
  documented as *"Defined by `web/usagi_fullscreen.js` and linked via
  `--js-library`. Routes the toggle through the browser's Fullscreen API since
  raylib's desktop fullscreen calls don't work on emscripten."*
- Session lifetime under the browser loop, flagged at `src/session.rs:378`:
  *"but the emscripten main loop owns lifetime, so the canvas freezes."*

**Hot reload.** mtime polling in `Session::maybe_reload_assets`, at a frame
boundary, with state preserved:

> Script reload: re-exec on mtime change to either any `.lua` file or any file
> under `data/`… State is preserved (no `_init`); F5 is the explicit reset. Errors
> are logged and the previous callbacks keep running so a half-saved file can't
> kill the session.

It calls `clear_user_modules`, re-runs `load_script`, re-fetches `_update`/`_draw`
from globals, and clears `last_error` on success. raylib is not involved: reload
is Lua-VM-level, same as DragonRuby's is Ruby-VM-level. **Neither precedent
implements reload against the platform library**, which is worth stating because
ADR-0001's argument assumed the platform layer was half of it.

**Error handling is not error-as-pause.** Usagi records the error and keeps
stepping: `record_err(&mut self.last_error, "_update", …)` and, at draw time,
`if let Some(ref err) = self.last_error { draw_error_overlay(…) }`, described in
`src/render.rs` as *"a full-width error banner at the bottom of the window. Shown
only when user Lua has errored; cleared on successful reload or F5 reset."* So on
this axis Usagi is weaker evidence than DragonRuby, whose `skip_tick_usr_engine?`
is the real thing.

**Did the boundary hold.** Yes, by Usagi never exposing raylib to Lua: the Lua API
is Usagi's own `gfx.*` (`spr`, `sspr_ex`, `spr_ex`, shape draws with a trailing
alpha and a palette index) and raylib types stay Rust-side. Notably Usagi's facade
is *narrower* than raylib's — a fixed palette, a fixed resolution, a render
texture with `pixel_perfect` fit — which is what makes the collision of §5.2
tractable for them. ludo's facade is not narrower than raylib's; it is the same
altitude with different semantics.

---

## 7. SDL3's GPU API against the WebGPU model (ADR-0002)

ADR-0002 adopts *"the WebGPU model — its resource, pipeline, bind-group and
command-encoder shape."* ADR-0056 §6 withdrew the implement-it-ourselves premise
and deferred the model question here.

### 7.1 Where SDL_GPU matches

From <https://wiki.libsdl.org/SDL3/CategoryGPU>, the object model is:

- Device: `SDL_CreateGPUDevice`, bound to a window with
  `SDL_ClaimWindowForGPUDevice`.
- Resources: `SDL_CreateGPUBuffer`, `SDL_CreateGPUTexture`,
  `SDL_CreateGPUSampler`, `SDL_CreateGPUShader`.
- Pipelines: `SDL_CreateGPUGraphicsPipeline`, `SDL_CreateGPUComputePipeline`,
  described as *"precalculated rendering state."*
- Command recording: `SDL_AcquireGPUCommandBuffer`, then
  `SDL_BeginGPURenderPass`, `SDL_BeginGPUComputePass`, `SDL_BeginGPUCopyPass`.

Resource / pipeline-object / command-encoder / pass is WebGPU's shape, near
one-to-one. The explicit copy pass in particular is WebGPU's, not OpenGL's.

### 7.2 Where it diverges — two places, one of them load-bearing

**Bind groups do not exist.** SDL_GPU binds per stage, per slot:
`SDL_BindGPUVertexBuffers`, `SDL_BindGPUFragmentSamplers`,
`SDL_BindGPUComputeStorageBuffers`, and uniforms are pushed rather than bound —
*"Uniform data pushed to a slot on a stage keeps its value throughout the command
buffer until you call the relevant Push function on that slot again."* WebGPU's
`GPUBindGroup` / `GPUBindGroupLayout` and its explicit binding-model validation
have no counterpart. Of ADR-0002's four named shapes — resource, pipeline,
bind-group, command-encoder — **three survive and the bind-group does not.**

**Shaders are backend-native, not one language.** The category page states *"Each
backend (Vulkan, Metal, D3D12) requires a different shader format,"* with
`SDL_shadercross` offered for runtime cross-compilation. WebGPU has exactly one
shader language, WGSL. ADR-0003 owns shaders and this document does not decide
them, but the divergence is recorded because ADR-0002's *"the deferred web backend
becomes a thin pass-through"* payoff is partly a shader-portability claim.

**The SDL_GPU documentation does not mention WebGPU at all.** The resemblance is
convergent design, not a stated compatibility goal, so nothing constrains SDL to
stay close to WebGPU as either evolves.

### 7.3 raylib forfeits the model entirely

raylib renders through `rlgl`, its own OpenGL abstraction, targeting *"OpenGL
(1.1, 2.1, 3.3, 4.3, ES2, ES3 — choose at compile)"* per `src/raylib.h`'s feature
list. There is no pipeline object, no bind group, no command encoder, no explicit
pass — it is immediate-mode-shaped over a batching layer, plus an optional
software renderer (`rlsw`). Choosing raylib means **ADR-0002's model adoption is
abandoned, not partially kept.** That is exactly the ceiling ADR-0002 rejected
under *"A GLES3 baseline… a 2012 ceiling on a language shipping in 2027 or
later."*

---

## 8. Web reachability

ADR-0006 keeps the web a shaping constraint with the backend deferred; ADR-0056
§4 confirms it. The question is which candidates reach a browser and what that
does to the loop.

**SDL3.** Emscripten is a first-class target and the entry-point document
(§4.1) states the browser *"absolutely requires"* the main callbacks — and that
the same callbacks work everywhere else, so there is one program shape. That is
the best web story of the three for R3's purposes.

**But SDL_GPU does not reach the browser today.** The WebGPU backend for SDL_GPU
exists only as open pull requests against `libsdl-org/SDL`: #12046 (*SDL3 GPU
WebGPU Backend*, klukaszek), #16020 (*GPU: Experimental WebGPU SDLGPU Backend*,
TheeStickmahn), #15722 (*Experimental WebGPU (wgpu-native & Dawn) Video Backend*),
tracked from feature request #10768. Consequence: a ludo web backend on SDL3 would
run over SDL's 2D render API on WebGL, and the desktop GPU path and the web path
would be **different renderer components** rather than one model with two
implementations — which is precisely what ADR-0002 hoped to avoid.

**Merge status, verified 2026-08-18** via the GitHub API, upgrading what this
document first recorded as unverified: **none of the three is merged.** #12046 is
**closed unmerged** (last activity 2025-03-16); #15722 is **closed unmerged**
(2026-05-31); **#16020 is open and was updated on 2026-08-18**, the day of the
check. Feature request #10768 remains open.

That is a different picture from a flat "not merged": two attempts have been
closed without landing and a third is **actively in progress**. The capability is
in flight, not abandoned, so this section's consequence is **time-dependent** — it
holds today and could stop holding on a timescale that matters to map #129. Re-run
the check before relying on it rather than citing this paragraph.

**Could not verify:** whether SDL's 2D render API has a WebGPU path.

**raylib.** Reaches the browser via emscripten with GLFW3 and WebGL, per the
project's own wiki (§4.4), at the cost of either an
`emscripten_set_main_loop`-shaped fork of the loop or `-s ASYNCIFY`. Usagi's
experience (§6.2) is that ASYNCIFY is avoidable but the `#[cfg]` split is not, and
that some raylib calls (`SetTargetFPS`, fullscreen) simply do not work there.

**sokol.** `sokol_app.h` lists `SOKOL_WGPU` among its selectable 3D APIs
alongside `SOKOL_GLES3`, so a WebGPU path exists in the same source tree as the
native ones. This is the only candidate found with a first-party WebGPU backend in
its released tree.

**Could not verify:** the maturity of sokol's WGPU backend, or whether
`sokol_audio.h`'s WebAudio path satisfies R6's cursor and free-space clauses. I
read only the header's prose, which notes the stream callback is *"called per-frame
in the WebAudio"* case rather than on a separate thread, and that push mode with
`saudio_expect()` — *"ask Sokol Audio how much room is [in the buffer]"* — is
available (`SOKOL_AUDIO_API_DECL int saudio_expect(void);`).

---

## 9. R6's play cursor: nobody supplies it directly

Ch6 §10.9 and §5.7.1 require *"a monotonic play cursor in sample frames."*
[Research 07](07-browser-semantics-of-the-mandated-facade.md) already found the
audio clauses supplied in the browser *because* R5 and R8 make the ring ours. The
same reasoning is what saves this rule natively — but only if the ring is
genuinely ours.

**SDL3.** `SDL_GetAudioStreamQueued`
(<https://wiki.libsdl.org/SDL3/SDL_GetAudioStreamQueued>) gives *"the number of
bytes currently queued"* in the stream, and `SDL_GetAudioDeviceFormat`
(<https://wiki.libsdl.org/SDL3/SDL_GetAudioDeviceFormat>) reports
`sample_frames`, *"device buffer size, in sample frames… the amount of data SDL
will feed to the physical hardware in each chunk."* No function reports a play
cursor. It is **derivable** — frames pushed, minus frames still queued, minus at
most one device chunk — with an uncertainty bounded by that chunk. Whether that
bound is tight enough for §5.7.1's frame-precise music sync is **not verified
here** and should be measured before the clause is treated as satisfied.

**raylib.** `GetMusicTimePlayed(Music)` exists but applies only to raylib's own
`Music` streaming type, not to a caller-owned `AudioStream`; for `AudioStream` the
only feedback is `IsAudioStreamProcessed`, a boolean *"check if any audio stream
buffers requires refill."* No frame count, no cursor. R6 fails on two of four
clauses, and because the mix is raylib's (§2.5) there is no ring of ours to derive
one from.

**sokol.** `saudio_expect()` gives free space in frames; no cursor found.

---

## 10. Licensing

The repo is Zlib-licensed.

- **SDL3** — zlib/libpng. `LICENSE.txt` at
  <https://github.com/libsdl-org/SDL>: *"Copyright (C) 1997-2026 Sam Lantinga… This
  software is provided 'as-is', without any express or implied warranty."*
  Static linking is unrestricted. Same licence as this repo.
- **raylib** — zlib/libpng, stated in `src/raylib.h`: *"raylib is licensed under
  an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license
  that allows static linking with closed source software."* Same licence as this
  repo.
- **sokol** — zlib/libpng, stated in `sokol_app.h`.
- **wgpu-native** — **could not verify.** The README fetch returned no licence
  section. The gfx-rs projects are generally MIT/Apache-2.0 dual-licensed, but I am
  not citing that from memory. If wgpu-native is ever reconsidered, this needs
  checking, because Apache-2.0's notice and patent terms are a different
  distribution obligation from Zlib's.

**Not verified for any candidate:** the licences of vendored third-party
dependencies. raylib vendors miniaudio, stb_image, glad, qoi, jar_xm, jar_mod,
sinfl/sdefl and rprand (per `src/raylib.h`'s DEPENDENCIES block); SDL3 vendors
its own set. Each has its own terms and none were read.

---

## 11. Recommendation

**SDL3.**

It is the only candidate that satisfies R1, R5 and R7 through its documented,
supported API rather than through a non-default build configuration; it is the
only one offering a single entry-point shape that works both natively and in a
browser, which is what R3 needs; it sits below the mandated facade rather than at
it; it keeps three of ADR-0002's four model shapes; and it is under the same Zlib
licence as this repo. DragonRuby — this project's closest structural precedent —
is on SDL2 and is moving to SDL3 with *"No. Breaking. Changes."* to its own
facade, which is the delegated-tier boundary being demonstrated rather than
argued.

The accepted costs, stated rather than waved past:

1. **The play cursor is derived, not reported** (§9), with an error bound of one
   device chunk that has not been measured.
2. **The non-blocking swapchain acquire is the non-recommended path** (§2.1), so
   R1 compliance is a standing constraint on the runner, not a property of using
   SDL correctly.
3. **`SDL_AsyncIO` on Emscripten is unverified** (§2.6).
4. **SDL_GPU has no browser backend today** (§8), so the deferred web backend is
   a second renderer component after all.
5. **Bind groups are gone** (§7.2).

## 12. What would have to be true for raylib to win

Not rhetorical — each is checkable, and if all four held the recommendation should
flip.

1. **`SUPPORT_CUSTOM_FRAME_CONTROL` would have to be a supported configuration
   rather than an at-your-own-risk one**, and the resulting build would have to be
   what raylib's own CI tests. Its source currently says the opposite (§2.2). Even
   then, `LoadFileData`'s `fopen`/`fread` (§2.3) leaves R7 unmet and R1's loading
   clause resting entirely on ch6 §10.3's preload carve-out.
2. **The audio mix would have to be ours.** R5 (§10.7) and R8 (§10.12) require
   mixing in the frame entry and a runner-owned ring. That means bypassing
   `raudio.c` and binding miniaudio directly — at which point raylib supplies
   window/input and drawing only, and the comparison becomes GLFW + miniaudio
   (§9), not raylib.
3. **ADR-0002's model adoption would have to be dropped deliberately**, by a
   decision that records the GLES-era ceiling as acceptable. ADR-0002 rejected
   exactly that ceiling under *"a 2012 ceiling on a language shipping in 2027 or
   later"*, so this is a reversal, not a consequence.
4. **The facade-altitude collision would have to be judged a shortcut rather than
   a hazard** (§5.2), and the engine-tier surface (§5.3) reconciled with #12.

The single fact that would most change the picture in raylib's favour: if
`SDL_GPU`'s browser story stays absent for years while raylib's WebGL path keeps
working, the "one renderer model, two implementations" argument for SDL3 weakens
to "two renderer components either way", and cost of construction starts to
dominate. That is a reason to re-check §8, not to decide differently today.

A note on **sokol**: it survives every rule check here and has the only
first-party WebGPU backend found in a released tree (§8). It was not recommended
because its callback-only entry point, its smaller ecosystem, and the unverified
maturity of its WGPU and WebAudio paths make it a larger bet on less evidence —
not because anything disqualified it. If SDL3's web GPU gap becomes decisive,
sokol is the candidate to re-examine, not raylib.

## 13. What this does to ADR-0002

ADR-0002 should be amended, not reversed, if SDL3 is chosen:

- **The model survives in part.** Resource, pipeline object and command
  encoder/pass are SDL_GPU's shape too (§7.1). **The bind-group shape does not
  survive** and should be struck from ADR-0002's sentence; SDL_GPU binds per stage
  per slot and pushes uniforms (§7.2).
- **The API ceiling clause changes owner.** ADR-0002 says *"the API ceiling is
  WebGPU's ceiling."* It becomes SDL_GPU's ceiling — close to WebGPU's, but set by
  a library we do not control and with no stated commitment to track WebGPU.
- **The payoff clause is withdrawn for now.** ADR-0002's *"the deferred web backend
  becomes a thin pass-through instead of a second renderer"* does not hold while
  SDL_GPU has no browser backend (§8). The web backend is a second renderer
  component again, over SDL's 2D render API on WebGL.
- **The shader premise moves to ADR-0003.** SDL_GPU takes backend-native shader
  formats with `SDL_shadercross` as the cross-compiler (§7.2), not one language.

If raylib were chosen instead, ADR-0002 would be **reversed in full**: there is no
WebGPU-shaped surface anywhere in `rlgl` to adopt.

---

## Sources

Primary — specification, wiki and first-party documentation:

- **SDL3 wiki** — <https://wiki.libsdl.org/SDL3/>
  (`CategoryAudio` on the push and callback models; `SDL_OpenAudioDeviceStream` on
  the NULL callback; `SDL_GetAudioStreamQueued`; `SDL_GetAudioDeviceFormat` on
  `sample_frames`; `CategoryAsyncIO` on non-blocking request-and-poll;
  `CategoryGPU` on the device/resource/pipeline/pass model, per-stage binding,
  pushed uniforms and per-backend shader formats; `SDL_AcquireGPUSwapchainTexture`
  on vblank blocking and the NULL-texture non-error; `CategoryRender` on the 2D
  primitive set and its stated relationship to SDL_GPU; `README-main-functions`
  on the optional main callbacks and Emscripten's requirement).
- **SDL LICENSE.txt** —
  <https://raw.githubusercontent.com/libsdl-org/SDL/main/LICENSE.txt> (zlib/libpng,
  1997–2026 Sam Lantinga).
- **raylib source** — <https://github.com/raysan5/raylib> at `master`,
  self-identifying as v6.1-dev. `src/raylib.h` (licence block; DEPENDENCIES and
  FEATURES blocks; the full `Draw*` verb set; `Camera2D`/`Camera3D`,
  `CheckCollision*`, model/animation API; `AudioStream` functions;
  `LoadFileData`/`SetLoadFileDataCallback`; `GetMusicTimePlayed`;
  `SwapScreenBuffer`/`PollInputEvents`/`WaitTime` and the custom-frame-control
  comment). `src/rcore.c` (`EndDrawing` frame-time wait; `WaitTime`'s
  `Sleep`/`nanosleep`/`usleep`; `LoadFileData`'s `fopen`/`fread`). `src/raudio.c`
  (the `AudioCallback callback; // …for buffer filling on audio threads` field;
  `OnSendAudioDataToDevice` and its mutex comment; `SetAudioStreamCallback`).
  `src/config.h` (`SUPPORT_CUSTOM_FRAME_CONTROL`, disabled by default, "use at
  your own risk").
- **raylib wiki, *Working for Web (HTML5)*** —
  <https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)>
  (browser control of execution; `emscripten_set_main_loop` refactor vs
  `-s ASYNCIFY`; `-s USE_GLFW=3`, `-s MAX_WEBGL_VERSION=2`).
- **sokol** — <https://github.com/floooh/sokol>. `sokol_app.h` (the
  `sokol_main`/`sapp_desc.frame_cb` inversion; `_sapp.desc.frame_cb()`;
  `SOKOL_NO_ENTRY`; the `SOKOL_GLCORE`/`SOKOL_GLES3`/`SOKOL_D3D11`/`SOKOL_METAL`/
  `SOKOL_WGPU`/`SOKOL_VULKAN`/`SOKOL_NOAPI` list; zlib/libpng licence).
  `sokol_audio.h` (callback vs push models; the separate-thread note and the
  WebAudio per-frame note; `saudio_push`; `saudio_expect`).
- **DragonRuby GTK contrib** —
  <https://github.com/DragonRuby/dragonruby-game-toolkit-contrib> (MIT).
  `dragon/runtime.rb` (`calc_wrapper`'s three-part tick; `skip_tick_usr_engine?`
  and its `@paused`/`@is_reloading`/`@reload_debounce` guards;
  `tick_gtk_engine_before`'s tick-count comment;
  `reload_requested_ruby_files_synchronously`; `reload_list_history`).
  `dragon/remote_hotload_client.rb`. `dragon/outputs.rb:278`. `dragon/docs.rb:707`
  (the SDL2 keycode wiki link — the in-repo evidence of the current SDL major
  version).
- **DragonRuby devlog, *DragonRuby's Seventh Year*** —
  <https://dragonruby.itch.io/dragonruby-gtk/devlog/1497015/> (*"The SDL3 upgrade
  of DragonRuby is near completion"*; the five named gains; *"No. Breaking.
  Changes."*).
- **Usagi** — <https://github.com/brettchalupa/usagi> (mirror of
  <https://codeberg.org/brettchalupa/usagi>), Brett Chalupa, Unlicense, read at
  `version = "1.4.0-dev"`. `Cargo.toml` (`mlua` 0.11 with `lua55`; `sola-raylib`
  6.3). `src/session.rs` (the module doc on one `frame()` driven two ways and on
  dropping ASYNCIFY; `while session.frame() {}`; `emscripten_set_main_loop_arg`
  with `frame_callback`; the `set_target_fps` / `emscripten_sleep` comment; the
  `usagi_fullscreen_toggle` extern and its comment; `maybe_reload_assets` and its
  state-preserving reload doc; `record_err` / `last_error` / `draw_error_overlay`).
  `src/render.rs` (`draw_error_overlay`'s doc comment). `src/pause.rs` (module
  doc). `README.md` (*"Usagi is powered by Raylib and the sola-raylib Rust
  bindings"*).

In-repo, cited as the test rather than as evidence:

- `docs/spec/06-stdlib.md` §10 (R1–R8, normative) and §4.3–§4.4 (`Paint`,
  `Mapping`, the ten drawing verbs).
- `docs/adr/0056-…md` (§3 the frame loop, §4 what does not change, §6 ADR-0002's
  lost premise, §8 what this owes), `docs/adr/0006-…md` (the rules in argument
  form), `docs/adr/0002-…md` (the WebGPU model, the rejected GLES3 baseline),
  `docs/adr/0001-…md`, `CONTEXT.md` (*Engine*, *Backend*).
- `docs/research/07-browser-semantics-of-the-mandated-facade.md` (why R5 and R8
  are what make the audio clauses satisfiable).

## What could not be verified

Listed once, in full, because this project treats an uncited claim as a defect.

1. Whether `SDL_AsyncIO` is implemented on the Emscripten target, and over what
   (§2.6).
2. ~~The merge status of the SDL_GPU WebGPU backend pull requests.~~
   **Verified 2026-08-18** and recorded in §8: none merged, #16020 open and moving.
   Still unverified: whether SDL's 2D render API has any WebGPU path.
3. Whether any SDL3 symbol has a target-dependent signature, which would bite R4
   at the binding layer (§3).
4. Whether SDL3 guarantees swapchain contents persist across frames without a
   redraw (§4.2). Render-to-texture makes it moot, so this was not pursued.
5. Whether the derived play cursor's one-device-chunk uncertainty is tight enough
   for ch6 §5.7.1's frame-precise sync (§9). This needs measurement, not reading.
6. The maturity of sokol's `SOKOL_WGPU` backend, and whether `sokol_audio.h`'s
   WebAudio path satisfies R6's cursor and free-space clauses (§8).
7. wgpu-native's licence (§10) — the README fetch returned no licence section, and
   nothing is asserted from memory.
8. The licences of every candidate's vendored dependencies (§10).
9. DragonRuby's C host, which is closed. Everything in §6.1 about who calls whom
   is inferred from the open Ruby side; the claim that the C host drives
   `calc_wrapper` once per frame is inference from that code's structure, not a
   quoted statement. Likewise, "what SDL2 made hard" for DragonRuby is inferred
   from the devlog's list of SDL3 gains and is not directly evidenced.
10. Whether DragonRuby's SDL3 upgrade has shipped since the devlog quoted in §6.1.
