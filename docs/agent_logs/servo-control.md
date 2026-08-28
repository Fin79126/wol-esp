# Agent Log: feat/servo-control

## 1. 作業開始と要件確認 (2026-08-28)
### 対象チケット
- TICKET-002: サーボモーター制御モジュールの実装 (`.agent/tickets/002-servo-control.md`)

### 実装要件
1. `ServoController` クラスの実装 (`src/servo_controller.h`, `src/servo_controller.cpp`)
   - 初期化: `init(int pin, int initialAngle = 0, int minPulseUs = 500, int maxPulseUs = 2400)`
   - 角度指定: `setAngle(int angle)`
   - トリガー動作: `triggerAction(int targetAngle, unsigned long holdTimeMs, int returnAngle = 0)`
     - 非ブロッキング（`millis()` タイマー監視）で targetAngle に動かした後、holdTimeMs 経過後に returnAngle に戻す
   - `update()` メソッド（メインループで毎周期呼び出され、タイマー経過を判定）
   - アイドル時にサーボのPWM信号をデタッチ/アタッチしてジッター・発熱・電力消費を抑える機能（オプションフラグで有効化可能）
   - 現在の状態取得（`isBusy()`, `getCurrentAngle()`, `getPin()`, `getState()`）

## 2. 設計方針
### 状態管理（State Machine）
トリガー動作（`triggerAction`）を非ブロッキングで実現するため、内部ステートマシンを導入：
- `IDLE`: 待機状態
- `ACTION_HOLDING`: 目標角度（`targetAngle`）に移動後、指定時間（`holdTimeMs`）保持している状態
- `RETURNING`: 復帰角度（`returnAngle`）へ指示し、サーボ安定化（250ms）を待つ状態

### 自動デタッチ（Auto-Detach / ジッター・省電力対策）
- サーボモーターはPWM信号が常時入力されていると、ジッター（微小な振動）やコイル発熱、待機電力が発生する。
- 角度指示後、サーボが目標角度まで物理的に移動する時間（デフォルト500ms、設定可能）を待った後、自動でPWM信号をデタッチ（`detach()`）するオプション機能を実装。
- 新たに角度指示（`setAngle` や `triggerAction`）が発生した際は、自動的に再アタッチ（`attach()`）してPWM出力を再開する。
- `setAutoDetach(bool enable, unsigned long delayMs = 500)` で動的に有効/無効および遅延時間を設定可能。

### 単体テスト・検証設計
- ネイティブ環境（Linux/g++）でも単体テスト可能なように、テスト用モック機構を `servo_controller.h` に内包。
- `test/test_servo_controller.cpp` による自動テストで以下を検証：
  1. 初期化および角度クランプ（0〜180度）
  2. 非同期トリガー動作のシーケンスと `isBusy()` の遷移
  3. 自動デタッチ / 自動再アタッチのタイマー動作
  4. アクション中断（`stopAction`）

## 3. 実装詳細
- `src/servo_controller.h`: クラス定義およびインターフェース
- `src/servo_controller.cpp`: ステートマシン、タイマー計算、アタッチ/デタッチ制御
- `test/test_servo_controller.cpp`: g++ による単体テスト

## 4. テスト結果
```
=== Running ServoController Unit Tests ===
[TEST] test_init_and_set_angle...
  -> PASSED
[TEST] test_trigger_action...
  -> PASSED
[TEST] test_auto_detach...
  -> PASSED
[TEST] test_stop_action...
  -> PASSED
=== ALL TESTS PASSED SUCCESSFULLY ===
```
すべてのテストケースがパス。
