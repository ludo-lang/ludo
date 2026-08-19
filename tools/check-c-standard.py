#!/usr/bin/env python3
"""Check the C subset mechanically -- the grep-shaped half of it.

The convention (docs/agents/c-standard.md, from #131): the subset is a ban list,
and prose bans only what a flag cannot see. The warning configuration in the root
Makefile is the primary artifact; this file covers the bans a warning has no flag
for, so that the written standard is left with only what a reader must judge.

Per ADR-0051 §6 a convention an author can forget ships with a checker. ADR-0020
names the reason this one is not optional: agents write confident undefined
behaviour, so an unenforced ban shifts the review burden rather than shrinking it.

WHAT THIS CHECKS, AND WHAT IT CANNOT

It is a line scanner over src/**.c and src/**.h with comments and string literals
stripped. It sees a token in the wrong file; it does not parse C. It therefore
cannot tell a tag-enum switch from a switch over an int -- so the rule is written
the other way round: a `default:` label must carry the marker comment
`/* not-a-tag-enum */` to be allowed, which turns an unprovable judgement into a
visible claim. Nor can it see varargs definitions, type-punning unions, backward
gotos or macros that should be functions; those are the review list at the end of
the standard, and they are named there as unmechanised on purpose.

Two carve-outs, from the standard's Scope section: src/driver/ owns the process
and may do I/O, and test binaries plus src/test.h are not library code and may
hold file-scope mutable state.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / "src"

ARENA_ALLOW = "ludo-allow-malloc"
TAG_ENUM_OPT_OUT = "not-a-tag-enum"

# (regex, message, predicate on the repo-relative path saying where it applies)
EVERYWHERE = lambda p: True
LIBRARIES_ONLY = lambda p: not p.startswith("src/driver/") and not _is_test(p)

RULES = [
    (r"\balloca\s*\(", "alloca: an unbounded stack allocation with no failure mode", EVERYWHERE),
    (r"\berrno\b", "errno: errors leave as diagnostics and status codes (#130)", EVERYWHERE),
    (r"#\s*include\s*<threads\.h>", "threads.h: ADR-0020 assumes no threads", EVERYWHERE),
    (r"#\s*include\s*<stdatomic\.h>", "stdatomic.h: ADR-0020 assumes no threads", EVERYWHERE),
    (r"\b_Thread_local\b|\bthread_local\b", "thread-local storage: ADR-0020 assumes no threads", EVERYWHERE),
    (r"\b_Generic\b", "_Generic: type-directed dispatch a reader cannot follow", EVERYWHERE),
    (r"#\s*include\s*<stdarg\.h>", "stdarg.h: no varargs functions of our own", LIBRARIES_ONLY),
    (r"\b(malloc|calloc|realloc|free)\s*\(",
     "allocation outside the arena (#130); mark the arena itself with " + ARENA_ALLOW,
     EVERYWHERE),
]


def _is_test(path: str) -> bool:
    return "/tests/" in path or path == "src/test.h"


def strip_noise(line: str) -> str:
    """Remove string literals, char literals and line comments.

    Block comments are handled by the caller, which tracks the open/close state
    across lines. This is a scanner, not a lexer: it is allowed to be crude, and
    the cost of being wrong is a false positive an author can see and fix.
    """
    line = re.sub(r'"(\\.|[^"\\])*"', '""', line)
    line = re.sub(r"'(\\.|[^'\\])*'", "''", line)
    line = re.sub(r"//.*", "", line)
    return line


def file_scope_mutable_global(line: str, brace_depth: int) -> bool:
    """A definition at file scope that is neither const nor a function.

    Only checked at brace depth 0. `static const`, `const`, function prototypes
    and definitions, and typedefs are all fine.
    """
    if brace_depth != 0:
        return False
    stripped = line.strip()
    if not stripped or stripped.startswith("#"):
        return False
    if not stripped.endswith(";") and not stripped.endswith("= {"):
        return False
    if "(" in stripped:  # a prototype or a function-pointer typedef
        return False
    if re.match(r"^(typedef|struct|union|enum|extern)\b", stripped):
        return False
    if re.search(r"\bconst\b", stripped):
        return False
    # A bare declarator with a type in front: `static int counter;`
    return re.match(r"^[A-Za-z_][A-Za-z0-9_ *]*\s+[A-Za-z_][A-Za-z0-9_]*\s*(\[[^\]]*\])?\s*(=|;)", stripped) is not None


def check_file(path: pathlib.Path) -> list[str]:
    rel = path.relative_to(ROOT).as_posix()
    problems = []
    in_block_comment = False
    brace_depth = 0

    for number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = raw
        if in_block_comment:
            if "*/" in line:
                line = line.split("*/", 1)[1]
                in_block_comment = False
            else:
                continue
        while "/*" in line:
            head, rest = line.split("/*", 1)
            if "*/" in rest:
                line = head + " " + rest.split("*/", 1)[1]
            else:
                line = head
                in_block_comment = True
                break

        code = strip_noise(line)

        for pattern, message, applies in RULES:
            if not applies(rel):
                continue
            if re.search(pattern, code):
                if ARENA_ALLOW in raw and "alloc" in message:
                    continue
                problems.append(f"{rel}:{number}: {message}")

        if "default:" in code and TAG_ENUM_OPT_OUT not in raw:
            problems.append(
                f"{rel}:{number}: default: in a switch -- banned over a tag enum, because it "
                f"is what silences -Wswitch-enum. If this switch is not over a tag enum, say "
                f"so with /* {TAG_ENUM_OPT_OUT} */."
            )

        if not _is_test(rel) and file_scope_mutable_global(code, brace_depth):
            problems.append(f"{rel}:{number}: file-scope mutable global: no globals, no shared state (#130)")

        brace_depth += code.count("{") - code.count("}")
        if brace_depth < 0:
            brace_depth = 0

    return problems


def main() -> int:
    if not SRC.is_dir():
        print("no src/ directory; nothing to check")
        return 0

    files = sorted(p for p in SRC.rglob("*") if p.suffix in {".c", ".h"})
    problems = []
    for path in files:
        problems.extend(check_file(path))

    if problems:
        print("C standard violations (docs/agents/c-standard.md):\n")
        for problem in problems:
            print(f"  {problem}")
        print(f"\n{len(problems)} violation(s) in {len(files)} file(s).")
        return 1

    print(f"C standard: {len(files)} file(s) clean.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
