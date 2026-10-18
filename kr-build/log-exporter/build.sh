#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
: "${ANDROID_JAR:?指定Android SDK的android.jar}"
: "${KEY:?指定APK签名私钥pk8}"
: "${CERT:?指定APK签名证书pem}"
for TOOL in javac jar d8 aapt2 apksigner zip; do
    command -v "$TOOL" >/dev/null || { echo "[ERROR] 缺少$TOOL" >&2; exit 1; }
done
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/classes" "$WORK/dex"
javac -source 8 -target 8 -Xlint:-options -cp "$ANDROID_JAR" \
    -d "$WORK/classes" "$SCRIPT_DIR"/src/org/bkkernel/logexport/*.java
jar cf "$WORK/classes.jar" -C "$WORK/classes" .
d8 --lib "$ANDROID_JAR" --min-api 26 --output "$WORK/dex" "$WORK/classes.jar"
aapt2 link -o "$WORK/unsigned.apk" -I "$ANDROID_JAR" \
    --manifest "$SCRIPT_DIR/AndroidManifest.xml" --version-code 5 --version-name 5 \
    --min-sdk-version 26 --target-sdk-version 36
zip -q -j "$WORK/unsigned.apk" "$WORK/dex/classes.dex"
APK=$SCRIPT_DIR/../modules/bkk-control/bin/bkk-log-exporter.apk
apksigner sign --v4-signing-enabled false --key "$KEY" --cert "$CERT" --out "$APK" "$WORK/unsigned.apk"
apksigner verify --verbose "$APK"
