from pathlib import Path
import os
import subprocess
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]
PACKAGES = ['com.tencent.mobileqq', 'tv.danmaku.bilibilihd']


class RetentionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.state = self.root / 'state'
        self.bin = self.root / 'bin'
        self.bin.mkdir()
        for package, bg in zip(PACKAGES, ['miuiAuto', 'noRestrict']):
            (self.root / f'{package}.bg').write_text(bg + '\n')
            (self.root / f'{package}.lock').write_text('0\n')
        prefix = f'ROOT="{self.root}"\n'
        scripts = {
            'busybox': 'exec "$@"',
            'id': 'echo 0',
            'getprop': 'echo sheng',
            'sha256sum': 'echo "${FRAMEWORK_HASH:-5cb9870cdf2dd4cb093e1d575e808a553f4a9f0ef38885674e4567f06f4e24d9}  framework"',
            'sync': ':',
            'content': prefix + '''ACTION=$1
WHERE=
while [ "$#" -gt 0 ]; do
    case "$1" in --where) WHERE=$2; shift;; --bind) BIND=$2; shift;; esac
    shift
done
PACKAGE=$(printf '%s' "$WHERE" | sed "s/pkgName='\\([^']*\\)'.*/\\1/")
if [ "$ACTION" = query ]; then
    echo "Row: 0 bgControl=$(cat "$ROOT/$PACKAGE.bg")"
else
    echo "${BIND#bgControl:s:}" > "$ROOT/$PACKAGE.bg"
    echo 'Updated 1 rows.'
fi''',
            'service': prefix + '''PACKAGE=$5
if [ "$3" = 3 ]; then
    if [ "${FAIL_BILI_LOCK:-0}" = 1 ] && [ "$PACKAGE" = tv.danmaku.bilibilihd ] && [ "$9" = 1 ]; then
        echo 'Exception: denied'
        exit 1
    fi
    echo "$9" > "$ROOT/$PACKAGE.lock"
    echo 'Result: Parcel(00000000)'
else
    printf 'Result: Parcel(00000000 %08d)\\n' "$(cat "$ROOT/$PACKAGE.lock")"
fi''',
        }
        for name, body in scripts.items():
            path = self.bin / name
            path.write_text('#!/bin/sh\n' + body + '\n')
            path.chmod(0o755)
        script = (BUILD / 'modules/bkk-control/retention.sh').read_text()
        script = script.replace('/data/adb/ksu/bin/busybox', str(self.bin / 'busybox'))
        script = script.replace('/data/adb/bkk-control/retention', str(self.state))
        self.script = self.root / 'retention.sh'
        self.script.write_text(script)

    def run_policy(self, command, **extra):
        env = dict(os.environ, PATH=f'{self.bin}:' + os.environ['PATH'], **extra)
        return subprocess.run(['sh', str(self.script), command], env=env,
                              capture_output=True, text=True, timeout=5)

    def test_apply_is_idempotent_and_restore_keeps_original_app_choices(self):
        for _ in range(2):
            result = self.run_policy('apply')
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.state / PACKAGES[0]).read_text(), 'miuiAuto|0\n')
        result = self.run_policy('restore')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / f'{PACKAGES[0]}.bg').read_text(), 'miuiAuto\n')
        self.assertEqual((self.root / f'{PACKAGES[1]}.bg').read_text(), 'noRestrict\n')
        for package in PACKAGES:
            self.assertEqual((self.root / f'{package}.lock').read_text(), '0\n')
        self.assertFalse((self.state / 'enabled').exists())

    def test_partial_failure_restores_both_apps(self):
        result = self.run_policy('apply', FAIL_BILI_LOCK='1')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.state / 'enabled').exists())
        self.assertEqual((self.root / f'{PACKAGES[0]}.bg').read_text(), 'miuiAuto\n')
        for package in PACKAGES:
            self.assertEqual((self.root / f'{package}.lock').read_text(), '0\n')

    def test_unknown_firmware_and_first_boot_do_not_change_apps(self):
        for command, extra in [('boot', {}), ('apply', {'FRAMEWORK_HASH': 'unknown'})]:
            result = self.run_policy(command, **extra)
            self.assertEqual(result.returncode, 0 if command == 'boot' else 1)
            self.assertEqual((self.root / f'{PACKAGES[0]}.bg').read_text(), 'miuiAuto\n')
            self.assertFalse((self.state / 'enabled').exists())

    def test_restore_keeps_later_user_changes(self):
        self.assertEqual(self.run_policy('apply').returncode, 0)
        (self.root / f'{PACKAGES[0]}.bg').write_text('restrictBackground\n')
        self.assertEqual(self.run_policy('restore').returncode, 0)
        self.assertEqual((self.root / f'{PACKAGES[0]}.bg').read_text(), 'restrictBackground\n')


if __name__ == '__main__':
    unittest.main()
