#!/usr/bin/env python3
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import zipfile
from version import built_metadata


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def package(out: Path, dist: Path, module: Path, clang: Path) -> Path:
    values = built_metadata(out)
    revision = values['MODULE_VERSION']
    release = values['KERNEL_RELEASE']
    vermagic = subprocess.check_output(['modinfo', '-F', 'vermagic', str(module)], text=True).split()
    if not vermagic or vermagic[0] != release:
        raise ValueError('LKM与内核vermagic不一致')
    if subprocess.check_output(['modinfo', '-F', 'version', str(module)], text=True).strip() != revision:
        raise ValueError('LKM版本与内核编号不一致')
    srcversion = subprocess.check_output(['modinfo', '-F', 'srcversion', str(module)], text=True).strip()
    if not re.fullmatch(r'[0-9A-F]{16,32}', srcversion):
        raise ValueError('LKM来源校验值无效')
    header = module.read_bytes()[:20]
    if header[:6] != b'\x7fELF\x02\x01' or int.from_bytes(header[18:20], 'little') != 183:
        raise ValueError('LKM不是ARM64 ELF')
    undefined = subprocess.check_output([str(clang / 'bin/llvm-nm'), '-u', str(module)], text=True)
    symbols = {line.split()[-1] for line in undefined.splitlines() if line.split()}
    protected = set(re.findall(r'"([A-Za-z0-9_]+)"', (out / 'kernel/gki_module_protected.h').read_text()))
    if symbols & protected:
        raise ValueError(f'LKM使用受保护符号：{sorted(symbols & protected)}')
    exported = {line.split()[1] for line in (out / 'Module.symvers').read_text().splitlines() if len(line.split()) >= 2}
    if symbols - exported:
        raise ValueError(f'LKM符号未导出：{sorted(symbols - exported)}')
    now = datetime.now(timezone.utc)
    code = int(now.timestamp()) - 1577836800
    if not 0 < code < 2**31:
        raise ValueError('versionCode超出范围')
    dist.mkdir(parents=True, exist_ok=True)
    destination = dist.resolve() / f'bkk-control-{revision}-{now.strftime("%Y%m%d-%H%M%S-%f")}'
    source = Path(__file__).with_name('modules') / 'bkk-control'
    with tempfile.TemporaryDirectory(prefix='.bkk-control-', dir=dist) as temp:
        stage = Path(temp)
        installer = stage / 'installer'
        shutil.copytree(source, installer)
        prop = installer / 'module.prop'
        prop.write_text(prop.read_text().replace('@REVISION@', revision).replace('@VERSION_CODE@', str(code)))
        shutil.copy2(module, installer / 'bkk_perf.ko')
        build_info = (out / 'build-info.txt').read_text()
        build_info += f'MODULE_VERSION={revision}\nMODULE_SHA256={digest(module)}\nMODULE_SRCVERSION={srcversion}\n'
        (installer / 'build-info.txt').write_text(build_info)
        (installer / 'webroot/version.json').write_text(json.dumps({'version': revision}) + '\n')
        for path in installer.iterdir():
            if path.suffix == '.sh' or path.name == 'bkctl':
                subprocess.run(['sh', '-n', str(path)], check=True)
                path.chmod(0o755)
        archive_name = f'bkk-control-{revision}-{values["PACKAGE_DATE"]}.zip'
        module_name = f'bkk_perf-{revision}-{values["PACKAGE_DATE"]}.ko'
        bundle = stage / 'bundle'
        bundle.mkdir()
        archive = bundle / archive_name
        shutil.copy2(module, bundle / module_name)
        manifest = {p.relative_to(installer).as_posix() for p in installer.rglob('*') if p.is_file()}
        if any(name.startswith(('Image', 'tools/', 'META-INF/', 'system/')) for name in manifest):
            raise ValueError('控制模块包含内核或系统覆盖文件')
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as target:
            for name in sorted(manifest):
                target.write(installer / name, name)
        with zipfile.ZipFile(archive) as target:
            if target.testzip() is not None or set(target.namelist()) != manifest:
                raise ValueError('控制模块ZIP完整性检查失败')
            if f'version={revision}\n' not in target.read('module.prop').decode():
                raise ValueError('module.prop版本不一致')
            if json.loads(target.read('webroot/version.json'))['version'] != revision:
                raise ValueError('界面版本不一致')
            if not (target.getinfo('bkctl').external_attr >> 16) & stat.S_IXUSR:
                raise ValueError('bkctl不可执行')
        (bundle / 'bkk-control-build-info.txt').write_text(build_info)
        (bundle / 'bkk-control-SHA256SUMS').write_text(''.join(
            f'{digest(path)}  {path.name}\n' for path in sorted(bundle.iterdir())
        ))
        bundle.rename(destination)
    print(f'[OK] bkk-control：{destination / archive_name}')
    print(f'[OK] LKM：{destination / module_name}')
    return destination


def main() -> None:
    parser = argparse.ArgumentParser(description='打包独立bkk-control模块')
    parser.add_argument('--out-dir', required=True, type=Path)
    parser.add_argument('--dist-dir', required=True, type=Path)
    parser.add_argument('--module', required=True, type=Path)
    parser.add_argument('--clang-dir', required=True, type=Path)
    args = parser.parse_args()
    package(args.out_dir.resolve(), args.dist_dir.resolve(), args.module.resolve(), args.clang_dir.resolve())


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, subprocess.SubprocessError, zipfile.BadZipFile) as error:
        print(f'[ERROR] {error}', file=sys.stderr)
        sys.exit(1)
