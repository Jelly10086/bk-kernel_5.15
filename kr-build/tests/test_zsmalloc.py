from pathlib import Path
import os
import re
import subprocess
import sys
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(BUILD))
from zsmalloc import MAGIC, verify_signature


class AllocatorTests(unittest.TestCase):
    def test_chain_selection_from_kernel_source(self):
        text = (BUILD.parent / 'mm/zsmalloc.c').read_text()
        function = re.search(r'static int calculate_zspage_chain_size\(.*?\n\}', text, re.S).group()
        main = r'''
#include <assert.h>
#include <limits.h>
#define PAGE_SIZE 4096
#define ZS_MAX_PAGES_PER_ZSPAGE 8
#define is_power_of_2(x) ((x) && !((x) & ((x) - 1)))
FUNCTION
int main(void) {
    assert(calculate_zspage_chain_size(208) == 3);
    for (int size = 32; size <= PAGE_SIZE; size += 16) {
        int selected = calculate_zspage_chain_size(size);
        assert(selected >= 1 && selected <= 8);
        if (is_power_of_2(size)) assert(selected == 1);
        for (int pages = 1; pages <= 8; pages++)
            assert((selected * PAGE_SIZE) % size <= (pages * PAGE_SIZE) % size);
    }
    return 0;
}
'''.replace('FUNCTION', function)
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'test.c').write_text(main)
            subprocess.run(['gcc', '-Wall', '-Wextra', '-Werror', str(root / 'test.c'),
                            '-o', str(root / 'test')], check=True)
            subprocess.run([str(root / 'test')], check=True)

    def test_signature_rejects_unsigned_or_truncated_module(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for data in [b'ELF', b'ELF' + MAGIC, b'ELF' + b'\0' * 12 + MAGIC]:
                (root / 'module.ko').write_bytes(data)
                with self.assertRaises(ValueError):
                    verify_signature(root / 'module.ko', root / 'certificate.der')

    def test_signature_matches_only_the_build_certificate_and_content(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for name in ['correct', 'wrong']:
                subprocess.run(['openssl', 'req', '-new', '-newkey', 'rsa:2048', '-nodes',
                                '-x509', '-days', '1', '-subj', '/CN=BKK Test/',
                                '-keyout', str(root / f'{name}.key'),
                                '-out', str(root / f'{name}.pem')], check=True, capture_output=True)
                subprocess.run(['openssl', 'x509', '-in', str(root / f'{name}.pem'),
                                '-outform', 'DER', '-out', str(root / f'{name}.der')], check=True)
            body = b'ELF module payload'
            (root / 'content').write_bytes(body)
            subprocess.run(['openssl', 'cms', '-sign', '-binary', '-noattr', '-outform', 'DER',
                            '-in', str(root / 'content'), '-signer', str(root / 'correct.pem'),
                            '-inkey', str(root / 'correct.key'), '-out', str(root / 'signature')], check=True)
            signature = (root / 'signature').read_bytes()
            header = b'\0\0\2' + b'\0' * 5 + len(signature).to_bytes(4, 'big')
            module = root / 'module.ko'
            module.write_bytes(body + signature + header + MAGIC)
            verify_signature(module, root / 'correct.der')
            with self.assertRaises(ValueError):
                verify_signature(module, root / 'wrong.der')
            module.write_bytes(b'BAD' + module.read_bytes()[3:])
            with self.assertRaises(ValueError):
                verify_signature(module, root / 'correct.der')


class EarlyLoaderTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.state = self.root / 'state'
        self.module = self.root / 'module'
        self.module.mkdir()
        self.bin = self.root / 'bin'
        self.bin.mkdir()
        self.calls = self.root / 'calls'
        busybox = self.bin / 'busybox'
        busybox.write_text('#!/bin/sh\nexec "$@"\n')
        busybox.chmod(0o755)
        self.swaps = self.root / 'swaps'
        self.swaps.write_text('Filename Type Size Used Priority\n')
        self.sys = self.root / 'sys'
        self.sys.mkdir()
        self.stock = self.root / 'stock'
        self.stock.mkdir()
        (self.module / 'build-info.txt').write_text('KERNEL_RELEASE=5.15.216_bkk-CaT_t1\nMODULE_SRCVERSION=123456789ABCDEF012345678\nMODULE_SHA256=optimized\nSTOCK_ZRAM_SHA256=zram\nSTOCK_ZSMALLOC_SHA256=zsmalloc\n')
        (self.module / 'zsmalloc.ko').write_text('optimized')
        (self.stock / 'zram.ko').write_text('zram')
        (self.stock / 'zsmalloc.ko').write_text('zsmalloc')
        script = (BUILD / 'modules/bkk-zsmalloc/load.sh').read_text()
        script = script.replace('/data/adb/ksu/bin/busybox', str(busybox))
        script = script.replace('/data/adb/bkk-zsmalloc', str(self.state))
        script = script.replace('/proc/swaps', str(self.swaps))
        script = script.replace('/sys/module', str(self.sys))
        script = script.replace('/sys/block', str(self.root / 'block'))
        script = script.replace('/system_dlkm/lib/modules/*/kernel/drivers/block/zram/zram.ko', str(self.stock / 'zram.ko'))
        script = script.replace('/system_dlkm/lib/modules/*/kernel/mm/zsmalloc.ko', str(self.stock / 'zsmalloc.ko'))
        self.script = self.module / 'load.sh'
        self.script.write_text(script)
        for name, body in {
            'uname': 'echo 5.15.216_bkk-CaT_t1',
            'getprop': 'case "$1" in ro.product.device) echo sheng;; ro.soc.model) echo SM8550;; sys.boot_completed) echo 1;; esac',
            'sha256sum': 'printf "%s  %s\\n" "$(cat "$1")" "$1"',
            'sync': ':',
            'rmmod': f'echo "remove:$1" >> "{self.calls}"; rm -rf "{self.sys}/$1"',
            'insmod': f'''echo "load:$1" >> "{self.calls}"
if [ "$1" = "{self.module}/zsmalloc.ko" ]; then
    [ "${{FAIL_OPTIMIZED:-0}}" = 0 ] || exit 1
    mkdir -p "{self.sys}/zsmalloc"
    echo bkk-zs1 > "{self.sys}/zsmalloc/version"
    echo 123456789ABCDEF012345678 > "{self.sys}/zsmalloc/srcversion"
else
    mkdir -p "{self.sys}/$(basename "$1" .ko)"
fi''',
        }.items():
            path = self.bin / name
            path.write_text('#!/bin/sh\n' + body + '\n')
            path.chmod(0o755)

    def run_loader(self, command='load', **extra):
        env = dict(os.environ, PATH=f'{self.bin}:' + os.environ['PATH'], **extra)
        return subprocess.run(['sh', str(self.script), command], env=env,
                              text=True, capture_output=True, timeout=5)

    def test_active_swap_is_never_unloaded(self):
        self.swaps.write_text('header\n/dev/block/zram0 swap 10 1 -2\n')
        result = self.run_loader()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(self.calls.exists())
        self.assertIn('swap_already_active', (self.state / 'status').read_text())

    def test_kernel_mismatch_never_touches_modules(self):
        info = self.module / 'build-info.txt'
        info.write_text(info.read_text().replace('5.15.216_bkk-CaT_t1', '5.15.216_bkk-CaT_r1'))
        self.assertEqual(self.run_loader().returncode, 0)
        self.assertFalse(self.calls.exists())
        self.assertIn('kernel_mismatch', (self.state / 'status').read_text())

    def test_unrecognized_stock_driver_is_never_unloaded(self):
        (self.stock / 'zram.ko').write_text('unknown-firmware')
        self.assertEqual(self.run_loader().returncode, 0)
        self.assertFalse(self.calls.exists())
        self.assertIn('stock_zram_mismatch', (self.state / 'status').read_text())

    def test_initialized_zram_or_writeback_is_never_unloaded(self):
        block = self.root / 'block/zram0'
        block.mkdir(parents=True)
        (block / 'initstate').write_text('1\n')
        self.assertEqual(self.run_loader().returncode, 0)
        self.assertFalse(self.calls.exists())
        self.assertIn('zram_initialized', (self.state / 'status').read_text())
        (block / 'initstate').write_text('0\n')
        (block / 'backing_dev').write_text('/dev/block/dm-49\n')
        self.assertEqual(self.run_loader().returncode, 0)
        self.assertFalse(self.calls.exists())
        self.assertIn('backing_already_configured', (self.state / 'status').read_text())

    def test_same_allocator_is_not_reloaded(self):
        self.assertEqual(self.run_loader().returncode, 0)
        self.calls.unlink()
        self.assertEqual(self.run_loader().returncode, 0)
        self.assertFalse(self.calls.exists())

    def test_failed_new_allocator_restores_both_original_modules(self):
        result = self.run_loader(FAIL_OPTIMIZED='1')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.calls.read_text().splitlines(), [
            'remove:zram', 'remove:zsmalloc', f'load:{self.module}/zsmalloc.ko',
            f'load:{self.stock}/zsmalloc.ko', f'load:{self.stock}/zram.ko'])
        self.assertIn('stock_restored', (self.state / 'status').read_text())
        self.calls.unlink()
        self.assertEqual(self.run_loader().returncode, 0)
        self.assertFalse(self.calls.exists())

    def test_new_allocator_is_confirmed_only_after_swap_works(self):
        result = self.run_loader()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue((self.state / 'pending').exists())
        self.assertEqual(self.run_loader('confirm').returncode, 0)
        self.assertTrue((self.state / 'pending').exists())
        self.swaps.write_text('header\n/dev/block/zram0 swap 10 1 -2\n')
        self.assertEqual(self.run_loader('confirm').returncode, 0)
        self.assertFalse((self.state / 'pending').exists())
        self.assertIn('state=active', (self.state / 'status').read_text())


if __name__ == '__main__':
    unittest.main()
