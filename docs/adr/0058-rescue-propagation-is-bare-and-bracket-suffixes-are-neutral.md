---
status: accepted
---

> **Absorbed by [spec ch1 §6.5.1, §7.14.1 and §9.8](../spec/01-grammar.md):** the chapter is normative; this ADR keeps the argument.

# Rescue propagation is bare, and bracket suffixes are neutral

Writing the parser for [The parser, the AST it mints, and the error nodes that
survive a bad parse](https://github.com/ludo-lang/ludo/issues/142) made three
existing grammar boundaries fail on the reference program. The driving dev
accepted these repairs on 2026-10-23.

- **`rescue return` is bare propagation.** Reusing `ReturnStatement` let it
  swallow the following `n` in `n := g() rescue return n`, because newlines
  are insignificant. Returning a chosen value uses a bound handler. Keeping
  optional values would require a delimiter or another ambiguity rule; neither
  earns its cost for propagation, whose purpose is already fixed.
- **A call statement may carry its rescue tail.** The reference frame already
  wrote that shape, but `CallStatement ::= PostfixExpr` excluded it. Arbitrary
  arithmetic does not thereby become a statement. A following `if … then`
  starts a compound statement; the `then` distinguishes it from a modifier.
- **Expression brackets retain a receiver and an argument list.** Chapter 2
  already required `compare[int]`, yet an expression suffix admitted only one
  Expr and therefore excluded multiple/type-only arguments. Reusing
  `GenericArgs` lets parsing stay independent of name lookup; checking decides
  indexing versus generic instantiation. The alternative, classifying names in
  the parser, would violate the frontend's seam and duplicate the name table.

The reference's leading-dot variants are repaired to qualified constructors,
not adopted as new syntax: chapter 2 §4.6.1 already decides that. Its multiple
value tail becomes an explicit `return`. `QualifiedName` gains the existing
`$.` root because chapter 6 §12.4 already requires it. These are corpus repairs,
not additional decisions. No older ADR states the three defective productions;
the amendments are to the grammar and chapter 1 in this commit.
