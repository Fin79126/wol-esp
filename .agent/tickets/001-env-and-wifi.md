# TICKET-001: PlatformIO環境構築とWi-Fi接続管理機能の実装

## 概要
ESP32-WROVER-E用の `platformio.ini` を定義し、Wi-Fiへの接続および切断時の自動再接続を管理するモジュールを実装する。

## 対象ブランチ / ディレクトリ
- ワークツリー: `.worktrees/feat-env-and-wifi`
- ブランチ: `feat/env-and-wifi`

## 実装要件
1. **`platformio.ini` の作成**:
   - プラットフォーム: `espressif32`
   - ボード: `esp32dev` または `esp-wrover-kit` (PSRAM有効化設定含む)
   - フレームワーク: `arduino`
   - 必要なライブラリ依存関係の定義 (`Links2004/WebSockets`, `madhephaestus/ESP32Servo`, `bblanchon/ArduinoJson` 等)
   - モニタ速度: `115200`
2. **設定ファイル `src/config.h` の作成**:
   - Wi-Fi SSID, パスワード、WebSocket URL, サーボ制御ピン、動作パラメータ等の定数定義
3. **Wi-Fiマネージャ `src/wifi_manager.h` / `src/wifi_manager.cpp` の実装**:
   - `WiFiManager` クラス
   - 初期化メソッド `init(const char* ssid, const char* password)`
   - ループ処理 `update()` での接続監視および自動再接続
   - 接続状態取得 `isConnected()`
   - IPアドレス取得 `getIPAddress()`
4. **ログ記録**:
   - `docs/agent_logs/env-and-wifi.md` に実装方針と進捗を記録。
