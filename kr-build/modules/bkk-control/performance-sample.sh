read_node() {
    [ -r "$1" ] || return 0
    printf '[%s]\n' "$1"
    cat "$1" 2>/dev/null || true
    printf '\n'
}
initialise_samples() {
    printf 'started=%s\n' "$(date '+%F %T %z')"
    uname -a
    printf 'device=%s\nsoc=%s\nbuild=%s\n' "$(getprop ro.product.device)" \
        "$(getprop ro.soc.model)" "$(getprop ro.build.version.incremental)"
    printf 'frame_timeline=SurfaceFlinger presentation, not Minecraft internal FPS\n'
    ZONES=
    for ZONE in /sys/class/thermal/thermal_zone*; do
        [ -r "$ZONE/type" ] || continue
        TYPE=$(cat "$ZONE/type")
        case "$TYPE" in battery|batt_therm|cpuss-*|cpu-*-*|*gpu*|*skin*)
            printf 'sensor=%s type=%s\n' "$ZONE" "$TYPE"
            ZONES="$ZONES $ZONE/temp" ;;
        esac
    done
    APP= APP_UID= PIDS=
    : > "$WORK/apps"
    for FILE in "$MODDIR/module.prop" "$MODDIR/build-info.txt" /data/adb/bkk-control/control.conf; do
        read_node "$FILE"
    done
    /system/bin/ps -A -o UID,PID,PPID,NAME 2>/dev/null || true
}
sample() {
    printf '\n--- uptime=%s wall=%s ---\n' "$(cat /proc/uptime)" "$(date '+%F %T')"
    grep -E '^(MemTotal|MemFree|MemAvailable|SwapTotal|SwapFree|AnonPages|Slab|SReclaimable|SUnreclaim):' /proc/meminfo || true
    grep -E '^(pswpin|pswpout|pgscan_direct|pgsteal_direct|allocstall|oom_kill|compact_stall)' /proc/vmstat || true
    for FILE in /proc/pressure/cpu /proc/pressure/memory /proc/pressure/io \
        /sys/block/zram0/mm_stat /sys/block/zram0/bd_stat /sys/kernel/bkk_control/status; do read_node "$FILE"; done
    grep '^cpu' /proc/stat || true
    for POLICY in /sys/devices/system/cpu/cpufreq/policy*; do
        for ITEM in scaling_cur_freq scaling_min_freq scaling_max_freq cpuinfo_max_freq; do read_node "$POLICY/$ITEM"; done
    done
    for ITEM in gpuclk gpubusy max_gpuclk thermal_pwrlevel; do read_node "/sys/class/kgsl/kgsl-3d0/$ITEM"; done
    for ITEM in temp current_now voltage_now capacity; do read_node "/sys/class/power_supply/battery/$ITEM"; done
    for FILE in $ZONES; do read_node "$FILE"; done
    if [ "$(($1 % 5))" -eq 0 ]; then
        APP=$(bounded dumpsys activity activities | sed -n \
            's/.*[Rr]esumedActivity[=:].* u[0-9][0-9]* \([A-Za-z0-9._]*\)\/.*/\1/p' | head -n 1)
        APP_UID=
        if [ -n "$APP" ]; then
            APP_UID=$(bounded cmd package list packages -U "$APP" | \
                awk -v package="package:$APP" '$1 == package {sub(/^uid:/, "", $2); print $2; exit}')
            case "$APP_UID" in ''|*[!0-9]*) APP_UID= ;; esac
            if ! grep -Fxq "$APP" "$WORK/apps" && [ "$(wc -l < "$WORK/apps")" -lt 8 ]; then
                printf '%s\n' "$APP" >> "$WORK/apps"
            fi
        fi
        PIDS=
        if [ -n "$APP_UID" ]; then
            PIDS=$(/system/bin/ps -A -o UID,PID,NAME 2>/dev/null | awk -v uid="$APP_UID" 'NR > 1 && $1 == uid {print $2}')
        fi
    fi
    printf 'foreground=%s uid=%s pids=%s\n' "$APP" "$APP_UID" "$PIDS"
    for PID in $PIDS; do
        [ "$(awk '/^Uid:/ {print $2}' "/proc/$PID/status" 2>/dev/null)" = "$APP_UID" ] || continue
        read_node "/proc/$PID/stat"
        grep -E '^(Name|Pid|VmRSS|VmSwap|VmSize|Threads):' "/proc/$PID/status" 2>/dev/null || true
        if [ "$(($1 % 5))" -eq 0 ]; then
            for TASK in /proc/$PID/task/*/stat; do read_node "$TASK"; done
        fi
    done
}
