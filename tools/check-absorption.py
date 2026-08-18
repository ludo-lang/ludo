#!/usr/bin/env python3
"""Check the ADR absorption-stamp convention mechanically.

The convention (docs/agents/domain.md, ADR-0051 §4): when a spec chapter
transcribes an ADR's clauses, the ADR carries a one-line stamp under its front
matter saying which chapter is now normative for it.

Per ADR-0051 §6 a convention an author can forget ships with a checker or with a
note saying why it cannot have one. ADR-0055 §4 specifies this one, and #120 is
the evidence it was needed: four chapters skipped the stamp and nothing detected
the skip for four chapters' worth of work. tools/check-stamps.py cannot see it --
an amendment is declared in the amending ADR, an absorption is declared nowhere.

The input is the *declared* absorption line in each docs/spec/coverage/NN-*.md:

    **Absorbed here:** ADR-0014, ADR-0021, ADR-0027, ADR-0054.

That line is authored rather than scraped from the file's prose, for two reasons
found while writing this: the prose source lists name absorbed and merely
*reached* ADRs in one breath (chapter 8 §7 reaches ADR-0044, ADR-0049 and
ADR-0051 while absorbing none), and they were written when the chapter landed, so
they miss the repair ADRs a chapter absorbs afterwards (ADR-0054 into chapter 4).

Both directions are checked: a listed ADR owes a stamp naming that chapter, and a
stamp naming a chapter owes a line in that chapter's list.

This is a FLOOR, in the same sense as check-stamps.py. A chapter that transcribes
an ADR and leaves it off its own list is invisible here. What it does catch is the
#120 failure: a chapter that lands, absorbs, and never stamps.

An ADR named by no chapter's list is not a finding. ADR-0055 §2: absorption has a
complement -- ADRs deciding an unmandated surface, and process ADRs binding
authors rather than programs, are absorbed by nothing and never will be.

Usage:  python3 tools/check-absorption.py [--adr-dir docs/adr] [--coverage-dir docs/spec/coverage]
Exit:   0 clean, 1 findings.
"""

import argparse
import pathlib
import re
import sys

FILE_NUM_RE = re.compile(r"^(\d{4})-")
COVERAGE_NUM_RE = re.compile(r"^0?(\d+)-")
ADR_REF_RE = re.compile(r"ADR-(\d{4})")
# **Absorbed here:** ADR-0014, ADR-0021, ADR-0027, ADR-0054.
DECLARED_RE = re.compile(r"^\*\*Absorbed here:?\*\*(.*?)(?:\n\n|\Z)", re.MULTILINE | re.DOTALL)
# > **Absorbed by [spec ch3 §11](../spec/03-memory.md):**
ABSORBED_RE = re.compile(
    r"^>\s*\*\*Absorbed by \[spec ch(\d+)[^\]]*\]\(([^)]+)\)", re.MULTILINE
)


def front_matter_block(text):
    """The stamp region: everything between the front matter and the H1."""
    if not text.startswith("---"):
        return text.split("\n# ", 1)[0]
    _, _, rest = text.partition("---\n")
    _, _, after = rest.partition("---\n")
    return after.split("\n# ", 1)[0]


def declared_absorptions(text):
    """The ADR numbers a coverage file declares it absorbed, or None if it declares nothing."""
    m = DECLARED_RE.search(text)
    if not m:
        return None
    return {int(n) for n in ADR_REF_RE.findall(m.group(1))}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--adr-dir", default="docs/adr")
    ap.add_argument("--coverage-dir", default="docs/spec/coverage")
    args = ap.parse_args()

    adr_dir = pathlib.Path(args.adr_dir)
    cov_dir = pathlib.Path(args.coverage_dir)
    for d in (adr_dir, cov_dir):
        if not d.is_dir():
            print(f"no such directory: {d}", file=sys.stderr)
            return 2

    findings = []

    # Chapter -> the ADRs its coverage file says it absorbed.
    declared = {}
    for path in sorted(cov_dir.glob("*.md")):
        m = COVERAGE_NUM_RE.match(path.name)
        if not m:
            continue
        chapter = int(m.group(1))
        nums = declared_absorptions(path.read_text(encoding="utf-8"))
        if nums is None:
            findings.append(f"{path.name}: no '**Absorbed here:**' line (ADR-0055 §4)")
            continue
        declared[chapter] = nums

    # ADR -> the chapters its stamps name.
    stamped = {}
    adrs = {}
    for path in sorted(adr_dir.glob("*.md")):
        m = FILE_NUM_RE.match(path.name)
        if not m:
            continue
        num = int(m.group(1))
        adrs[num] = path
        text = path.read_text(encoding="utf-8")
        for chapter, href in ABSORBED_RE.findall(front_matter_block(text)):
            chapter = int(chapter)
            stamped.setdefault(num, set()).add(chapter)

            # The link must resolve, and to the chapter the label names.
            target = (path.parent / href.split("#")[0]).resolve()
            if not target.exists():
                findings.append(f"{path.name}: absorption stamp links to a missing file: {href}")
                continue
            fm = COVERAGE_NUM_RE.match(target.name)
            if fm and int(fm.group(1)) != chapter:
                findings.append(
                    f"{path.name}: stamp says ch{chapter} but links to {target.name}"
                )

    # Direction 1 -- the #120 failure: a chapter absorbed it and never stamped it.
    for chapter, nums in sorted(declared.items()):
        for num in sorted(nums):
            if num not in adrs:
                findings.append(f"coverage ch{chapter}: lists ADR-{num:04d}, which does not exist")
            elif chapter not in stamped.get(num, set()):
                findings.append(
                    f"{adrs[num].name}: missing stamp -- coverage ch{chapter} lists it as absorbed"
                )

    # Direction 2 -- a stamp naming a chapter that does not claim it.
    for num, chapters in sorted(stamped.items()):
        for chapter in sorted(chapters):
            if chapter not in declared:
                findings.append(
                    f"{adrs[num].name}: stamped by ch{chapter}, which has no coverage file"
                )
            elif num not in declared[chapter]:
                findings.append(
                    f"coverage ch{chapter}: does not list ADR-{num:04d}, which is stamped by it"
                )

    for f in findings:
        print(f"FINDING  {f}")
    absorbed = len(stamped)
    print(
        f"\n{len(adrs)} ADRs, {len(declared)} chapters, {absorbed} absorbed, "
        f"{len(adrs) - absorbed} not absorbed, {len(findings)} finding(s)."
    )
    print("Floor only: a chapter that absorbs without listing the source is invisible here.")
    print("Not absorbed is not a finding: absorption has a complement (ADR-0055 §2).")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
