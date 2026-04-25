#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path

# direct execution support
if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import argparse
from typing import List

from third_party.safe_result import Result, Ok, Err, safe


def find_yaml_files(root: Path) -> Result[List[Path], Exception]:
    if not root.is_dir():
        return Err(NotADirectoryError(root))
    files = [
        p for p in root.rglob("*")
        if p.is_file() and p.suffix.lower() in {".yaml", ".yml"}
    ]
    return Ok(sorted(files))


@safe
def read_file(path: Path) -> str:
    return path.read_text(encoding="utf-8").strip()


def ensure_opening(content: str) -> str:
    return content if content.lstrip().startswith("---") else f"---\n{content}"


def ensure_closing(content: str) -> str:
    lines = content.rstrip().splitlines()
    if not lines:
        return "...\n"
    if lines[-1].strip() == "...":
        return content.rstrip() + "\n"
    return content.rstrip() + "\n...\n"


def build_doc(path: Path, root: Path) -> Result[str, Exception]:
    rel = path.relative_to(root)
    return read_file(path).map(
        lambda content: (
            f"# Source: {rel}\n"
            f"{ensure_closing(ensure_opening(content))}"
        )
    )


def combine(root: Path) -> Result[str, Exception]:
    files_res = find_yaml_files(root)
    if isinstance(files_res, Err):
        return files_res
    docs: List[str] = []
    for p in files_res.value:
        doc_res = build_doc(p, root)
        if isinstance(doc_res, Err):
            return doc_res
        docs.append(doc_res.value)
    return Ok("\n".join(docs).rstrip() + "\n")


@safe
def write_output(path: Path, content: str) -> None:
    path.write_text(content, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Combine YAML files into a multi-document stream",
    )
    p.add_argument("input_dir", type=Path)
    p.add_argument("output_file", type=Path)
    p.add_argument(
        "--dry-run",
        action="store_true",
        help="Process files but do not write output",
    )
    return p.parse_args()


def main() -> int:
    args = parse_args()
    result = combine(args.input_dir)
    if isinstance(result, Err):
        print(f"Error: {result.error}", file=sys.stderr)
        return 1
    if args.dry_run:
        print(result.value)
        return 0
    write_res = write_output(args.output_file, result.value)
    if isinstance(write_res, Err):
        print(f"Error: {write_res.error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
