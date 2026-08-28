#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#if defined(ARDUINO)
  #include <Arduino.h>
  #include <ESP32Servo.h>
#else
  #include <cstdint>
  #include <algorithm>
  // Mock Servo for non-Arduino / Unit Testing environment
  class Servo {
  public:
      Servo() : attachedPin(-1), angle(0), isAttached(false) {}
      int attach(int pin, int min = 500, int max = 2400) {
          attachedPin = pin;
          minPulse = min;
          maxPulse = max;
          isAttached = true;
          return 1;
      }
      void detach() {
          isAttached = false;
      }
      void write(int val) {
          angle = val;
      }
      int read() const {
          return angle;
      }
      bool attached() const {
          return isAttached;
      }
      void setPeriodHertz(int hertz) {
          periodHertz = hertz;
      }
      int getAttachedPin() const { return attachedPin; }
  private:
      int attachedPin;
      int minPulse;
      int maxPulse;
      int angle;
      bool isAttached;
      int periodHertz;
  };
  unsigned long millis();
#endif

/**
 * @brief ESP32用 非ブロッキングサーボモーター制御クラス
 */
class ServoController {
public:
    enum class State {
        IDLE,            ///< 待機状態
        ACTION_HOLDING,  ///< triggerAction: targetAngle で保持中
        RETURNING        ///< triggerAction: returnAngle へ移動中
    };

    ServoController();
    ~ServoController();

    /**
     * @brief サーボコントローラの初期化
     * @param pin サーボのPWM信号ピン番号
     * @param initialAngle 初期角度（0〜180度、デフォルト0）
     * @param minPulseUs 最小パルス幅(us)（デフォルト 500us）
     * @param maxPulseUs 最大パルス幅(us)（デフォルト 2400us）
     * @return true: 成功, false: 失敗
     */
    bool init(int pin, int initialAngle = 0, int minPulseUs = 500, int maxPulseUs = 2400);

    /**
     * @brief 角度を直接指定して移動
     * @param angle 目標角度 (0〜180度)
     * @return true: 成功, false: 未初期化など
     */
    bool setAngle(int angle);

    /**
     * @brief 指定角度へ移動後、一定時間保持して復帰角度に戻す（非ブロッキング）
     * @param targetAngle 動作時目標角度 (0〜180度)
     * @param holdTimeMs 保持時間 (ミリ秒)
     * @param returnAngle 復帰角度 (0〜180度)
     * @return true: 実行開始成功, false: 未初期化など
     */
    bool triggerAction(int targetAngle, unsigned long holdTimeMs, int returnAngle = 0);

    /**
     * @brief メインループで毎周期呼び出される更新処理（タイマー・状態監視）
     */
    void update();

    /**
     * @brief トリガー動作実行中（ビジー状態）かどうか
     * @return true: 動作中, false: アイドル
     */
    bool isBusy() const;

    /**
     * @brief 現在のサーボ指示角度を取得
     * @return 現在角度 (0〜180度)
     */
    int getCurrentAngle() const;

    /**
     * @brief 設定されたGPIOピン番号を取得
     * @return GPIOピン番号 (未初期化時は -1)
     */
    int getPin() const;

    /**
     * @brief 現在の動作状態を取得
     * @return State enum
     */
    State getState() const;

    /**
     * @brief 自動デタッチ（ジッター・省電力対策）の設定
     * @param enable true: 有効, false: 無効
     * @param delayMs 移動完了からPWMデタッチまでの猶予時間 (ミリ秒, デフォルト500ms)
     */
    void setAutoDetach(bool enable, unsigned long delayMs = 500);

    /**
     * @brief 自動デタッチ機能が有効かどうか
     */
    bool isAutoDetachEnabled() const;

    /**
     * @brief 手動でサーボPWM出力をアタッチ
     */
    void attach();

    /**
     * @brief 手動でサーボPWM出力をデタッチ
     */
    void detach();

    /**
     * @brief サーボPWMが出力中（アタッチ状態）かどうか
     */
    bool isAttached() const;

    /**
     * @brief 実行中のトリガー動作を即時停止してアイドルに戻す
     */
    void stopAction();

private:
    void ensureAttached();
    int clampAngle(int angle) const;

    Servo _servo;
    int _pin;
    int _currentAngle;
    int _minPulseUs;
    int _maxPulseUs;
    bool _initialized;

    // ステートマシン
    State _state;
    unsigned long _stateStartTime;
    unsigned long _holdDurationMs;
    int _returnAngle;

    // 自動デタッチ
    bool _autoDetachEnabled;
    unsigned long _autoDetachDelayMs;
    unsigned long _lastMoveTime;
    bool _pendingDetach;

    // 復帰時の安定化時間 (ms)
    static const unsigned long SETTLE_TIME_MS = 250;
};

#endif // SERVO_CONTROLLER_H
