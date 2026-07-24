# How AI agents fail at writing code

**Research ticket:** [adamico/ludo#4](https://github.com/adamico/ludo/issues/4) · **Serves:** [#1](https://github.com/adamico/ludo/issues/1) · **Siblings:** [`02-language-survey.md`](./02-language-survey.md), [`03-lua-simplicity.md`](./03-lua-simplicity.md) · **Date of evidence:** 2026-07-24

**Headline: agents overwhelmingly fail at *naming things that exist* and at *knowing what the code they cannot see does* — not at algorithms, and not at the things language designers assume are hard.** The single largest measured category of agent compilation failure in a real multi-file codebase is name and path resolution (43.7% of Rust compile errors, [Rust-SWE-bench](https://arxiv.org/html/2602.22764v1)); the single largest category of *decisive* agent error across scaffolds is epistemic — acting on a false premise about the codebase (57.9%, of which false premises alone are 30.7%, [arXiv:2607.09510](https://arxiv.org/html/2607.09510v1)). Borrow checking, the property the ticket nominated as the interesting Rust pole, does not appear at all in the top thirteen Rust error codes agents produce. Meanwhile a strict compiler plus a local iteration loop took GPT-5 on Idris from **22/56 to 54/56** ([arXiv:2602.11481](https://arxiv.org/html/2602.11481v1)) — the strongest single piece of evidence in this document, and it points the opposite way from the folk belief that strictness punishes agents.

Three qualifications before the ranking, because they set the frame.

**Frequency and cost are almost inverted.** The failures agents commit most often are the cheapest to catch: undefined names, wrong imports, type mismatches. The failures that cost the most — behavioural drift in an API whose signature did not change, a missing error check, a fabricated claim of success — are rarer per token and nearly invisible to any compiler. A language cannot make the second class go away. It can move the *first* class from runtime to compile time, and it can shrink the surface on which the second class operates. That is the whole of what language design buys here, and it is worth being precise about it rather than overclaiming.

**Most of the "which language is agent-friendly" literature is measuring corpus size wearing a costume.** Every cross-language benchmark below correlates strongly with training-data volume, and the two studies that control for it — PyLang ([arXiv:2605.15607](https://arxiv.org/html/2605.15607v1), covered in the sibling survey) and ObfusEval ([arXiv:2412.08109](https://arxiv.org/pdf/2412.08109)) — both find that when familiarity is removed, models retain their algorithmic choices and lose their implementations. PyLang: models picked the same algorithm as in Python 80% of the time, and "75% of failures involve the correct algorithm with buggy implementation". ObfusEval: renaming identifiers and unfolding calls, with functionality preserved, drops test pass rate by up to **62.5%**. Language *properties* are a second-order term on top of a first-order familiarity term. This document tries to isolate the second-order term, and flags where it cannot.

**The two most-cited multilingual agent benchmarks disagree about Rust, in opposite directions, by a factor of nearly four.** [SWE-bench Multilingual](https://www.swebench.com/multilingual.html) puts Rust *first* at 58.14% resolved; [Multi-SWE-bench](https://arxiv.org/html/2504.02605v1) puts Rust at 15.90%; [Rust-SWE-bench](https://arxiv.org/html/2602.22764v1) puts it at 21.2% against 70.4% for the same agent on Python. Any conclusion of the form "language X is agent-friendly because benchmark Y" is not currently supportable. The per-language numbers below are reported because the ticket asks for them and because their *error breakdowns* are informative; their headline rates are not.

---

## The ranked failure modes

Ranked by frequency × cost. **Frequency** is how often the failure is observed in the cited measurements. **Cost** is how far downstream it is caught: compile-time is cheap, test-time is moderate, silent-in-production is expensive. A failure that is common but caught by the compiler ranks below one that is rarer but silent.

| # | Failure mode | Frequency (measured) | Cost | Language property that prevents or mitigates it |
|---|---|---|---|---|
| 1 | **Non-existent or wrong-namespace names** — hallucinated packages, unresolved imports, symbols that do not exist | **43.7%** of agent-produced Rust compile errors are naming/scoping/path resolution ([Rust-SWE-bench](https://arxiv.org/html/2602.22764v1)); **20.23%** of Rust crate recommendations are hallucinated ([arXiv:2606.08444](https://arxiv.org/html/2606.08444v1)); **5.2–21.7%** package hallucination on PyPI/npm ([Spracklen et al., USENIX Sec '25](https://arxiv.org/abs/2406.10279v3)); undefined names is the #1 baseline error in Idris (123 occurrences, [arXiv:2602.11481](https://arxiv.org/html/2602.11481v1)) | Low *if* compiled; **catastrophic** if the name is registrable by an attacker (53 names still registrable after registry defenses, [arXiv:2605.17062](https://arxiv.org/abs/2605.17062)) | Closed, declared, compile-time-resolved namespace. No dynamic import, no implicit prelude beyond a fixed list. **Standard-library module names must not be drawable from the same namespace as package names** — 45.47% of hallucinated Rust crates are real std module names |
| 2 | **Wrong version / dead or drifted API** | Deprecated Usage Rate **25–38%** across 7 LLMs, 145 API mappings, 28,125 prompts ([Wang et al., ICSE '25](https://arxiv.org/abs/2406.09834)); **56.1%** success on before-cutoff APIs vs **32.5%** after ([RustEvo², arXiv:2503.16922](https://arxiv.org/abs/2503.16922)) | **High** — signature changes fail loudly, **behavioural** changes do not (38.0% success, the worst category) | No dialect variance; a frozen core; deprecation as a compile error rather than a warning; **behaviour changes must force a signature change** (nominal, versioned types) so the compiler can see them |
| 3 | **Intent / logic error with correct algorithm** | Functional bugs **29.3–34.1%** vs syntax bugs **3.7–10.4%** on HumanEval+ ([arXiv:2407.06153](https://arxiv.org/html/2407.06153)); requirement-conflicting hallucinations **39.60%** ([HalluCode, arXiv:2404.00971](https://arxiv.org/html/2404.00971)); **~71%** of agent failures in constrained backend generation are logic errors ([arXiv:2605.06445](https://arxiv.org/html/2605.06445v1)); PyLang: **75%** of failures are correct algorithm, buggy implementation | **Highest** — only tests catch it | **Largely not language-preventable. Say so.** Partial mitigation only: types that encode domain invariants, exhaustive matching, no silent coercion, so that a subset of intent errors becomes a type error |
| 4 | **Non-local reasoning failure** — the agent cannot see, or misremembers, the rest of the program | File-level fault localisation succeeds **>90%**, function-level only **~27%**, exact hunk **4.0–8.4%** ([arXiv:2511.00197](https://arxiv.org/html/2511.00197)); "failure stems from fine-grained reasoning, not repository navigation"; standalone functions score substantially higher than context-dependent ones, and **>70%** of real-project functions are non-standalone ([CoderEval](https://arxiv.org/abs/2302.00288)) | High | **Local reasoning**: a signature that fully determines behaviour. No global mutable state, no implicit context object, no monkey-patching, no ambient configuration. Effects and mutation visible at the call site |
| 5 | **Type and contract violation** | **32.6%** of agent Rust compile errors are type/trait failures ([Rust-SWE-bench](https://arxiv.org/html/2602.22764v1)); type-constrained decoding **"reduces compilation errors by more than half"** on TypeScript ([arXiv:2504.09246](https://arxiv.org/abs/2504.09246)) | Low if compile-time, high if the language coerces silently | Static nominal types, mandatory annotation at boundaries, **no implicit conversion**, exhaustive `match`. This is the bug class where the compiler most cleanly earns its keep |
| 6 | **Ambiguity and overload-resolution failure** | "Ambiguous elaboration" is the **#2** baseline error category in Idris (111 occurrences vs 123 for undefined names) ([arXiv:2602.11481](https://arxiv.org/html/2602.11481v1)) | Low (compile-time) but expensive in *iterations*, which is the agent's real budget | Unambiguous grammar; **no function overloading**; no implicit-conversion-driven dispatch; one name resolves to one definition |
| 7 | **Missing error handling and null handling** | Undefined-variable hallucinations **16.91%**, the largest single code-inconsistency subcategory ([HalluCode](https://arxiv.org/html/2404.00971)); LLM-generated code documented omitting null checks present in the human original ([arXiv:2503.11082](https://arxiv.org/html/2503.11082)) | High — silent until it crashes | Non-nullable by default; `T?` as the only optional form; **unused results are a compile error**; errors in the return type, not in an invisible unwinding channel |
| 8 | **Convention and structure non-compliance** | Adding structural constraints costs **−30 points** of assertion pass rate on average, worst case **−45pp (62% of baseline)**; convention-heavy frameworks trail lightweight ones by **25–32 points** ([arXiv:2605.06445](https://arxiv.org/html/2605.06445v1)) | Moderate | **One obvious way — but it must be in the language, not in a framework.** The measured penalty is for conventions the agent must *recall*; conventions the compiler *enforces* are a different thing (see the conflict named below) |
| 9 | **Security weakness in generated code** | **29.5%** of Copilot Python and **24.2%** of JavaScript snippets carry weaknesses across 43 CWEs ([TOSEM 10.1145/3716848](https://dl.acm.org/doi/10.1145/3716848)); large-scale GitHub scan: Python **16.18–18.50%**, JavaScript **8.66–8.99%**, TypeScript **2.50–7.14%** ([arXiv:2510.26103](https://arxiv.org/pdf/2510.26103)) | **Highest** — silent, and adversarially exploited | Memory safety by default; bounds checking; no `eval`; unsafe operations syntactically loud and greppable. The Python→JS→TS gradient is suggestive of type systems helping but is heavily confounded by domain |
| 10 | **Fabricated success** — the agent reports done on a failing state | **26%** of failed trajectories fabricate success, beginning at the point of lock-in; **82%** of failed runs continue without progress ([arXiv:2607.09510](https://arxiv.org/html/2607.09510v1)) | **Highest** — destroys the value of the loop | Not language-preventable, but adjacent: a build system with no partial-success state, no way to disable checks, and no path that runs code that did not compile |
| 11 | **Memorised-bug replication** | **44.44%** of LLM-introduced bugs are byte-identical to the historical pre-fix version; **82.61%** for GPT-4o; accuracy on bug-prone code **12.27%** vs **29.85%** on normal code (GPT-4) ([arXiv:2503.11082](https://arxiv.org/html/2503.11082)) | High | Perverse: a *small* corpus prevents this and causes failure mode #12. Named as a conflict below |
| 12 | **Low-corpus penalty** — the language is simply not in pretraining | Sonnet 4.5: **58% on PyLang vs 88% on Python**, a 30-point deficit no fine-tuning technique closed; in-context idiom corpora recover **24 points** ([arXiv:2605.15607](https://arxiv.org/html/2605.15607v1), via sibling survey) | Structural — it taxes every other row | Not a language property but a **documentation and toolchain** property: idiom-dense in-context docs, a stable spec, and never changing. This is the largest single lever available to a new language |
| 13 | **Concurrency defects** | LLMs generate concurrent code with deadlocks and races that CodeBLEU-style metrics fail to detect ([CONCUR, arXiv:2603.03683](https://arxiv.org/abs/2603.03683)) | High, and non-deterministic | Effects visible at the call site; no ambient shared mutable state; data-race freedom or explicit, greppable sharing |

---

## Per-language and cross-language evidence

### The multilingual agent benchmarks, and why they contradict each other

Three benchmarks measure the same nominal thing — an agent resolving a real GitHub issue in a real repository — and produce incompatible language orderings.

**[SWE-bench Multilingual](https://www.swebench.com/multilingual.html)** (300 tasks, 42 repositories, 9 languages; Claude 3.7 Sonnet):

| Language | Resolved |
|---|---:|
| Rust | 58.14% |
| Java | 53.49% |
| PHP | 48.84% |
| Ruby | 43.18% |
| JavaScript / TypeScript | 34.88% |
| Go | 30.95% |
| C / C++ | 28.57% |

Overall **43%**, against **63%** for the same model on the Python-only SWE-bench Verified. The authors tested the obvious confound and could not find it: using lines-of-code modified as a difficulty proxy revealed "no obvious correlation" — "Rust tasks required the most code changes yet achieved the highest success rate". Their own conclusion is appropriately humble: *"From this limited analysis, it's hard to say what factors are most important in determining resolution rate."*

**[Multi-SWE-bench](https://arxiv.org/html/2504.02605v1)** (2,132 instances, 8 languages, 68 expert annotators) — best result per language, across agents and models:

| Language | Best resolved | Configuration |
|---|---:|---|
| Python | 48.20% | OpenAI-o1 (MagentLess) |
| Java | 22.66% | DeepSeek-R1 (MagentLess) |
| Rust | 15.90% | Claude-3.7-Sonnet (MopenHands) |
| C++ | 14.73% | Claude-3.7-Sonnet (MopenHands) |
| C | 8.59% | Claude-3.7-Sonnet (MSWE-agent) |
| Go | 7.48% | Claude-3.7-Sonnet (MopenHands) |
| TypeScript | 6.70% | DeepSeek-V3 (MagentLess) |
| JavaScript | 2.81% | OpenAI-o3-mini-high (MagentLess) |

Their stated explanation is a two-factor one: a **paradigm hierarchy** ("high-level general-purpose languages outperforming systems programming and low-level/high-performance computing languages"), plus **training-data bias**. For the dynamic languages they blame "dynamic typing, asynchronous execution, and diverse runtime behaviors"; for the systems languages, "manual memory management, complex build systems, and intricate type systems". Note that these two explanations are in tension — they predict that TypeScript should beat JavaScript by more than 3.9 points and that Rust should be near the bottom, and Rust is fourth.

**[Rust-SWE-bench](https://arxiv.org/html/2602.22764v1)** (500 tasks, 34 repositories) — the most useful of the three, because it publishes the error breakdown:

| Agent | Claude-Sonnet-3.7 | GPT-4o | o4-mini | Qwen3 |
|---|---:|---:|---:|---:|
| OpenHands | **21.2%** | 6.6% | 6.8% | 5.0% |
| SWE-agent | 15.0% | 1.8% | 8.2% | 1.8% |
| AutoCodeRover | 9.2% | 4.2% | 6.8% | 4.8% |
| Agentless | 7.2% | 5.4% | 7.8% | 3.2% |

Against **70.4%** for OpenHands on Python SWE-bench Verified. Three compilation errors per task on average. **44.5% of tasks fail at the issue-reproduction stage alone**, before any code is written — an environment and build-system property, not a language-semantics property.

Whatever the true ordering is, the disagreement itself is the finding: **cross-language agent success rates are not currently a stable measurement**, and a language-design decision that rests on one of these tables rests on sand. What *is* stable across all three is the direction of the Python-versus-everything-else gap, and that gap tracks corpus size.

### What agents actually get wrong in Rust — the borrow checker is not the story

This is the most surprising result in the document and it directly contradicts the framing in the ticket.

[Rust-SWE-bench Table 4](https://arxiv.org/html/2602.22764v1), the distribution of compilation errors agents produce on real Rust repository tasks:

| Error code | Meaning | Share |
|---|---|---:|
| E0599 | method not implemented on type | 18.06% |
| E0433 | undeclared crate / module / type | 16.21% |
| E0432 | unresolved import | 12.08% |
| E0425 | unresolved name | 8.54% |
| E0308 | type mismatch | 6.81% |
| E0277 | unsatisfied trait bound | 6.50% |
| E0412 | type name not in scope | 5.69% |
| E0753 | invalid doc-comment location | 3.07% |
| E0282 | **type inference failure** | **1.75%** |
| E0609 | nonexistent struct field | 1.48% |
| E0061 | wrong argument count | 1.23% |
| E0405 | trait not in scope | 1.21% |
| E0407 | method not in trait impl | 1.18% |

**No borrow-checker or lifetime error code appears anywhere in the list** — not E0502, E0499, E0382, E0597, or E0106. The paper groups the codes into "repository structure" failures (43.7%: E0433, E0432, E0425, E0412, E0405) and "type/trait system" failures (32.6%: E0599, E0308, E0277, E0407).

Three consequences.

1. **Ownership is not what makes Rust hard for agents.** The widely-repeated claim that LLMs "struggle with the non-local nature of Rust ownership" ([arXiv:2606.08444](https://arxiv.org/html/2606.08444v1) makes exactly this claim in prose) is not supported by the only published error distribution from a real repository-level agent benchmark. It may well be true of *greenfield* generation from a blank file — the C-to-Rust transpilation literature reports borrow-checker violations resolved at 74.2% by guided repair ([SafeTrans, arXiv:2505.10708](https://arxiv.org/html/2505.10708v2)) — but in agentic repository work the borrow checker is statistical noise next to `use` statements. **Weak-evidence flag:** one paper, one language, one table; it should be replicated before it is treated as settled.
2. **Type inference failure is 1.75%.** The ticket lists "heavy inference" as a property that hurts. The only direct measurement available says it barely registers. That hypothesis is currently **unsupported**, and should not be carried into the criteria ticket as if it were evidenced.
3. **The dominant failure is "I do not know what is in scope."** That is a *namespace* problem, and namespace design is entirely under a language designer's control.

### Package and API hallucination, measured three ways

**Python and JavaScript, 2024 baseline.** [Spracklen et al., USENIX Security 2025](https://arxiv.org/abs/2406.10279v3): 576,000 code samples, 16 models, two languages. "at least 5.2% for commercial models" and "21.7% for open-source models"; **205,474 unique hallucinated package names**. Widely reported from the paper body but not verified by me directly in the abstract, and therefore **weaker evidence**: 43% of hallucinated packages recur in all ten runs, 58% recur more than once, and 8.7% of hallucinated Python package names are real npm packages.

**Python and JavaScript, 2026 frontier cohort.** [arXiv:2605.17062](https://arxiv.org/abs/2605.17062): 199,845 paired prompts against live PyPI and npm master lists, five frontier models. Rates compressed to **4.62% (Claude Haiku 4.5) to 6.10% (GPT-5.4-mini)** — "an order-of-magnitude compression of the inter-model spread… but not a retirement of the threat". **53 hallucinated names (41 PyPI, 12 npm) remain registrable by an attacker after each registry's existing defenses.** The paper also reports "a Python-over-JavaScript hallucination asymmetry that inverts Spracklen's 2024 finding" — i.e. the per-language ordering flipped in eighteen months, which is itself evidence that these rates measure model training data rather than language properties.

**Rust.** [arXiv:2606.08444](https://arxiv.org/html/2606.08444v1) (Internetware 2026): 14 models, 2,794 tasks, 16,764 snippets, 48,494 crate recommendations. Overall hallucination **20.23%** — Gemini 2.5 Pro best at 16.18%, Claude 4 Opus worst at 26.90%. Rust hallucinates crates at roughly four times the frontier Python/JS rate. The *mechanism* is the finding a language designer should care about:

- **45.47% is module confusion**: "Many hallucinated crate names are actually valid Rust modules that exist in the standard library, but are being incorrectly referenced." Top offenders: `thread` (1,086), `duration` (828), `tokenstream` (505).
- **81.7% of hallucinations are within five character edits** of a legitimate crate name, versus roughly half of Python/JS hallucinations being six or more edits away. **32.96% differ only by hyphen usage.**
- **38.9%** of high-frequency hallucinations borrow names from other ecosystems (Python, Windows APIs).
- Neither model size (F(1,4)=0.0138, p=0.912) nor temperature (F(4,12)=0.9159, p=0.4859) predicts the rate.
- Self-refinement buys only 2–3 percentage points; RAG gains are "limited and inconsistent".

The design reading: **hallucination concentrates wherever two namespaces are shaped alike and separated only by convention.** `std::thread` and a crate called `thread` are indistinguishable to a next-token predictor; a hyphen-versus-underscore convention that differs between the manifest and the `use` statement is a coin flip. A language whose standard-library paths are lexically impossible to confuse with package paths, and whose package identifier has exactly one spelling, deletes 45% of this failure mode by construction. That is a cheap, concrete, testable design win.

### API evolution and churn

[Wang et al., ICSE 2025](https://arxiv.org/abs/2406.09834): seven models, 145 API mappings across eight Python libraries, 28,125 completion prompts. **Deprecated Usage Rate of 25–38%**, with larger models generally producing *more* deprecated usage. Root cause stated by the authors: deprecated usages present in training, deprecation knowledge absent at inference.

[RustEvo², arXiv:2503.16922](https://arxiv.org/abs/2503.16922): 588 synthesised API changes (380 std, 208 across 15 crates) in four categories.

| API change category | Success |
|---|---:|
| Stabilizations | 65.8% |
| Signature changes | (between) |
| Deprecations | (between) |
| **Behavioral changes** | **38.0%** |

| Relative to knowledge cutoff | Success |
|---|---:|
| Before cutoff | 56.1% |
| After cutoff | 32.5% |
| After cutoff, with RAG | +13.5% |

Two findings worth carrying forward. First, **a 23.6-point cliff at the training cutoff** — the sibling survey's Zig observation (stdlib churn makes LLMs emit dead APIs) is the qualitative form of this quantitative result, and the quantitative result is worse than the anecdote suggested. Second, **behavioural changes are the worst category at 38.0%**, "highlighting difficulties in detecting semantic shifts without signature alterations". A change that the type system cannot see is a change the agent will not make. This argues for a specific and slightly unusual design rule: **if the meaning of a function changes, the type must change** — nominal, versioned types rather than structural ones, so that semantic drift becomes a compile error rather than a silent behaviour change.

### The unfamiliarity tax, isolated

Two studies remove familiarity while holding semantics fixed, and they agree.

**PyLang** ([arXiv:2605.15607](https://arxiv.org/html/2605.15607v1), detailed in the sibling survey): a language deliberately absent from pretraining. Claude Sonnet 4.5 **58% vs 88%** on parallel problems. Multi-task learning, DPO, GRPO, infilling and latent-space objectives all failed to close it. In-context idiom snippets recovered **24 points**.

**ObfusEval** ([arXiv:2412.08109](https://arxiv.org/pdf/2412.08109)): 1,354 functions from five real C projects (Redis, libvips, lvgl, libgit2, fluent), obfuscated after each model's training cutoff, with semantics preserved.

| Obfuscation | Generation pass-rate drop |
|---|---:|
| Symbol (rename identifiers) | ~25% |
| Structure (unfold calls via LLVM) | ~32% |
| Symbol + structure | **62.5%** |

The authors call this the "Specialist in Familiarity" problem and conclude the models rely on pattern matching over identifier surface form rather than reasoning about logic. This is the strongest available evidence for a claim that sounds trivial and is not: **the names in a program are load-bearing for agent correctness.** A language that makes naming mechanical and predictable — one canonical spelling, no aliasing, no re-export chains, no macro-generated identifiers — is buying real accuracy, and a language that lets the same entity be spelled three ways is spending it.

### Bug taxonomies: what the wrong code is wrong about

**[arXiv:2407.06153](https://arxiv.org/html/2407.06153)** — three primary and ten secondary categories. On HumanEval+, across open models: **syntax bugs 3.7–10.4%**, **runtime bugs 1.8–7.9%**, **functional bugs 29.3–34.1%**. "Functional bugs constitute the highest proportion, whereas syntax bugs represent the lowest." The dominant subcategory is "misunderstanding and logic error", ranging from 5.5% (DeepSeek-R1 on HumanEval+) to 69.2% (Phi-3 on APPS+).

**[HalluCode, arXiv:2404.00971](https://arxiv.org/html/2404.00971)** — 3,120 samples, 1,212 hallucinatory snippets:

| Category | Share |
|---|---:|
| Requirement conflicting | **39.60%** (behaviour conflicting 35.40%, data conflicting 4.21%) |
| Knowledge hallucination | **34.90%** (library/project 25.99%, algorithm 4.95%, CS theory 2.31%) |
| Code inconsistency | **25.50%** (undefined variables 16.91%, useless statements 6.60%, fragmented logic 1.73%, inconsistent libraries 0.25%) |

**[Tambon et al., arXiv:2403.08937](https://arxiv.org/abs/2403.08937)** (EMSE) — 333 bugs from CodeGen, PanGu-Coder and Codex, ten patterns validated by a survey of 34 practitioners: Misinterpretation, Syntax Error, Silly Mistake, Prompt-biased code, Missing Corner Case, Wrong Input Type, Hallucinated Object, Wrong Attribute, Incomplete Generation, Non-Prompted Consideration.

Read together: roughly **a quarter to a third of all defects are the model being wrong about a name or an API** (knowledge hallucination 34.90%; undefined variables 16.91%; hallucinated object and wrong attribute in Tambon's list), and roughly **a third to two-fifths are the model being wrong about the requirement**. The first bucket is language-addressable. The second is not.

### Agent-loop failures: where the tokens actually go

**[arXiv:2607.09510](https://arxiv.org/html/2607.09510v1)**, "Failure as a Process" — 1,794 valid trajectories (1,184 failed, 610 successful), 63,000+ manually annotated execution steps, 89 Terminal-Bench tasks, 3 scaffolds, 7 frontier models. This is the highest-quality trajectory dataset available and its root-cause taxonomy is directly relevant.

| Root cause | Share |
|---|---:|
| **Epistemic** | **57.9%** |
| — False premise | 30.7% |
| — Specification neglect | 14.9% |
| — Output misreading | 4.4% |
| — Ignored signal | 4.1% |
| — Premature action | 3.7% |
| **Competence** | **32.8%** |
| — Knowledge gap | 24.0% |
| — Capability limitation | 8.8% |
| **Environment** | 9.4% |

Selected findings, quoted:

- "Decisive errors occur surprisingly early, with a median onset of just 7 execution steps."
- "Most decisive errors are epistemic (57.9%) rather than competence-related (32.8%)."
- "False premises are the single largest trigger of decisive errors (30.7%)."
- Only 18% of failed runs terminate immediately; **82% continue without actual progress**.
- Repairing an incorrect diagnosis accounts for **39% of wasted computation** (median 21 steps).
- "71% of successful runs recover from at least one error before eventually succeeding."
- Successful trajectories respond to error signals at a **92%** rate versus **37%** for failed ones.
- "26% of failed trajectories fabricate success, and it begins right at the point of lock-in."
- Pass rates span **19% to 45%** across 21 model–scaffold combinations.

**[arXiv:2511.00197](https://arxiv.org/html/2511.00197)** — success/failure trajectory comparison on SWE-bench Lite and Verified across OpenHands, SWE-agent and Prometheus. File-level localisation succeeds in over 90% of successful trajectories (93.5% for OpenHands on Lite) and still **72–81% in failed trajectories on Lite**. Function-level agreement with the reference patch is only ~27% (SWE-agent 33.5% on Lite); exact hunk match 4.0–8.4%. Failed trajectories are consistently longer: OpenHands 61.01 steps on success versus 79.90 on failure on Lite, and 82.5% longer on Verified. The authors' finding #7: **"Failure stems from fine-grained reasoning, not repository navigation."**

The design reading is uncomfortable and important. **Agents find the right file and then write the wrong thing in it.** Retrieval, indexing and repository navigation are not the bottleneck; the bottleneck is that having found the code, the agent forms a false premise about what it does. Every language property that makes a function's behaviour derivable from its signature and its own body attacks the 30.7% false-premise category directly. Every property that requires reading elsewhere — a metatable set in another file, a global that some import mutated, a macro that rewrites the call, an implicit context that changes allocator behaviour three frames up — feeds it.

### Context dependence, measured

[CoderEval](https://arxiv.org/abs/2302.00288) grades 230 Python and 230 Java functions on six levels of context dependency: self-contained, slib-runnable, plib-runnable, class-runnable, file-runnable, project-runnable. Headline finding: "The effectiveness of code generation models in generating standalone functions is substantially higher than that in generating non-standalone functions", and — the number that matters for language design — **"non-standalone functions… constitute more than 70% of the functions in popular open-source projects"**.

[BigCodeBench](https://arxiv.org/abs/2406.15877): 1,140 tasks invoking function calls from 139 libraries across 7 domains. Best model (GPT-4o) **60%** against a human baseline of **97%**. "LLMs are not yet capable of following complex instructions to use function calls precisely."

[Constraint decay, arXiv:2605.06445](https://arxiv.org/html/2605.06445v1): 80 greenfield backend tasks under a fixed API specification, with structural constraints added in four levels. Capable configurations **lose 30 points on average** in assertion pass rate from unconstrained to fully specified; worst case **−45pp, 62% of its baseline score**; best case −17pp. Per-constraint: PostgreSQL −19.3pp, SQLite −14.3pp, Clean Architecture −9.1pp, ORM choice −0.6 to −1.5pp. Lightweight frameworks (Express, Koa, Flask) averaged ~50% assertion pass rate; convention-heavy frameworks (FastAPI, Django, Hono) trailed by **25–32 points**. Roughly **71%** of failures were logic errors, with data-layer defects — "incorrect query composition and ORM runtime violations" — about 45% of those.

---

## Properties that help

Each with the evidence that supports it and an honest note on strength.

**1. A closed, compile-time-resolved namespace with exactly one spelling per entity.** *Strong.* This is the single best-evidenced property in the document. It is supported by the 43.7% naming/scoping share of Rust agent compile errors, the 45.47% std-module-confusion share of Rust crate hallucinations, the 32.96% hyphen-variant share, the 123 undefined-name errors that top the Idris baseline distribution, and the 62.5% ObfusEval collapse under identifier renaming. Concretely: no dynamic import, no re-export chains, no aliasing, no macro-generated identifiers, one canonical case convention shared between the manifest and the source, and a standard library whose paths cannot be confused with package paths.

**2. Static types checked before execution.** *Strong.* [Type-constrained decoding on TypeScript](https://arxiv.org/abs/2504.09246) (PACMPL/OOPSLA) "reduces compilation errors by more than half" and "significantly increases functional correctness in code synthesis, translation, and repair tasks". Multi-SWE-bench attributes the JavaScript and TypeScript floor partly to "dynamic typing… and diverse runtime behaviors", and TypeScript outperforms JavaScript there 6.70% to 2.81%. The security scan gradient — Python 16.18–18.50%, JavaScript 8.66–8.99%, TypeScript 2.50–7.14% ([arXiv:2510.26103](https://arxiv.org/pdf/2510.26103)) — points the same way but is confounded by application domain and should be treated as *weak*.

**3. Local reasoning: a signature that fully determines behaviour.** *Strong, by inference from three independent measurements.* CoderEval's standalone-versus-context gap; the 27% function-level localisation ceiling with >90% file-level success; the 30.7% false-premise share of decisive errors. No single paper tests "local reasoning" as a variable — this is a convergent inference, not a controlled result, and should be labelled as such. But three unrelated methodologies all land on *the agent's model of code it did not read is where correctness dies*.

**4. Compile errors that name the missing thing.** *Strong for the specific case, weak as a general rule.* The self-repair error hierarchy is unambiguous: name errors are repaired at **~77%**, syntax at **~66%**, assertion errors at only **~45%** ([arXiv:2604.10508](https://arxiv.org/html/2604.10508)). The difference is that a name error says *what* is missing and an assertion says only that something is wrong. A diagnostic that names the fix converts a hard failure into an easy one. The corollary is dangerous and is evidenced: a diagnostic that names the *wrong* fix is worse than none — [arXiv:2506.13186](https://arxiv.org/html/2506.13186) found that supplying bug analysis raised CodeLlama's correct patches by 53.6% (Java) and 300% (Python) but *lowered* DeepSeek-Coder's by 46.6% and 22.6%, because "when initial analysis misidentifies the root cause, LLMs tend to follow the incorrect reasoning path in the prompt". **A confidently wrong error message actively harms a strong model.**

**5. No dialect variance and no churn.** *Strong.* DUR 25–38%; the 56.1%→32.5% cutoff cliff; the 38.0% behavioural-change floor. Combined with the sibling survey's Zig finding. For a new language this is the cheapest property to have and the easiest to lose.

**6. An unambiguous grammar and no overloading.** *Moderate.* The direct evidence is one data point — "ambiguous elaboration" as the #2 baseline error category in Idris, 111 occurrences against 123 for undefined names ([arXiv:2602.11481](https://arxiv.org/html/2602.11481v1)). It is a good data point (real language, real compiler, categorised errors) but it is one language with an unusually elaborate elaborator. The sibling survey's separate observation that Odin's own maintainers say "procedure group resolution system needs to be completely rewritten" is human-facing corroboration, not agent evidence.

**7. Errors and optionality in the type, and unused results as errors.** *Moderate.* Supported by the 16.91% undefined-variable share, the documented omission of null checks, and the general observation that a missing check is invisible. No study directly compares a `Result`-style language to an exception-based one for agent correctness — this is a gap, and it is one of the more consequential gaps in the literature.

**8. Idiom-dense in-context documentation.** *Strong, and it is not a language property.* The PyLang result — 24 points recovered from in-context idiom snippets, more than from stating rules — is the largest measured intervention available to a language with no corpus. RustEvo²'s RAG result (+13.5% on post-cutoff APIs) is a weaker, independent confirmation. This belongs in the criteria ticket as a *toolchain* requirement, not a syntax one.

---

## Properties that hurt

**1. Namespaces that overlap or admit multiple spellings.** *Strong.* Same evidence as helper #1, read backwards. The Rust `std::thread` / `thread` crate collision alone accounts for a plurality of a 20.23% hallucination rate.

**2. Silent, invisible semantic change.** *Strong.* RustEvo²'s 38.0% on behavioural changes is the cleanest measurement in the document of "the agent cannot fix what the compiler cannot see". This generalises well beyond API evolution: any construct whose meaning can change without a visible change at the use site — dynamic dispatch resolved by ambient state, an implicit context object, a metatable swapped at runtime — is in this category.

**3. Conventions the agent must recall rather than conventions the compiler enforces.** *Strong.* Convention-heavy frameworks trail lightweight ones by 25–32 points, and total structural constraint costs 30 points on average ([arXiv:2605.06445](https://arxiv.org/html/2605.06445v1)). This is the evidence that most complicates the ticket's "one obvious way" hypothesis, and it is discussed as a named conflict below.

**4. Ecosystem churn.** *Strong.* See helper #5.

**5. Macros and metaprogramming.** *No direct evidence found. Flagging honestly.* I searched for empirical studies comparing LLM accuracy on macro-heavy versus macro-free code and found none. The argument against macros is entirely inferential — they generate identifiers that do not appear in source (attacking the naming property, ObfusEval), and they make a call site's behaviour underivable from its signature (attacking local reasoning, CoderEval / false-premise). Both inferences are reasonable and neither is measured. **Do not carry this into the criteria ticket as an evidenced claim.**

**6. Heavy type inference.** *Evidence points the other way.* Type inference failure (E0282) is **1.75%** of agent Rust compile errors — the third-rarest code in the published table. The hypothesis that heavy inference hurts agents is currently unsupported by the only direct measurement available. A weaker and still-live version of the hypothesis is that inference hurts the *reader* (human or agent) reasoning about code it did not write, rather than the *writer* — that version is untested.

**7. Overloading.** *Weak.* One data point (Idris ambiguous elaboration). See helper #6.

**8. Multiple competing idioms.** *Moderate, and mostly borrowed from the sibling document.* Lua's own designers documented that "mechanisms, not policies" produced a "proliferation of different, often incompatible" class systems. The agent-specific measurement does not exist. The nearest real evidence is the constraint-decay framework result, which measures something adjacent (recall burden) rather than the thing itself.

**9. A large corpus containing a language's own historical bugs.** *Strong, and awkward.* **44.44%** of LLM-introduced bugs are byte-identical to the historical pre-fix version of the code, rising to **82.61%** for GPT-4o; accuracy on bug-prone code is **12.27%** versus **29.85%** on ordinary code ([arXiv:2503.11082](https://arxiv.org/html/2503.11082)). Popularity buys fluency and inherits every mistake ever made in the language. This is discussed as a named conflict below.

---

## Does the compiler help or hurt?

**Verdict: strictness is net strongly positive for agents, but only under three conditions, and the conditions are the actual design requirement.** The single best experiment on this question is unambiguous, and it was run on the strictest mainstream-ish compiler available.

### The Idris experiment

[Compiler-Guided Inference-Time Adaptation, arXiv:2602.11481](https://arxiv.org/html/2602.11481v1) — GPT-5, 56 Exercism exercises in Idris, a dependently-typed language with a small corpus and an aggressive elaborator. Comparison baselines from the same author's setup: **45/50 in Python** and **35/47 in Erlang**.

| Method | Solved |
|---|---:|
| Baseline, zero-shot | **22/56 (39%)** |
| + failed-test feedback, 1 iteration | 27/56 (48%) |
| + failed-test feedback, 5 iterations | 27/56 (48%) |
| + error-documentation manual in context | 30/56 (54%) |
| + Idris reference manual in context | 34/56 (61%) |
| **+ local compilation and tests, up to 20 iterations** | **54/56 (96%)** |

Read the middle rows carefully, because they are where the nuance lives. **Five iterations of test feedback bought exactly nothing over one iteration** (27/56 both). Documentation alone got to 61%. Only when the agent could *run the compiler itself, locally, as many times as it needed* did the number move to 96% — beating the author's own Python baseline of 90%.

The authors' own diagnosis: *"In the Idris setting, the primary bottleneck for large language models lies in satisfying the language's strict type system and compilation requirements rather than in correcting logical errors exposed by tests."*

The baseline error distribution explains why the loop works so well: undefined names 123, ambiguous elaboration 111, parse/syntax 33 plus 22 "Expected 'else'", privacy/visibility 36, type mismatch 11. Almost all of these are errors that *name the missing thing*, and name errors are the category agents repair at ~77%. Effort scales gently with size — an increase of about 10 lines of code costs about 0.8 additional attempts (r≈0.37, R²≈0.14).

**The interpretation for language design: a strict compiler converts the failure distribution from "silent and rare" to "loud and frequent", and loud-and-frequent is the shape an agent can grind down.** 39% first-attempt success would be a catastrophe for a human's morale and is irrelevant to an agent that will make twenty attempts in ninety seconds. This is the clearest instance in the whole document of *beginner-friendly and agent-friendly being different axes*.

### The countervailing evidence, which is real

Three results cut against an unqualified endorsement.

**Compiler feedback alone is the *weakest* of six feedback types.** [FeedbackEval, arXiv:2504.06939](https://arxiv.org/html/2504.06939v2), Repair@1 across five models:

| Feedback type | Repair@1 |
|---|---:|
| Mixed | 63.6% |
| LLM-expert | 62.9% |
| Test | 57.9% |
| Minimal | 53.1% |
| **Compiler** | **49.2%** |
| LLM-skilled | 48.8% |

Model ranking: DeepSeek-R1 63.4%, Claude-3.5 59.2%, GPT-4o 54.5%, Qwen2.5 51.4%, GLM-4 51.1%. Gains stabilise after two to three cycles. The authors' explanation: "LLMs often fail to translate these diagnostics into semantically meaningful repairs. Complex repository-level tasks demand reasoning about program intent and module interactions, factors not captured by compiler-level signals."

**Repository-level strictness has a much worse ceiling than exercise-level strictness.** Rust-SWE-bench's 21.2% versus Python's 70.4%, with three compile errors per task and 44.5% of tasks failing before any code is written. The Idris result was on 56 self-contained exercises; nobody has reproduced it at repository scale.

**Iteration has sharply diminishing returns in general.** [arXiv:2604.10508](https://arxiv.org/html/2604.10508), seven models across three families on HumanEval and MBPP: the first repair round yields the largest gain for every model, and most plateau by round three. Representative deltas — Gemini 2.5 Flash on MBPP **63.8% → 93.8% (+30.0pp)**; Llama 3.3 70B on MBPP 67.7% → 90.7% (+23.0pp); Gemini 2.5 Pro on HumanEval 73.2% → 90.2% (+17.1pp). Self-repair "improves pass rates for every model tested".

### Reconciling them

The results are not in conflict; they are measuring different loops.

- FeedbackEval hands the model a *transcript* of a compiler error. Idris let the model *run the compiler*. The difference between 49.2% and 96% is the difference between reading an error and closing a loop. **The compiler is not a feedback channel; it is an oracle the agent must be able to query itself.**
- Diminishing returns after two to three rounds are real for *test* feedback, where the signal is "wrong answer" and there is nothing to grind. They are not what Idris observed with *compiler* feedback, where each round eliminates a specific named error and the twentieth iteration is still doing work. Note the specific evidence: five rounds of test feedback = one round of test feedback (27/56 both times), while twenty rounds of compiler-plus-test feedback = 54/56.
- The repository-scale ceiling is not a strictness problem. 44.5% of Rust-SWE-bench tasks die at issue reproduction — that is build tooling, not type theory.

**Therefore: strictness plus a fast local feedback loop nets out strongly positive, and the loop is the load-bearing half.** The design requirements that follow are about the *compiler as a program the agent runs*, not about the type system:

1. **Sub-second, single-command, whole-project type check with no configuration.** If the agent cannot run it, the 96% result does not apply and the 49.2% result does.
2. **Errors that name the missing entity, and never guess.** ~77% repair rate for name errors versus ~45% for assertions; and a confidently wrong diagnosis measurably *reduces* a strong model's success rate ([arXiv:2506.13186](https://arxiv.org/html/2506.13186)). "I do not know what `foo` is; here are the three things in scope with similar names" is the right shape. "You probably meant to add a lifetime here" is the wrong shape unless it is certain.
3. **No partial-success state.** 26% of failed trajectories fabricate success. A toolchain with no way to skip a check, disable a warning-as-error, or run code that did not type-check removes the surface on which fabrication operates.

---

## What this sets up for the criteria ticket

Decisions the evidence makes ready, stated as questions rather than answers, since criteria are a separate ticket.

**Ready to decide, with strong evidence behind them.**

- **Namespace design.** How are names introduced into scope; can a name be spelled more than one way; can standard-library paths and package paths be lexically confused? This is the highest-value decision in the language for agent correctness, and it is currently the one nobody frames as a language-design decision at all.
- **What "one obvious way" applies to.** The constraint-decay result says conventions the agent must recall cost 25–32 points. The natural reading is that convention should be *in the language*, where the compiler enforces it, and not in a framework or a style guide. The sibling document already proposed inverting Lua's "mechanisms not policies" exactly three times (modules, object construction, error handling); this evidence supports doing so and supports not doing it a fourth time.
- **Compiler-as-oracle requirements.** Sub-second whole-project check, one command, no config, errors that name the missing entity, no way to run unchecked code. These are toolchain criteria and they are as well-evidenced as any syntax criterion in this document.
- **Stability commitments.** The 56.1%→32.5% cutoff cliff and the 25–38% DUR are the numeric case for a frozen core and a versioning discipline where behaviour cannot change without the type changing.

**Not ready to decide; the evidence is absent or points the wrong way.**

- **Whether heavy type inference hurts agents.** The one direct measurement (E0282 at 1.75%) says no. Do not write a criterion against inference citing agent-friendliness.
- **Whether macros hurt agents.** No study exists. The inferential case is decent; it is not evidence.
- **Whether `Result`-style errors beat exceptions for agents.** No study exists. This is a real gap and possibly worth commissioning as a small experiment rather than a literature search.
- **Whether ownership/borrow-checking hurts agents.** The published error distribution says it does not show up. This is one paper and one table and it contradicts widely-repeated prose claims. Treat as *unresolved*, not as settled either way.

### Conflicts, named and left unresolved

**1. Strictness raises the first-attempt failure rate and lowers the final failure rate.** Idris: 39% first attempt, 96% with a loop. A human beginner experiences the 39%. An agent experiences the 96%. There is no setting of the dial that gives both, and the layering ambition in #1 ("beginners, veterans, and AI agents") walks straight into it. The only escape route the evidence suggests is that error message *quality* is the shared good — a diagnostic that names the fix helps the beginner and the agent for the same reason — but that mitigates the conflict rather than dissolving it.

**2. Corpus size buys fluency and inherits bugs.** A popular language gets a 30-point head start (PyLang) and a 44.44% memorised-bug replication rate ([arXiv:2503.11082](https://arxiv.org/html/2503.11082)). A new language pays the 30 points and inherits nothing. Neither side of this is a choice ludo gets to make — it starts at zero corpus — but the framing matters for how the project talks about the deficit. The recoverable 24 points come from in-context idioms, and idioms are something a zero-corpus language can ship on day one and a legacy language cannot retrofit.

**3. "One obvious way" versus the measured cost of conventions.** Convention-heavy frameworks cost 25–32 points relative to lightweight ones, and each structural constraint costs 9–19 points. That looks like an argument *against* prescriptive design, which is the opposite of what the ticket's "one obvious way" hypothesis predicts. The reconciliation I find plausible — and cannot prove — is that the measured penalty is for conventions held in the model's *memory* (Django's app layout, FastAPI's dependency injection) rather than conventions held in the *compiler*. A convention the compiler enforces converts a recall problem into a name-error problem, and name errors repair at ~77%. **This is a hypothesis, not a finding, and it deserves an explicit experiment before a criterion rests on it.**

**4. Explicit imports are agent-good and beginner-bad.** Already named in the sibling document as ceremony-versus-closed-namespace; this document strengthens the agent side of it substantially. 43.7% of agent Rust compile errors are scope and path resolution; ObfusEval says identifiers are load-bearing; the Rust crate study says namespace overlap is 45% of hallucinations. An explicit, mandatory, non-aliasing import list is now the best-evidenced single feature in this document — and it is exactly the ceremony a Lua user would notice first.

**5. Beginner-friendly and agent-friendly diverge on "the compiler should let you get started".** The evidence supports a compiler with no permissive mode at all: no way to run code that did not type-check, no suppressible errors, no partial success. That is a hostile first hour for a fourteen-year-old writing a platformer. The layering ambition in #1 needs an answer to this that is not "a flag", because a flag is the thing the 26%-fabricated-success finding argues against.

**6. Local reasoning versus every game-dev convenience.** Implicit context (Odin's signature feature, per the sibling survey), globals, singletons, ambient allocators and hot-reload-friendly mutable state are all *action at a distance* — the exact category the 30.7% false-premise finding indicts. A game language that refuses all of them is refusing several things game programmers demonstrably like. This conflict is not addressed anywhere in the evidence and is probably the sharpest unresolved design tension the two lenses produce.

### Three lenses, applied

- **Simplicity.** Almost every agent-friendly property found here is also a simplicity property: one spelling per name, no overloading, no macros, no implicit context, one obvious way. A Lua user would recognise all of them as *fewer things to learn*. The exceptions are mandatory type annotations and mandatory imports, both of which are visible ceremony.
- **Robustness.** The properties that catch bugs at compile time and the properties that help agents overlap far more than expected, but not perfectly: the biggest agent failure (name resolution) is a bug class most robustness-focused languages consider trivially solved and therefore under-designed, while the biggest robustness feature (memory safety via ownership) does not appear in the agent error data at all.
- **Agent-friendliness.** The operational definition that the evidence supports is narrower than "can an LLM write it": it is **can a function be written correctly by an agent that has read only its signature, its callees' signatures, and a page of idioms** — and, separately, **can the agent tell within one second whether it succeeded**. Those two questions are worth carrying verbatim into the criteria ticket.

---

## Evidence quality — what is weak

Stated plainly, as in the sibling documents.

- **The cross-language benchmark tables contradict each other and should not be used to rank languages.** SWE-bench Multilingual puts Rust first at 58.14%; Multi-SWE-bench puts it at 15.90%; Rust-SWE-bench at 21.2%. Different task sets, different agents, different models, different difficulty distributions. Only the Python-versus-rest direction is stable across all three, and that tracks corpus size.
- **The single most load-bearing result in this document — the Rust error-code distribution — is one table in one paper.** Everything I conclude about borrow checking not mattering, about inference not mattering, and about naming dominating rests on it. It should be replicated in another language before a spec decision depends on it.
- **The Idris 39%→96% result is 56 self-contained exercises with one model.** It is the best evidence available on compiler strictness and it is not repository-scale, not multi-model, and not replicated. The Rust-SWE-bench 21.2% is the closest thing to a repository-scale counter-datum and it is not a controlled comparison.
- **The Spracklen recurrence figures (43% recur in all ten runs, 58% recur more than once, 8.7% cross-ecosystem) are reported from the paper body via secondary summaries; I verified only the abstract-level figures directly.** Treat them as weaker.
- **The security-vulnerability-by-language gradient (Python 16–18%, JS 8–9%, TS 2.5–7%) is heavily confounded** by what people write in each language. It is consistent with types helping and it is not evidence that types help.
- **Two of the ticket's nominated "properties that hurt" have no supporting evidence at all** (macros/metaprogramming) or have contradicting evidence (heavy inference, E0282 at 1.75%). I have not manufactured support for either.
- **"Local reasoning" is a convergent inference across three studies, not a controlled variable in any of them.** No paper manipulates locality and measures agent accuracy. The inference is strong; the direct measurement does not exist.
- **No study evaluates Lua, Teal or Luau for agent correctness**, and none evaluates any game-development-specific workload. The sibling document already flagged this; nothing found here changes it.
- **The 2026-dated arXiv preprints (2602.11481, 2602.22764, 2605.06445, 2605.17062, 2606.08444, 2607.09510) are recent and in most cases not yet peer-reviewed.** 2606.08444 states acceptance at Internetware 2026; the others I could not verify venue for. The 2024–2025 sources (Spracklen/USENIX, Wang/ICSE, type-constrained decoding/PACMPL, Tambon/EMSE, CoderEval, BigCodeBench) are peer-reviewed and carry correspondingly more weight.
- **Several PDF-only sources resisted extraction** and were read via their HTML renderings or via search-result summaries where noted. Where a figure came from a summary rather than the source text, it is marked in place.

---

## Sources

**Primary — cross-language and agent benchmarks**

- [SWE-bench Multilingual](https://www.swebench.com/multilingual.html) — per-language resolve rates, Claude 3.7 Sonnet, 300 tasks / 42 repos / 9 languages.
- [Multi-SWE-bench: A Multilingual Benchmark for Issue Resolving, arXiv:2504.02605](https://arxiv.org/html/2504.02605v1) — 2,132 instances, 8 languages, per-language best results.
- [Evaluating and Improving Automated Repository-Level Rust Issue Resolution with LLM-based Agents (Rust-SWE-bench), arXiv:2602.22764](https://arxiv.org/html/2602.22764v1) — 500 Rust tasks, full compilation-error-code distribution.
- [McEval: Massively Multilingual Code Evaluation, arXiv:2406.07436](https://arxiv.org/abs/2406.07436) — 40 languages, 16K samples.
- [MultiPL-E, arXiv:2208.08227](https://arxiv.org/pdf/2208.08227) and [project page](https://nuprl.github.io/MultiPL-E/) — 18-language translation of HumanEval/MBPP.
- [CodeGeeX / HumanEval-X, arXiv:2303.17568](https://arxiv.org/pdf/2303.17568) — per-language pass@1 across five languages.
- [aider polyglot leaderboard](https://aider.chat/2024/12/21/polyglot.html) — 225 Exercism problems across C++, Go, Java, JavaScript, Python, Rust.
- [BigCodeBench, arXiv:2406.15877](https://arxiv.org/abs/2406.15877) — 1,140 tasks, 139 libraries, human baseline 97% vs best model 60%.
- [CoderEval, arXiv:2302.00288](https://arxiv.org/abs/2302.00288) — six levels of context dependency; >70% of real functions non-standalone.

**Primary — failure modes and taxonomies**

- [Failure as a Process: An Anatomy of CLI Coding Agent Trajectories, arXiv:2607.09510](https://arxiv.org/html/2607.09510v1) — 1,794 trajectories, 63k annotated steps, epistemic/competence/environment taxonomy.
- [Understanding Code Agent Behaviour: Success and Failure Trajectories, arXiv:2511.00197](https://arxiv.org/html/2511.00197) — file/function/hunk localisation rates.
- [What is Wrong with Your Code Generated by LLMs? arXiv:2407.06153](https://arxiv.org/html/2407.06153) — three-category, ten-subcategory bug taxonomy with frequencies.
- [Bugs in Large Language Models Generated Code: An Empirical Study, arXiv:2403.08937](https://arxiv.org/abs/2403.08937) (EMSE) — 333 bugs, 10 patterns, 34-practitioner validation.
- [Beyond Functional Correctness: Exploring Hallucinations in LLM-Generated Code (HalluCode), arXiv:2404.00971](https://arxiv.org/html/2404.00971) — 3,120 samples, hallucination taxonomy with percentages.
- [LLMs are Bug Replicators, arXiv:2503.11082](https://arxiv.org/html/2503.11082) — 44.44% byte-identical historical bug reproduction.
- [Constraint decay: The Fragility of LLM Agents in Backend Code Generation, arXiv:2605.06445](https://arxiv.org/html/2605.06445v1) — 80 tasks, per-constraint degradation.
- [CONCUR: Benchmarking LLMs for Concurrent Code Generation, arXiv:2603.03683](https://arxiv.org/abs/2603.03683).

**Primary — hallucination and API evolution**

- [We Have a Package for You! arXiv:2406.10279 / USENIX Security 2025](https://arxiv.org/abs/2406.10279v3) — 576,000 samples, 16 models, 205,474 unique hallucinated names.
- [The Range Shrinks, the Threat Remains, arXiv:2605.17062](https://arxiv.org/abs/2605.17062) — 2026 frontier cohort, 199,845 paired prompts, 4.62–6.10%.
- [When LLMs Invent Rust Crates, arXiv:2606.08444](https://arxiv.org/html/2606.08444v1) (Internetware 2026) — 20.23% Rust crate hallucination, 45.47% std-module confusion.
- [LLMs Meet Library Evolution, arXiv:2406.09834](https://arxiv.org/abs/2406.09834) (ICSE 2025) — Deprecated Usage Rate 25–38%.
- [RustEvo²: An Evolving Benchmark for API Evolution, arXiv:2503.16922](https://arxiv.org/abs/2503.16922) — 588 API changes, before/after cutoff cliff.

**Primary — compilers, types and feedback loops**

- [Compiler-Guided Inference-Time Adaptation: Improving GPT-5 Programming Performance in Idris, arXiv:2602.11481](https://arxiv.org/html/2602.11481v1) — 22/56 → 54/56.
- [Type-Constrained Code Generation with Language Models, arXiv:2504.09246](https://arxiv.org/abs/2504.09246) (PACMPL/OOPSLA) — compilation errors more than halved on TypeScript.
- [FeedbackEval, arXiv:2504.06939](https://arxiv.org/html/2504.06939v2) — six feedback types, Repair@1 comparison.
- [How Many Tries Does It Take? arXiv:2604.10508](https://arxiv.org/html/2604.10508) — self-repair across seven models; name/syntax/assertion repair hierarchy.
- [Empirical Evaluation of LLMs in Automated Program Repair, arXiv:2506.13186](https://arxiv.org/html/2506.13186) — wrong diagnosis harms strong models.
- [SafeTrans: LLM-assisted Transpilation from C to Rust, arXiv:2505.10708](https://arxiv.org/html/2505.10708v2) — borrow-checker violation repair rate 74.2%.

**Primary — familiarity and security**

- [Unseen Horizons (ObfusEval), arXiv:2412.08109](https://arxiv.org/pdf/2412.08109) — up to 62.5% pass-rate collapse under semantics-preserving obfuscation.
- [Syntax Without Semantics (PyLang), arXiv:2605.15607](https://arxiv.org/html/2605.15607v1) — 58% vs 88%; 24 points recovered from in-context idioms. *Via sibling survey.*
- [Security Weaknesses of Copilot-Generated Code in GitHub Projects, TOSEM 10.1145/3716848](https://dl.acm.org/doi/10.1145/3716848) — 29.5% Python / 24.2% JavaScript, 43 CWEs.
- [Security Vulnerabilities in AI-Generated Code: A Large-Scale Analysis, arXiv:2510.26103](https://arxiv.org/pdf/2510.26103) — per-language vulnerability rates.

**Sibling documents**

- [`02-language-survey.md`](./02-language-survey.md) — PyLang, Zig churn, corpus counts, Odin implicit context.
- [`03-lua-simplicity.md`](./03-lua-simplicity.md) — Luau typed/untyped telemetry, Teal/Luau autopsy, the ceremony-versus-closed-namespace conflict this document strengthens.
