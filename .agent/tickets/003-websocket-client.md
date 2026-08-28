# TICKET-003: WebSocketクライアント通信モジュールの実装

## 概要
ESP32からWebSocketサーバーへ常時接続し、Ping/Pong死活監視とメッセージ受信イベントを処理するクライアントモジュールを実装する。

## 対象ブランチ / ディレクトリ
- ワークツリー: `.worktrees/feat-websocket-client`
- ブランチ: `feat/websocket-client`

## 実装要件
1. **`src/ws_client.h` / `src/ws_client.cpp` の実装**:
   - `WebSocketManager` クラス
   - 初期化メソッド `init(const char* host, uint16_t port, const char* path, const char* protocol = nullptr)`
   - ループ処理 `update()` でのWebSocketイベントループ処理
   - 接続コールバック・切断コールバック・メッセージ受信コールバックの登録機能 (`std::function` または関数ポインタ)
   - サーバー切断時の自動再接続機能
   - メッセージ送信機能 `sendText(const String& message)`
   - ハートビート（Ping/Pong）の維持
2. **ログ記録**:
   - `docs/agent_logs/websocket-client.md` に実装内容とメッセージフォーマット仕様を記録。
