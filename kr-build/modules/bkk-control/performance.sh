#!/system/bin/sh
set -eu
umask 077
BB=/data/adb/ksu/bin/busybox
[ "$(id -u)" = 0 ] && [ -x "$BB" ] || exit 1
if [ "${BKK_PERFORMANCE_SHELL:-0}" != 1 ]; then
    BKK_PERFORMANCE_SHELL=1 exec "$BB" sh "$0" "$@"
fi
unset BKK_PERFORMANCE_SHELL
exec 9>&-
MODDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BASE=/data/adb/bkk-control/performance
mkdir -p "$BASE"
die() { echo "[ERROR] $*" >&2; exit 1; }
valid_name() { printf '%s\n' "$1" | "$BB" grep -Eq '^bkk-performance-[0-9]{8}-[0-9]{6}-[A-Za-z0-9]{6}$'; }
value() { sed -n "s/^$1=//p" "$BASE/status" 2>/dev/null || true; }
running() {
    exec 7>"$BASE/capture.lock"
    if "$BB" flock -n 7; then "$BB" flock -u 7; return 1; fi
    return 0
}
# Binder命令的输出经管道转存，system_server不能写模块私有文件
bounded() { "$BB" timeout -s KILL 4 "$@" 2>/dev/null | cat || true; }
own_file() {
    chown "$EXPORT_UID:$EXPORT_UID" "$1" && chmod 0600 "$1" || return 1
    [ -z "$EXPORT_CONTEXT" ] || chcon "$EXPORT_CONTEXT" "$1" 2>/dev/null
}
publish() {
    printf '%s\n' "$2" > "$BASE/.$NAME.$1.tmp" || return 1
    own_file "$BASE/.$NAME.$1.tmp" || return 1
    "$BB" mv -fT "$BASE/.$NAME.$1.tmp" "$EXPORT_DIR/$NAME.tar.gz.$1"
}
report() {
    RETRY=0
    if [ "$1" != saved ] && { [ -f "$BASE/$NAME.tar.gz" ] || [ -f "$BASE/$NAME/metadata.txt" ]; }; then RETRY=1; fi
    printf 'performance_state=%s\nperformance_name=%s.tar.gz\nperformance_started=%s\nperformance_retry=%s\n' \
        "$1" "$NAME" "${START:-0}" "$RETRY" > "$BASE/.status.$$"
    mv -f "$BASE/.status.$$" "$BASE/status"
    publish progress "$1 $(date +%s)"
}
exporter() {
    EXPORT_DATA=$1
    EXPORT_UID=$2
    case "$EXPORT_DATA" in /data/user/0/org.bkkernel.logexport|/data/data/org.bkkernel.logexport) ;; *) die '日志组件目录无效' ;; esac
    case "$EXPORT_UID" in ''|*[!0-9]*) die '日志组件UID无效' ;; esac
    EXPORT_DIR=$EXPORT_DATA/files
    mkdir -p "$EXPORT_DIR"
    chown "$EXPORT_UID:$EXPORT_UID" "$EXPORT_DIR"
    chmod 0700 "$EXPORT_DIR"
    EXPORT_CONTEXT=$(/system/bin/ls -Zd "$EXPORT_DATA" 2>/dev/null | awk '{print $1}')
    case "$EXPORT_CONTEXT" in u:object_r:app_data_file:*) ;; *) EXPORT_CONTEXT= ;; esac
    [ -z "$EXPORT_CONTEXT" ] || chcon "$EXPORT_CONTEXT" "$EXPORT_DIR" 2>/dev/null || true
}
autostart_mode() {
    cmd appops get --user 0 org.bkkernel.logexport 10008 2>/dev/null |
        sed -n 's/^MIUIOP(10008): \([a-z]*\).*/\1/p'
}
restore_exporter() (
    [ -f "$BASE/autostart-mode" ] || return 0
    MODE=$(cat "$BASE/autostart-mode")
    case "$MODE" in allow|ignore|deny|default) ;; *) return 1 ;; esac
    # 只撤销本轮临时允许，不覆盖用户在采集期间改成的其他状态
    if [ "$(autostart_mode)" = allow ]; then
        cmd appops set --user 0 org.bkkernel.logexport 10008 "$MODE" >/dev/null 2>&1 || return 1
    fi
    rm -f "$BASE/autostart-mode"
)
prepare_exporter() {
    restore_exporter || return 1
    [ -n "$(getprop ro.miui.ui.version)$(getprop ro.mi.os.version.name)" ] || return 0
    MODE=$(autostart_mode)
    case "$MODE" in ignore|deny|default) ;; *) return 0 ;; esac
    printf '%s\n' "$MODE" > "$BASE/.autostart-mode.tmp"
    mv -f "$BASE/.autostart-mode.tmp" "$BASE/autostart-mode"
    # 澎湃OS会拦截被回收后的导出服务，仅在用户采集和保存期间放行
    cmd appops set --user 0 org.bkkernel.logexport 10008 allow >/dev/null 2>&1
}
birth() { sed 's/.*) //' "/proc/$1/stat" 2>/dev/null | awk '$1 != "Z" {print $20}'; }
stop_child() {
    [ -n "$1" ] && [ -n "$2" ] || return 0
    if [ "$(birth "$1")" = "$2" ]; then kill "-$3" "$1" 2>/dev/null || true; fi
    ATTEMPT=0
    while [ "$(birth "$1")" = "$2" ] && [ "$ATTEMPT" -lt 50 ]; do
        sleep 0.1
        ATTEMPT=$((ATTEMPT + 1))
    done
    [ "$(birth "$1")" != "$2" ] || kill -KILL "$1" 2>/dev/null || true
    wait "$1" 2>/dev/null || true
}
supervise_child() {
    CHILD_SIGNAL=$1
    shift
    trap - EXIT INT TERM
    trap 'exit 0' TERM
    child_cleanup() {
        trap - EXIT TERM
        [ -n "${CHILD_BIRTH:-}" ] || return 0
        [ "$(birth "$CHILD_PID")" != "$CHILD_BIRTH" ] || kill "-$CHILD_SIGNAL" "$CHILD_PID" 2>/dev/null || true
        COUNT=0
        while [ "$(birth "$CHILD_PID")" = "$CHILD_BIRTH" ] && [ "$COUNT" -lt 30 ]; do sleep 0.1; COUNT=$((COUNT + 1)); done
        [ "$(birth "$CHILD_PID")" != "$CHILD_BIRTH" ] || kill -KILL "$CHILD_PID" 2>/dev/null || true
        wait "$CHILD_PID" 2>/dev/null || true
    }
    # 采样主进程被杀后也回收Perfetto和logcat，子进程不持有会话锁
    trap child_cleanup EXIT
    "$@" &
    CHILD_PID=$!
    CHILD_BIRTH=$(birth "$CHILD_PID")
    while [ -n "$CHILD_BIRTH" ] && [ "$(birth "$OWNER_PID")" = "$OWNER_BIRTH" ] && \
        [ "$(birth "$CHILD_PID")" = "$CHILD_BIRTH" ]; do sleep 1; done
}
prepare_trace() {
    TRACE_PID= TRACE_BIRTH=
    command -v perfetto >/dev/null 2>&1 || return 0
    cat > "$WORK/perfetto.cfg" <<'CFG'
buffers { size_kb: 16384 fill_policy: RING_BUFFER }
write_into_file: true
file_write_period_ms: 1000
flush_period_ms: 5000
data_sources { config { name: "linux.ftrace" ftrace_config {
  ftrace_events: "power/cpu_frequency"
  ftrace_events: "power/cpu_idle"
} } }
data_sources { config { name: "linux.sys_stats" sys_stats_config {
  meminfo_period_ms: 1000
  vmstat_period_ms: 1000
  stat_period_ms: 1000
  stat_counters: STAT_CPU_TIMES
} } }
data_sources { config { name: "android.surfaceflinger.frametimeline" } }
CFG
    supervise_child INT perfetto --txt -c "$WORK/perfetto.cfg" -o "$WORK/performance.perfetto-trace" 8>&- > "$WORK/perfetto.log" 2>&1 &
    TRACE_PID=$!
    TRACE_BIRTH=$(birth "$TRACE_PID")
}
save_archive() {
    if [ "$(cat "$EXPORT_DIR/$NAME.tar.gz.result" 2>/dev/null)" = saved ]; then
        report saved
        touch "$BASE/$NAME.saved"
        rm -f "$EXPORT_DIR/$NAME.tar.gz.request" "$EXPORT_DIR/$NAME.tar.gz.choice" "$EXPORT_DIR/$NAME.tar.gz.stop"
        return 0
    fi
    rm -f "$EXPORT_DIR/$NAME.tar.gz.result" "$EXPORT_DIR/$NAME.tar.gz.failure"
    if ! cp "$BASE/$NAME.tar.gz" "$BASE/.$NAME.archive.tmp" || \
        ! own_file "$BASE/.$NAME.archive.tmp" || \
        ! "$BB" mv -fT "$BASE/.$NAME.archive.tmp" "$EXPORT_DIR/$NAME.tar.gz"; then
        rm -f "$BASE/.$NAME.archive.tmp"
        report error
        return 1
    fi
    # 完整归档发布后才通知组件复制，避免重试时读到上次残留文件
    report saving
    if [ "${RESUME:-0}" = 1 ]; then
        "$BB" timeout -s KILL 10 am start --user 0 -n org.bkkernel.logexport/.PerformanceActivity \
            -f 0x10000000 --es operation resume --es name "$NAME.tar.gz" >/dev/null 2>&1 || { report error; return 1; }
    else
        # 通知进程可能已被系统回收，归档完成后显式恢复导出服务
        "$BB" timeout -s KILL 10 am start-foreground-service --user 0 \
            -n org.bkkernel.logexport/.CaptureService --es name "$NAME.tar.gz" >/dev/null 2>&1 || {
                publish failure 'Cannot restart export service'
                report error
                return 1
            }
    fi
    COUNT=0
    while [ "$COUNT" -lt 600 ]; do
        RESULT=$(cat "$EXPORT_DIR/$NAME.tar.gz.result" 2>/dev/null || true)
        case "$RESULT" in
            saved)
                report saved
                touch "$BASE/$NAME.saved"
                rm -f "$EXPORT_DIR/$NAME.tar.gz.request" "$EXPORT_DIR/$NAME.tar.gz.choice" "$EXPORT_DIR/$NAME.tar.gz.stop"
                return 0 ;;
            error) report error; return 1 ;;
        esac
        sleep 0.2
        COUNT=$((COUNT + 1))
    done
    report error
    return 1
}
finish_capture() {
    RC=$?
    trap - EXIT INT TERM
    set +e
    report stopping
    stop_child "${TRACE_PID:-}" "${TRACE_BIRTH:-}" TERM
    stop_child "${LOG_PID:-}" "${LOG_BIRTH:-}" TERM
    {
        printf 'finished=%s\nreason=%s\nexit_code=%s\n' "$(date '+%F %T %z')" "${REASON:-error}" "$RC"
        for APP in $(cat "$WORK/apps" 2>/dev/null); do
            bounded dumpsys meminfo "$APP"
            bounded dumpsys activity exit-info "$APP"
        done
        dmesg 2>/dev/null | grep -Ei 'out of memory|oom-kill|killed process|page allocation failure|kgsl.*(fault|error)|zram.*(fail|error)' | tail -n 100
    } > "$WORK/final.txt"
    if ! "$BB" tar -czf "$BASE/.$NAME.tar.gz.tmp" -C "$WORK" .; then
        report error
        restore_exporter
        exit 1
    fi
    if ! mv -f "$BASE/.$NAME.tar.gz.tmp" "$BASE/$NAME.tar.gz"; then report error; restore_exporter; exit 1; fi
    PREVIOUS_ARCHIVE=$(cat "$BASE/latest" 2>/dev/null || true)
    printf '%s.tar.gz\n' "$NAME" > "$BASE/.latest.tmp"
    mv -f "$BASE/.latest.tmp" "$BASE/latest"
    if valid_name "${PREVIOUS_ARCHIVE%.tar.gz}" && [ "$PREVIOUS_ARCHIVE" != "$NAME.tar.gz" ] &&
        [ -f "$BASE/${PREVIOUS_ARCHIVE%.tar.gz}.saved" ]; then
        rm -f "$BASE/$PREVIOUS_ARCHIVE" "$BASE/${PREVIOUS_ARCHIVE%.tar.gz}.saved"
    fi
    if [ "$(cat "$EXPORT_DIR/$NAME.tar.gz.stop" 2>/dev/null)" = cancel ]; then
        report ready
    else
        save_archive || RC=1
    fi
    # 本轮原始文件仅在压缩包完成后清理，失败时保留以便恢复
    rm -f "$WORK"/*
    rmdir "$WORK" 2>/dev/null || true
    restore_exporter
    exit "$RC"
}
worker() {
    NAME=$1
    valid_name "$NAME" || exit 1
    WORK=$BASE/$NAME
    [ -d "$WORK" ] && "$BB" flock -n 8 || exit 1
    exporter "$2" "$3"
    trap restore_exporter EXIT
    prepare_exporter || { report error; return 1; }
    renice 10 $$ >/dev/null 2>&1 || true
    "$BB" ionice -c 3 -p $$ >/dev/null 2>&1 || true
    printf '%s\n' "$$" > "$WORK/ready"
    case "${4:-capture}" in
        save)
            RESUME=1
            save_archive || return 1
            rm -f "$WORK/ready" "$WORK/worker.log"
            rmdir "$WORK" 2>/dev/null || true
            return ;;
        recover) RESUME=1; REASON=recovered; finish_capture ;;
    esac
    COUNT=0
    while [ "$COUNT" -lt 1500 ]; do
        CHOICE=$(cat "$EXPORT_DIR/$NAME.tar.gz.choice" 2>/dev/null || true)
        [ ! -f "$EXPORT_DIR/$NAME.tar.gz.stop" ] || CHOICE=cancelled
        case "$CHOICE" in
            selected) break ;;
            cancelled|error)
                report "$CHOICE"
                rm -f "$EXPORT_DIR/$NAME.tar.gz.request"
                rm -f "$WORK/ready" "$WORK/worker.log"
                rmdir "$WORK" 2>/dev/null || true
                return ;;
        esac
        sleep 0.2
        COUNT=$((COUNT + 1))
    done
    if [ "$CHOICE" != selected ]; then
        report cancelled
        rm -f "$EXPORT_DIR/$NAME.tar.gz.request"
        return
    fi
    . "$MODDIR/performance-sample.sh"
    OWNER_PID=$$
    OWNER_BIRTH=$(birth "$$")
    START=$(cut -d. -f1 /proc/uptime)
    REASON=stopped
    report recording
    trap finish_capture EXIT
    trap 'REASON=interrupted; exit 130' INT TERM
    initialise_samples > "$WORK/metadata.txt"
    prepare_trace
    LOG_PID= LOG_BIRTH=
    if command -v logcat >/dev/null 2>&1; then
        supervise_child TERM logcat -b main -b system -b events -b crash -v threadtime -T 1 \
            -f "$WORK/events.log" -r 1024 -n 2 \
            'lmkd:I' 'lowmemorykiller:I' 'AndroidRuntime:E' 'libc:F' \
            'am_kill:I' 'am_proc_died:I' 'am_crash:I' 'am_anr:I' '*:S' 8>&- > "$WORK/logcat.log" 2>&1 &
        LOG_PID=$!
        LOG_BIRTH=$(birth "$LOG_PID")
    fi
    INDEX=0
    HEARTBEAT=$(cat "$EXPORT_DIR/$NAME.tar.gz.heartbeat" 2>/dev/null || true)
    MISSING_HEARTBEATS=0
    while [ ! -f "$EXPORT_DIR/$NAME.tar.gz.stop" ]; do
        [ ! -e "$MODDIR/disable" ] && [ ! -e "$MODDIR/remove" ] || break
        CURRENT_HEARTBEAT=$(cat "$EXPORT_DIR/$NAME.tar.gz.heartbeat" 2>/dev/null || true)
        if [ "$CURRENT_HEARTBEAT" != "$HEARTBEAT" ]; then MISSING_HEARTBEATS=0; else MISSING_HEARTBEATS=$((MISSING_HEARTBEATS + 1)); fi
        HEARTBEAT=$CURRENT_HEARTBEAT
        if [ "$MISSING_HEARTBEATS" -ge 20 ]; then REASON=notification_lost; break; fi
        sample "$INDEX" >> "$WORK/samples.txt"
        if [ "$((INDEX % 5))" -eq 0 ]; then
            report recording
            FREE=$(df -Pk "$BASE" | awk 'END {print $4}')
            USED=$(du -sk "$WORK" | awk '{print $1}')
            if [ "$FREE" -lt 524288 ] || [ "$USED" -ge 131072 ]; then REASON=storage_limit; break; fi
        fi
        INDEX=$((INDEX + 1))
        sleep 1
    done
}
case "${1:-status}" in
    start)
        [ "$#" = 3 ] || die '日志组件参数缺失'
        exporter "$2" "$3"
        exec 8>"$BASE/capture.lock"
        "$BB" flock -n 8 || die '性能采样仍在运行或保存'
        PREVIOUS=$(cat "$BASE/active" 2>/dev/null || true)
        if valid_name "$PREVIOUS" && [ "$(value performance_state)" = saved ]; then
            for SUFFIX in request choice stop result progress heartbeat lock; do rm -f "$EXPORT_DIR/$PREVIOUS.tar.gz.$SUFFIX"; done
        fi
        WORK=$("$BB" mktemp -d "$BASE/bkk-performance-$(date '+%Y%m%d-%H%M%S')-XXXXXX")
        NAME=${WORK##*/}
        printf '%s\n' "$NAME" > "$BASE/active"
        printf '%s|%s\n' "$EXPORT_DATA" "$EXPORT_UID" > "$BASE/.exporter.tmp"
        mv -f "$BASE/.exporter.tmp" "$BASE/exporter"
        publish request pending
        report choosing
        "$BB" setsid "$BB" sh "$0" worker "$NAME" "$EXPORT_DATA" "$EXPORT_UID" </dev/null > "$WORK/worker.log" 2>&1 &
        ATTEMPT=0
        while [ ! -f "$WORK/ready" ] && [ "$ATTEMPT" -lt 50 ]; do sleep 0.1; ATTEMPT=$((ATTEMPT + 1)); done
        [ -f "$WORK/ready" ] || { publish stop cancel; report error; die '采样进程未启动'; }
        "$BB" timeout -s KILL 10 am start --user 0 -n org.bkkernel.logexport/.PerformanceActivity \
            -f 0x10000000 --es operation choose --es name "$NAME.tar.gz" >/dev/null 2>&1 || {
                publish stop cancel; die '无法打开性能日志保存位置'; }
        echo '请选择保存位置，确认后开始采样'
        ;;
    worker) shift; worker "$@" ;;
    stop|cancel)
        NAME=$(cat "$BASE/active" 2>/dev/null || true)
        valid_name "$NAME" || exit 0
        [ -r "$BASE/exporter" ] || exit 0
        IFS='|' read -r DATA UID_VALUE < "$BASE/exporter"
        exporter "$DATA" "$UID_VALUE"
        if [ "$1" = stop ]; then
            case "$(value performance_state)" in
                error|ready|interrupted)
                    ATTEMPT=0
                    while running && [ "$ATTEMPT" -lt 50 ]; do sleep 0.1; ATTEMPT=$((ATTEMPT + 1)); done ;;
            esac
        fi
        if running; then
            publish stop "$1"
            echo '正在停止采样并保存'
            if [ "$1" = cancel ]; then
                ATTEMPT=0
                while running && [ "$ATTEMPT" -lt 600 ]; do sleep 0.2; ATTEMPT=$((ATTEMPT + 1)); done
                ! running || die '采样尚未停止，请稍后重试'
            fi
        elif [ "$1" = stop ] && [ "$(value performance_state)" != saved ] && \
            { [ -f "$BASE/$NAME.tar.gz" ] || [ -f "$BASE/$NAME/metadata.txt" ]; }; then
            exec 8>"$BASE/capture.lock"
            "$BB" flock -n 8 || die '性能日志仍在保存'
            rm -f "$EXPORT_DIR/$NAME.tar.gz.stop"
            mkdir -p "$BASE/$NAME"
            RETRY_MODE=save
            [ -f "$BASE/$NAME.tar.gz" ] || RETRY_MODE=recover
            "$BB" setsid "$BB" sh "$0" worker "$NAME" "$EXPORT_DATA" "$EXPORT_UID" "$RETRY_MODE" </dev/null > "$BASE/$NAME/worker.log" 2>&1 &
            echo '正在重试保存性能日志'
        fi
        ;;
    status)
        if [ -f "$BASE/autostart-mode" ]; then
            (exec 8>"$BASE/capture.lock"; "$BB" flock -n 8 || exit 0; restore_exporter)
        fi
        if [ ! -f "$BASE/status" ]; then echo performance_state=idle; echo performance_retry=0; exit 0; fi
        PHASE=$(value performance_state)
        if ! running; then
            case "$PHASE" in choosing|recording|stopping|saving) PHASE=interrupted ;; esac
        fi
        sed '/^performance_state=/d; /^performance_retry=/d' "$BASE/status"
        NAME=$(cat "$BASE/active" 2>/dev/null || true)
        if valid_name "$NAME" && [ -r "$BASE/exporter" ]; then
            IFS='|' read -r DATA UID_VALUE < "$BASE/exporter"
            case "$DATA" in
                /data/user/0/org.bkkernel.logexport|/data/data/org.bkkernel.logexport)
                    if [ "$(cat "$DATA/files/$NAME.tar.gz.result" 2>/dev/null)" = saved ]; then PHASE=saved; fi ;;
            esac
        fi
        RETRY=0
        if valid_name "$NAME" && [ "$PHASE" != saved ] && \
            { [ -f "$BASE/$NAME.tar.gz" ] || [ -f "$BASE/$NAME/metadata.txt" ]; }; then RETRY=1; fi
        printf 'performance_retry=%s\n' "$RETRY"
        printf 'performance_state=%s\n' "$PHASE"
        ;;
    *) die '用法：bkctl perf start|stop|status' ;;
esac
