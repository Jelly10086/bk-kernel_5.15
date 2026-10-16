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
    suffix = value[segment.end():]
    if segment.start() == 0 and suffix.startswith("_bkk-"):
        label = suffix[5:]
    else:
        prefix = re.sub(r"_\d+-$", "", value[:segment.start()])
        label = prefix.rstrip("_-") + suffix
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*_[A-Za-z0-9][A-Za-z0-9.-]*", label):
        raise ValueError("版本格式无效，例如 5.15.xxx_bkk-CaT_r1")
    return f"{placeholder}_bkk-{label}"


def resolve_version_name(value: str, kernel_version: str) -> str:
    segment = version_segment(value)
    if not re.fullmatch(r"\d+\.\d+\.\d+", kernel_version):
        raise ValueError(f"真实内核版本无效：{kernel_version}")
    return value[:segment.start()] + kernel_version + value[segment.end():]


def kernel_release(value: str, kernel_version: str, commit: str | None = None) -> str:
    release = resolve_version_name(version_template(value), kernel_version)
    if commit is not None:
        if not re.fullmatch(r"[0-9a-f]{12,40}(?:-dirty)?", commit):
            raise ValueError("Git 提交号格式无效")
        release += f"-c{commit}"
    if len(release) > 64:
        raise ValueError("内核版本超过 64 字符，请缩短名称")
    return release


def package_version(value: str, kernel_version: str) -> str:
    release = kernel_release(value, kernel_version)
    label = release.split("_bkk-", 1)[1]
    brand, revision = label.rsplit("_", 1)
    return f"{brand}_13-{kernel_version}_{revision}"


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
    parser.add_argument("--commit")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--template", action="store_true")
    mode.add_argument("--release", action="store_true")
    args = parser.parse_args()
    if args.commit is not None and not args.release:
        parser.error("--commit 需要与 --release 一起使用")
    if args.template:
        print(version_template(args.value))
    elif args.release:
        print(kernel_release(args.value, source_kernel_version(), args.commit))
    else:
        print(package_version(args.value, source_kernel_version()))


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        sys.exit(1)
