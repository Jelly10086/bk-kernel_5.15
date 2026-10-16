#!/usr/bin/env bash
set -Eeuo pipefail

source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
KERNEL_DIR=$(cd -- "$KR_BUILD_DIR/.." && pwd)
case "${1:-}" in
    -h|--help)
        printf '用法：bash kr-build/build.sh [--check]\n环境变量：CLANG_DIR、OUT_DIR、DIST_DIR、JOBS、CCACHE\n'
        exit 0 ;;
    ''|--check) ;;
    *) die "未知参数：$1" ;;
esac
[[ $# -le 1 ]] || die "参数过多"
started_at=$SECONDS
phase=依赖检查
build_succeeded=0
log_ready=0
result_ready=0
failure_line=
run_id=$(date -u +%Y%m%dT%H%M%S)-$$
write_result() {
    printf '%s %s %s\n' "$run_id" "$1" "$2" > "$OUT_DIR/.kr-build-result.tmp"
    mv -f -- "$OUT_DIR/.kr-build-result.tmp" "$OUT_DIR/.kr-build-result"
}
finish_build() {
    local rc=$? elapsed state hint=
    trap - EXIT ERR INT TERM
    set +e
    elapsed=$(( SECONDS - started_at ))
    if (( log_ready )); then
        hint=" - 日志：$OUT_DIR/build.log"
    fi
    if (( rc == 0 && build_succeeded == 1 )); then
        state=SUCCESS
        printf '[OK] 构建完成 - 用时 %s秒%s\n' "$elapsed" "$hint"
    elif (( rc == 130 || rc == 143 )); then
        state=INTERRUPTED
        printf '[WARN] 构建已中断 - 阶段：%s - 退出码：%s - 用时 %s秒%s\n' "$phase" "$rc" "$elapsed" "$hint" >&2
    else
        state=FAILED
        (( rc != 0 )) || rc=1
        printf '[FAILED] 构建失败 - 阶段：%s - 退出码：%s - 行：%s - 用时 %s秒%s\n' \
            "$phase" "$rc" "${failure_line:-未知}" "$elapsed" "$hint" >&2
    fi
    if (( result_ready )); then
        write_result "$state" "$rc"
    fi
    if (( log_ready )); then
        exec 1>&3 2>&4
        exec 3>&- 4>&-
        wait "$tee_pid"
    fi
    exit "$rc"
}
build_failed() {
    local rc=$?
    failure_line=${BASH_LINENO[0]}
    exit "$rc"
}
trap finish_build EXIT
trap build_failed ERR
trap 'exit 130' INT
trap 'exit 143' TERM
PACKAGE_VERSION=$(read_package_version)
kernel_release="bkk-$PACKAGE_VERSION"
(( ${#kernel_release} <= 64 )) || die "内核版本超过 64 字符，请缩短 kr-build/version 中的名称"
require_commands make gcc g++ bc bison flex perl python3 openssl pkg-config pahole \
    git gzip cpio rsync flock sha256sum
check_clang
kernel_cc=clang
if [[ -n "${CCACHE:-}" ]]; then
    require_commands "$CCACHE"
    kernel_cc="$CCACHE clang"
fi
[[ -f "$KERNEL_DIR/KernelSU/kernel/Kbuild" ]] || die "先执行 git submodule update --init KernelSU"
[[ $(git -C "$KERNEL_DIR/KernelSU" rev-parse --is-shallow-repository) == false ]] || \
    die "BakaSU 版本号需要完整历史：git -C KernelSU fetch --unshallow"
submodule_head=$(git -C "$KERNEL_DIR/KernelSU" rev-parse HEAD)
submodule_pin=$(git -C "$KERNEL_DIR" ls-files -s KernelSU | awk '{print $2}')
[[ "$submodule_head" == "$submodule_pin" ]] || die "KernelSU 与仓库固定的提交不符"
pkg-config --exists libelf openssl || die "缺少 libelf-dev 或 libssl-dev"
[[ $(uname -m) == x86_64 ]] || die "此 GoogleClang 包需要 x86_64 Linux"
if [[ "${1:-}" == --check ]]; then
    log "依赖与工具链检查通过 - $CLANG_PACKAGE"
    trap - EXIT ERR INT TERM
    exit 0
fi

CLANG_DIR=$(realpath -- "$CLANG_DIR")
OUT_DIR=$(realpath -m -- "$OUT_DIR")
DIST_DIR=$(realpath -m -- "$DIST_DIR")
[[ "$OUT_DIR" != "$KERNEL_DIR" && "$OUT_DIR" != / ]] || die "OUT_DIR 不能是源码根目录或 /"
[[ "$KERNEL_DIR$CLANG_DIR$OUT_DIR" != *[[:space:]]* ]] || die "Kbuild 路径不能包含空白字符"
if [[ -z "${JOBS:-}" ]]; then
    available_kib=$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)
    JOBS=$(( (available_kib - 2097152) / 1572864 ))
    (( JOBS > 0 )) || JOBS=1
    cpus=$(nproc)
    (( JOBS <= cpus )) || JOBS=$cpus
fi
[[ "$JOBS" =~ ^[1-9][0-9]*$ ]] || die "JOBS 必须是正整数"
mkdir -p -- "$OUT_DIR"
exec 9>"$OUT_DIR/.kr-build.lock"
flock -n 9 || die "另一个构建正在使用 $OUT_DIR"
result_ready=1
write_result RUNNING 0
exec 3>&1 4>&2
exec > >(tee -a "$OUT_DIR/build.log") 2>&1
tee_pid=$!
log_ready=1

fingerprint="$KERNEL_DIR|$CLANG_DIR|$CLANG_COMMIT|arm64"
if [[ -f "$OUT_DIR/.kr-build-toolchain" ]]; then
    [[ $(cat "$OUT_DIR/.kr-build-toolchain") == "$fingerprint" ]] || \
        die "输出目录使用了另一工具链，请指定新的 OUT_DIR"
elif [[ -f "$OUT_DIR/.config" ]]; then
    die "现有输出目录的工具链来源未知，请指定新的 OUT_DIR"
fi
printf '%s\n' "$fingerprint" > "$OUT_DIR/.kr-build-toolchain"
export PATH="$CLANG_DIR/bin:$PATH"
export KBUILD_BUILD_USER=${KBUILD_BUILD_USER:-Jelly10086}
export KBUILD_BUILD_HOST=${KBUILD_BUILD_HOST:-kr-build}
export KBUILD_BUILD_TIMESTAMP=${KBUILD_BUILD_TIMESTAMP:-$(git -C "$KERNEL_DIR" show -s --format=%cD HEAD)}
export KBUILD_BUILD_VERSION=${KBUILD_BUILD_VERSION:-1}
BRANCH=$(sed -n 's/^BRANCH=//p' "$KERNEL_DIR/build.config.constants")
KMI_GENERATION=$(sed -n 's/^KMI_GENERATION=//p' "$KERNEL_DIR/build.config.common")
make_kernel() {
    make -C "$KERNEL_DIR" O="$OUT_DIR" ARCH=arm64 LLVM=1 LLVM_IAS=1 \
        CC="$kernel_cc" HOSTCC=gcc HOSTCXX=g++ BRANCH="$BRANCH" KMI_GENERATION="$KMI_GENERATION" \
        KERNELRELEASE="$kernel_release" \
        KCFLAGS=-D__ANDROID_COMMON_KERNEL__ "$@"
}

log "Google Clang $CLANG_RELEASE - $CLANG_PACKAGE - JOBS=$JOBS"
log "内核名称 - bkk-$PACKAGE_VERSION"
log "编译缓存 - ${CCACHE:-disabled}"
phase=生成配置
make_kernel gki_defconfig
for symbol in LRU_GEN LRU_GEN_ENABLED RCU_EXPERT RCU_NOCB_CPU RCU_LAZY \
    KSU KSU_TRACEPOINT_HOOK REKERNEL_X ANDROID_VENDOR_HOOKS KPROBES KALLSYMS_ALL \
    ANDROID_BINDER_IPC CGROUP_FREEZER NETFILTER INET SYSVIPC POSIX_MQUEUE \
    USER_NS PID_NS IPC_NS NET_NS LTO_CLANG_FULL CFI_CLANG MODVERSIONS DEBUG_INFO_BTF; do
    grep -qx "CONFIG_$symbol=y" "$OUT_DIR/.config" || die "必要配置未启用：CONFIG_$symbol"
done
if grep -qx 'CONFIG_RCU_LAZY_DEFAULT_OFF=y' "$OUT_DIR/.config"; then
    die "RCU Lazy 被默认关闭"
fi
grep -E '^CONFIG_CMDLINE=".*rcu_nocbs=all([[:space:]"]|$)' "$OUT_DIR/.config" >/dev/null || \
    die "缺少 rcu_nocbs=all"
make_kernel -s include/config/kernel.release
[[ $(cat "$OUT_DIR/include/config/kernel.release") == "$kernel_release" ]] || \
    die "生成的内核版本与 kr-build/version 不符"
log "内核版本 - $kernel_release"
log "编译 Image.gz 与 GKI 模块"
phase=编译和链接
make_kernel -j"$JOBS" Image.gz modules
phase=校验产物
gzip -t "$OUT_DIR/arch/arm64/boot/Image.gz"
"$CLANG_DIR/bin/llvm-readelf" -S "$OUT_DIR/vmlinux" > "$OUT_DIR/vmlinux-sections.txt"
grep -q '\.BTF[[:space:]]' "$OUT_DIR/vmlinux-sections.txt" || die "vmlinux 缺少 BTF"
"$CLANG_DIR/bin/llvm-nm" --defined-only "$OUT_DIR/vmlinux" > "$OUT_DIR/vmlinux-symbols.txt"
grep -E '[[:space:]]rkx_init([.][^[:space:]]*)?$' "$OUT_DIR/vmlinux-symbols.txt" >/dev/null || \
    die "vmlinux 中没有内建 ReKernel-X 初始化函数"
{
    printf 'KERNEL_COMMIT=%s\n' "$(git -C "$KERNEL_DIR" rev-parse HEAD)"
    printf 'KERNELSU_COMMIT=%s\n' "$submodule_head"
    printf 'CLANG_COMMIT=%s\nCLANG_PACKAGE=%s\n' "$CLANG_COMMIT" "$CLANG_PACKAGE"
    printf 'ANYKERNEL_COMMIT=%s\nJOBS=%s\n' "$ANYKERNEL_COMMIT" "$JOBS"
    printf 'KERNEL_RELEASE=%s\n' "$(cat "$OUT_DIR/include/config/kernel.release")"
    printf 'CCACHE=%s\n' "${CCACHE:-disabled}"
    "$CLANG_DIR/bin/clang" --version
    pahole --version
    git -C "$KERNEL_DIR" status --short
} > "$OUT_DIR/build-info.txt"
phase=AK3打包
PACKAGE_VERSION="$PACKAGE_VERSION" OUT_DIR="$OUT_DIR" DIST_DIR="$DIST_DIR" bash "$KR_BUILD_DIR/pack.sh"
build_succeeded=1
