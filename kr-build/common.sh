#!/usr/bin/env bash

KR_BUILD_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
# shellcheck source=pins.env
source "$KR_BUILD_DIR/pins.env"
CLANG_DIR=${CLANG_DIR:-$HOME/project/GoogleClang}
OUT_DIR=${OUT_DIR:-$KR_BUILD_DIR/out}
DIST_DIR=${DIST_DIR:-$KR_BUILD_DIR/dist}

die() { printf '[ERROR] %s\n' "$*" >&2; exit 1; }
log() { printf '[INFO] %s\n' "$*"; }

valid_package_version() {
    [[ "$1" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,127}$ && "$1" != bkk-* ]]
}

read_package_version() {
    local version
    version=${PACKAGE_VERSION:-}
    if [[ -z "$version" ]]; then
        version=$(cat "$KR_BUILD_DIR/version") || die "无法读取 kr-build/version"
    fi
    valid_package_version "$version" || die "内核版本格式无效"
    python3 "$KR_BUILD_DIR/version.py" "$version"
}

require_commands() {
    local tool
    for tool in "$@"; do
        command -v "$tool" >/dev/null 2>&1 || die "缺少依赖：$tool"
    done
}

check_clang() {
    local tool version
    for tool in clang ld.lld llvm-ar llvm-nm llvm-objcopy llvm-objdump llvm-readelf llvm-strip; do
        [[ -x "$CLANG_DIR/bin/$tool" ]] || die "缺少工具链程序：$CLANG_DIR/bin/$tool"
    done
    version=$("$CLANG_DIR/bin/clang" --version)
    [[ "$version" == *"based on ${CLANG_PACKAGE#clang-}"* ]] || die "需要 Google Clang $CLANG_PACKAGE"
    [[ "$version" == *"clang version $CLANG_RELEASE "* ]] || die "Clang 版本与 pins.env 不符"
}
