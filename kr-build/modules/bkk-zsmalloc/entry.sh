#!/system/bin/sh
RELEASE=$(uname -r)
case "$RELEASE" in ''|*[!A-Za-z0-9._+-]*) exit 0 ;; esac
LOADER=/data/adb/bkk-zsmalloc/$RELEASE/load.sh
[ -x "$LOADER" ] || exit 0
case "$0" in
    */boot-completed.d/*) exec "$LOADER" confirm ;;
    *) exec "$LOADER" ;;
esac
