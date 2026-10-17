from pathlib import Path
import hashlib
import os
import subprocess
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]


class FlashVerificationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'tools').mkdir()
        (self.root / 'anykernel.sh').write_text((BUILD / 'anykernel.sh').read_text())
        (self.root / 'kernel.release').write_text('5.15.216_bkk-CaT_t1-c123456789abc-dirty\n')
        (self.root / 'block').write_bytes(b'old boot image')
        (self.root / 'tools/bkk-memory.sh').write_text('''
prepare_allocator() {
    [ "${SIMULATE_DATA_FAILURE:-0}" = 0 ] || abort 'data unavailable'
}
commit_allocator() { touch "$AKHOME/allocator_committed"; }
cleanup_allocator() { :; }
''')
        (self.root / 'tools/ak3-core.sh').write_text('''
AKHOME=$PWD
BLOCK=$AKHOME/block
ui_print() { echo "$*"; }
abort() { echo "$*" >&2; exit 1; }
split_boot() { cp "$BLOCK" "$AKHOME/boot.img"; }
flash_boot() {
    touch "$AKHOME/flash_called"
    printf 'new boot image' > "$AKHOME/boot-new.img"
    if [ "$SIMULATE_FAILURE" = 0 ]; then
        cp "$AKHOME/boot-new.img" "$BLOCK"
    fi
    return 0
}
sync() { :; }
''')

    def flash(self, failure=False, data_failure=False):
        return subprocess.run(['sh', 'anykernel.sh'], cwd=self.root,
                              env=dict(os.environ, SIMULATE_FAILURE='1' if failure else '0',
                                       SIMULATE_DATA_FAILURE='1' if data_failure else '0'),
                              text=True, capture_output=True, timeout=5)

    def test_actual_bytes_must_match_before_success(self):
        result = self.flash()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('Boot write verified', result.stdout)
        self.assertIn('t1-c123456789abc-dirty', result.stdout)
        self.assertTrue((self.root / 'allocator_committed').exists())

    def test_masked_write_error_is_rejected(self):
        result = self.flash(failure=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Boot readback mismatch', result.stderr)
        self.assertNotIn('Boot write verified', result.stdout)
        self.assertFalse((self.root / 'allocator_committed').exists())

    def test_allocator_data_failure_aborts_before_flash(self):
        result = self.flash(data_failure=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.root / 'flash_called').exists())

    def test_missing_identity_aborts_before_flash(self):
        (self.root / 'kernel.release').unlink()
        result = self.flash()
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.root / 'flash_called').exists())


class AllocatorInstallTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.data = self.root / 'data'
        self.payload = self.root / 'allocator'
        self.payload.mkdir()
        (self.root / 'tools').mkdir()
        (self.root / 'tools/bkk-zsmalloc-entry.sh').write_text('#!/system/bin/sh\n')
        self.busybox = self.data / 'adb/ksu/bin/busybox'
        self.busybox.parent.mkdir(parents=True)
        self.busybox.write_text('#!/bin/sh\n')
        self.busybox.chmod(0o755)
        (self.root / 'kernel.release').write_text('5.15.216_bkk-CaT_t1\n')
        (self.payload / 'zsmalloc.ko').write_bytes(b'signed test allocator')
        (self.payload / 'load.sh').write_text('#!/system/bin/sh\n')
        checksum = hashlib.sha256((self.payload / 'zsmalloc.ko').read_bytes()).hexdigest()
        (self.payload / 'build-info.txt').write_text(
            f'KERNEL_RELEASE=5.15.216_bkk-CaT_t1\nMODULE_SHA256={checksum}\n')
        helper = (BUILD / 'ak3-memory.sh').read_text().replace('/data/adb', str(self.data / 'adb'))
        self.script = self.root / 'test.sh'
        self.script.write_text('''set -eu
AKHOME=$PWD
abort() { echo "$*" >&2; exit 1; }
ui_print() { echo "$*"; }
sync() { :; }
''' + helper + '''
trap cleanup_allocator EXIT
prepare_allocator
[ "${COMMIT:-0}" = 0 ] || commit_allocator
''')

    def install(self, commit=False):
        return subprocess.run(['sh', str(self.script)], cwd=self.root,
                              env=dict(os.environ, COMMIT='1' if commit else '0'),
                              text=True, capture_output=True, timeout=5)

    def test_preparation_leaves_no_active_hooks_or_temporary_payload(self):
        result = self.install()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(list((self.data / 'adb/bkk-zsmalloc').iterdir()), [])
        self.assertEqual(list((self.data / 'adb/post-fs-data.d').iterdir()), [])

    def test_activation_preserves_previous_payload(self):
        target = self.data / 'adb/bkk-zsmalloc/5.15.216_bkk-CaT_t1'
        target.mkdir(parents=True)
        (target / 'previous').write_text('original')
        result = self.install(commit=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((target / 'zsmalloc.ko').read_bytes(), b'signed test allocator')
        self.assertTrue((target / 'load.sh').stat().st_mode & 0o100)
        backups = list(target.parent.glob('.previous-*'))
        self.assertEqual(len(backups), 1)
        self.assertEqual((backups[0] / 'previous').read_text(), 'original')
        self.assertTrue((self.data / 'adb/post-fs-data.d/bkk-zsmalloc.sh').exists())
        self.assertTrue((self.data / 'adb/boot-completed.d/bkk-zsmalloc.sh').exists())

    def test_missing_root_or_bad_checksum_rejects_install(self):
        self.busybox.unlink()
        self.assertNotEqual(self.install(commit=True).returncode, 0)
        self.busybox.write_text('#!/bin/sh\n')
        self.busybox.chmod(0o755)
        (self.payload / 'zsmalloc.ko').write_bytes(b'corrupt')
        self.assertNotEqual(self.install(commit=True).returncode, 0)
        self.assertFalse((self.data / 'adb/post-fs-data.d/bkk-zsmalloc.sh').exists())


if __name__ == '__main__':
    unittest.main()
