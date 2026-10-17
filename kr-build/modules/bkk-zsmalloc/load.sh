#!/system/bin/sh
set -eu
umask 077
BB=/data/adb/ksu/bin/busybox
[ -x "$BB" ] || exit 0
if [ "${BKK_ZS_SHELL:-0}" != 1 ]; then
    BKK_ZS_SHELL=1 exec "$BB" sh "$0" "$@"
fi
unset BKK_ZS_SHELL
MODDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
STATE=/data/adb/bkk-zsmalloc
mkdir -p "$STATE"
exec 9>"$STATE/load.lock"
"$BB" flock -n 9 || exit 0
value() { sed -n "s/^$1=//p" "$MODDIR/build-info.txt"; }
report() {
    printf 'state=%s\nkernel=%s\n' "$1" "$(uname -r)" > "$STATE/status.tmp"
    mv -f "$STATE/status.tmp" "$STATE/status"
    printf '[%s] %s\n' "$(date '+%F %T')" "$1" >> "$STATE/load.log"
}
loaded() {
    [ "$(cat /sys/module/zsmalloc/version 2>/dev/null)" = bkk-zs1 ] &&
        [ "$(cat /sys/module/zsmalloc/srcversion 2>/dev/null)" = "$SOURCE" ]
}
case "${1:-load}" in
    status) cat "$STATE/status" 2>/dev/null || echo state=not_loaded; exit 0 ;;
    disable) touch "$STATE/disable"; echo '下次开机使用原厂分配器'; exit 0 ;;
    enable) rm -f "$STATE/disable" "$STATE/pending"; echo '下次开机尝试加载配套分配器'; exit 0 ;;
esac
[ -r "$MODDIR/build-info.txt" ] || exit 0
RELEASE=$(value KERNEL_RELEASE)
SOURCE=$(value MODULE_SRCVERSION)
[ "$(uname -r)" = "$RELEASE" ] || { report kernel_mismatch; exit 0; }
case "$SOURCE" in ''|*[!0-9A-F]*) report invalid_source; exit 0 ;; esac
if [ "${1:-load}" = confirm ]; then
    if [ "$(getprop sys.boot_completed)" = 1 ] && loaded &&
        grep -q '/dev/block/zram0' /proc/swaps; then
        rm -f "$STATE/pending"
        report active
    fi
    exit 0
fi
[ "$(getprop ro.product.device)" = sheng ] && [ "$(getprop ro.soc.model)" = SM8550 ] || exit 0
[ ! -e "$STATE/disable" ] || { report disabled; exit 0; }
if loaded; then exit 0; fi
KEY=$RELEASE-$SOURCE-$(value MODULE_SHA256)
if [ -f "$STATE/pending" ] && grep -Fxq "$KEY" "$STATE/pending"; then
    report previous_attempt_unconfirmed
    exit 0
fi
# 仅在交换区和原厂回写设备尚未初始化时替换，不执行swapoff或reset
[ "$(wc -l < /proc/swaps)" -eq 1 ] || { report swap_already_active; exit 0; }
for NODE in /sys/block/zram*/initstate; do
    [ ! -r "$NODE" ] || [ "$(cat "$NODE")" = 0 ] || { report zram_initialized; exit 0; }
done
for NODE in /sys/block/zram*/backing_dev; do
    [ ! -r "$NODE" ] || [ "$(cat "$NODE")" = none ] || { report backing_already_configured; exit 0; }
done
find_stock() {
    EXPECTED=$1
    shift
    for FILE in "$@"; do
        [ -f "$FILE" ] || continue
        if [ "$(sha256sum "$FILE" | cut -d' ' -f1)" = "$EXPECTED" ]; then
            printf '%s\n' "$FILE"
            return 0
        fi
    done
    return 1
}
STOCK_ZRAM=$(find_stock "$(value STOCK_ZRAM_SHA256)" \
    /system_dlkm/lib/modules/*/kernel/drivers/block/zram/zram.ko) || { report stock_zram_mismatch; exit 0; }
STOCK_ZS=$(find_stock "$(value STOCK_ZSMALLOC_SHA256)" \
    /system_dlkm/lib/modules/*/kernel/mm/zsmalloc.ko) || { report stock_allocator_mismatch; exit 0; }
[ "$(sha256sum "$MODDIR/zsmalloc.ko" | cut -d' ' -f1)" = "$(value MODULE_SHA256)" ] || { report checksum_mismatch; exit 0; }
printf '%s\n' "$KEY" > "$STATE/pending"
sync
if ! rmmod zram; then
    rm -f "$STATE/pending"
    report stock_zram_busy
    exit 0
fi
if ! rmmod zsmalloc; then
    insmod "$STOCK_ZRAM" || { report rollback_failed; exit 0; }
    report stock_allocator_busy
    exit 0
fi
if insmod "$MODDIR/zsmalloc.ko" && loaded && insmod "$STOCK_ZRAM"; then
    report awaiting_boot_confirmation
    exit 0
fi
# 原厂文件保持原样，加载失败时恢复原厂的完整模块组合
if [ -d /sys/module/zram ]; then rmmod zram || { report rollback_failed; exit 0; }; fi
if [ -d /sys/module/zsmalloc ]; then rmmod zsmalloc || { report rollback_failed; exit 0; }; fi
if insmod "$STOCK_ZS" && insmod "$STOCK_ZRAM"; then
    report stock_restored
else
    report rollback_failed
fi
