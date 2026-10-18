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
        self.shell_calls = self.root / 'shell-calls'
        busybox = self.root / 'busybox'
        busybox.write_text(f'#!/bin/sh\n[ "$1" != sh ] || echo shell >> "{self.shell_calls}"\nexec "$@"\n')
        busybox.chmod(0o755)
        script = (BUILD / 'modules/bkk-control/bkctl').read_text()
        script = script.replace('/data/adb/bkk-control', str(self.state))
        script = script.replace('/data/adb/ksu/bin/busybox', str(busybox))
        script = script.replace('/sys/kernel/bkk_control', str(self.root / 'sysfs'))
        script = script.replace('/sys/class/thermal', str(self.root / 'thermal'))
        script = script.replace('/sys/class/backlight/panel0-backlight/brightness', str(self.root / 'brightness'))
        script = script.replace('/sys/module/bkk_perf', str(self.root / 'loaded'))
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

    def test_each_invocation_enters_busybox_before_locking(self):
        for command in ['boot', 'restore', 'status']:
            result = self.run_ctl(command)
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.shell_calls.read_text().splitlines(), ['shell'] * 3)

    def test_kernel_mismatch_fails_before_insmod(self):
        result = self.run_ctl('set', 'mode', 'auto')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(self.calls.exists())
        self.assertIn('configured_mode=disabled', self.run_ctl('status').stdout)

    def test_ui_read_keeps_unmatched_kernel_disabled(self):
        result = self.run_ctl('get-all')
        self.assertEqual(result.returncode, 0, result.stderr)
        values = dict(line.split('=', 1) for line in result.stdout.splitlines())
        self.assertEqual(values['kernel_compatible'], '0')
        self.assertEqual(values['runtime_mode'], 'waiting_kernel')
        self.assertEqual(values['mode'], 'disabled')
        self.assertEqual(values['module_version'], 'P1')
        self.assertFalse(self.calls.exists())
        self.assertFalse((self.state / 'control.conf').exists())
        self.assertFalse((self.state / 'daemon.pid').exists())

    def test_ui_queries_skip_temperatures_and_runtime_skips_static_properties(self):
        script = self.module / 'bkctl'
        script.write_text(script.read_text().replace('find_sensors() {\n',
            f'find_sensors() {{\n    echo scanned >> "{self.calls}"\n'))
        properties = self.root / 'properties'
        (self.mock / 'getprop').write_text(f'#!/bin/sh\necho "$1" >> "{properties}"\necho value\n')
        result = self.run_ctl('get-all')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(self.calls.exists())
        self.assertEqual(len(properties.read_text().splitlines()), 2)
        properties.unlink()
        result = self.run_ctl('get-runtime')
        self.assertEqual(result.returncode, 0, result.stderr)
        values = dict(line.split('=', 1) for line in result.stdout.splitlines())
        self.assertEqual(values['kernel_compatible'], '0')
        self.assertEqual(values['mode'], 'disabled')
        self.assertEqual(values['runtime_mode'], 'waiting_kernel')
        self.assertEqual(values['performance_state'], 'idle')
        self.assertNotIn('system_version', values)
        self.assertNotIn('module_version', values)
        self.assertNotIn('cpu_temperature', values)
        self.assertFalse(properties.exists())
        self.assertFalse(self.calls.exists())

    def test_ui_maps_kernel_state_without_loading_module(self):
        (self.module / 'build-info.txt').write_text('KERNEL_RELEASE=5.15.216_bkk-CaT_r1\n')
        self.state.mkdir()
        (self.state / 'control.conf').write_text('mode=auto\n')
        status = self.root / 'sysfs/status'
        status.parent.mkdir()
        for kernel_state, ui_state in [('boost', 'active'), ('thermal', 'cooling'), ('idle', 'warming')]:
            status.write_text(f'mode=1\nstate={kernel_state}\ncluster1_floor=1651200\n')
            result = self.run_ctl('get-all')
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('kernel_compatible=1\n', result.stdout)
            self.assertIn(f'runtime_mode={ui_state}\n', result.stdout)
        self.assertFalse(self.calls.exists())

    def test_ui_waits_for_the_selected_mode_and_actual_votes(self):
        (self.module / 'build-info.txt').write_text('KERNEL_RELEASE=5.15.216_bkk-CaT_r1\n')
        self.state.mkdir()
        status = self.root / 'sysfs/status'
        status.parent.mkdir()
        cases = [
            ('manual', 0, 'disabled', 0, 'pending'),
            ('manual', 1, 'boost', 1651200, 'pending'),
            ('auto', 2, 'boost', 1651200, 'pending'),
            ('auto', 1, 'boost', 0, 'pending'),
            ('manual', 2, 'idle', 0, 'pending'),
            ('manual', 2, 'boost', 1651200, 'active'),
            ('auto', 1, 'idle', 0, 'warming'),
            ('manual', 2, 'thermal', 0, 'cooling'),
            ('manual', 0, 'screen_off', 0, 'screen_off'),
        ]
        for configured, applied, reason, floor, expected in cases:
            with self.subTest(configured=configured, applied=applied, reason=reason, floor=floor):
                (self.state / 'control.conf').write_text(f'mode={configured}\n')
                status.write_text(f'mode={applied}\nstate={reason}\ncluster1_floor={floor}\n')
                result = self.run_ctl('get-all')
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn(f'runtime_mode={expected}\n', result.stdout)
        self.assertFalse(self.calls.exists())

    def test_log_names_reject_traversal_before_resolving_exporter(self):
        for command in ['open-log', 'cleanup-log']:
            for name in ['../../control.conf', 'bkk-control-1/../../control.conf.tar.gz', 'bkk-control-1.tar.gz']:
                result = self.run_ctl(command, name)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('日志文件名无效', result.stderr)

    def test_restore_is_idempotent_and_withdraws_request(self):
        request = self.root / 'sysfs/request'
        request.parent.mkdir()
        request.write_text('auto 30000 60000 1\n')
        for _ in range(2):
            result = self.run_ctl('restore')
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(request.read_text(), 'off\n')
            self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')

    def prepare_matching_control(self):
        (self.module / 'build-info.txt').write_text(
            'KERNEL_RELEASE=5.15.216_bkk-CaT_r1\nMODULE_SRCVERSION=CHECKED\n')
        status = self.root / 'sysfs/status'
        status.parent.mkdir()
        status.write_text('version=P1\nstate=idle\n')
        (status.parent / 'request').write_text('off\n')
        (self.root / 'loaded').mkdir()
        (self.root / 'loaded/srcversion').write_text('CHECKED\n')
        for index, kind, temp in [(0, 'cpuss-0', 70000), (1, 'battery', 40000)]:
            zone = self.root / f'thermal/thermal_zone{index}'
            zone.mkdir(parents=True)
            (zone / 'type').write_text(kind + '\n')
            (zone / 'temp').write_text(str(temp) + '\n')
        (self.root / 'brightness').write_text('100\n')
        path = self.module / 'bkctl'
        path.write_text(path.read_text().replace('start_daemon() {\n',
            f'start_daemon() {{\n    cat "$REQUEST" > "{self.calls}"\n    return 0\n'))

    def test_mode_change_sends_fresh_sensors_before_starting_daemon(self):
        self.prepare_matching_control()
        for selected in ['manual', 'auto']:
            result = self.run_ctl('set', 'mode', selected)
            self.assertEqual(result.returncode, 0, result.stderr)
            expected = f'{selected} 40000 70000 1\n'
            self.assertEqual((self.root / 'sysfs/request').read_text(), expected)
            self.assertEqual(self.calls.read_text(), expected)
            self.assertEqual((self.state / 'control.conf').read_text(), f'mode={selected}\n')

    def test_missing_sensor_cancels_mode_change_and_withdraws_old_request(self):
        self.prepare_matching_control()
        (self.root / 'thermal/thermal_zone0/temp').unlink()
        result = self.run_ctl('set', 'mode', 'manual')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual((self.root / 'sysfs/request').read_text(), 'off\n')
        self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')
        self.assertFalse(self.calls.exists())

    def test_missing_runtime_status_fails_closed(self):
        self.prepare_matching_control()
        (self.root / 'sysfs/status').write_text('version=P1\n')
        result = self.run_ctl('set', 'mode', 'auto')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual((self.root / 'sysfs/request').read_text(), 'off\n')
        self.assertEqual((self.state / 'control.conf').read_text(), 'mode=disabled\n')
        self.assertFalse(self.calls.exists())


if __name__ == '__main__':
    unittest.main()
