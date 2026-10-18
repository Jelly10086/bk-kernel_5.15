from pathlib import Path
import os
import shutil
import signal
import subprocess
import tarfile
import tempfile
import time
import unittest

BUILD = Path(__file__).resolve().parents[1]


class PerformanceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.module = self.root / 'module'
        self.module.mkdir()
        self.base = self.root / 'state/performance'
        self.data = self.root / 'app'
        self.files = self.data / 'files'
        self.files.mkdir(parents=True)
        self.bin = self.root / 'bin'
        self.bin.mkdir()
        self.env = dict(os.environ, PATH=f'{self.bin}:' + os.environ['PATH'])
        self.calls = self.root / 'calls'
        self.actor_pids = self.root / 'actors'
        busybox = self.root / 'busybox'
        busybox.write_text('#!/bin/sh\nexec "$@"\n')
        busybox.chmod(0o755)
        if os.environ.get('BKK_TEST_BUSYBOX'):
            real_busybox = Path(os.environ['BKK_TEST_BUSYBOX'])
            # Ubuntu静态BusyBox未启用flock，锁仍使用真实的flock系统调用
            busybox.write_text(f'#!/bin/sh\nif [ "$1" = flock ]; then exec "$@"; fi\nexec "{real_busybox}" "$@"\n')
        script = (BUILD / 'modules/bkk-control/performance.sh').read_text()
        script = script.replace('/system/bin/ls', '/bin/ls')
        for before, after in {
            '/data/adb/ksu/bin/busybox': str(busybox),
            '/data/adb/bkk-control/performance': str(self.base),
            '/data/user/0/org.bkkernel.logexport': str(self.data),
            '/data/data/org.bkkernel.logexport': str(self.data),
            '"$COUNT" -lt 600': '"$COUNT" -lt 15',
        }.items():
            script = script.replace(before, after)
        if os.environ.get('BKK_TEST_BUSYBOX'):
            # ash可能优先调用内置applet，夹具用函数替换Android运行时命令
            commands = ['id', 'chown', 'chcon', 'getprop', 'am', 'dumpsys', 'cmd', 'ps', 'dmesg', 'df']
            script = '\n'.join(f'{name}() {{ "{self.bin / name}" "$@"; }}' for name in commands) + '\n' + script
        self.script = self.module / 'performance.sh'
        self.script.write_text(script)
        source = (BUILD / 'modules/bkk-control/performance-sample.sh').read_text()
        source = source.replace('/system/bin/ps', str(self.bin / 'ps'))
        source = source.replace('/proc/', str(self.root / 'proc') + '/')
        source = source.replace('/sys/', str(self.root / 'sys') + '/')
        (self.module / 'performance-sample.sh').write_text(source)
        (self.module / 'module.prop').write_text('version=t1\n')
        (self.module / 'build-info.txt').write_text('KERNEL_RELEASE=test\n')
        for name, content in {
            'meminfo': 'MemTotal: 12000000 kB\nMemAvailable: 4000000 kB\nSwapFree: 1000000 kB\n',
            'vmstat': 'pswpin 10\npswpout 20\noom_kill 1\n',
            'stat': 'cpu 100 0 50 1000\ncpu0 10 0 5 100\n',
            'uptime': '1234.00 1000.00\n',
            '42/status': 'Name:\tjava\nUid:\t10123\t10123\t10123\t10123\nVmRSS:\t2000000 kB\n',
            '42/stat': '42 (java) R 1 1 1 0 0 0 0 0 0 0 100 20 0 0 0 0 0 0 100 0\n',
            '42/task/42/stat': '42 (Render thread) R 1 1 1 0 0 0 0 0 0 0 80 10\n',
            '43/status': 'Name:\tunrelated\nUid:\t10999\t10999\t10999\t10999\n',
            '43/stat': 'PRIVATE_OTHER_UID\n',
            'pressure/memory': 'some avg10=1.00 total=100\n',
        }.items():
            path = self.root / 'proc' / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
        for name, body in {
            'id': 'echo 0', 'chown': ':', 'chcon': ':',
            'getprop': 'echo sheng',
            'am': f'echo "$*" >> "{self.calls}"',
            'dumpsys': 'case "$*" in "activity activities") echo "topResumedActivity=ActivityRecord{ab u0 net.kdt.pojavlaunch/.Main}";; *) echo "exit_reason=java_crash";; esac',
            'cmd': 'echo "package:net.kdt.pojavlaunch uid:10123"',
            'ps': 'printf "UID PID NAME\\n10123 42 java\\n10123 43 reused\\n"',
            'dmesg': 'echo "lowmemorykiller: Killed process"',
            'df': 'printf "Filesystem 1024-blocks Used Available Capacity Mounted\\nfixture 5000000 10000 4990000 1%% /\\n"',
        }.items():
            path = self.bin / name
            path.write_text('#!/bin/sh\n' + body + '\n')
            path.chmod(0o755)
        for name in ('perfetto', 'logcat'):
            actor = self.bin / name
            actor.write_text(f'''#!/usr/bin/env python3
import os, signal, sys, time
with open({str(self.actor_pids)!r}, 'a') as out: out.write(str(os.getpid()) + '\\n')
args = sys.argv[1:]
if '-o' in args:
    with open(args[args.index('-o') + 1], 'w') as out: out.write('trace')
def stop(*_): sys.exit(0)
signal.signal(signal.SIGINT, stop)
signal.signal(signal.SIGTERM, stop)
while True: time.sleep(0.1)
''')
            actor.chmod(0o755)

    def tearDown(self):
        for path in self.base.glob('*/ready') if self.base.exists() else []:
            try: os.kill(int(path.read_text()), signal.SIGTERM)
            except (ValueError, ProcessLookupError): pass
        if self.actor_pids.exists():
            for pid in self.actor_pids.read_text().splitlines():
                try: os.kill(int(pid), signal.SIGTERM)
                except ProcessLookupError: pass
        time.sleep(0.05)
        self.temp.cleanup()

    def call(self, *args):
        return subprocess.run(['sh', str(self.script), *args], env=self.env,
                              text=True, capture_output=True, timeout=10)

    def status(self):
        result = self.call('status')
        self.assertEqual(result.returncode, 0, result.stderr)
        return dict(line.split('=', 1) for line in result.stdout.splitlines())

    def wait_for(self, predicate, timeout=10):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if predicate(): return
            time.sleep(0.05)
        logs = ''.join(p.read_text() for p in self.base.glob('*/worker.log'))
        self.fail('Timed out\n' + logs + '\n' + str(self.status()))

    def start(self, select=True):
        result = self.call('start', str(self.data), '10000')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.name = (self.base / 'active').read_text().strip()
        if select:
            self.marker('choice').write_text('selected\n')
            self.wait_for(lambda: self.status()['performance_state'] == 'recording')
            self.wait_for(lambda: (self.base / self.name / 'samples.txt').exists())

    def marker(self, suffix):
        return self.files / (self.name + '.tar.gz.' + suffix)

    def archive_ready(self):
        staged = self.files / (self.name + '.tar.gz')
        return staged.exists() and self.status()['performance_state'] == 'saving'

    def acknowledge(self, result='saved'):
        self.wait_for(self.archive_ready)
        self.marker('result').write_text(result + '\n')
        self.wait_for(lambda: self.status()['performance_state'] == ('saved' if result == 'saved' else 'error'))
        self.wait_for(lambda: subprocess.run(['flock', '-n', str(self.base / 'capture.lock'), 'true']).returncode == 0)

    def test_reading_status_does_not_start_sampling(self):
        self.assertEqual(self.status()['performance_state'], 'idle')
        self.assertFalse(self.calls.exists())
        self.assertFalse(self.actor_pids.exists())
        self.assertFalse((self.base / 'active').exists())

    def test_cancel_picker_does_not_sample_or_create_archive(self):
        self.start(select=False)
        self.assertEqual(self.status()['performance_state'], 'choosing')
        self.assertFalse(self.actor_pids.exists())
        self.assertFalse((self.base / self.name / 'metadata.txt').exists())
        self.marker('choice').write_text('cancelled\n')
        self.wait_for(lambda: self.status()['performance_state'] == 'cancelled')
        self.assertFalse((self.base / (self.name + '.tar.gz')).exists())
        self.assertFalse(self.marker('request').exists())

    def test_notification_stop_produces_archive_and_releases_lock(self):
        self.start()
        self.wait_for(lambda: self.actor_pids.exists() and len(self.actor_pids.read_text().splitlines()) == 2)
        second = self.call('start', str(self.data), '10000')
        self.assertNotEqual(second.returncode, 0)
        self.marker('stop').write_text('stop\n')
        self.acknowledge()
        with tarfile.open(self.base / (self.name + '.tar.gz')) as archive:
            samples = archive.extractfile('./samples.txt').read().decode()
            self.assertIn('MemAvailable: 4000000 kB', samples)
            self.assertIn('net.kdt.pojavlaunch', samples)
            self.assertIn('Render thread', samples)
            self.assertNotIn('PRIVATE_OTHER_UID', samples)
            self.assertIn('./performance.perfetto-trace', archive.getnames())
        self.assertEqual(self.status()['performance_retry'], '0')
        self.assertFalse((self.base / self.name).exists())
        for pid in self.actor_pids.read_text().splitlines():
            self.assertFalse(Path('/proc', pid).exists(), pid)
        self.start(select=False)
        self.call('cancel')

    def test_failed_save_retains_archive_and_can_retry(self):
        self.start()
        self.call('stop')
        self.acknowledge('error')
        self.assertEqual(self.status()['performance_retry'], '1')
        original = (self.base / (self.name + '.tar.gz')).read_bytes()
        self.assertNotEqual(self.call('start', str(self.data), '10000').returncode, 0)
        result = self.call('stop')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.wait_for(self.archive_ready)
        self.wait_for(lambda: '.PerformanceActivity' in self.calls.read_text() and 'operation resume' in self.calls.read_text())
        self.marker('result').write_text('saved\n')
        self.wait_for(lambda: self.status()['performance_state'] == 'saved')
        self.assertEqual((self.base / (self.name + '.tar.gz')).read_bytes(), original)

    def test_uninstall_cancel_stops_children_and_preserves_archive(self):
        self.start()
        result = self.call('cancel')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.status()['performance_state'], 'ready')
        self.assertTrue((self.base / (self.name + '.tar.gz')).is_file())

    def test_invalid_helper_data_path_rejected_before_launch(self):
        result = self.call('start', str(self.root), '10000')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(self.calls.exists())

    def test_killed_sampler_reaps_children_and_recovery_saves_raw_data(self):
        self.start()
        self.wait_for(lambda: self.actor_pids.exists() and len(self.actor_pids.read_text().splitlines()) == 2)
        worker = int((self.base / self.name / 'ready').read_text())
        os.kill(worker, signal.SIGKILL)
        self.wait_for(lambda: self.status()['performance_state'] == 'interrupted')
        self.wait_for(lambda: all(not Path('/proc', pid).exists() for pid in self.actor_pids.read_text().splitlines()))
        self.assertEqual(self.status()['performance_retry'], '1')
        result = self.call('stop')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.acknowledge()
        with tarfile.open(self.base / (self.name + '.tar.gz')) as archive:
            self.assertIn('reason=recovered', archive.extractfile('./final.txt').read().decode())

    def test_lost_notification_stops_capture_without_deleting_log(self):
        self.script.write_text(self.script.read_text().replace('"$MISSING_HEARTBEATS" -ge 20', '"$MISSING_HEARTBEATS" -ge 3'))
        self.start()
        self.wait_for(lambda: self.status()['performance_state'] == 'error')
        self.assertEqual(self.status()['performance_retry'], '1')
        with tarfile.open(self.base / (self.name + '.tar.gz')) as archive:
            self.assertIn('reason=notification_lost', archive.extractfile('./final.txt').read().decode())


if __name__ == '__main__':
    unittest.main()
