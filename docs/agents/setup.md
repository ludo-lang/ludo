# Agent setup

`AGENTS.md` is the shared project instruction file. Keep `CLAUDE.md` as the
`@AGENTS.md` import so Claude uses the same guidance. An agent that does not
discover `AGENTS.md` automatically should be told to read it before working.
Keep project conventions in `docs/agents/`, reached by pointers in `AGENTS.md`.

## Skills

The repo vendors its installed skills in `.agents/skills/`, alongside their
resources and `agents/openai.yaml` metadata. Include those files,
`skills-lock.json`, and the relative symlinks under `.claude/skills/` in version
control. A clone then carries the same locally adapted workflows without a
network install. Codex discovers `.agents/skills/`; the `.claude/skills/` links
provide Claude's entry points into the same files. Other agents may need their
own discovery adapter or an explicit instruction to read a skill's `SKILL.md`.

Load a named skill through the current agent's mechanism, or read its `SKILL.md`
directly. References to slash commands describe workflow entry points; use the
current agent's equivalent for skills and session controls. Agent-specific
metadata applies where supported; the host's permissions still govern execution.
Claude's `disable-model-invocation` and `argument-hint` fields stay in `SKILL.md`;
Codex's explicit-invocation policy is carried by `agents/openai.yaml`.
Use the host's tools for questions and delegation. When a workflow requires a
capability the host lacks, report that limit before proceeding with dependent work.

`skills-lock.json` records the installed upstream sources and hashes. The vendored
files also contain local portability edits: neutral skill-loading language,
`AGENTS.md` as the setup target, and an explicit Claude-only handoff adapter.
When updating from upstream, review the diff and retain those edits. The lock file
alone does not reconstruct this locally adapted snapshot.

## Agent-specific operations

- `handoff` writes a document another agent can read. `claude-handoff` additionally
  launches the Claude CLI; use it only when that is the requested destination.
- `git-guardrails-claude-code` installs Claude hooks. Configure other agents through
  their own permission systems; the presence of a skill does not install a hook.
- Keep credentials, personal memory, and machine-specific settings outside the
  shared instructions and skills. Share reproducible validation through `Makefile`,
  `tools/`, and CI.

## Check the shared setup

Run `python3 tools/check-agent-setup.py` after changing instruction adapters or
installing skills. CI runs the same check. It checks the shared entry point, the
lock-file inventory, and the Claude skill links. It cannot prove that every agent
loads or interprets a workflow correctly: verify discovery in a fresh session of
each agent you use, asking it to identify the project instructions, build command,
and an installed skill.
