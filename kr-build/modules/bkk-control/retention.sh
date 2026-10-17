#!/system/bin/sh
set -eu
umask 077
BB=/data/adb/ksu/bin/busybox
[ "$(id -u)" = 0 ] && [ -x "$BB" ] || exit 1
if [ "${BKK_RETENTION_SHELL:-0}" != 1 ]; then
    BKK_RETENTION_SHELL=1 exec "$BB" sh "$0" "$@"
fi
unset BKK_RETENTION_SHELL
STATE=/data/adb/bkk-control/retention
PACKAGES='com.tencent.mobileqq tv.danmaku.bilibilihd'
URI=content://com.miui.powerkeeper.configure/userTable
mkdir -p "$STATE"
exec 9>"$STATE/policy.lock"
"$BB" flock 9
die() { echo "[ERROR] $*" >&2; exit 1; }
compatible() {
    [ "$(getprop ro.product.device)" = sheng ] &&
        [ "$(sha256sum /system_ext/framework/miui-framework.jar | cut -d' ' -f1)" = 5cb9870cdf2dd4cb093e1d575e808a553f4a9f0ef38885674e4567f06f4e24d9 ]
}
get_bg() {
    RESULT=$(content query --uri "$URI" --projection bgControl --where "pkgName='$1' AND userId=0") || return 1
    BG=$(printf '%s\n' "$RESULT" | sed -n 's/.*bgControl=\([A-Za-z][A-Za-z]*\).*/\1/p')
    case "$BG" in ''|*[!A-Za-z]*) return 1 ;; esac
    printf '%s\n' "$BG"
}
set_bg() {
    content update --uri "$URI" --bind "bgControl:s:$2" --where "pkgName='$1' AND userId=0" >/dev/null || return 1
    [ "$(get_bg "$1")" = "$2" ]
}
get_lock() {
    RESULT=$(service call ProcessManager 9 s16 "$1" i32 0) || return 1
    WORDS=$(printf '%s\n' "$RESULT" | "$BB" grep -Eo '[0-9a-fA-F]{8}' | tr '\n' ' ')
    case "$WORDS" in
        '00000000 00000000 ') echo 0 ;;
        '00000000 00000001 ') echo 1 ;;
        *) return 1 ;;
    esac
}
set_lock() {
    service call ProcessManager 3 s16 "$1" i32 0 i32 "$2" >/dev/null || return 1
    [ "$(get_lock "$1")" = "$2" ]
}
restore_values() {
    FAILED=0
    rm -f "$STATE/enabled"
    for PACKAGE in $PACKAGES; do
        [ -f "$STATE/$PACKAGE" ] || continue
        IFS='|' read -r ORIGINAL_BG ORIGINAL_LOCK < "$STATE/$PACKAGE"
        CURRENT_BG=$(get_bg "$PACKAGE") || { FAILED=1; continue; }
        CURRENT_LOCK=$(get_lock "$PACKAGE") || { FAILED=1; continue; }
        # 只恢复仍由本策略持有的值，保留用户后来做的其他选择
        if [ "$CURRENT_BG" = noRestrict ] && [ "$ORIGINAL_BG" != noRestrict ]; then
            set_bg "$PACKAGE" "$ORIGINAL_BG" || FAILED=1
        fi
        if [ "$CURRENT_LOCK" = 1 ] && [ "$ORIGINAL_LOCK" = 0 ]; then
            set_lock "$PACKAGE" 0 || FAILED=1
        fi
    done
    [ "$FAILED" = 0 ] || return 1
    for PACKAGE in $PACKAGES; do rm -f "$STATE/$PACKAGE"; done
}
case "${1:-status}" in
    boot) [ -f "$STATE/enabled" ] || exit 0 ;;
    apply|restore|status) ;;
    *) die '用法：retention.sh apply|restore|status' ;;
esac
if [ "${1:-status}" = restore ] && [ ! -f "$STATE/enabled" ] &&
    [ ! -f "$STATE/com.tencent.mobileqq" ] && [ ! -f "$STATE/tv.danmaku.bilibilihd" ]; then
    exit 0
fi
compatible || die '系统版本未核对，保活策略未修改'
case "${1:-status}" in
    apply|boot)
        rollback() {
            trap - EXIT
            restore_values || echo '[ERROR] 保活恢复未完成，请执行retain restore' >&2
        }
        trap rollback EXIT
        for PACKAGE in $PACKAGES; do
            if [ ! -f "$STATE/$PACKAGE" ]; then
                ORIGINAL_BG=$(get_bg "$PACKAGE") || die '无法读取原后台策略'
                ORIGINAL_LOCK=$(get_lock "$PACKAGE") || die '无法读取原锁定状态'
                printf '%s|%s\n' "$ORIGINAL_BG" "$ORIGINAL_LOCK" > "$STATE/$PACKAGE.tmp"
                mv "$STATE/$PACKAGE.tmp" "$STATE/$PACKAGE"
            fi
        done
        sync
        for PACKAGE in $PACKAGES; do
            set_bg "$PACKAGE" noRestrict || die '后台策略设置失败'
            set_lock "$PACKAGE" 1 || die '应用锁定设置失败'
        done
        touch "$STATE/enabled"
        trap - EXIT
        ;;
    restore) restore_values || die '保活恢复未完成，可再次执行retain restore' ;;
esac
if [ -f "$STATE/enabled" ]; then echo retention_enabled=1; else echo retention_enabled=0; fi
for PACKAGE in $PACKAGES; do
    printf 'package=%s bg=%s locked=%s\n' "$PACKAGE" "$(get_bg "$PACKAGE")" "$(get_lock "$PACKAGE")"
done
