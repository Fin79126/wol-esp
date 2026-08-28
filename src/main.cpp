#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi_manager.h"
#include "ws_client.h"
#include "servo_controller.h"

// =============================================================================
// Global Module Instances
// =============================================================================
WiFiManager wifiManager;
WebSocketManager wsManager;
ServoController servoController;

// =============================================================================
// Forward Declarations
// =============================================================================
void sendStatusResponse();
void handleCommandMessage(const String& payload);

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * @brief Send detailed device and servo status as JSON via WebSocket
 */
void sendStatusResponse() {
    JsonDocument res;
    res["event"] = "status";
    res["wifi_connected"] = wifiManager.isConnected();
    res["ip"] = wifiManager.getIPAddressString();
    res["rssi"] = wifiManager.getRSSI();
    res["mac"] = wifiManager.getMacAddress();
    res["ws_connected"] = wsManager.isConnected();
    res["servo_pin"] = servoController.getPin();
    res["servo_angle"] = servoController.getCurrentAngle();
    res["servo_busy"] = servoController.isBusy();
    res["servo_attached"] = servoController.isAttached();
    res["free_heap"] = ESP.getFreeHeap();
    res["uptime_ms"] = millis();

    String resStr;
    serializeJson(res, resStr);
    wsManager.sendText(resStr);
}

/**
 * @brief Parse and execute incoming WebSocket message (JSON or plain text)
 * @param rawPayload Received payload string
 */
void handleCommandMessage(const String& rawPayload) {
    String payload = rawPayload;
    payload.trim();
    if (payload.length() == 0) {
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.is<JsonObject>()) {
        // =====================================================================
        // 1. JSON Command Dispatch
        // =====================================================================
        const char* action = doc["action"] | doc["command"] | "";

        if (strcasecmp(action, "servo") == 0 ||
            strcasecmp(action, "trigger") == 0 ||
            strcasecmp(action, "SERVO_TRIGGER") == 0 ||
            strcasecmp(action, "press") == 0) {

            int angle = doc["angle"] | SERVO_TRIGGER_ANGLE;
            unsigned long holdMs = doc["hold_ms"] | (doc["duration"] | SERVO_HOLD_TIME_MS);
            int returnAngle = doc["return_angle"] | SERVO_STANDBY_ANGLE;

            Serial.printf("[MAIN] Triggering servo: target=%d deg, hold=%lu ms, return=%d deg\n",
                          angle, holdMs, returnAngle);

            bool ok = servoController.triggerAction(angle, holdMs, returnAngle);

            JsonDocument res;
            res["event"] = "servo_triggered";
            res["status"] = ok ? "ok" : "busy";
            res["target_angle"] = angle;
            res["hold_ms"] = holdMs;
            res["return_angle"] = returnAngle;
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (strcasecmp(action, "set_angle") == 0 || strcasecmp(action, "angle") == 0) {
            int angle = doc["angle"] | SERVO_STANDBY_ANGLE;
            Serial.printf("[MAIN] Setting servo angle to %d deg\n", angle);

            bool ok = servoController.setAngle(angle);

            JsonDocument res;
            res["event"] = "angle_set";
            res["status"] = ok ? "ok" : "error";
            res["angle"] = angle;
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (strcasecmp(action, "status") == 0 || strcasecmp(action, "get_status") == 0) {
            sendStatusResponse();
        }
        else if (strcasecmp(action, "ping") == 0) {
            JsonDocument res;
            res["event"] = "pong";
            res["timestamp"] = millis();
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (strcasecmp(action, "detach") == 0) {
            servoController.detach();
            JsonDocument res;
            res["event"] = "detached";
            res["status"] = "ok";
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (strcasecmp(action, "attach") == 0) {
            servoController.attach();
            JsonDocument res;
            res["event"] = "attached";
            res["status"] = "ok";
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else {
            Serial.printf("[MAIN] Warning: Unknown JSON action '%s'\n", action);
            JsonDocument res;
            res["event"] = "error";
            res["message"] = "Unknown JSON action/command";
            res["action"] = action;
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
    } else {
        // =====================================================================
        // 2. Plain Text Command Dispatch
        // =====================================================================
        if (payload.equalsIgnoreCase("TRIGGER") ||
            payload.equalsIgnoreCase("SERVO_TRIGGER") ||
            payload.equalsIgnoreCase("PRESS")) {

            Serial.printf("[MAIN] Triggering servo (Default): target=%d deg, hold=%d ms, return=%d deg\n",
                          SERVO_TRIGGER_ANGLE, SERVO_HOLD_TIME_MS, SERVO_STANDBY_ANGLE);

            bool ok = servoController.triggerAction(SERVO_TRIGGER_ANGLE, SERVO_HOLD_TIME_MS, SERVO_STANDBY_ANGLE);

            JsonDocument res;
            res["event"] = "servo_triggered";
            res["status"] = ok ? "ok" : "busy";
            res["target_angle"] = SERVO_TRIGGER_ANGLE;
            res["hold_ms"] = SERVO_HOLD_TIME_MS;
            res["return_angle"] = SERVO_STANDBY_ANGLE;
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (payload.equalsIgnoreCase("PING")) {
            JsonDocument res;
            res["event"] = "pong";
            res["timestamp"] = millis();
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (payload.equalsIgnoreCase("STATUS")) {
            sendStatusResponse();
        }
        else if (payload.equalsIgnoreCase("DETACH")) {
            servoController.detach();
            JsonDocument res;
            res["event"] = "detached";
            res["status"] = "ok";
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else if (payload.equalsIgnoreCase("ATTACH")) {
            servoController.attach();
            JsonDocument res;
            res["event"] = "attached";
            res["status"] = "ok";
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
        else {
            Serial.printf("[MAIN] Warning: Unknown text command '%s'\n", payload.c_str());
            JsonDocument res;
            res["event"] = "error";
            res["message"] = "Invalid command format";
            res["raw"] = payload;
            String resStr;
            serializeJson(res, resStr);
            wsManager.sendText(resStr);
        }
    }
}

// =============================================================================
// Arduino Setup & Loop
// =============================================================================

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    Serial.println("\n========================================");
    Serial.println("  ESP32 WOL / Remote Button Presser");
    Serial.println("  System Integration Starting...");
    Serial.println("========================================");

    // 1. Initialize Servo Controller
    Serial.printf("[MAIN] Initializing Servo on GPIO %d (Standby: %d deg)...\n", SERVO_PIN, SERVO_STANDBY_ANGLE);
    if (servoController.init(SERVO_PIN, SERVO_STANDBY_ANGLE)) {
        servoController.setAutoDetach(true, 500);
        Serial.println("[MAIN] ServoController initialized successfully (AutoDetach: enabled, 500ms).");
    } else {
        Serial.println("[MAIN] ERROR: Failed to initialize ServoController!");
    }

    // 2. Setup WebSocket Callbacks
    wsManager.onConnected([]() {
        Serial.println("[MAIN] WebSocket server connected!");
        // Send initial handshake / device registration message
        JsonDocument helloDoc;
        helloDoc["event"] = "connected";
        helloDoc["device"] = "esp32-wrover-e";
        helloDoc["ip"] = wifiManager.getIPAddressString();
        helloDoc["mac"] = wifiManager.getMacAddress();
        helloDoc["rssi"] = wifiManager.getRSSI();
        helloDoc["servo_pin"] = SERVO_PIN;
        helloDoc["servo_angle"] = servoController.getCurrentAngle();
        String json;
        serializeJson(helloDoc, json);
        wsManager.sendText(json);
    });

    wsManager.onDisconnected([]() {
        Serial.println("[MAIN] WebSocket disconnected from server.");
    });

    wsManager.onMessage([](const String& message) {
        Serial.printf("[MAIN] Received WS message: %s\n", message.c_str());
        handleCommandMessage(message);
    });

    // 3. Setup Wi-Fi Callbacks & initiate connection
    wifiManager.onConnected([](const IPAddress& ip) {
        Serial.printf("[MAIN] Wi-Fi Connected! IP: %s, RSSI: %d dBm\n",
                      ip.toString().c_str(), wifiManager.getRSSI());
        Serial.printf("[MAIN] Connecting WebSocket to %s:%d%s...\n",
                      WS_SERVER_HOST, WS_SERVER_PORT, WS_SERVER_PATH);

        wsManager.init(
            WS_SERVER_HOST,
            WS_SERVER_PORT,
            WS_SERVER_PATH,
            (strlen(WS_SERVER_PROTOCOL) > 0) ? WS_SERVER_PROTOCOL : nullptr
        );
        wsManager.enableHeartbeat(WS_PING_INTERVAL_MS, WS_PONG_TIMEOUT_MS, WS_DISCONNECT_TIMEOUTS);
        wsManager.setReconnectInterval(WS_RECONNECT_INTERVAL_MS);
    });

    wifiManager.onDisconnected([]() {
        Serial.println("[MAIN] Wi-Fi Disconnected! Disconnecting WebSocket...");
        wsManager.disconnect();
    });

    Serial.printf("[MAIN] Connecting to Wi-Fi SSID: %s ...\n", WIFI_SSID);
    wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT_MS);
    wifiManager.setReconnectInterval(WIFI_RECONNECT_INTERVAL_MS);
    wifiManager.init(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("[MAIN] Setup complete. Entering main loop.");
}

void loop() {
    wifiManager.update();
    wsManager.update();
    servoController.update();
}
