$ErrorActionPreference = "Stop"

$ArduinoCli = if ($env:ARDUINO_CLI) { $env:ARDUINO_CLI } else { "arduino-cli.exe" }
$CoreVersion = if ($env:ESP32_CORE_VERSION) { $env:ESP32_CORE_VERSION } else { "2.0.17" }
$Esp32Index = "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"

& $ArduinoCli core update-index --additional-urls $Esp32Index
if ($LASTEXITCODE -ne 0) { throw "Arduino CLIのインデックス更新に失敗しました。" }

$Installed = & $ArduinoCli core list
if (-not ($Installed -match "esp32:esp32\s+$CoreVersion")) {
  & $ArduinoCli core install "esp32:esp32@$CoreVersion"
  if ($LASTEXITCODE -ne 0) { throw "ESP32コアのインストールに失敗しました。" }
}

if (Get-Command node -ErrorAction SilentlyContinue) {
  node scripts/generate-site-header.mjs
} else {
  Write-Warning "Node.jsが見つからないため、既存のsite_html.hを使用します。site/を変更した場合はNode.jsをインストールしてください。"
}

Write-Host "ESP32コアとサイト生成の準備が完了しました。"
