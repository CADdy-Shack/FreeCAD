#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path

# direct execution support
if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import argparse
import re
from typing import List, Tuple

from third_party.safe_result import Result, Ok, Err, safe


SOURCE_RE = re.compile(r"^# Source:\s+(?P<path>.+)$")


def split_documents(text: str) -> List[str]:
    docs: List[List[str]] = []
    current: List[str] | None = None
    for line in text.splitlines():
        if SOURCE_RE.match(line.strip()):
            if current is not None:
                docs.append(current)
            current = [line]
        elif current is not None:
            current.append(line)
    if current is not None:
        docs.append(current)
    return ["\n".join(d).strip() + "\n" for d in docs]


def extract(doc: str, idx: int) -> Result[Tuple[Path, List[str]], Exception]:
    lines = doc.splitlines()
    for i, line in enumerate(lines):
        m = SOURCE_RE.match(line.strip())
        if m:
            rel = Path(m.group("path"))
            del lines[i]
            while i - 1 >= 0 and not lines[i - 1].strip():
                del lines[i - 1]
                i -= 1
            while i < len(lines) and not lines[i].strip():
                del lines[i]
            return Ok((rel, lines))
    preview = "\n".join(lines[:5])
    return Err(ValueError(
        f"Missing '# Source:' in doc #{idx}\n{preview}"
    ))


def validate_path(p: Path) -> Result[None, Exception]:
    if p.is_absolute():
        return Err(ValueError(f"absolute path: {p}"))
    if any(part == ".." for part in p.parts):
        return Err(ValueError(f"path traversal: {p}"))
    return Ok(None)


def strip_closing(lines: List[str]) -> List[str]:
    while lines and not lines[-1].strip():
        lines.pop()
    if lines and lines[-1].strip() == "...":
        lines.pop()
    return lines


@safe
def write(base: Path, rel: Path, content: str) -> None:
    out = base / rel
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(content, encoding="utf-8")


def process(
    input_file: Path,
    output_dir: Path,
    dry_run: bool,
) -> Result[None, Exception]:
    try:
        text = input_file.read_text(encoding="utf-8")
    except Exception as e:
        return Err(e)
    docs = split_documents(text)
    for idx, doc in enumerate(docs, 1):
        res = extract(doc, idx)
        if isinstance(res, Err):
            return res
        rel, lines = res.value
        valid = validate_path(rel)
        if isinstance(valid, Err):
            return valid
        content = "\n".join(strip_closing(lines)).rstrip() + "\n"
        if dry_run:
            print(f"# would write: {rel}")
            continue
        write_res = write(output_dir, rel, content)
        if isinstance(write_res, Err):
            return write_res
    return Ok(None)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Split combined YAML into individual files",
    )
    p.add_argument("input_file", type=Path)
    p.add_argument("output_dir", type=Path)
    p.add_argument(
        "--dry-run",
        action="store_true",
        help="Validate and show actions without writing files",
    )
    return p.parse_args()


def main() -> int:
    args = parse_args()
    result = process(args.input_file, args.output_dir, args.dry_run)
    if isinstance(result, Err):
        print(f"Error: {result.error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
