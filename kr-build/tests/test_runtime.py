from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]


class RuntimeTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.module = self.root / 'module'
        self.module.mkdir()
        self.state = self.root / 'state'
        self.mock = self.root / 'bin'
        self.mock.mkdir()
        self.calls = self.root / 'calls'
        busybox = self.root / 'busybox'
        busybox.write_text('#!/bin/sh\nexec "$@"\n')
        busybox.chmod(0o755)
        script = (BUILD / 'modules/bkk-control/bkctl').read_text()
        script = script.replace('/data/adb/bkk-control', str(self.state))
        script = script.replace('/data/adb/ksu/bin/busybox', str(busybox))
        script = script.replace('/sys/kernel/bkk_control', str(self.root / 'sysfs'))
        (self.module / 'bkctl').write_text(script)
        (self.module / 'module.prop').write_text('version=P1\n')
        (self.module / 'build-info.txt').write_text('KERNEL_RELEASE=5.15.216_bkk-CaT_P1\n')
        for name, body in {
            'id': 'echo 0',
            'getprop': 'case "$1" in ro.product.device) echo sheng;; ro.soc.model) echo SM8550;; esac',
            'uname': 'echo 5.15.216_bkk-CaT_r1',
            'insmod': f'echo insmod >> "{self.calls}"; exit 1',
        }.items():
            target = self.mock / name
            target.write_text('#!/bin/sh\n' + body + '\n')
            target.chmod(0o755)

    def run_ctl(self, *args):
        import os
        env = dict(os.environ, PATH=f'{self.mock}:' + os.environ['PATH'])
        return subprocess.run(['sh', str(self.module / 'bkctl'), *args],
                              env=env, text=True, capture_output=True, timeout=5)

    def test_first_boot_does_not_load_or_select_auto(self):
        result = self.run_ctl('boot')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(self.calls.exists())
        self.assertFalse((self.state / 'daemon.pid').exists())
        self.assertIn('configured_mode=disabled', self.run_ctl('status').stdout)

    def test_kernel_mismatch_fails_before_insmod(self):
        result = self.run_ctl('set', 'mode', 'auto')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(self.calls.exists())
        self.assertIn('configured_mode=disabled', self.run_ctl('status').stdout)

    def test_restore_is_idempotent_and_withdraws_request(self):
        request = self.root / 'sysfs/request'
        request.parent.mkdir()
        request.write_text('auto 30000 60000 1\n')
        for _ in range(2):
            result = self.run_ctl('restore')
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(request.read_text(), 'off\n')
            self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')


if __name__ == '__main__':
    unittest.main()
