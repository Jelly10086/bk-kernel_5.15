#!/usr/bin/env bash
set -Eeuo pipefail

source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
case "${1:-}" in
    -h|--help) printf '用法：bash kr-build/set-version.sh [版本]\n'; exit 0 ;;
esac
[[ $# -le 1 ]] || die "参数过多"
require_commands python3 date
printf '当前内核版本: %s\n' "$(python3 "$KR_BUILD_DIR/version.py" --release "$(read_package_version)")"
if (( $# == 1 )); then
    template=$(python3 "$KR_BUILD_DIR/version.py" --template "$1")
    version=$(python3 "$KR_BUILD_DIR/version.py" "$template")
else
    while true; do
        IFS= read -r -p '更改内核版本: ' version || die "未输入版本，未修改"
        if template=$(python3 "$KR_BUILD_DIR/version.py" --template "$version" 2>/dev/null); then
            version=$(python3 "$KR_BUILD_DIR/version.py" "$template")
            break
        fi
        printf '格式示例：5.15.xxx_bkk-CaT_r1\n' >&2
    done
fi
release=$(python3 "$KR_BUILD_DIR/version.py" --release "$template")
stage=$(mktemp "$KR_BUILD_DIR/.version.XXXXXX")
trap 'rm -f -- "$stage"' EXIT
printf '%s\n' "$template" > "$stage"
chmod 644 "$stage"
mv -f -- "$stage" "$KR_BUILD_DIR/version"
printf '内核版本: %s\n安装包: bkk-%s-AK3-%s.zip\n' \
    "$release" "$version" "$(TZ=UTC-8 date +%y%m%d)"
