#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include "../src/servo_controller.h"

// テスト用 millis() 実装（手動で時刻を進められるモック時間）
static unsigned long g_simulated_millis = 0;
unsigned long millis() {
    return g_simulated_millis;
}

void advanceTime(unsigned long ms) {
    g_simulated_millis += ms;
}

void resetTime() {
    g_simulated_millis = 0;
}

void test_init_and_set_angle() {
    std::cout << "[TEST] test_init_and_set_angle..." << std::endl;
    resetTime();

    ServoController controller;
    assert(!controller.setAngle(90)); // 未初期化なのでfalse

    bool ok = controller.init(18, 0);
    assert(ok);
    assert(controller.getPin() == 18);
    assert(controller.getCurrentAngle() == 0);
    assert(controller.isAttached());
    assert(!controller.isBusy());

    // setAngle (正常範囲)
    assert(controller.setAngle(90));
    assert(controller.getCurrentAngle() == 90);

    // setAngle (クランプ確認: 200 -> 180, -10 -> 0)
    assert(controller.setAngle(200));
    assert(controller.getCurrentAngle() == 180);

    assert(controller.setAngle(-10));
    assert(controller.getCurrentAngle() == 0);

    std::cout << "  -> PASSED" << std::endl;
}

void test_trigger_action() {
    std::cout << "[TEST] test_trigger_action..." << std::endl;
    resetTime();

    ServoController controller;
    controller.init(18, 0);

    // triggerAction: 90度へ移動 -> 1000ms保持 -> 0度に戻す
    assert(controller.triggerAction(90, 1000, 0));
    assert(controller.isBusy());
    assert(controller.getCurrentAngle() == 90);
    assert(controller.getState() == ServoController::State::ACTION_HOLDING);

    // 500ms経過 -> まだ90度保持中
    advanceTime(500);
    controller.update();
    assert(controller.isBusy());
    assert(controller.getCurrentAngle() == 90);
    assert(controller.getState() == ServoController::State::ACTION_HOLDING);

    // さらに500ms経過 (計1000ms) -> 0度へ復帰開始
    advanceTime(500);
    controller.update();
    assert(controller.isBusy());
    assert(controller.getCurrentAngle() == 0);
    assert(controller.getState() == ServoController::State::RETURNING);

    // 復帰安定時間 (250ms) 経過 -> アイドルへ
    advanceTime(250);
    controller.update();
    assert(!controller.isBusy());
    assert(controller.getCurrentAngle() == 0);
    assert(controller.getState() == ServoController::State::IDLE);

    std::cout << "  -> PASSED" << std::endl;
}

void test_auto_detach() {
    std::cout << "[TEST] test_auto_detach..." << std::endl;
    resetTime();

    ServoController controller;
    controller.setAutoDetach(true, 500); // 500ms後にデタッチ
    assert(controller.isAutoDetachEnabled());

    controller.init(18, 0);
    assert(controller.isAttached());

    // 300ms経過 -> まだアタッチ状態
    advanceTime(300);
    controller.update();
    assert(controller.isAttached());

    // さらに300ms経過 (計600ms > 500ms) -> 自動デタッチ実行
    advanceTime(300);
    controller.update();
    assert(!controller.isAttached());

    // setAngle で自動再アタッチされること
    controller.setAngle(45);
    assert(controller.isAttached());
    assert(controller.getCurrentAngle() == 45);

    // 500ms経過で再び自動デタッチ
    advanceTime(500);
    controller.update();
    assert(!controller.isAttached());

    // triggerAction 開始で自動再アタッチ
    controller.triggerAction(90, 1000, 0);
    assert(controller.isAttached());
    assert(controller.getCurrentAngle() == 90);

    // 動作中（保持中）はデタッチされないこと
    advanceTime(600); // アクション開始から600ms (AutoDetachDelayの500ms超え)
    controller.update();
    assert(controller.isAttached()); // 保持中はアタッチ維持
    assert(controller.isBusy());

    // 保持完了 (計1000ms) -> 復帰角度0へ
    advanceTime(400);
    controller.update();
    assert(controller.getCurrentAngle() == 0);
    assert(controller.isAttached());

    // 復帰安定時間 (250ms) 経過 -> IDLEへ
    advanceTime(250);
    controller.update();
    assert(!controller.isBusy());
    assert(controller.isAttached()); // まだdelay500ms経っていない

    // 復帰後 500ms 経過で自動デタッチ
    advanceTime(500);
    controller.update();
    assert(!controller.isAttached());

    std::cout << "  -> PASSED" << std::endl;
}

void test_stop_action() {
    std::cout << "[TEST] test_stop_action..." << std::endl;
    resetTime();

    ServoController controller;
    controller.init(18, 0);
    controller.triggerAction(90, 2000, 0);
    assert(controller.isBusy());

    advanceTime(500);
    controller.update();
    assert(controller.isBusy());

    // 途中で停止
    controller.stopAction();
    assert(!controller.isBusy());
    assert(controller.getState() == ServoController::State::IDLE);

    std::cout << "  -> PASSED" << std::endl;
}

int main() {
    std::cout << "=== Running ServoController Unit Tests ===" << std::endl;
    test_init_and_set_angle();
    test_trigger_action();
    test_auto_detach();
    test_stop_action();
    std::cout << "=== ALL TESTS PASSED SUCCESSFULLY ===" << std::endl;
    return 0;
}
