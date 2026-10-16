# sheng 的构建

工具链：Google Clang 23.0.1 / `r614150`。工具链与 AnyKernel3 的来源提交固定在 `pins.env`

debian 依赖：

```bash
sudo apt-get update
sudo apt-get install -y bc bison build-essential ca-certificates cpio curl dwarves flex git libelf-dev libssl-dev pkg-config python3 rsync zlib1g-dev
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

GitHub Actions 的 `build bkk` 在启动时可填写 `version_name`，调用同一脚本改版本；留空使用仓库版本

`CLANG_DIR`、`OUT_DIR`、`DIST_DIR`、`JOBS` 可通过环境变量指定。默认输出到 `kr-build/out`，保留增量产物；更换工具链时使用新的 `OUT_DIR`。

```bash
JOBS=8 bash kr-build/build.sh
OUT_DIR=/path/to/existing/out bash kr-build/pack.sh
```
