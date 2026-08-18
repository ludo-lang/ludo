#!/usr/bin/env python3
"""Check the induced-hole ledger convention mechanically.

The convention (docs/agents/domain.md): a chapter's docs/spec/coverage/NN-*.md
carries one line per *induced* hole -- a hole opened by writing a repair, not by
reading -- naming the ticket whose repair opened it. The ledger answers a trend
question (ADR-0051 §1): a transcription hole is finite, an induced hole is
feedback and is the only class that can diverge.

Per ADR-0051 §6 a convention an author can forget ships with a checker or with a
note saying why it cannot have one. #127 is the evidence it was needed: chapter 4
shipped with no ledger section and nothing detected it. Chapters 5-8 all kept it,
so the rule was skipped once, at the first chapter it bound -- not abandoned.

The ledger starts at chapter 4. Chapters 1-3 are exempt: reconstructing cause for
closed tickets is archaeology. Chapter 4 was backfilled by #127 on a narrower
ground -- its §8 enumerates its own repairs, written in the same commit -- and
that ground is evidence, not cost, so it does not license backfilling 1-3.

WHAT THIS CHECKS, AND WHAT IT CANNOT

Structural only: that the section exists. It does not check that the section is
*current*. Nothing mechanical can know a repair opened a hole -- that judgement
is the author's, and it is the whole content of the convention. So a chapter that
carries an empty ledger while induced holes pile up passes here.

That blind spot is the ADR-0051 §6 disclosure for the other half of this rule,
and it is why the check is worth having anyway: the failure that actually
happened was a missing section, and a missing section is exactly what a machine
can see.
"""

import argparse
import pathlib
import re
import sys

COVERAGE_NUM_RE = re.compile(r"^0?(\d+)-")
# Chapter 5, 6 and 7 head it; chapter 8 states it inline in the prose. Both are
# the convention kept, so match the sentence rather than the heading.
LEDGER_RE = re.compile(r"Induced holes opened by this chapter", re.IGNORECASE)

# The ledger starts here. See the module docstring.
FIRST_BOUND_CHAPTER = 4


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--coverage-dir", default="docs/spec/coverage")
    args = ap.parse_args()

    coverage_dir = pathlib.Path(args.coverage_dir)
    if not coverage_dir.is_dir():
        print(f"FINDING  no coverage directory at {coverage_dir}")
        return 1

    findings = []
    checked = 0
    exempt = 0

    for path in sorted(coverage_dir.glob("*.md")):
        m = COVERAGE_NUM_RE.match(path.name)
        if not m:
            continue
        chapter = int(m.group(1))
        if chapter < FIRST_BOUND_CHAPTER:
            exempt += 1
            continue
        checked += 1
        if not LEDGER_RE.search(path.read_text()):
            findings.append(
                f"{path.name}: no induced-hole ledger -- "
                f"add a section saying 'Induced holes opened by this chapter', "
                f"'None' included"
            )

    for f in findings:
        print(f"FINDING  {f}")
    print(
        f"\n{checked} chapter(s) checked, {exempt} exempt (ch1-{FIRST_BOUND_CHAPTER - 1}), "
        f"{len(findings)} finding(s)."
    )
    print("Structural only: an empty ledger and a correct ledger are the same to this check.")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
