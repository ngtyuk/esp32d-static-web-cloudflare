#!/usr/bin/env bash
set -euo pipefail

if ! command -v cloudflared >/dev/null 2>&1; then
  echo "cloudflared が必要です。macOSなら brew install cloudflared で導入できます。"
  exit 1
fi
if [[ -z "${ESP32D_IP:-}" ]]; then
  echo "ESP32D_IPを指定してください。例: ESP32D_IP=192.168.1.42 scripts/tunnel.sh"
  exit 1
fi

echo "ESP-32DをCloudflare Quick Tunnelで公開します。終了はCtrl-Cです。"
cloudflared tunnel --url "http://${ESP32D_IP}"
