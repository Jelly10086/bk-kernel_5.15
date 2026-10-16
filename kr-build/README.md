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

更改显示名称和安装包名：

```bash
bash kr-build/set-version.sh
```

提示 `更改内核版本:` 时输入 `CaT_13-5.15.xxx_b4`，不带 `bkk-` 前缀。`xxx` 自动跟随真实内核版本，例如生成 `bkk-CaT_13-5.15.216_b4-AK3.zip`

`CLANG_DIR`、`OUT_DIR`、`DIST_DIR`、`JOBS` 可通过环境变量指定。默认输出到 `kr-build/out`，保留增量产物；更换工具链时使用新的 `OUT_DIR`。

```bash
JOBS=8 bash kr-build/build.sh
OUT_DIR=/path/to/existing/out bash kr-build/pack.sh
```
