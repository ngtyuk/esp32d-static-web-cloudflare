param([string]$ESP32IP = $env:ESP32D_IP)

$ErrorActionPreference = "Stop"
$Cloudflared = if ($env:CLOUDFLARED) { $env:CLOUDFLARED } else { "cloudflared.exe" }

if (-not $ESP32IP) {
  throw "ESP32IPを指定してください。例: .\scripts\tunnel.ps1 -ESP32IP 192.168.1.42"
}

Write-Host "ESP-32DをCloudflare Quick Tunnelで公開します。終了はCtrl-Cです。"
& $Cloudflared tunnel --url "http://$ESP32IP"
