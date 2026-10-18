#!/usr/bin/env bash
set -euo pipefail
BUILD_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
command -v gradle >/dev/null || { echo '[ERROR] 需要Gradle 9和JDK 21' >&2; exit 1; }
gradle -p "$BUILD_DIR/webui" --no-daemon --console=plain jsBrowserDistribution
python3 - "$BUILD_DIR" <<'PY'
from pathlib import Path
import re
import shutil
import sys

root = Path(sys.argv[1])
source = root / 'webui/build/dist/js/productionExecutable'
target = root / 'modules/bkk-control/webroot'
required = ['index.html', 'bridge.js', 'style.css', 'bkControl.js', 'composeResources']
bundle = (source / 'bkControl.js').read_text()
wasm = [path for path in source.glob('*.wasm')
        if re.fullmatch(r'[0-9a-f]{20,64}\.wasm', path.name) and path.name in bundle]
if not all((source / name).exists() for name in required) or not wasm:
    raise SystemExit('[ERROR] WebUI产物不完整')
for name in required:
    path = source / name
    if path.is_dir():
        shutil.copytree(path, target / name, dirs_exist_ok=True)
    else:
        shutil.copy2(path, target / name)
for path in target.glob('*.wasm'):
    if path.name not in {item.name for item in wasm}:
        path.unlink()
for path in wasm:
    shutil.copy2(path, target / path.name)
shutil.copy2(root / 'webui/LICENSE-miuix.txt', target / 'LICENSE-miuix.txt')
shutil.copy2(root / 'webui/src/jsMain/resources/LICENSE-remixicon.txt', target / 'LICENSE-remixicon.txt')
print(f'[OK] WebUI：{target}')
PY
