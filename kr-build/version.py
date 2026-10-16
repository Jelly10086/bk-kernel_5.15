#!/usr/bin/env python3
import argparse
from pathlib import Path
import re
import sys


def version_segment(value: str) -> re.Match:
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]{0,127}", value) or value.startswith("bkk-"):
        raise ValueError("版本格式无效，应输入不带 bkk- 前缀的名称")
    segments = list(re.finditer(r"(?<![A-Za-z0-9.])\d+\.\d+\.(?:xxx|\d+)(?![A-Za-z0-9.])", value))
    if len(segments) != 1:
        raise ValueError("版本中应包含一项 5.15.xxx 形式的内核版本")
    return segments[0]


def version_template(value: str) -> str:
    segment = version_segment(value)
    placeholder = segment.group().rsplit(".", 1)[0] + ".xxx"
    return value[:segment.start()] + placeholder + value[segment.end():]


def resolve_version_name(value: str, kernel_version: str) -> str:
    segment = version_segment(value)
    if not re.fullmatch(r"\d+\.\d+\.\d+", kernel_version):
        raise ValueError(f"真实内核版本无效：{kernel_version}")
    return value[:segment.start()] + kernel_version + value[segment.end():]


def short_version_name(value: str) -> str:
    segment = version_segment(value)
    prefix = re.sub(r"[_-](?:android)?\d+[-_]$", "", value[:segment.start()]).rstrip("._-")
    suffix = value[segment.end():].lstrip("._-")
    name = "_".join(part for part in (prefix, suffix) if part)
    if not name:
        raise ValueError("版本中应包含内核名称或构建编号")
    return name


def source_kernel_version() -> str:
    text = Path(__file__).resolve().parent.parent.joinpath("Makefile").read_text()
    parts = []
    for name in ("VERSION", "PATCHLEVEL", "SUBLEVEL"):
        match = re.search(rf"^{name}\s*=\s*(\d+)\s*$", text, flags=re.MULTILINE)
        if match is None:
            raise ValueError(f"Makefile 缺少 {name}")
        parts.append(match.group(1))
    return ".".join(parts)


def main() -> None:
    parser = argparse.ArgumentParser(description="解析内核显示版本")
    parser.add_argument("value")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--template", action="store_true")
    mode.add_argument("--short", action="store_true")
    args = parser.parse_args()
    if args.template:
        print(version_template(args.value))
    elif args.short:
        print(short_version_name(args.value))
    else:
        print(resolve_version_name(args.value, source_kernel_version()))


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        sys.exit(1)
