#!/usr/bin/env bash
set -euo pipefail

ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
ARDUINO_CONFIG_DIR="${ARDUINO_CONFIG_DIR:-}"
if [[ "$ARDUINO_CLI" == "arduino-cli" && ! -x "$(command -v arduino-cli 2>/dev/null || true)" && -x "work/tools/arduino-cli" ]]; then
  ARDUINO_CLI="work/tools/arduino-cli"
  ARDUINO_CONFIG_DIR="${ARDUINO_CONFIG_DIR:-work/arduino-cli}"
fi
CLI_ARGS=()
if [[ -n "$ARDUINO_CONFIG_DIR" ]]; then CLI_ARGS+=(--config-dir "$ARDUINO_CONFIG_DIR"); fi

PORT="${ESP32D_PORT:-/dev/cu.wchusbserial110}"
FQBN="${ESP32D_FQBN:-esp32:esp32:esp32}"
BUILD_DIR="$(pwd)/work/arduino-build"

if ! command -v "$ARDUINO_CLI" >/dev/null 2>&1 && [[ ! -x "$ARDUINO_CLI" ]]; then
  echo "arduino-cli が必要です。先に scripts/setup.sh を実行してください。"
  exit 1
fi
if [[ ! -e "$PORT" ]]; then
  echo "シリアルポートが見つかりません: $PORT"
  echo "ESP32を接続し、ESP32D_PORTで正しいポートを指定してください。"
  exit 1
fi

node scripts/generate-site-header.mjs
"$ARDUINO_CLI" "${CLI_ARGS[@]}" compile --fqbn "$FQBN" --build-path "$BUILD_DIR" firmware/esp32d_static_site
"$ARDUINO_CLI" "${CLI_ARGS[@]}" upload --fqbn "$FQBN" --port "$PORT" --input-dir "$BUILD_DIR" firmware/esp32d_static_site
echo "書き込み完了。初回は ESP32D-Setup に接続し、http://192.168.4.1 でWi-Fiを設定してください。"
