# TICKET-004: システム統合・サーボ駆動連動と動作確認ガイドの作成

## 概要
Wi-Fi接続、WebSocket通信、サーボ制御の各モジュールを結合し、WebSocket経由で特定の通信メッセージを受信した際にサーボを回転させるメイン処理 (`src/main.cpp`) を実装する。また、動作確認用の手順書・配線仕様書を作成する。

## 対象ブランチ / ディレクトリ
- ワークツリー: `.worktrees/feat-integration`
- ブランチ: `feat/integration`

## 実装要件
1. **`src/main.cpp` の実装**:
   - `setup()` で各モジュール（WiFiManager, WebSocketManager, ServoController）の初期化
   - WebSocket受信メッセージの解析（JSON形式 `{"action": "servo", "angle": 90, "hold_ms": 1000}` やプレーンテキスト `"TRIGGER"` の処理）
   - コマンド受信時にサーボを駆動し、完了または受信応答をWebSocket経由で返信
   - `loop()` で各モジュールの `update()` 呼び出し
2. **ドキュメント整備**:
   - `docs/hardware_spec.md`: ESP32-WROVER-E とサーボモータの配線図、電源供給の注意事項
   - `docs/usage_guide.md`: 書き込み方法、Wi-Fi/WebSocket設定方法、テスト用WebSocketサーバーの立ち上げ・疎通確認手順
3. **ログ記録**:
   - `docs/agent_logs/integration.md` に結合確認内容を記録。
