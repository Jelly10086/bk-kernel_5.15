#!/system/bin/sh
MODDIR=${0%/*}
"$MODDIR/bkctl" perf cancel || exit 1
"$MODDIR/retention.sh" restore || exit 1
[ ! -x "$MODDIR/zsmalloc/load.sh" ] || "$MODDIR/zsmalloc/load.sh" disable
"$MODDIR/bkctl" restore || exit 1
if [ -d /sys/module/bkk_perf ]; then
    rmmod bkk_perf || exit 1
fi
