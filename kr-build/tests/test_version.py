from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(BUILD))
from version import built_metadata, built_version, kernel_release


class VersionTests(unittest.TestCase):
    def test_built_revision_and_package(self):
        for revision in ('r1', 'P1', 'RC-1', 'test.2', '123'):
            for git in ('', '-c1302066c856f', '-c1302066c856f-dirty'):
                with self.subTest(revision=revision, git=git):
                    release = kernel_release(f'5.15.xxx_bkk-CaT_{revision}', '5.15.216') + git
                    self.assertEqual(built_version(release), (f'CaT_13-5.15.216_{revision}', revision))

    def test_metadata_uses_frozen_build(self):
        with tempfile.TemporaryDirectory() as temp:
            out = Path(temp)
            (out / 'include/config').mkdir(parents=True)
            (out / 'include/config/kernel.release').write_text('5.15.216_bkk-CaT_P1-c1302066c856f\n')
            metadata = ('KERNEL_RELEASE=5.15.216_bkk-CaT_P1-c1302066c856f\n'
                        f'KERNEL_COMMIT={"a" * 40}\nPACKAGE_DATE=261017\n')
            (out / 'build-info.txt').write_text(metadata)
            values = built_metadata(out)
            self.assertEqual(values['MODULE_VERSION'], 'P1')
            self.assertEqual(values['PACKAGE_DATE'], '261017')
            (out / 'build-info.txt').write_text(metadata.replace('_P1-', '_r1-'))
            with self.assertRaises(ValueError):
                built_metadata(out)
            (out / 'build-info.txt').write_text(metadata + 'PACKAGE_DATE=261018\n')
            with self.assertRaises(ValueError):
                built_metadata(out)


if __name__ == '__main__':
    unittest.main()
