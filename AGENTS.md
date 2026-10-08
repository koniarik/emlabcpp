# Agent Instructions

The rules below are for coding agents working on emlabcpp.

## Source comments

Write each comment for a future reader of the file as it exists now, who has never seen
this change, its discussion, the previous version, or the code that calls it. A comment
states what the code cannot show: a contract (ownership, lifetime, units, what a failed
call leaves untouched), an invariant, a hidden constraint, or a workaround. Anything else
is already in the code or is true only of this change, and it goes stale silently because
nothing fails when it does.

This applies equally to doc comments on types, fields and functions (`///`, `/** */`) and
to inline `//` comments.

Before writing a comment, ask: would I write this sentence if I were writing the file from
scratch today, with no diff in mind and without knowing who calls this code?

| Instead of | Write |
|---|---|
| `/// Alignment of the node.` above `alignment` | nothing, or what the name cannot carry: `/// Power of two.` |
| `Allocator& alloc_; // for allocation and deallocation` | nothing, or the contract: `// Must outlive this pool.` |
| `// Moved out of pool.cpp; now returns a span.` | nothing: describe what the code does, not what it did |
| `// A comma fold reads more simply but measured +18 bytes.` | nothing, or a present-tense constraint if a reader would otherwise undo it: `// Must stay a loop: the fold form exceeds the size budget.` |

Put history, measurements, rejected alternatives and lists of callers in your reply to the
user or in the commit message. Leave existing comments alone unless they are wrong or the
code they describe changed. Tool directives (`NOLINT`, `eslint-disable`, `@ts-expect-error`)
and license headers are exempt.

After each edit, `.claude/hooks/comment_check.py` lists added comments whose wording often
signals one of these problems ("now", "no longer", "used by", "rather than", "measured").
Rewrite or delete each listed comment, or keep it if it states a present-tense contract.
