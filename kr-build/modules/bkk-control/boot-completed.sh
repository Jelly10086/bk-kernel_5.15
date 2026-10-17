#!/system/bin/sh
MODDIR=${0%/*}
[ ! -x "$MODDIR/zsmalloc/load.sh" ] || "$MODDIR/zsmalloc/load.sh" confirm
