#!/usr/bin/env bash
set -Eeuo pipefail

source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
case "${1:-}" in
    -h|--help) printf '用法：bash kr-build/fetch-clang.sh [工具链目录]\n'; exit 0 ;;
    --*) die "未知参数：$1" ;;
esac
[[ $# -le 1 ]] || die "只接受一个目标目录"
CLANG_DIR=${1:-$CLANG_DIR}
require_commands curl tar python3 flock
CLANG_DIR=$(realpath -m -- "$CLANG_DIR")
mkdir -p -- "$(dirname -- "$CLANG_DIR")"
exec 9>"${CLANG_DIR}.download.lock"
flock -n 9 || die "另一个进程正在下载此工具链"
if [[ -d "$CLANG_DIR" ]]; then
    check_clang
    log "复用 $CLANG_DIR - $CLANG_PACKAGE"
    exit 0
fi
[[ ! -e "$CLANG_DIR" ]] || die "目标已存在：$CLANG_DIR"
stage=$(mktemp -d "${CLANG_DIR}.download.XXXXXX")
trap 'rm -rf -- "$stage"' EXIT
log "下载 $CLANG_PACKAGE - $CLANG_RELEASE"
curl --fail --location --show-error --retry 5 --retry-delay 3 \
    --connect-timeout 30 --speed-limit 1024 --speed-time 120 \
    --output "$stage/clang.tar.gz" \
    "$CLANG_REPOSITORY/+archive/$CLANG_COMMIT/$CLANG_PACKAGE.tar.gz"
mkdir "$stage/toolchain"
tar -xzf "$stage/clang.tar.gz" -C "$stage/toolchain"
CLANG_DIR="$stage/toolchain" check_clang
python3 - "$stage/toolchain/toolchain-source.json" "$CLANG_REPOSITORY" \
    "$CLANG_COMMIT" "$CLANG_PACKAGE" "$CLANG_RELEASE" <<'PY'
import json
import sys
from pathlib import Path

path, repository, commit, directory, version = sys.argv[1:]
Path(path).write_text(json.dumps({
    "repository": repository, "branch": "main-kernel", "commit": commit,
    "directory": directory, "version": version,
}, indent=2) + "\n")
PY
[[ ! -e "$CLANG_DIR" ]] || die "下载期间目标目录已出现：$CLANG_DIR"
mv -T -- "$stage/toolchain" "$CLANG_DIR"
log "工具链已就绪：$CLANG_DIR"
