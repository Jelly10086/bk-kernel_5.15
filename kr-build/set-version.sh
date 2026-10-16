#!/usr/bin/env bash
set -Eeuo pipefail

source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
case "${1:-}" in
    -h|--help) printf '用法：bash kr-build/set-version.sh\n'; exit 0 ;;
esac
[[ $# == 0 ]] || die "此脚本通过交互输入版本"
require_commands python3
printf '当前内核版本: bkk-%s\n' "$(read_package_version)"
while true; do
    IFS= read -r -p '更改内核版本: ' version || die "未输入版本，未修改"
    if template=$(python3 "$KR_BUILD_DIR/version.py" --template "$version" 2>/dev/null); then
        version=$(python3 "$KR_BUILD_DIR/version.py" "$template")
        break
    fi
    printf '格式示例：CaT_13-5.15.xxx_b3，不带 bkk- 前缀\n' >&2
done
stage=$(mktemp "$KR_BUILD_DIR/.version.XXXXXX")
trap 'rm -f -- "$stage"' EXIT
printf '%s\n' "$template" > "$stage"
chmod 644 "$stage"
mv -f -- "$stage" "$KR_BUILD_DIR/version"
printf '内核版本: bkk-%s\n安装包: bkk-%s-AK3.zip\n' "$version" "$version"
