---
status: accepted
---

> **Amended by [ADR-0056](0056-the-platform-layer-is-delegated-to-a-third-party-library.md):**
> **reversed in full** — the platform layer is delegated to a third-party library, not
> ours. The spec never required ownership (ch6 §10.1 leaves the platform-layer API
> unspecified; ADR-0055's audit found this ADR unabsorbable), and the #19 argument below
> does not hold: the runner keeps the frame loop, so the half said to be lost is not lost.
> ADR-0056 §4 lists what survives.

# The platform layer is ours, written against OS and GPU APIs — not bound from SDL

Issue #12 settled that ludo is standalone and binding-first: every pixel, sample
and input event arrives through the C ABI. That left open *what* we bind. We
bind the OS and GPU APIs directly — Win32, Cocoa, X11/Wayland, Vulkan, Metal,
D3D12, WASAPI, CoreAudio, ALSA/PipeWire — and write the platform layer ourselves
in ludo on top of those externs, rather than binding SDL or GLFW.

The reason is the experience contract (#19), not differentiation. #12's Flutter
cross-reference established that state-preserving reload and error-as-pause each
need two halves, and the second half is a platform-layer responsibility. If that
layer is a third-party C library we do not control, the contract is unenforceable
— which turns ownership from a preference into a consequence of a decision
already made. Dogfooding is the secondary benefit: the platform layer is the
largest honest test of #8's explicit allocators and #11's monomorphisation.

## Considered options

- **Bind SDL3 or GLFW.** Rejected: this is the Odin position, which #2 already
  called redundant, and it puts the reload/pause half of #19 outside our control.
- **Bind nothing above libc** — raw syscalls, own loader. Rejected as the Beef
  failure mode: a decade of full-time work with nothing shipped.
- **Ship no platform layer at all.** Rejected on #12's Flame finding: a community
  engine only appears if the language ships a general-purpose renderer first.
  Language-plus-bindings gets no Flame-shaped project.

## Consequences

These are the non-obvious ones. Several are load-bearing constraints on work
that has not started yet.

- **SDL was the portability story; we now own it.** Four subsystems across five
  platform families is roughly eighteen backends, not four — and Linux is two
  windowing systems, not one.
- **The backend matrix is a conformance matrix.** Because the motivation is #19,
  every backend must satisfy the experience contract identically, on real
  hardware, in CI. That is the actual cost — not the code.
- **Platform set:** Linux, Windows and macOS are tier-1. The web is a *shaping
  constraint* on the API from day one but its backend is deferred; #12 already
  overruled a design decision on browser grounds, and retrofitting the browser's
  model later is a rewrite. Consoles are out of scope for any public repo — the
  SDKs are NDA'd C++ — and are reachable only through the backend seam below.
- **One API, N backends, with the seam a documented extension point.** This is
  what lets a console backend live in a private repo without forking the
  language, and what makes #19 testable as one predicate across the matrix. It
  costs us an API pinned to the intersection of what all backends can do.
- **Binding is by runtime dynamic loading** (`dlopen`/`LoadLibrary` plus symbol
  resolution), not link-time linking. This is what makes `ludo build --target X`
  work from any host with no platform SDK installed, and #12 made the toolchain
  load-bearing. The cost: resolution failure moves from link time to launch time,
  so an unavailable backend must be a normal condition rather than a crash.
- **An unavailable backend is an error value, not a bug.** Per #10 it is returned
  in the return type; it must *not* route through #18's error-as-pause, which
  would pause a shipped binary on a user's machine over an environment fact.
- **macOS does not break the C ABI premise.** Metal and Cocoa have no C API, but
  `objc_msgSend` is a C symbol and is called through ordinary `extern`
  declarations with correct concrete prototypes per call site. A hand-written
  C/Objective-C shim is used only where blocks, ARC or Objective-C exceptions
  force it. The spec must not name Objective-C — see #29.
- **The platform layer lives in its own repo**, versioned independently and
  blessed by reference. Keeping it out of `adamico/ludo` preserves the
  destination's completion test (*an implementer could start a compiler*) and
  stops an eighteen-backend conformance matrix from dominating a tracker that is
  still designing a language.
- **This does not decide #28.** Ownership and spec-coverage stay orthogonal:
  building the platform layer does not mandate it in the spec, and mandating it
  would require every conforming implementation to ship a renderer across four
  backends. #28 still decides on #27's evidence.
- **Nothing here is built yet.** The platform layer is written in ludo, so it
  cannot exist before the compiler does. These decisions are recorded now because
  they are shaping constraints on an API that must be designed before it is
  written — not because work starts.
