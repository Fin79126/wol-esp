# Agent-Env-and-wifi 作業ログ

## タスク概要
- チケット: `001-env-and-wifi.md` (TICKET-001)
- 目的: ESP32-WROVER-E用 PlatformIO環境定義 (`platformio.ini`)、全体設定定数定義 (`src/config.h`)、Wi-Fi接続・非同期再接続マネージャ (`src/wifi_manager.h`, `src/wifi_manager.cpp`) の実装

## 実装内容と設計詳細

### 1. `platformio.ini`
- ボード定義: `board = esp-wrover-kit` (ESP32-WROVER-E用 PSRAM / Flash構成対応)
- フレームワーク: `arduino`
- ビルドフラグ:
  - `-DBOARD_HAS_PSRAM`: PSRAMの有効化
  - `-mfix-esp32-psram-cache-issue`: ESP32 PSRAMのキャッシュバグ回避
  - `-DCORE_DEBUG_LEVEL=3`: デバッグログレベル設定
- 依存ライブラリ (`lib_deps`):
  - `Links2004/WebSockets @ ^2.4.1`
  - `madhephaestus/ESP32Servo @ ^3.0.5`
  - `bblanchon/ArduinoJson @ ^7.0.4`
- シリアルモニタ速度: `monitor_speed = 115200`

### 2. `src/config.h`
- Wi-Fi設定:
  - `WIFI_SSID`: 初期値 `"YOUR_WIFI_SSID"`
  - `WIFI_PASSWORD`: 初期値 `"YOUR_WIFI_PASSWORD"`
  - `WIFI_CONNECT_TIMEOUT_MS`: タイムアウト時間 (15000ms)
  - `WIFI_RECONNECT_INTERVAL_MS`: 再接続インターバル (5000ms)
- WebSocket設定:
  - `WS_SERVER_HOST`: 初期値 `"192.168.1.100"`
  - `WS_SERVER_PORT`: 初期値 `8080`
  - `WS_SERVER_PATH`: 初期値 `"/ws"`
  - `WS_SERVER_PROTOCOL`: 初期値 `""`
  - ハートビート・再接続定数設定
- サーボモータ設定:
  - `SERVO_PIN`: `18` (GPIO 18)
  - `SERVO_STANDBY_ANGLE`: `0` 度
  - `SERVO_TRIGGER_ANGLE`: `90` 度
  - `SERVO_HOLD_TIME_MS`: `500` ms
- システム設定:
  - `SERIAL_BAUD_RATE`: `115200`

### 3. `src/wifi_manager.h` & `src/wifi_manager.cpp`
- `WiFiManager` クラス:
  - 初期化: `init(const char* ssid, const char* password)`
  - ループ処理: `update()` による非ブロッキング状態遷移（接続試行・タイムアウト処理・切断検出・自動再接続）
  - 状態確認: `isConnected()`
  - IP取得: `getIPAddress()`, `getIPAddressString()`
  - コールバック登録: `onConnected()`, `onDisconnected()`
  - ユーティリティ: `getSSID()`, `getRSSI()`, `getMacAddress()`, `disconnect()`

## 動作確認・テスト手順
1. `src/config.h` の `WIFI_SSID`, `WIFI_PASSWORD` をお使いのWi-Fiアクセスポイントの情報に変更する。
2. アプリケーション (`src/main.cpp` 等) から `WiFiManager wifi;` を生成し、`setup()` で `wifi.init()` を呼び出し、`loop()` で `wifi.update()` を実行する。
3. ESP32をシリアルモニタ（ボーレート115200）に接続し、起動時にSSID接続ログおよびIPアドレスが出力されることを確認する。
4. Wi-Fiルータの電源OFFや電波遮断時に自動切断ログが出力され、復帰後に自動再接続されることを確認する。
