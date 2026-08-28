# TICKET-002: サーボモーター制御モジュールの実装

## 概要
ESP32のPWM/LEDC機能（またはESP32Servoライブラリ）を用いて、サーボモーターの回転角度を非ブロッキングで制御するモジュールを実装する。

## 対象ブランチ / ディレクトリ
- ワークツリー: `.worktrees/feat-servo-control`
- ブランチ: `feat/servo-control`

## 実装要件
1. **`src/servo_controller.h` / `src/servo_controller.cpp` の実装**:
   - `ServoController` クラス
   - 初期化メソッド `init(int pin, int initialAngle = 0)`
   - 角度指定メソッド `setAngle(int angle)`
   - トリガー動作メソッド `triggerAction(int targetAngle, unsigned long holdTimeMs, int returnAngle)`
     - 例: 90度へ移動 → 指定ms待機（非ブロッキング） → 元の角度に戻す
   - ループ処理 `update()` での非同期タイマー処理
   - サーボの省電力/ジッター防止用のアタッチ/デタッチ制御
2. **ログ記録**:
   - `docs/agent_logs/servo-control.md` に設計方針とテスト方法を記録。
