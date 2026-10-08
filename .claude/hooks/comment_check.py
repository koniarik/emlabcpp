#!/usr/bin/env python3
"""Claude Code PostToolUse hook: list added source comments that break the comment rules.

Vendor as .claude/hooks/comment_check.py and register it in .claude/settings.json:

    {
      "hooks": {
        "PostToolUse": [
          {
            "matcher": "Edit|Write",
            "hooks": [
              {
                "type": "command",
                "command": "python3 \\"$CLAUDE_PROJECT_DIR/.claude/hooks/comment_check.py\\""
              }
            ]
          }
        ]
      }
    }

The hook never edits files. It exits 2 when an added comment matches a pattern below, which
makes Claude Code show the findings to the agent; the agent decides what to change.
"""

from __future__ import annotations

import bisect
import json
import re
import subprocess
import sys
from pathlib import Path

SOURCE_SUFFIXES = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".ipp", ".tpp",
    ".js", ".jsx", ".mjs", ".cjs", ".ts", ".tsx", ".mts", ".cts",
}

# Tool directives and license headers are not prose.
EXEMPT_RE = re.compile(r"NOLINT|clang-format|eslint-|@ts-|prettier-ignore|SPDX-|Copyright")

# The negative lookbehind keeps "http://" from reading as a comment.
LINE_COMMENT_RE = re.compile(r"(?<!:)//[/!]?(.*)$")
BLOCK_START_RE = re.compile(r"/\*[*!]?(.*?)(?:\*/|$)")
# A continuation line of a block comment, but not a dereference such as `*p = 0;`.
BLOCK_CONTINUATION_RE = re.compile(r"^\*(?:/|\s|$)(.*?)(?:\*/)?$")

PATTERNS = [
    (
        "narrates history",
        re.compile(
            r"\b(?:no longer|now|currently|previously|formerly|originally|replaces|replaced"
            r"|renamed|legacy)\b"
            r"|(?<!is )(?<!are )(?<!be )\bused to\b"
            r"|\bmoved (?:out of|from|into|to)\b|\bextracted from\b"
            r"|\b(?:old|previous) (?:code|version|implementation)\b",
            re.IGNORECASE,
        ),
    ),
    (
        "names its callers or users",
        re.compile(
            r"\b(?:used|called|invoked|needed) (?:by|from|in)\b|\bcall sites?\b"
            r"|\bonly (?:used|called)\b|\b(?:used|called) only\b",
            re.IGNORECASE,
        ),
    ),
    (
        "argues against an alternative",
        re.compile(
            r"\b(?:rather than|instead of|as opposed to|measured)\b"
            r"|\bwould (?:cost|require|need|add)\b"
            r"|\b(?:cheaper|faster|slower|simpler) than\b"
            r"|[+-]\d+\s?(?:bytes\b|B\b|KiB\b|%)",
            re.IGNORECASE,
        ),
    ),
]


def comment_text(line: str) -> str | None:
    stripped = line.strip()
    continuation = BLOCK_CONTINUATION_RE.match(stripped)
    if continuation:
        return continuation.group(1).strip()
    match = LINE_COMMENT_RE.search(line) or BLOCK_START_RE.search(line)
    return match.group(1).strip() if match else None


def committed_lines(path: Path) -> set[str]:
    result = subprocess.run(
        ["git", "show", f"HEAD:./{path.name}"],
        cwd=path.parent,
        capture_output=True,
        text=True,
        check=False,
    )
    return set(result.stdout.splitlines()) if result.returncode == 0 else set()


def added_lines(tool_name: str, tool_input: dict) -> list[str]:
    if tool_name == "Write":
        new = tool_input.get("content", "")
        old = committed_lines(Path(tool_input["file_path"]))
    else:
        new = tool_input.get("new_string", "")
        old = set(tool_input.get("old_string", "").splitlines())
    return [line for line in new.splitlines() if line not in old]


def comment_blocks(lines: list[str]) -> list[list[str]]:
    """Group the comment text of consecutive comment lines."""
    blocks: list[list[str]] = []
    current: list[str] = []
    for line in lines:
        text = comment_text(line)
        if text is None:
            if current:
                blocks.append(current)
                current = []
        elif text:
            current.append(text)
    if current:
        blocks.append(current)
    return blocks


def findings(blocks: list[list[str]]) -> list[str]:
    """Report each flagged comment line once; a block is searched as one text so that a
    phrase split across lines still matches, and the match is reported on its first line."""
    found = []
    for block in blocks:
        joined = " ".join(block)
        starts = []
        offset = 0
        for text in block:
            starts.append(offset)
            offset += len(text) + 1
        flagged: dict[int, str] = {}
        for reason, pattern in PATTERNS:
            for match in pattern.finditer(joined):
                index = bisect.bisect_right(starts, match.start()) - 1
                text = block[index]
                if index in flagged or EXEMPT_RE.search(text):
                    continue
                quote = text if len(text) <= 120 else text[:117] + "..."
                flagged[index] = f'- "{quote}": {reason} ("{match.group(0)}")'
        found.extend(flagged[index] for index in sorted(flagged))
    return found


def main() -> int:
    payload = json.load(sys.stdin)
    tool_input = payload.get("tool_input", {})
    path = tool_input.get("file_path", "")
    if Path(path).suffix not in SOURCE_SUFFIXES:
        return 0

    found = findings(comment_blocks(added_lines(payload.get("tool_name", ""), tool_input)))
    if not found:
        return 0

    print(f"Comments added to {path} may break the comment rules in AGENTS.md:", file=sys.stderr)
    for line in found:
        print(line, file=sys.stderr)
    print(
        "Rewrite or delete each one. Keep a comment only if it states a contract, invariant or"
        " constraint of the code as it is now, for a reader who has never seen this change or"
        " its callers.",
        file=sys.stderr,
    )
    return 2


if __name__ == "__main__":
    sys.exit(main())
