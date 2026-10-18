from pathlib import Path
import hashlib
import importlib.util
import stat
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

BUILD = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(BUILD))
spec = importlib.util.spec_from_file_location('module_package', BUILD / 'module-package.py')
module_package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module_package)


class ModulePackageTests(unittest.TestCase):
    def test_capture_assets_scripts_and_helper_are_in_the_installer(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            out = root / 'out'
            (out / 'kernel').mkdir(parents=True)
            (out / 'kernel/gki_module_protected.h').write_text('')
            (out / 'Module.symvers').write_text('0x1234 test_symbol vmlinux EXPORT_SYMBOL_GPL\n')
            (out / 'build-info.txt').write_text('KERNEL_RELEASE=test-release\n')
            lkm = root / 'fixture.ko'
            header = bytearray(20)
            header[:6] = b'\x7fELF\x02\x01'
            header[18:20] = (183).to_bytes(2, 'little')
            lkm.write_bytes(header)
            metadata = {'MODULE_VERSION': 'P2', 'KERNEL_RELEASE': 'test-release', 'PACKAGE_DATE': '261018'}

            def modinfo(args, **kwargs):
                if args[0] != 'modinfo': return 'U test_symbol\n'
                return {'vermagic': 'test-release SMP', 'version': 'P2',
                        'srcversion': '0123456789ABCDEF012345678'}[args[2]]

            def allocator(_out, destination):
                destination.mkdir()
                (destination / 'zsmalloc.ko').write_bytes(b'allocator fixture')

            with patch.object(module_package, 'built_metadata', return_value=metadata), \
                    patch.object(module_package, 'stage_allocator', side_effect=allocator), \
                    patch.object(module_package.subprocess, 'check_output', side_effect=modinfo):
                bundle = module_package.package(out, root / 'dist', lkm, root / 'clang')
            with zipfile.ZipFile(bundle / 'bkk-control-P2-261018.zip') as archive:
                for name in ['performance.sh', 'performance-sample.sh', 'bin/bkk-log-exporter.apk',
                             'webroot/LICENSE-remixicon.txt',
                             'webroot/composeResources/org.bkkernel.control.generated.resources/drawable/play_fill.svg',
                             'webroot/composeResources/org.bkkernel.control.generated.resources/drawable/stop_fill.svg']:
                    self.assertEqual(archive.read(name), (BUILD / 'modules/bkk-control' / name).read_bytes())
                self.assertTrue((archive.getinfo('performance.sh').external_attr >> 16) & stat.S_IXUSR)
                self.assertIn('author=RinnRei', archive.read('module.prop').decode())
                self.assertEqual(archive.testzip(), None)
                self.assertFalse(any(name.startswith(('Image', 'META-INF/', 'system/')) for name in archive.namelist()))
            for line in (bundle / 'bkk-control-SHA256SUMS').read_text().splitlines():
                checksum, name = line.split('  ', 1)
                self.assertEqual(hashlib.sha256((bundle / name).read_bytes()).hexdigest(), checksum)


if __name__ == '__main__':
    unittest.main()
