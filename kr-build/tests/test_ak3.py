from pathlib import Path
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

    def flash(self, failure=False):
        return subprocess.run(['sh', 'anykernel.sh'], cwd=self.root,
                              env=dict(os.environ, SIMULATE_FAILURE='1' if failure else '0'),
                              text=True, capture_output=True, timeout=5)

    def test_actual_bytes_must_match_before_success(self):
        result = self.flash()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('Boot write verified', result.stdout)
        self.assertIn('t1-c123456789abc-dirty', result.stdout)

    def test_masked_write_error_is_rejected(self):
        result = self.flash(failure=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Boot readback mismatch', result.stderr)
        self.assertNotIn('Boot write verified', result.stdout)

    def test_missing_identity_aborts_before_flash(self):
        (self.root / 'kernel.release').unlink()
        result = self.flash()
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.root / 'flash_called').exists())


if __name__ == '__main__':
    unittest.main()
