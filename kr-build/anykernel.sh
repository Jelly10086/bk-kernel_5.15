#!/sbin/sh
# AnyKernel3 installer for sheng, based on osm0sis/AnyKernel3

properties() { '
kernel.string=bkk-@VERSION@
do.devicecheck=1
do.modules=0
do.systemless=1
do.cleanup=1
do.cleanuponabort=0
device.name1=sheng
supported.versions=
supported.patchlevels=
supported.vendorpatchlevels=
'; }

BLOCK=boot
IS_SLOT_DEVICE=1
SLOT_SELECT=active
RAMDISK_COMPRESSION=auto
PATCH_VBMETA_FLAG=0
NO_VBMETA_PARTITION_PATCH=1

. tools/ak3-core.sh
ui_print "kernel by Github@Jelly10086"
split_boot
flash_boot
