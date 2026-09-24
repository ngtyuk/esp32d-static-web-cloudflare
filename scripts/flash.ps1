param(
  [string]$Port = $env:ESP32D_PORT,
  [string]$Fqbn = $(if ($env:ESP32D_FQBN) { $env:ESP32D_FQBN } else { "esp32:esp32:esp32" })
)

$ErrorActionPreference = "Stop"
$ArduinoCli = if ($env:ARDUINO_CLI) { $env:ARDUINO_CLI } else { "arduino-cli.exe" }

if (-not $Port) {
  $Port = Read-Host "ESP32のCOMポートを入力してください（例: COM3）"
}
if (-not $Port) { throw "COMポートが指定されていません。" }

if (Get-Command node -ErrorAction SilentlyContinue) {
  node scripts/generate-site-header.mjs
} elseif (-not (Test-Path "firmware/esp32d_static_site/site_html.h")) {
  throw "site_html.hがありません。Node.jsをインストールしてサイトを生成してください。"
}

& $ArduinoCli compile --fqbn $Fqbn firmware/esp32d_static_site
if ($LASTEXITCODE -ne 0) { throw "コンパイルに失敗しました。" }

& $ArduinoCli upload --fqbn $Fqbn --port $Port firmware/esp32d_static_site
if ($LASTEXITCODE -ne 0) { throw "ESP32への書き込みに失敗しました。" }

Write-Host "書き込み完了。初回はESP32D-Setupへ接続し、http://192.168.4.1 を開いてください。"
