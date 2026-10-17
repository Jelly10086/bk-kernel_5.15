from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]


class InstallTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.module = self.root / 'module'
        shutil.copytree(BUILD / 'modules/bkk-control', self.module)
        self.state = self.root / 'state'
        source = (self.module / 'customize.sh').read_text()
        source = source.replace('/data/adb/bkk-control', str(self.state))
        source = source.replace('/data/adb/modules/bkk-control/bkctl', str(self.root / 'old/bkctl'))
        source = source.replace('/data/adb/ksu/bin/busybox', str(self.root / 'busybox'))
        (self.module / 'customize.sh').write_text(source)
        (self.module / 'build-info.txt').write_text('KERNEL_RELEASE=5.15.216_bkk-CaT_t1-c123456789abc-dirty\n')
        self.driver = self.root / 'install.sh'
        self.driver.write_text('''#!/bin/sh
set -eu
abort() { echo "$*" >&2; exit 1; }
ui_print() { echo "$*"; }
set_perm() { :; }
getprop() {
    case "$1" in
        ro.product.device) echo "$DEVICE" ;;
        ro.soc.model) echo SM8550 ;;
    esac
}
uname() { echo "$RUNNING_RELEASE"; }
insmod() { echo "unexpected insmod" >&2; exit 99; }
. "$MODPATH/customize.sh"
''')

    def install(self, release, device='sheng'):
        env = dict(os.environ, KSU='true', MODPATH=str(self.module),
                   DEVICE=device, RUNNING_RELEASE=release)
        return subprocess.run(['sh', str(self.driver)], env=env,
                              text=True, capture_output=True, timeout=5)

    def test_install_before_companion_boot_stays_disabled(self):
        result = self.install('5.15.216_bkk-CaT_b3-c335817a2dcb7')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('允许先安装', result.stdout)
        self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')

    def test_matching_kernel_also_starts_disabled(self):
        result = self.install('5.15.216_bkk-CaT_t1-c123456789abc-dirty')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, 'bkk-control - 安装完成\n')
        self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')

    def test_upgrade_restores_old_script_through_busybox(self):
        old = self.root / 'old/bkctl'
        old.parent.mkdir()
        old.write_text('#!/bin/sh\n[ "$BKK_TEST_BUSYBOX" = 1 ] && [ "$1" = restore ]\n')
        old.chmod(0o755)
        busybox = self.root / 'busybox'
        busybox.write_text('#!/bin/sh\nexport BKK_TEST_BUSYBOX=1\nexec "$@"\n')
        busybox.chmod(0o755)
        result = self.install('5.15.216_bkk-CaT_t1-c123456789abc-dirty')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')

    def test_unsupported_device_does_not_install(self):
        result = self.install('5.15.216_bkk-CaT_t1', 'nabu')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(self.state.exists())

    def test_missing_target_release_does_not_install(self):
        (self.module / 'build-info.txt').write_text('KERNEL_COMMIT=invalid\n')
        result = self.install('5.15.216_bkk-CaT_t1')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(self.state.exists())


if __name__ == '__main__':
    unittest.main()
