#!/system/bin/sh
MODDIR=${0%/*}
"$MODDIR/bkctl" restore || exit 1
if [ -d /sys/module/bkk_perf ]; then
    rmmod bkk_perf || exit 1
fi
