#include "servo_controller.h"

ServoController::ServoController()
    : _pin(-1),
      _currentAngle(0),
      _minPulseUs(500),
      _maxPulseUs(2400),
      _initialized(false),
      _state(State::IDLE),
      _stateStartTime(0),
      _holdDurationMs(0),
      _returnAngle(0),
      _autoDetachEnabled(false),
      _autoDetachDelayMs(500),
      _lastMoveTime(0),
      _pendingDetach(false) {
}

ServoController::~ServoController() {
    detach();
}

bool ServoController::init(int pin, int initialAngle, int minPulseUs, int maxPulseUs) {
    _pin = pin;
    _minPulseUs = minPulseUs;
    _maxPulseUs = maxPulseUs;
    _currentAngle = clampAngle(initialAngle);
    _state = State::IDLE;

#if defined(ESP32)
    // ESP32Servo では必要に応じてタイマーの割り当てを行う
    // ESP32Servo::allocateTimer(0);
#endif
    _servo.setPeriodHertz(50); // 標準サーボ 50Hz (20ms周期)
    _servo.attach(_pin, _minPulseUs, _maxPulseUs);
    _servo.write(_currentAngle);

    _initialized = true;
    _lastMoveTime = millis();
    _pendingDetach = _autoDetachEnabled;

    return true;
}

bool ServoController::setAngle(int angle) {
    if (!_initialized) {
        return false;
    }

    // 手動で角度を設定した場合、進行中のトリガー動作はリセット
    _state = State::IDLE;
    _currentAngle = clampAngle(angle);

    ensureAttached();
    _servo.write(_currentAngle);
    _lastMoveTime = millis();
    _pendingDetach = _autoDetachEnabled;

    return true;
}

bool ServoController::triggerAction(int targetAngle, unsigned long holdTimeMs, int returnAngle) {
    if (!_initialized) {
        return false;
    }

    int target = clampAngle(targetAngle);
    _returnAngle = clampAngle(returnAngle);
    _holdDurationMs = holdTimeMs;

    ensureAttached();
    _servo.write(target);
    _currentAngle = target;

    _stateStartTime = millis();
    _lastMoveTime = _stateStartTime;
    _state = State::ACTION_HOLDING;
    _pendingDetach = false; // 動作中はデタッチを保留しない

    return true;
}

void ServoController::update() {
    if (!_initialized) {
        return;
    }

    unsigned long now = millis();

    switch (_state) {
        case State::ACTION_HOLDING:
            if (now - _stateStartTime >= _holdDurationMs) {
                // 保持時間満了 -> 復帰角度へ移動
                ensureAttached();
                _servo.write(_returnAngle);
                _currentAngle = _returnAngle;
                _stateStartTime = now;
                _lastMoveTime = now;
                _state = State::RETURNING;
                _pendingDetach = _autoDetachEnabled;
            }
            break;

        case State::RETURNING:
            if (now - _stateStartTime >= SETTLE_TIME_MS) {
                // 復帰角度への移動・安定化完了 -> アイドルへ
                _state = State::IDLE;
            }
            break;

        case State::IDLE:
        default:
            break;
    }

    // アイドル時の自動デタッチ判定
    if (_pendingDetach && _autoDetachEnabled) {
        if (_state == State::IDLE || _state == State::RETURNING) {
            if (now - _lastMoveTime >= _autoDetachDelayMs) {
                detach();
                _pendingDetach = false;
            }
        }
    }
}

bool ServoController::isBusy() const {
    return _state != State::IDLE;
}

int ServoController::getCurrentAngle() const {
    return _currentAngle;
}

int ServoController::getPin() const {
    return _pin;
}

ServoController::State ServoController::getState() const {
    return _state;
}

void ServoController::setAutoDetach(bool enable, unsigned long delayMs) {
    _autoDetachEnabled = enable;
    _autoDetachDelayMs = delayMs;
    if (_autoDetachEnabled && _state == State::IDLE) {
        _lastMoveTime = millis();
        _pendingDetach = true;
    } else if (!_autoDetachEnabled) {
        _pendingDetach = false;
    }
}

bool ServoController::isAutoDetachEnabled() const {
    return _autoDetachEnabled;
}

void ServoController::attach() {
    ensureAttached();
}

void ServoController::detach() {
    if (_servo.attached()) {
        _servo.detach();
    }
    _pendingDetach = false;
}

bool ServoController::isAttached() const {
    return _servo.attached();
}

void ServoController::stopAction() {
    _state = State::IDLE;
    _pendingDetach = _autoDetachEnabled;
    _lastMoveTime = millis();
}

void ServoController::ensureAttached() {
    if (!_servo.attached() && _pin >= 0) {
        _servo.attach(_pin, _minPulseUs, _maxPulseUs);
    }
}

int ServoController::clampAngle(int angle) const {
    if (angle < 0) return 0;
    if (angle > 180) return 180;
    return angle;
}
