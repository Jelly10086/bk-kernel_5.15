import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from version import built_metadata


ABI = {
    'zs_unmap_object': '0x924c46f8', 'zs_create_pool': '0xbcf1f0e6',
    'zs_free': '0x958df3ac', 'zs_pool_stats': '0x7cceaf92',
    'zs_destroy_pool': '0xc69b7ee5', 'zs_get_total_pages': '0xcbe56bc2',
    'zs_compact': '0x5da67adc', 'zs_malloc': '0xb29533ee',
    'zs_map_object': '0xc341ae6d', 'zs_huge_class_size': '0x3d8baf3b',
}
MAGIC = b'~Module signature appended~\n'


def verify_signature(module: Path, certificate: Path) -> None:
    data = module.read_bytes()
    if not data.endswith(MAGIC) or len(data) < len(MAGIC) + 12:
        raise ValueError('zsmalloc缺少内核签名')
    end = len(data) - len(MAGIC)
    header = data[end - 12:end]
    length = int.from_bytes(header[8:12], 'big')
    start = end - 12 - length
    if header[2] != 2 or header[3:8] != b'\0' * 5 or start <= 0:
        raise ValueError('zsmalloc签名格式无效')
    with tempfile.TemporaryDirectory(prefix='bkk-signature-') as temp:
        root = Path(temp)
        (root / 'signature.der').write_bytes(data[start:end - 12])
        (root / 'module.bin').write_bytes(data[:start])
        subprocess.run(['openssl', 'x509', '-inform', 'DER', '-in', str(certificate),
                        '-out', str(root / 'certificate.pem')], check=True, capture_output=True)
        result = subprocess.run([
            'openssl', 'cms', '-verify', '-binary', '-inform', 'DER',
            '-in', str(root / 'signature.der'), '-content', str(root / 'module.bin'),
            '-certfile', str(root / 'certificate.pem'), '-nointern', '-noverify',
        ], capture_output=True)
        if result.returncode:
            raise ValueError('zsmalloc签名与本轮内核证书不匹配，禁止打包')


def sign_allocator(out: Path) -> None:
    module = out / 'mm/zsmalloc.ko'
    values = built_metadata(out)
    for field, expected in [('intree', 'Y'), ('version', 'bkk-zs1')]:
        actual = subprocess.check_output(['modinfo', '-F', field, str(module)], text=True).strip()
        if actual != expected:
            raise ValueError('只能签名本轮内核中的CaT分配器')
    vermagic = subprocess.check_output(['modinfo', '-F', 'vermagic', str(module)], text=True).split()
    if not vermagic or vermagic[0] != values['KERNEL_RELEASE']:
        raise ValueError('待签名分配器的内核版本不匹配')
    certificate = out / 'certs/signing_key.x509'
    if not module.read_bytes().endswith(MAGIC):
        subprocess.run([str(out / 'scripts/sign-file'), 'sha1',
                        str(out / 'certs/signing_key.pem'), str(certificate), str(module)], check=True)
    verify_signature(module, certificate)


def stage_allocator(out: Path, destination: Path) -> None:
    module = out / 'mm/zsmalloc.ko'
    values = built_metadata(out)
    config = (out / '.config').read_text().splitlines()
    if 'CONFIG_ZSMALLOC_CHAIN_SIZE=8' not in config or 'CONFIG_ZSMALLOC=m' not in config:
        raise ValueError('需要CONFIG_ZSMALLOC=m和CONFIG_ZSMALLOC_CHAIN_SIZE=8')
    def info(field: str) -> str:
        return subprocess.check_output(['modinfo', '-F', field, str(module)], text=True).strip()
    vermagic = info('vermagic').split()
    if not vermagic or vermagic[0] != values['KERNEL_RELEASE']:
        raise ValueError('zsmalloc与本轮内核版本不匹配')
    if info('intree') != 'Y' or info('version') != 'bkk-zs1':
        raise ValueError('zsmalloc不是本轮内核中的CaT分配器')
    srcversion = info('srcversion')
    if not re.fullmatch(r'[0-9A-F]{16,32}', srcversion):
        raise ValueError('zsmalloc缺少来源校验值')
    exports = {line.split()[1]: line.split()[0] for line in
               (out / 'Module.symvers').read_text().splitlines() if len(line.split()) >= 2}
    if any(exports.get(name) != crc for name, crc in ABI.items()):
        raise ValueError('zsmalloc接口与sheng原厂ZRAM不匹配')
    certificate = out / 'certs/signing_key.x509'
    verify_signature(module, certificate)
    source = Path(__file__).with_name('modules') / 'bkk-zsmalloc'
    destination.mkdir(parents=True)
    shutil.copy2(module, destination / 'zsmalloc.ko')
    shutil.copy2(source / 'load.sh', destination / 'load.sh')
    (destination / 'load.sh').chmod(0o755)
    metadata = {
        'KERNEL_RELEASE': values['KERNEL_RELEASE'], 'MODULE_VERSION': 'bkk-zs1',
        'MODULE_SRCVERSION': srcversion,
        'MODULE_SHA256': hashlib.sha256(module.read_bytes()).hexdigest(),
        'SIGNING_CERT_SHA256': hashlib.sha256(certificate.read_bytes()).hexdigest(),
    }
    for line in (source / 'compat.env').read_text().splitlines():
        key, value = line.split('=', 1)
        metadata[key] = value
    (destination / 'build-info.txt').write_text(''.join(f'{k}={v}\n' for k, v in metadata.items()))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='签名并校验本轮内核的zsmalloc模块')
    parser.add_argument('--out-dir', required=True, type=Path)
    args = parser.parse_args()
    try:
        sign_allocator(args.out_dir.resolve())
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        parser.exit(1, f'[ERROR] {error}\n')
    print('[OK] zsmalloc签名与本轮内核证书一致')
