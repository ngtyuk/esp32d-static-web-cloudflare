#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" || "$(uname -m)" != "arm64" ]]; then
  exit 0
fi

ARDUINO_CONFIG_DIR="${ARDUINO_CONFIG_DIR:-work/arduino-cli}"
ESPTOOL_PATH="$(find "$ARDUINO_CONFIG_DIR/packages/esp32/tools/esptool_py" -type f -name esptool 2>/dev/null | head -1 || true)"
CTAGS_PATH="$(find "$ARDUINO_CONFIG_DIR/packages/builtin/tools/ctags" -type f -name ctags 2>/dev/null | head -1 || true)"
if [[ -z "$ESPTOOL_PATH" || -z "$CTAGS_PATH" ]]; then
  echo "Apple Silicon用の補助ツールを置き換えられませんでした。ARDUINO_CONFIG_DIRを確認してください。"
  exit 1
fi

tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT
curl -fsSL https://github.com/espressif/esptool/releases/download/v5.3.1/esptool-v5.3.1-macos-arm64.tar.gz -o "$tmp_dir/esptool.tar.gz"
tar -xzf "$tmp_dir/esptool.tar.gz" -C "$tmp_dir"
cp "$tmp_dir/esptool-macos-arm64/esptool" "$ESPTOOL_PATH"
chmod +x "$ESPTOOL_PATH"
cp tools/ctags-stub "$CTAGS_PATH"
chmod +x "$CTAGS_PATH"
echo "Apple Silicon用のesptoolとctagsを準備しました。"
