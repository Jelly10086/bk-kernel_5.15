#!/usr/bin/env bash
set -Eeuo pipefail

source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
case "${1:-}" in
    -h|--help) printf '用法：OUT_DIR=输出目录 bash kr-build/pack.sh\n'; exit 0 ;;
esac
[[ $# == 0 ]] || die "未知参数：$1"
require_commands python3
PACKAGE_VERSION=${PACKAGE_VERSION:-$(read_package_version)}
valid_package_version "$PACKAGE_VERSION" || die "版本格式无效：$PACKAGE_VERSION"
exec python3 "$KR_BUILD_DIR/package.py" --out-dir "$OUT_DIR" --dist-dir "$DIST_DIR" \
    --cache-dir "$KR_BUILD_DIR/.cache" --anykernel-commit "$ANYKERNEL_COMMIT" \
    --version-name "$PACKAGE_VERSION"
