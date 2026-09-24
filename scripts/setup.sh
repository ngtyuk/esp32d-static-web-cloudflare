#!/usr/bin/env bash
set -euo pipefail

ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
ARDUINO_CONFIG_DIR="${ARDUINO_CONFIG_DIR:-}"
ESP32_CORE_VERSION="${ESP32_CORE_VERSION:-2.0.17}"
if [[ "$ARDUINO_CLI" == "arduino-cli" && ! -x "$(command -v arduino-cli 2>/dev/null || true)" && -x "work/tools/arduino-cli" ]]; then
  ARDUINO_CLI="work/tools/arduino-cli"
  ARDUINO_CONFIG_DIR="${ARDUINO_CONFIG_DIR:-work/arduino-cli}"
fi

if ! command -v "$ARDUINO_CLI" >/dev/null 2>&1 && [[ ! -x "$ARDUINO_CLI" ]]; then
  echo "arduino-cli が見つかりません。公式手順でインストールしてください: https://arduino.github.io/arduino-cli/latest/installation/"
  exit 1
fi

CLI_ARGS=()
if [[ -n "$ARDUINO_CONFIG_DIR" ]]; then CLI_ARGS+=(--config-dir "$ARDUINO_CONFIG_DIR"); fi

"$ARDUINO_CLI" "${CLI_ARGS[@]}" core update-index
if ! "$ARDUINO_CLI" "${CLI_ARGS[@]}" core list | awk 'NR > 1 {print $1, $2}' | grep -qx "esp32:esp32 $ESP32_CORE_VERSION"; then
  "$ARDUINO_CLI" "${CLI_ARGS[@]}" core install "esp32:esp32@$ESP32_CORE_VERSION"
fi

if [[ -n "$ARDUINO_CONFIG_DIR" ]]; then
  ARDUINO_CONFIG_DIR="$ARDUINO_CONFIG_DIR" scripts/patch-apple-silicon-tools.sh
fi

node scripts/generate-site-header.mjs
echo "ESP32コアとサイト生成の準備が完了しました。"
