# sheng 的构建

工具链：Google Clang 23.0.1 / `r614150`。工具链与 AnyKernel3 的来源提交固定在 `pins.env`

debian 依赖：

```bash
sudo apt-get update
sudo apt-get install -y bc bison build-essential ca-certificates cpio curl dwarves flex git kmod libelf-dev libssl-dev pkg-config python3 rsync zlib1g-dev
```

在内核仓库根目录执行：

```bash
git submodule update --init KernelSU
bash kr-build/fetch-clang.sh
bash kr-build/build.sh --check
bash kr-build/build.sh
```

默认使用 `~/project/GoogleClang` 和 `gki_defconfig`

更改内核版本和安装包名：

```bash
bash kr-build/set-version.sh
```

提示 `更改内核版本:` 时输入 `5.15.xxx_bkk-CaT_b4`，后缀也可用 `r1`、`P2` 等。`xxx` 自动跟随真实内核版本；AK3 仍生成 `bkk-CaT_13-5.15.216_b4-AK3-261016.zip`，日期按北京时间使用 `YYMMDD`

构建时自动追加 `-c<Git短提交号>`，例如 `5.15.216_bkk-CaT_b4-c1302066c856f`；有未提交改动时追加 `-dirty`。AK3 包名不加提交号

GitHub Actions 的 `build bkk` 在启动时可填写 `version_name`，只覆盖本次构建的版本；留空使用仓库版本。成功后自动发布到 Releases，AK3 可直接下载刷入，文件名与本地一致

`CLANG_DIR`、`OUT_DIR`、`DIST_DIR`、`JOBS` 可通过环境变量指定。默认输出到 `kr-build/out`，保留增量产物；更换工具链时使用新的 `OUT_DIR`。

```bash
JOBS=8 bash kr-build/build.sh
OUT_DIR=/path/to/existing/out bash kr-build/pack.sh
OUT_DIR=/path/to/existing/out bash kr-build/build-module.sh
```

本地和CI单独输出`bkk-control-r1-261017.zip`、灰烬模式`bkk-ash-mode-r1-261017.ko`及校验文件，编号和日期跟随本次内核，Release同时发布

在BakaSU安装控制模块，启动配套内核后再启用。灰烬模式默认关闭，WebUI可选关闭、开启、自动。只调整SM8550性能簇下限，保留GPU和120Hz策略

CPU超过87°C或电池达到46°C撤销提升，回落至80°C和43°C允许恢复。熄屏、挂起或调用超时也撤销

WebUI沿用k2的Compose/Miuix布局。修改`kr-build/webui`后，用JDK 21和Gradle 9执行`bash kr-build/build-webui.sh`；日常内核构建直接使用已生成的UI
