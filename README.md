# ESP-32D static web

ESP-32Dのフラッシュに静的サイトを埋め込み、ESP32自身からWi-Fi経由で配信するための最小構成です。サイトのHTML/CSS/JavaScriptは `site/` にあり、書き込み前に自動でArduino用ヘッダーへ変換されます。

## 1. 初回セットアップ

Arduino CLIをインストールしてから、ESP32コアを準備します。

```bash
scripts/setup.sh
```

このプロジェクトで想定するボードは `esp32:esp32:esp32`（ESP32 Dev Module）です。別の型番なら `ESP32D_FQBN` で上書きできます。
Apple SiliconのMacでは、セットアップ時に書き込みツールの互換性も自動調整します。

## 2. ESP-32Dへ書き込む

```bash
scripts/flash.sh
```

接続ポートが異なる場合:

```bash
ESP32D_PORT=/dev/cu.wchusbserial110 scripts/flash.sh
```

初回起動時は `ESP32D-Setup` というWi-Fiアクセスポイントが起動します。パスワードは `esp32d-setup`。そのWi-Fiに接続し、`http://192.168.4.1` を開いて家庭・オフィスのWi-Fi情報を保存してください。再起動後、シリアルログに表示されるIPアドレスでサイトへアクセスできます。

## 3. サイトを更新する

`site/index.html`、`site/styles.css`、`site/app.js` を編集して、もう一度書き込みます。

```bash
scripts/flash.sh
```

## 4. 「元気？」に答える

サイトの「元気？」ボタンを押すと、D2（GPIO2）のLEDが点滅し続けます。ESP-32DのD13に接続したボタンを押すと点滅が止まり、ブラウザに「元気！」と表示されます。

D13のボタンは、押したときにGNDへ接続されるように配線してください。入力は内部プルアップを使用しています。

質問日時と回答日時はブラウザ側に記録して表示します。ESP32からブラウザへの通知にはSSEを使用するため、Cloudflare Tunnelは名前付きTunnelを使用してください。Quick Tunnel（`trycloudflare.com`）はSSEに対応していません。

サイトを更新した場合は、サイトの生成とファームウェアの書き込みをもう一度実行してください。

## 5. Cloudflare経由で外部公開する

ESP32とCloudflare Tunnelを実行するPCが同じLANに接続された状態で、ESP32のIPアドレスを指定します。

```bash
ESP32D_IP=192.168.1.42 scripts/tunnel.sh
```

表示された `trycloudflare.com` のURLからアクセスできます。これは開発・検証用のQuick Tunnelです。固定ドメインで運用する場合は、Cloudflare Dashboardで名前付きTunnelを作り、公開ホスト名のサービス先を `http://ESP32のIPアドレス` に設定してください。

## 注意

- ESP32とCloudflare Tunnelを実行するPCは、同じLAN上にいる必要があります。
- ESP32のIPが変わらないように、ルーター側でDHCP予約を設定するのがおすすめです。
- `ESP32D-Setup` の初期パスワードはサンプル値なので、運用時はファームウェア内の値を変更してください。

## 使用したアイテム

- [ESP-32D開発ボード（AliExpress）](https://ja.aliexpress.com/item/1005008503831020.html)
