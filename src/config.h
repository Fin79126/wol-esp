#pragma once

#include <Arduino.h>

// =============================================================================
// Wi-Fi Configuration
// =============================================================================
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#define WIFI_CONNECT_TIMEOUT_MS    15000  // Connection attempt timeout in ms
#define WIFI_RECONNECT_INTERVAL_MS 5000   // Retry interval when disconnected in ms

// =============================================================================
// WebSocket Server Configuration
// =============================================================================
#ifndef WS_SERVER_HOST
#define WS_SERVER_HOST "192.168.1.100"   // Default WebSocket server IP / Hostname
#endif

#ifndef WS_SERVER_PORT
#define WS_SERVER_PORT 8080              // Default WebSocket server port
#endif

#ifndef WS_SERVER_PATH
#define WS_SERVER_PATH "/ws"             // WebSocket endpoint path
#endif

#ifndef WS_SERVER_PROTOCOL
#define WS_SERVER_PROTOCOL ""            // WebSocket subprotocol ("" for default)
#endif

#define WS_RECONNECT_INTERVAL_MS 5000    // Reconnect attempt interval in ms
#define WS_PING_INTERVAL_MS      15000   // Heartbeat ping interval in ms
#define WS_PONG_TIMEOUT_MS       3000    // Heartbeat pong timeout in ms
#define WS_DISCONNECT_TIMEOUTS   2       // Number of timeouts before disconnect

// =============================================================================
// Servo Motor Configuration
// =============================================================================
#define SERVO_PIN            18          // GPIO Pin for Servo PWM control
#define SERVO_STANDBY_ANGLE  0           // Angle in standby/idle position (deg)
#define SERVO_TRIGGER_ANGLE  90          // Angle when pressing/triggering (deg)
#define SERVO_HOLD_TIME_MS   500         // Time to hold the trigger angle in ms

// =============================================================================
// System / Debug Configuration
// =============================================================================
#define SERIAL_BAUD_RATE     115200      // Serial monitor baud rate
