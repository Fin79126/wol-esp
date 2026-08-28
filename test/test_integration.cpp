#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include "../src/servo_controller.h"

// =============================================================================
// Mock millis and Environment for Native Testing
// =============================================================================
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

// =============================================================================
// Simple JSON / Command Dispatch Mock Tester
// =============================================================================

struct Response {
    std::string event;
    std::string status;
    int target_angle;
    unsigned long hold_ms;
    int return_angle;
    std::string raw_message;
};

class MockIntegrationHandler {
public:
    ServoController servoController;
    std::vector<std::string> sentMessages;
    bool wifiConnected;
    bool wsConnected;

    MockIntegrationHandler() : wifiConnected(false), wsConnected(false) {}

    void init() {
        servoController.init(18, 0);
        servoController.setAutoDetach(true, 500);
    }

    void simulateConnected() {
        wifiConnected = true;
        wsConnected = true;
        sentMessages.push_back("{\"event\":\"connected\",\"device\":\"esp32-wrover-e\"}");
    }

    void handleMessage(const std::string& raw) {
        std::string payload = raw;
        // Trim
        while (!payload.empty() && (payload.front() == ' ' || payload.front() == '\r' || payload.front() == '\n' || payload.front() == '\t')) {
            payload.erase(payload.begin());
        }
        while (!payload.empty() && (payload.back() == ' ' || payload.back() == '\r' || payload.back() == '\n' || payload.back() == '\t')) {
            payload.pop_back();
        }

        if (payload.empty()) return;

        // Plain text checks
        if (payload == "TRIGGER" || payload == "SERVO_TRIGGER" || payload == "PRESS") {
            bool ok = servoController.triggerAction(90, 500, 0);
            std::string res = std::string("{\"event\":\"servo_triggered\",\"status\":\"") + (ok ? "ok" : "busy") + "\",\"target_angle\":90,\"hold_ms\":500,\"return_angle\":0}";
            sentMessages.push_back(res);
            return;
        }

        if (payload == "PING") {
            sentMessages.push_back("{\"event\":\"pong\",\"timestamp\":" + std::to_string(millis()) + "}");
            return;
        }

        if (payload == "STATUS") {
            std::string res = "{\"event\":\"status\",\"servo_angle\":" + std::to_string(servoController.getCurrentAngle()) +
                              ",\"servo_busy\":" + (servoController.isBusy() ? "true" : "false") +
                              ",\"servo_attached\":" + (servoController.isAttached() ? "true" : "false") + "}";
            sentMessages.push_back(res);
            return;
        }

        if (payload == "DETACH") {
            servoController.detach();
            sentMessages.push_back("{\"event\":\"detached\",\"status\":\"ok\"}");
            return;
        }

        // JSON check (simple parser for testing)
        if (payload.front() == '{' && payload.back() == '}') {
            if (payload.find("\"action\":\"servo\"") != std::string::npos || payload.find("\"action\": \"servo\"") != std::string::npos ||
                payload.find("\"action\":\"trigger\"") != std::string::npos || payload.find("\"command\":\"TRIGGER\"") != std::string::npos) {
                
                int angle = 90;
                unsigned long holdMs = 500;
                int returnAngle = 0;

                // Extract angle if present
                size_t anglePos = payload.find("\"angle\":");
                if (anglePos != std::string::npos) {
                    angle = std::stoi(payload.substr(anglePos + 8));
                }

                // Extract hold_ms if present
                size_t holdPos = payload.find("\"hold_ms\":");
                if (holdPos != std::string::npos) {
                    holdMs = std::stoul(payload.substr(holdPos + 10));
                }

                bool ok = servoController.triggerAction(angle, holdMs, returnAngle);
                std::string res = "{\"event\":\"servo_triggered\",\"status\":\"" + std::string(ok ? "ok" : "busy") +
                                  "\",\"target_angle\":" + std::to_string(angle) +
                                  ",\"hold_ms\":" + std::to_string(holdMs) +
                                  ",\"return_angle\":" + std::to_string(returnAngle) + "}";
                sentMessages.push_back(res);
                return;
            }
            else if (payload.find("\"action\":\"set_angle\"") != std::string::npos || payload.find("\"action\": \"set_angle\"") != std::string::npos) {
                int angle = 0;
                size_t anglePos = payload.find("\"angle\":");
                if (anglePos != std::string::npos) {
                    angle = std::stoi(payload.substr(anglePos + 8));
                }
                bool ok = servoController.setAngle(angle);
                std::string res = "{\"event\":\"angle_set\",\"status\":\"" + std::string(ok ? "ok" : "error") +
                                  "\",\"angle\":" + std::to_string(angle) + "}";
                sentMessages.push_back(res);
                return;
            }
            else if (payload.find("\"action\":\"status\"") != std::string::npos || payload.find("\"action\": \"status\"") != std::string::npos) {
                std::string res = "{\"event\":\"status\",\"servo_angle\":" + std::to_string(servoController.getCurrentAngle()) +
                                  ",\"servo_busy\":" + (servoController.isBusy() ? "true" : "false") +
                                  ",\"servo_attached\":" + (servoController.isAttached() ? "true" : "false") + "}";
                sentMessages.push_back(res);
                return;
            }
            else if (payload.find("\"action\":\"ping\"") != std::string::npos) {
                sentMessages.push_back("{\"event\":\"pong\",\"timestamp\":" + std::to_string(millis()) + "}");
                return;
            }
            else if (payload.find("\"action\":\"detach\"") != std::string::npos) {
                servoController.detach();
                sentMessages.push_back("{\"event\":\"detached\",\"status\":\"ok\"}");
                return;
            }
            else {
                sentMessages.push_back("{\"event\":\"error\",\"message\":\"Unknown JSON action/command\"}");
                return;
            }
        }

        sentMessages.push_back("{\"event\":\"error\",\"message\":\"Invalid command format\"}");
    }

    void update() {
        servoController.update();
    }
};

// =============================================================================
// Integration Tests
// =============================================================================

void test_initialization_and_handshake() {
    std::cout << "[TEST] test_initialization_and_handshake..." << std::endl;
    resetTime();

    MockIntegrationHandler handler;
    handler.init();
    assert(handler.servoController.getCurrentAngle() == 0);
    assert(handler.servoController.isAttached());

    handler.simulateConnected();
    assert(handler.sentMessages.size() == 1);
    assert(handler.sentMessages[0].find("\"event\":\"connected\"") != std::string::npos);

    std::cout << "  -> PASSED" << std::endl;
}

void test_json_servo_trigger_and_lifecycle() {
    std::cout << "[TEST] test_json_servo_trigger_and_lifecycle..." << std::endl;
    resetTime();

    MockIntegrationHandler handler;
    handler.init();
    handler.sentMessages.clear();

    // Send JSON command: {"action": "servo", "angle": 90, "hold_ms": 500}
    handler.handleMessage("{\"action\":\"servo\",\"angle\":90,\"hold_ms\":500}");
    assert(handler.sentMessages.size() == 1);
    assert(handler.sentMessages.back().find("\"event\":\"servo_triggered\"") != std::string::npos);
    assert(handler.sentMessages.back().find("\"status\":\"ok\"") != std::string::npos);

    // Servo should now be at 90 deg and busy holding
    assert(handler.servoController.isBusy());
    assert(handler.servoController.getCurrentAngle() == 90);
    assert(handler.servoController.isAttached());

    // Advance 250ms -> still holding
    advanceTime(250);
    handler.update();
    assert(handler.servoController.isBusy());
    assert(handler.servoController.getCurrentAngle() == 90);

    // Advance 250ms (total 500ms) -> return to 0 deg
    advanceTime(250);
    handler.update();
    assert(handler.servoController.isBusy());
    assert(handler.servoController.getCurrentAngle() == 0);

    // Advance 250ms (settle time) -> idle
    advanceTime(250);
    handler.update();
    assert(!handler.servoController.isBusy());
    assert(handler.servoController.getCurrentAngle() == 0);
    assert(handler.servoController.isAttached());

    // Advance 500ms -> auto-detach triggers
    advanceTime(500);
    handler.update();
    assert(!handler.servoController.isAttached());

    std::cout << "  -> PASSED" << std::endl;
}

void test_plain_text_trigger() {
    std::cout << "[TEST] test_plain_text_trigger..." << std::endl;
    resetTime();

    MockIntegrationHandler handler;
    handler.init();
    handler.sentMessages.clear();

    // Send plain text TRIGGER
    handler.handleMessage("TRIGGER");
    assert(handler.sentMessages.size() == 1);
    assert(handler.sentMessages.back().find("\"event\":\"servo_triggered\"") != std::string::npos);
    assert(handler.servoController.isBusy());
    assert(handler.servoController.getCurrentAngle() == 90);

    // Complete cycle
    advanceTime(500); // Hold time
    handler.update();
    advanceTime(250); // Settle time
    handler.update();
    assert(!handler.servoController.isBusy());
    assert(handler.servoController.getCurrentAngle() == 0);

    std::cout << "  -> PASSED" << std::endl;
}

void test_status_and_ping_queries() {
    std::cout << "[TEST] test_status_and_ping_queries..." << std::endl;
    resetTime();

    MockIntegrationHandler handler;
    handler.init();
    handler.sentMessages.clear();

    // Query Ping
    advanceTime(1000);
    handler.handleMessage("PING");
    assert(handler.sentMessages.size() == 1);
    assert(handler.sentMessages.back().find("\"event\":\"pong\"") != std::string::npos);
    assert(handler.sentMessages.back().find("\"timestamp\":1000") != std::string::npos);

    // Query Status
    handler.handleMessage("STATUS");
    assert(handler.sentMessages.size() == 2);
    assert(handler.sentMessages.back().find("\"event\":\"status\"") != std::string::npos);
    assert(handler.sentMessages.back().find("\"servo_busy\":false") != std::string::npos);

    // Set Angle directly
    handler.handleMessage("{\"action\":\"set_angle\",\"angle\":45}");
    assert(handler.sentMessages.size() == 3);
    assert(handler.sentMessages.back().find("\"event\":\"angle_set\"") != std::string::npos);
    assert(handler.servoController.getCurrentAngle() == 45);

    std::cout << "  -> PASSED" << std::endl;
}

void test_invalid_commands() {
    std::cout << "[TEST] test_invalid_commands..." << std::endl;
    resetTime();

    MockIntegrationHandler handler;
    handler.init();
    handler.sentMessages.clear();

    handler.handleMessage("INVALID_CMD");
    assert(handler.sentMessages.size() == 1);
    assert(handler.sentMessages.back().find("\"event\":\"error\"") != std::string::npos);

    handler.handleMessage("{\"action\":\"unknown_action\"}");
    assert(handler.sentMessages.size() == 2);
    assert(handler.sentMessages.back().find("\"event\":\"error\"") != std::string::npos);

    std::cout << "  -> PASSED" << std::endl;
}

int main() {
    std::cout << "=== Running System Integration Tests ===" << std::endl;
    test_initialization_and_handshake();
    test_json_servo_trigger_and_lifecycle();
    test_plain_text_trigger();
    test_status_and_ping_queries();
    test_invalid_commands();
    std::cout << "=== ALL INTEGRATION TESTS PASSED SUCCESSFULLY ===" << std::endl;
    return 0;
}
