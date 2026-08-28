# Agent Log: WebSocket Client Module

## 1. 作業概要
- **チケット**: TICKET-003: WebSocketクライアント通信モジュールの実装
- **担当エージェント**: Agent-Websocket-client
- **作業ディレクトリ**: `/workspaces/wol-esp/.worktrees/feat-websocket-client`
- **対象ブランチ**: `feat/websocket-client`

## 2. 思考メモ・設計方針
### 要件
ESP32から常時WebSocket接続を確立し、以下をサポートするモジュール `WebSocketManager` を設計・実装した。
1. **ライブラリ**: `Links2004/WebSockets` (`WebSocketsClient.h`)
2. **主要機能**:
   - 初期化: `init(const char* host, uint16_t port, const char* path = "/", const char* protocol = nullptr)`
   - ループ処理: `update()`（内部で `_client.loop()` を呼び出し）
   - コールバック登録:
     - `onMessage(MessageCallback callback)`: テキスト受信時
     - `onConnected(ConnectedCallback callback)`: サーバー接続成功時
     - `onDisconnected(DisconnectedCallback callback)`: サーバー切断時
     - `onBinary(BinaryCallback callback)`: バイナリ受信時
   - データ送信:
     - `sendText(const String& message)`: テキスト送信
     - `sendBinary(const uint8_t* payload, size_t length)`: バイナリ送信
   - ハートビート・再接続設定:
     - 自動再接続間隔: `setReconnectInterval(5000)` (デフォルト5000ms)
     - Ping/Pong死活監視: `enableHeartbeat(15000, 3000, 2)` (15秒間隔Ping, 3秒Pongタイムアウト, 2回連続タイムアウトで切断・再接続)
   - 状態取得:
     - `isConnected()`: 接続中かどうか

## 3. 作成・変更ファイル一覧
- `src/ws_client.h`: `WebSocketManager` クラスのインターフェース定義
- `src/ws_client.cpp`: `WebSocketManager` の実装
- `docs/agent_logs/websocket-client.md`: 実装ログおよび仕様書

## 4. API使用例
```cpp
#include "ws_client.h"

WebSocketManager wsClient;

void setup() {
    Serial.begin(115200);

    // コールバックの登録
    wsClient.onConnected([]() {
        Serial.println("[APP] WS Connected!");
        wsClient.sendText("{\"event\":\"hello\",\"device\":\"esp32\"}");
    });

    wsClient.onDisconnected([]() {
        Serial.println("[APP] WS Disconnected!");
    });

    wsClient.onMessage([](const String& msg) {
        Serial.printf("[APP] Received message: %s\n", msg.c_str());
        // JSONやテキストコマンドの解析処理
    });

    // 初期化と接続開始
    wsClient.init("192.168.1.10", 8080, "/ws");
}

void loop() {
    wsClient.update();
}
```

## 5. 通信メッセージフォーマット仕様（TICKET-004連携用）
- **サーバー -> ESP32（コマンド受信）**:
  - JSON形式: `{"action": "servo", "angle": 90, "hold_ms": 1000}`
  - プレーンテキスト: `TRIGGER`
- **ESP32 -> サーバー（ステータス応答・イベント通知）**:
  - 接続通知: `{"event": "connected", "device": "esp32-wrover-e", "ip": "192.168.1.xxx"}`
  - 実行完了通知: `{"event": "servo_executed", "status": "ok", "angle": 90}`
  - ハートビート/Ping: ライブラリ標準のWebSocket Ping/Pongフレーム

## 6. テスト・確認手順
1. **WebSocketサーバーの起動（例: Node.js / Python wsサーバー）**:
   ```bash
   # Python簡易WebSocketサーバー例
   python3 -m pip install websockets
   python3 -c "
   import asyncio, websockets
   async def echo(websocket):
       print('Client connected')
       async for message in websocket:
           print(f'Received: {message}')
           await websocket.send(f'Echo: {message}')
   asyncio.run(websockets.serve(echo, '0.0.0.0', 8080))
   asyncio.get_event_loop().run_forever()
   "
   ```
2. **ESP32側での疎通確認**:
   - `wsClient.init("<Server_IP>", 8080, "/")` で接続。
   - シリアルモニタ（115200 baud）で `[WebSocketManager] Connected to server` が出力されることを確認。
   - サーバーからメッセージ送信し `[WebSocketManager] Received text` および `onMessage` コールバックがトリガーされることを確認。
   - サーバーを強制終了した際、`[WebSocketManager] Disconnected from server` となり5秒おきに自動再接続が試行されることを確認。
