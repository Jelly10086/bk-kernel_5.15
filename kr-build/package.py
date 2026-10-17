#!/usr/bin/env python3
import argparse
import gzip
import hashlib
import os
from pathlib import Path
import re
import shutil
import stat
import sys
import tarfile
import tempfile
import time
import urllib.error
import urllib.request
import zipfile
from datetime import datetime, timezone
from version import built_metadata, built_version, package_version, version_template


def fetch_anykernel(cache: Path, commit: str) -> Path:
    target = cache / f"anykernel-{commit}"
    required = (
        "META-INF/com/google/android/update-binary",
        "META-INF/com/google/android/updater-script",
        "tools/ak3-core.sh", "tools/busybox", "tools/magiskboot",
    )
    if not target.exists():
        cache.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="ak3-download-", dir=cache) as temp:
            staging = Path(temp)
            archive = staging / "source.tar.gz"
            url = f"https://codeload.github.com/osm0sis/AnyKernel3/tar.gz/{commit}"
            for attempt in range(4):
                try:
                    request = urllib.request.Request(url, headers={"User-Agent": "kr-build"})
                    with urllib.request.urlopen(request, timeout=60) as response:
                        with archive.open("wb") as output:
                            shutil.copyfileobj(response, output)
                    break
                except (OSError, urllib.error.URLError):
                    if attempt == 3:
                        raise
                    time.sleep(2 ** attempt)
            with tarfile.open(archive, "r:gz") as source:
                for member in source.getmembers():
                    relative = Path(member.name)
                    if (relative.is_absolute() or ".." in relative.parts
                            or not relative.parts
                            or relative.parts[0] != f"AnyKernel3-{commit}"):
                        raise ValueError(f"AnyKernel3 归档路径无效：{member.name}")
                    path = staging / relative
                    if member.isdir():
                        path.mkdir(parents=True, exist_ok=True)
                    elif member.isfile():
                        path.parent.mkdir(parents=True, exist_ok=True)
                        content = source.extractfile(member)
                        if content is None:
                            raise ValueError(f"AnyKernel3 归档内容缺失：{member.name}")
                        with content, path.open("wb") as output:
                            shutil.copyfileobj(content, output)
                        path.chmod(member.mode & 0o777)
                    else:
                        raise ValueError(f"AnyKernel3 归档文件类型无效：{member.name}")
            extracted = staging / f"AnyKernel3-{commit}"
            for relative in required:
                if not (extracted / relative).is_file():
                    raise ValueError(f"AnyKernel3 缺少文件：{relative}")
            (extracted / "kr-build-source").write_text(commit + "\n")
            extracted.rename(target)
    stamp = target / "kr-build-source"
    if not stamp.is_file() or stamp.read_text().strip() != commit:
        raise ValueError(f"AnyKernel3 缓存来源不符：{target}")
    for relative in required:
        if not (target / relative).is_file():
            raise ValueError(f"AnyKernel3 缓存不完整：{relative}")
    return target


def checksum(path: Path) -> str:
    with path.open("rb") as stream:
        digest = hashlib.sha256()
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def zip_tree(source: Path, target: Path) -> None:
    with zipfile.ZipFile(target, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(source.rglob("*")):
            relative = path.relative_to(source).as_posix()
            if path.is_symlink():
                info = zipfile.ZipInfo(relative)
                info.create_system = 3
                info.external_attr = (stat.S_IFLNK | 0o777) << 16
                archive.writestr(info, os.readlink(path))
            elif path.is_file():
                archive.write(path, relative)
    with zipfile.ZipFile(target) as archive:
        bad_file = archive.testzip()
        if bad_file:
            raise ValueError(f"ZIP 校验失败：{bad_file}")
        for relative in ("Image.gz", "kernel.release", "anykernel.sh", "tools/ak3-core.sh",
                         "META-INF/com/google/android/update-binary"):
            if relative not in archive.namelist():
                raise ValueError(f"ZIP 缺少文件：{relative}")
        if any(name.startswith(('module/', 'modules/')) or name.endswith('.ko')
               for name in archive.namelist()):
            raise ValueError('AK3不能包含独立控制模块')


def main() -> None:
    parser = argparse.ArgumentParser(description="打包 sheng AnyKernel3 ZIP")
    parser.add_argument("--out-dir", required=True, type=Path)
    parser.add_argument("--dist-dir", required=True, type=Path)
    parser.add_argument("--cache-dir", required=True, type=Path)
    parser.add_argument("--anykernel-commit", required=True)
    parser.add_argument("--version-name")
    args = parser.parse_args()
    version = args.version_name
    if version is None:
        version = Path(__file__).with_name("version").read_text().rstrip("\n")
    version = version_template(version)
    if len(args.anykernel_commit) != 40 or any(c not in "0123456789abcdef" for c in args.anykernel_commit):
        raise ValueError("AnyKernel3 提交必须是完整 SHA")
    out = args.out_dir.resolve()
    image = out / "arch/arm64/boot/Image.gz"
    inputs = {
        "gki.config": out / ".config",
        "Module.symvers": out / "Module.symvers",
        "build-info.txt": out / "build-info.txt",
        "kernel.release": out / "include/config/kernel.release",
    }
    for path in (image, *inputs.values()):
        if not path.is_file() or not path.stat().st_size:
            raise ValueError(f"缺少构建产物：{path}")
    config = (out / ".config").read_text().splitlines()
    if "CONFIG_REKERNEL_X=y" not in config:
        raise ValueError("内核没有启用内建 ReKernel-X")
    release = inputs["kernel.release"].read_text().strip()
    if len(release) > 64 or any(c not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._+-" for c in release):
        raise ValueError(f"内核版本格式无效：{release}")
    release_version = re.match(r"(5\.15\.\d+)(?:[-_]|$)", release)
    if release_version is None:
        raise ValueError(f"内核版本必须以 5.15 数字版本开头，否则 Android init 无法启动：{release}")
    custom_prefix = release_version.group(1) + "_bkk-"
    if release.startswith(custom_prefix):
        version = built_version(release)[0]
    else:
        if "-android13-8" not in release:
            raise ValueError(f"内核版本与 android13-5.15 / KMI 8 不符：{release}")
        version = package_version(version, release_version.group(1))
    with gzip.open(image, "rb") as stream:
        if stream.read(64)[56:60] != b"ARM\x64":
            raise ValueError("Image.gz 不是 ARM64 内核镜像")
        while stream.read(1024 * 1024):
            pass
    source = fetch_anykernel(args.cache_dir.resolve(), args.anykernel_commit)
    args.dist_dir.mkdir(parents=True, exist_ok=True)
    now = datetime.now(timezone.utc)
    stamp = now.strftime("%Y%m%d-%H%M%S-%f")
    package_date = built_metadata(out)["PACKAGE_DATE"]
    name = f"bkk-{version}-AK3"
    destination = args.dist_dir.resolve() / f"{name}-{stamp}"
    with tempfile.TemporaryDirectory(prefix=".package-", dir=args.dist_dir) as temp:
        stage = Path(temp)
        installer = stage / "installer"
        installer.mkdir()
        for directory in ("META-INF", "tools"):
            shutil.copytree(source / directory, installer / directory, symlinks=True)
        for license_name in ("LICENSE", "COPYING", "NOTICE"):
            if (source / license_name).is_file():
                shutil.copy2(source / license_name, installer / license_name)
        shutil.copy2(image, installer / "Image.gz")
        shutil.copy2(inputs["kernel.release"], installer / "kernel.release")
        template = Path(__file__).with_name("anykernel.sh").read_text()
        template, count = re.subn(r"^kernel\.string=.*$", f"kernel.string=bkk-{version}", template, flags=re.MULTILINE)
        if count != 1:
            raise ValueError("anykernel.sh 中的 kernel.string 必须只有一项")
        (installer / "anykernel.sh").write_text(template)
        (installer / "anykernel.sh").chmod(0o755)
        bundle = stage / name
        bundle.mkdir()
        zip_path = bundle / f"{name}-{package_date}.zip"
        zip_tree(installer, zip_path)
        for relative, path in inputs.items():
            shutil.copy2(path, bundle / relative)
        files = sorted(bundle.iterdir())
        (bundle / "SHA256SUMS").write_text("".join(
            f"{checksum(path)}  {path.name}\n" for path in files
        ))
        bundle.rename(destination)
    print(f"[OK] AK3：{destination / zip_path.name}")
    print(f"[OK] 校验文件：{destination / 'SHA256SUMS'}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, EOFError, ValueError, tarfile.TarError, zipfile.BadZipFile) as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        sys.exit(1)
