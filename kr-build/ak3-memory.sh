prepare_allocator() {
    MEMORY_BASE=/data/adb/bkk-zsmalloc
    MEMORY_RELEASE=$(cat "$AKHOME/kernel.release")
    case "$MEMORY_RELEASE" in ''|*[!A-Za-z0-9._+-]*) abort 'Invalid allocator target release.' ;; esac
    [ -x /data/adb/ksu/bin/busybox ] || abort 'Mount and decrypt /data with KernelSU installed before flashing.'
    mkdir -p "$MEMORY_BASE" /data/adb/post-fs-data.d /data/adb/boot-completed.d || abort 'Allocator install directory is not writable.'
    MEMORY_STAGE=$(mktemp -d "$MEMORY_BASE/.install.XXXXXX") || abort 'Unable to stage allocator.'
    for FILE in load.sh build-info.txt zsmalloc.ko; do
        cp "$AKHOME/allocator/$FILE" "$MEMORY_STAGE/$FILE" || abort 'Unable to copy allocator payload.'
    done
    [ "$(sed -n 's/^KERNEL_RELEASE=//p' "$MEMORY_STAGE/build-info.txt")" = "$MEMORY_RELEASE" ] || abort 'Allocator release mismatch.'
    EXPECTED=$(sed -n 's/^MODULE_SHA256=//p' "$MEMORY_STAGE/build-info.txt")
    ACTUAL=$(sha256sum "$MEMORY_STAGE/zsmalloc.ko" | cut -d' ' -f1)
    [ "${#EXPECTED}" -eq 64 ] && [ "$ACTUAL" = "$EXPECTED" ] || abort 'Allocator checksum mismatch.'
    chmod 0700 "$MEMORY_STAGE"
    chmod 0755 "$MEMORY_STAGE/load.sh"
    chmod 0600 "$MEMORY_STAGE/build-info.txt" "$MEMORY_STAGE/zsmalloc.ko"
    MEMORY_POST=/data/adb/post-fs-data.d/.bkk-zsmalloc.$$
    MEMORY_CONFIRM=/data/adb/boot-completed.d/.bkk-zsmalloc.$$
    cp "$AKHOME/tools/bkk-zsmalloc-entry.sh" "$MEMORY_POST" || abort 'Unable to stage allocator hook.'
    cp "$AKHOME/tools/bkk-zsmalloc-entry.sh" "$MEMORY_CONFIRM" || abort 'Unable to stage allocator confirmation.'
    chmod 0755 "$MEMORY_POST" "$MEMORY_CONFIRM"
}

commit_allocator() {
    MEMORY_TARGET=$MEMORY_BASE/$MEMORY_RELEASE
    MEMORY_PREVIOUS=$MEMORY_BASE/.previous-$MEMORY_RELEASE-$$
    if [ -e "$MEMORY_TARGET" ]; then
        mv "$MEMORY_TARGET" "$MEMORY_PREVIOUS" || abort 'Unable to preserve previous allocator.'
    fi
    if ! mv "$MEMORY_STAGE" "$MEMORY_TARGET"; then
        [ ! -e "$MEMORY_PREVIOUS" ] || mv "$MEMORY_PREVIOUS" "$MEMORY_TARGET"
        abort 'Boot written, but allocator activation failed.'
    fi
    MEMORY_STAGE=
    mv "$MEMORY_POST" /data/adb/post-fs-data.d/bkk-zsmalloc.sh || abort 'Unable to activate allocator hook.'
    mv "$MEMORY_CONFIRM" /data/adb/boot-completed.d/bkk-zsmalloc.sh || abort 'Unable to activate allocator confirmation.'
    sync
    ui_print 'Matching zsmalloc installed - stock ZRAM writeback retained.'
}

cleanup_allocator() {
    if [ -n "${MEMORY_STAGE:-}" ]; then
        rm -f "$MEMORY_STAGE/load.sh" "$MEMORY_STAGE/build-info.txt" "$MEMORY_STAGE/zsmalloc.ko"
        rmdir "$MEMORY_STAGE" 2>/dev/null || true
    fi
    [ -z "${MEMORY_POST:-}" ] || rm -f "$MEMORY_POST"
    [ -z "${MEMORY_CONFIRM:-}" ] || rm -f "$MEMORY_CONFIRM"
}
