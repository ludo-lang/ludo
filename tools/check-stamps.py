#!/usr/bin/env python3
"""Check the ADR amendment-stamp convention mechanically.

The convention (docs/agents/domain.md): when a later ADR changes, narrows,
corrects or overrides part of an earlier one, the earlier one carries a
one-line forward-pointer stamp under its front matter.

Per ADR-0051 §6 a convention that an author can forget ships with a checker or
with a note saying why it cannot have one. This is that checker, and it is a
FLOOR: it catches declared amendments that were never stamped, and malformed
stamps. It cannot catch an ADR that amends something without declaring it --
that case stays human, assigned to the spec chapter covering the ADR (#72).

Usage:  python3 tools/check-stamps.py [--adr-dir docs/adr]
Exit:   0 clean, 1 findings.
"""

import argparse
import pathlib
import re
import sys

# > **Amended by [ADR-0009](0009-....md):**  /  > **Absorbed by [spec §6.2](...):**
STAMP_RE = re.compile(
    r"^>\s*\*\*(Amended|Absorbed) by \[([^\]]+)\]\(([^)]+)\)", re.MULTILINE
)
ADR_REF_RE = re.compile(r"ADR-(\d{4})")
FILE_NUM_RE = re.compile(r"^(\d{4})-")
# A heading that introduces a list of amendments this ADR makes to others.
# An entry that says the target did not move. Verified against all nine
# first-run findings: three of the eight false positives read this way.
NOT_AN_AMENDMENT_RE = re.compile(
    r"\b(unchanged|not amended here|untouched|no stamp)\b", re.IGNORECASE
)
AMEND_SECTION_RE = re.compile(
    r"^#{1,4} .*\b(amendment|amendments|stamps? this adr|amends)\b.*$",
    re.MULTILINE | re.IGNORECASE,
)


def adr_number(path):
    m = FILE_NUM_RE.match(path.name)
    return int(m.group(1)) if m else None


def front_matter_block(text):
    """The stamp region: everything between the front matter and the H1."""
    if not text.startswith("---"):
        return text.split("\n# ", 1)[0]
    _, _, rest = text.partition("---\n")
    _, _, after = rest.partition("---\n")
    return after.split("\n# ", 1)[0]


def declared_amendment_targets(text, self_num):
    """ADR numbers this ADR says, in an amendments section, that it amends.

    Only the *bullet key* counts. An amendments list reads

        - **ADR-0004** — the bit-exactness bullet is deleted, on ADR-0022's
          reasoning, which reached ADR-0005 and not this ADR.

    where ADR-0004 is the target and ADR-0022 / ADR-0005 are prose. Reading
    every reference in the section instead of the key produced eight false
    positives out of nine on the first run, so the key is the whole rule.

    An entry that marks itself unchanged is not an amendment either.
    """
    targets = set()
    for m in AMEND_SECTION_RE.finditer(text):
        # Section body runs to the next heading of the same or higher level.
        start = m.end()
        level = len(m.group(0)) - len(m.group(0).lstrip("#"))
        nxt = re.compile(r"^#{1,%d} " % level, re.MULTILINE).search(text, start)
        body = text[start : nxt.start() if nxt else len(text)]

        # Split into bullets; each bullet's bolded lead is its key.
        bullets = re.split(r"^\s*[-*]\s+", body, flags=re.MULTILINE)[1:]
        for b in bullets:
            key = re.match(r"\*\*([^*]+)\*\*", b)
            if not key:
                continue
            if NOT_AN_AMENDMENT_RE.search(b):
                continue
            for n in ADR_REF_RE.findall(key.group(1)):
                n = int(n)
                if n != self_num:
                    targets.add(n)
    return targets


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--adr-dir", default="docs/adr")
    args = ap.parse_args()

    adr_dir = pathlib.Path(args.adr_dir)
    if not adr_dir.is_dir():
        print(f"no such directory: {adr_dir}", file=sys.stderr)
        return 2

    paths = sorted(p for p in adr_dir.glob("*.md") if adr_number(p) is not None)
    by_num = {adr_number(p): p for p in paths}
    text = {adr_number(p): p.read_text(encoding="utf-8") for p in paths}

    findings = []

    # stamped_by[target] = {stamping ADR numbers}
    stamped_by = {n: set() for n in by_num}

    for num, path in by_num.items():
        block = front_matter_block(text[num])
        for kind, label, href in STAMP_RE.findall(block):
            if kind == "Absorbed":
                continue  # target is a spec chapter, not an ADR
            ref = ADR_REF_RE.search(label)
            if not ref:
                findings.append(f"{path.name}: stamp label names no ADR: [{label}]")
                continue
            src = int(ref.group(1))

            # The link must resolve to a file on disk.
            target = (path.parent / href).resolve()
            if not target.exists():
                findings.append(f"{path.name}: stamp by ADR-{src:04d} links to a missing file: {href}")

            # The link filename must agree with the number in the label.
            hm = FILE_NUM_RE.match(pathlib.Path(href).name)
            if hm and int(hm.group(1)) != src:
                findings.append(
                    f"{path.name}: stamp says ADR-{src:04d} but links to {pathlib.Path(href).name}"
                )

            # An amendment is always backwards.
            if src <= num:
                findings.append(
                    f"{path.name}: stamped by ADR-{src:04d}, which is not later than ADR-{num:04d}"
                )

            stamped_by.setdefault(num, set()).add(src)

    # Declared amendments must be reciprocated by a stamp on the target.
    for num, path in by_num.items():
        for tgt in sorted(declared_amendment_targets(text[num], num)):
            if tgt not in by_num:
                findings.append(f"{path.name}: declares an amendment to ADR-{tgt:04d}, which does not exist")
                continue
            if tgt > num:
                continue  # forward reference in prose, not an amendment it makes
            if num not in stamped_by.get(tgt, set()):
                findings.append(
                    f"{by_num[tgt].name}: missing stamp -- ADR-{num:04d} declares it amends this ADR"
                )

    for f in findings:
        print(f"FINDING  {f}")
    print(f"\n{len(by_num)} ADRs checked, {len(findings)} finding(s).")
    print("Floor only: an ADR that amends without declaring it is invisible here (#72).")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
