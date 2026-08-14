# Source authority: which issues still decide things

**Non-normative.** Wayfinding, required by [#73](https://github.com/ludo-lang/ludo/issues/73).

[ADR-0033](0033-a-stamp-can-go-stale-and-the-companion-count-is-reconciled.md) §1
names the common cause of every Tier-1 finding in the corpus audit:

> a claim is checked against the issue that produced it rather than against the
> ADR that superseded it

The corpus cites issues and ADRs interchangeably, and the citations are *true* —
the issue really did settle the thing. What an author cannot tell from a citation
is whether it is still the **whole** truth. This file answers that, and only
that. It edits nothing and decides nothing.

## How to use it

Before transcribing a clause into a spec chapter, look its source up here.

- **Authoritative** — the issue is still the sole source. Read it and stop.
- **Authoritative, with a named change** — read the issue, then read the named
  ADR clause, which moved part of it.
- **Superseded** — an ADR carries the decision now. The issue is history.

An issue not listed here either produced an ADR directly (see §3) or is cited
too rarely to have caused a mistake yet. Absence is not a guarantee.

---

## 1. Issues that are the sole authority, with no ADR

These are the ones an author cannot verify without proving a negative across all
46 ADRs. Each of them **originates no ADR** — no ADR opens by naming it as its
ticket — while being cited throughout the corpus.

| Issue | Title | Status | What moved, and where |
|---|---|---|---|
| [#4](https://github.com/ludo-lang/ludo/issues/4) | Gather evidence on how AI agents fail at writing code | authoritative | Research findings. Cited as evidence, never as a rule. |
| [#5](https://github.com/ludo-lang/ludo/issues/5) | Decide what agent-friendliness means for this language | authoritative | The Tier-1 criteria. Unchanged; cited as a veto throughout. |
| [#6](https://github.com/ludo-lang/ludo/issues/6) | Decide whether layers are tiers or profiles | authoritative | The beginner-versus-agent axis. Unchanged. |
| [#7](https://github.com/ludo-lang/ludo/issues/7) | Write the same small game in three candidate syntaxes | authoritative | Candidate B wins. Transcribed in spec ch1 §1.2, §7.1. |
| [#8](https://github.com/ludo-lang/ludo/issues/8) | Choose the memory model | **authoritative, narrowed** | ADR-0042 adds the allocator's *shape*; the model is otherwise untouched. **Narrowed by [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md) §3**: §3's *aliasing is deleted* holds at the **container** level, not the view level — any number of views of one place may co-exist, and only `!` or a move on the origin kills them. §8's guarantee table gains four compile-error rows it does not itself list (dangling view, use-after-move, iterator invalidation), from ADR-0047 §3 and #15 Q14/Q19. |
| [#9](https://github.com/ludo-lang/ludo/issues/9) | Decide the fate of nil | **authoritative, contradicted** | `persist` without an initialiser is a **live contradiction** with ADR-0045 §1/§8. Unresolved by design — spec ch1 §14.5 defers it to chapter 5. |
| [#10](https://github.com/ludo-lang/ludo/issues/10) | Choose the error model | authoritative | Errors as values, `rescue`, closed nominal error sets. |
| [#11](https://github.com/ludo-lang/ludo/issues/11) | Set the depth of the type system | **authoritative, narrowed and extended** | **Narrowed** by ADR-0045: integer generic parameters are *not* constraint-bounded, "a narrow, stated exception to #11". **Extended** by ADR-0017 §3 (a `fn` type satisfies a single-function interface by a blessed rule) and ADR-0017 §6 (UFCS never applies to a function value). |
| [#15](https://github.com/ludo-lang/ludo/issues/15) | Decide what replaces Lua's one-table aggregate | **authoritative, under challenge** | Q11's string rule is **superseded by ADR-0043** (`string` is a view into the runner's constant blob). Q10's view rule is **re-cut by [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md)**, which resolved the #83 challenge: *never returned* is reversed (a view may be returned when it derives from the receiver or the constant blob), *parameter position only* is clarified to permit a local binding, and the struct-field, `persist` and capture bans stand. ADR-0043 §1's narrowing is unaffected. |
| [#17](https://github.com/ludo-lang/ludo/issues/17) | Decide how a compiled language delivers state-preserving reload | **authoritative, respelled** | §4's `persist name: str = "boss"` is respelled `string` by ADR-0043 §3, and the type it names is narrowed by ADR-0043 §1. **§2's `$.mem.heap` is respelled a bare `heap` by ADR-0042 §8**, which also confines it to `persist` initialisers in the entry file. The reload semantics are unchanged. |
| [#18](https://github.com/ludo-lang/ludo/issues/18) | Decide what error-as-pause means for a compiled program | authoritative | ADR-0018 is a *different* subject (the diagnostic stream, from #53). Do not conflate the numbers. |
| [#19](https://github.com/ludo-lang/ludo/issues/19) | Specify the experience contract an implementation must meet | **authoritative, with corrections** | ADR-0041 corrects six sentences that #78/#79 got wrong about the runner half of this contract. The contract itself stands. |
| [#22](https://github.com/ludo-lang/ludo/issues/22) | Decide the analyzability guarantees the spec owes an oracle | authoritative | Confirmed, not moved, by ADR-0045 (the oracle still never monomorphises). |
| [#24](https://github.com/ludo-lang/ludo/issues/24) | Decide whether the spec binds a grammar budget | authoritative | The budget and its counting rules. Every ADR's `#24 delta` section reports against it. The comparison table has an open loose end in [#93](https://github.com/ludo-lang/ludo/issues/93). |
| [#25](https://github.com/ludo-lang/ludo/issues/25) | Decide how a type declares its memory layout | **authoritative, with two named changes** | The whole layout chapter: layout belongs to the container, guaranteed declaration order, `#align(n)`, no bit fields, the one-level SoA split, and sum-type layout with no niche optimisation. **§2's mandated padding diagnostic is withdrawn by ADR-0023 §1** — a spec-owned diagnostic is an error or it does not exist, so the padding report is vendor-space; §2's layout rule is untouched. **§9's column exclusivity is contradicted by [ADR-0047](0047-a-returned-view-is-derived-from-its-receiver-and-mutation-kills-it.md) §3**, which removes exclusivity between views entirely; open as [#103](https://github.com/ludo-lang/ludo/issues/103). **Do not conflate #25 with ADR-0025**, which is *Fullscreen is a player preference* and decides nothing about memory — ADR-0026 §83, ADR-0042 §1, spec ch1 §12.2 and spec ch2 §4.8/§6.6/§20 all cite "ADR-0025" where they mean **#25**. |
| [#26](https://github.com/ludo-lang/ludo/issues/26) | Decide whether top-level code runs, and how it orders against frames | **authoritative, with stale prose** | `use "world" as World` is **stale prose** predating ADR-0014 §3/§7 — there is no import alias (spec ch1 §14.9). The frame-entry parameter question it raised was settled against it by #47 / ADR-0013. **`$.mem.heap` is respelled a bare `heap` by ADR-0042 §8**, and is legal only in a `persist` initialiser in the entry file. |
| [#29](https://github.com/ludo-lang/ludo/issues/29) | Specify the C ABI and FFI boundary | authoritative | ADR-0017 §4 **generalises** its callback restriction (top-level, named, non-capturing) into a whole-language rule. The ABI clauses are unchanged. |
| [#60](https://github.com/ludo-lang/ludo/issues/60) | Decide whether the platform layer has a program-visible surface… | **authoritative, partly withdrawn** | Its **blanket ban on program-visible backend state is withdrawn** by ADR-0025 §8; ADR-0028 replaces it with an admission test. Check ADR-0028 before citing #60 on any query. |

## 2. The rows this file does not carry

Per #73 and the grilling that scoped it, the **supersession** rows — one per
resolved issue that an ADR narrowed, reversed or spelled — are a byproduct of
chapter work, not a separate build. Each chapter's file in
[`../spec/coverage/`](../spec/coverage/) records where its sources landed, and
the index is assembled from those when chapter 8 closes.

This file carries only the rows a chapter author **cannot** derive: the ones
where the absence of an ADR is the fact.

## 3. Issues that originate an ADR

For completeness, so that "not listed in §1" is checkable. Each of these opens an
ADR by name, and the ADR is authoritative from that point:

| Issue | ADR |
|---|---|
| #12 | ADR-0001, ADR-0003 |
| #28 | ADR-0004 |
| #40 | ADR-0005 |
| #42 | ADR-0009, ADR-0015 |
| #51 | ADR-0016 |
| #52 | ADR-0017 |
| #53 | ADR-0018 |
| #54 | ADR-0019 |
| #55 | ADR-0021 |
| #58 | ADR-0022 |
| #59 | ADR-0023 |
| #61 | ADR-0024 |
| #62 | ADR-0025 |
| #63 | ADR-0027 |
| #64 | ADR-0026 |
| #65 | ADR-0028 |
| #66 | ADR-0029 |
| #67 | ADR-0030 |
| #69 | ADR-0034 |
| #70 | ADR-0035 |
| #74 | ADR-0037 |
| #75 | ADR-0038 |
| #79 | ADR-0041 |
| #80 | ADR-0042 |
| #81 | ADR-0043 |
| #83 | ADR-0047 |
| #84 | ADR-0045 |

ADR-0002, 0006, 0007, 0008, 0010, 0011, 0012, 0013, 0014, 0020, 0031, 0032,
0033, 0036, 0039, 0040, 0044 and 0046 open from an earlier ADR or from a session
rather than from a ticket.
