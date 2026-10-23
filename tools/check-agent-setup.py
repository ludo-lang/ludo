#!/usr/bin/env python3
"""Check the shared instruction entry point and vendored skill adapters."""

import json
from pathlib import Path
import sys


def check(root):
    findings = []
    shared = root / "AGENTS.md"
    adapter = root / "CLAUDE.md"
    if not shared.is_file() or not shared.read_text().strip():
        findings.append("AGENTS.md must contain the shared project instructions")
    if not adapter.is_file() or adapter.read_text().strip() != "@AGENTS.md":
        findings.append("CLAUDE.md must import the shared instructions with @AGENTS.md")

    try:
        lock = json.loads((root / "skills-lock.json").read_text())
        names = set(lock["skills"])
    except (OSError, ValueError, KeyError, TypeError) as error:
        findings.append(f"cannot read the skill inventory: {error}")
        return findings

    skills = root / ".agents/skills"
    adapters = root / ".claude/skills"
    installed = {p.name for p in skills.iterdir() if p.is_dir()} if skills.is_dir() else set()
    linked = {p.name for p in adapters.iterdir()} if adapters.is_dir() else set()
    for name in sorted(names - installed):
        findings.append(f"locked skill {name} is missing from .agents/skills/")
    for name in sorted(installed - names):
        findings.append(f"installed skill {name} is missing from skills-lock.json")
    for name in sorted(linked - names):
        findings.append(f"Claude skill adapter {name} has no locked skill")

    for name in sorted(names & installed):
        skill = skills / name
        if skill.is_symlink() or not (skill / "SKILL.md").is_file():
            findings.append(f"{skill.relative_to(root)} must vendor its own SKILL.md")
        link = adapters / name
        expected = Path("../../.agents/skills") / name
        if not link.is_symlink() or link.readlink() != expected or not link.is_dir():
            findings.append(f".claude/skills/{name} must symlink to {expected}")

    return findings


def main():
    root = Path(__file__).resolve().parent.parent
    findings = check(root)
    for finding in findings:
        print(f"FINDING  {finding}")
    if findings:
        return 1
    print("Agent setup: shared instructions and all skill adapters agree.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
