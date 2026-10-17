#!/system/bin/sh
[ "$KSU" = true ] || abort '需要KernelSU/BakaSU'
[ "$(getprop ro.product.device)" = sheng ] || abort '仅支持sheng'
[ "$(getprop ro.soc.model)" = SM8550 ] || abort 'CPU配置不匹配'
BUILT_RELEASE=$(sed -n 's/^KERNEL_RELEASE=//p' "$MODPATH/build-info.txt")
case "$BUILT_RELEASE" in
    5.15.*_bkk-*) ;;
    *) abort '配套内核版本信息无效' ;;
esac
CURRENT_RELEASE=$(uname -r)
if [ "$CURRENT_RELEASE" != "$BUILT_RELEASE" ]; then
    ui_print "当前内核: $CURRENT_RELEASE"
    ui_print "配套内核: $BUILT_RELEASE"
    ui_print '允许先安装；启动配套内核后才能启用'
fi
OLD=/data/adb/modules/bkk-control/bkctl
if [ -x "$OLD" ]; then
    /data/adb/ksu/bin/busybox sh "$OLD" restore || abort '旧模块恢复失败'
fi
mkdir -p /data/adb/bkk-control
chmod 0700 /data/adb/bkk-control
printf 'mode=disabled\n' > /data/adb/bkk-control/control.conf
for FILE in bkctl service.sh action.sh uninstall.sh post-fs-data.sh boot-completed.sh retention.sh; do
    [ -f "$MODPATH/$FILE" ] || abort "缺少$FILE"
    set_perm "$MODPATH/$FILE" 0 0 0755
done
if [ -f "$MODPATH/zsmalloc/load.sh" ]; then
    set_perm "$MODPATH/zsmalloc/load.sh" 0 0 0755
fi
ui_print 'bkk-control - 安装完成'
