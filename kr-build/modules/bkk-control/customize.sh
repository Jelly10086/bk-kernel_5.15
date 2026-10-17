#!/system/bin/sh
[ "$KSU" = true ] || abort '需要KernelSU/BakaSU'
[ "$(getprop ro.product.device)" = sheng ] || abort '仅支持sheng'
[ "$(getprop ro.soc.model)" = SM8550 ] || abort 'CPU配置不匹配'
BUILT_RELEASE=$(sed -n 's/^KERNEL_RELEASE=//p' "$MODPATH/build-info.txt")
[ "$(uname -r)" = "$BUILT_RELEASE" ] || abort '先启动本包配套的CaT内核，再安装控制模块'
OLD=/data/adb/modules/bkk-control/bkctl
if [ -x "$OLD" ]; then
    "$OLD" restore || abort '旧模块恢复失败'
fi
mkdir -p /data/adb/bkk-control
chmod 0700 /data/adb/bkk-control
printf 'mode=disabled\n' > /data/adb/bkk-control/control.conf
for FILE in bkctl service.sh action.sh uninstall.sh; do
    [ -f "$MODPATH/$FILE" ] || abort "缺少$FILE"
    set_perm "$MODPATH/$FILE" 0 0 0755
done
ui_print 'bkk-control - 灰烬模式默认关闭'
