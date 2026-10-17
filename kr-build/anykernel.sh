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
[ -s "$AKHOME/kernel.release" ] || abort 'Missing kernel.release. Aborting...'
ui_print "Kernel: $(cat "$AKHOME/kernel.release")"
ui_print "Target: $BLOCK"
. tools/bkk-memory.sh
trap cleanup_allocator EXIT
prepare_allocator
split_boot
flash_boot

image_size=$(wc -c < "$AKHOME/boot-new.img")
[ "$image_size" -gt 0 ] || abort 'Empty repacked boot image. Aborting...'
sync
expected_hash=$(sha256sum "$AKHOME/boot-new.img" | cut -d' ' -f1)
[ "${#expected_hash}" -eq 64 ] || abort 'Unable to hash repacked boot image. Aborting...'
actual_hash=$(head -c "$image_size" "$BLOCK" | sha256sum | cut -d' ' -f1)
[ "$actual_hash" = "$expected_hash" ] || abort 'Boot readback mismatch. Flash verification failed.'
commit_allocator
ui_print 'Boot write verified. Reboot to use this kernel.'
