#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
case "${1:-}" in
    -h|--help) printf '用法：OUT_DIR=已有内核输出目录 bash kr-build/build-module.sh\n'; exit 0 ;;
esac
[[ $# == 0 ]] || die "未知参数：$1"
require_commands python3 make flock modinfo
check_clang
OUT_DIR=$(realpath -- "$OUT_DIR")
CLANG_DIR=$(realpath -- "$CLANG_DIR")
KERNEL_DIR=$(cd -- "$KR_BUILD_DIR/.." && pwd)
[[ "$KERNEL_DIR$CLANG_DIR$OUT_DIR" != *[[:space:]]* ]] || die 'Kbuild路径不能包含空白字符'
[[ -s "$OUT_DIR/Module.symvers" && -s "$OUT_DIR/vmlinux" ]] || die '先完成配套内核构建'
exec 8>"$OUT_DIR/.bkk-module.lock"
flock -n 8 || die '控制模块正在构建'
mapfile -t identity < <(python3 - "$KR_BUILD_DIR" "$OUT_DIR" <<'PY'
from pathlib import Path
import sys
sys.path.insert(0, sys.argv[1])
from version import built_metadata
try:
    values = built_metadata(Path(sys.argv[2]))
    print(values['KERNEL_RELEASE'])
    print(values['MODULE_VERSION'])
    print(values['CLANG_COMMIT'])
except (OSError, ValueError, KeyError) as error:
    print(f'[ERROR] {error}', file=sys.stderr)
    sys.exit(1)
PY
)
[[ ${#identity[@]} == 3 ]] || die '构建元数据无效'
[[ "${identity[2]}" == "$CLANG_COMMIT" ]] || die '必须使用本次内核的配套工具链'
module_build="$OUT_DIR/bkk-perf"
mkdir -p -- "$module_build"
cp -- "$KR_BUILD_DIR/modules/bkk-perf/"{Makefile,bkk_perf.c,policy.h} "$module_build/"
export PATH="$CLANG_DIR/bin:$PATH"
log "构建独立 bkk_perf.ko - ${identity[0]}"
make -C "$KERNEL_DIR" O="$OUT_DIR" ARCH=arm64 LLVM=1 LLVM_IAS=1 \
    HOSTCC=gcc HOSTCXX=g++ KCFLAGS=-D__ANDROID_COMMON_KERNEL__ \
    KERNELRELEASE="${identity[0]}" BKK_VERSION="${identity[1]}" \
    M="$module_build" -j"${JOBS:-2}" modules
python3 "$KR_BUILD_DIR/module-package.py" --out-dir "$OUT_DIR" \
    --dist-dir "$DIST_DIR" --module "$module_build/bkk_perf.ko" --clang-dir "$CLANG_DIR"
