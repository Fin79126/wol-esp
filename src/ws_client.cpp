#include "ws_client.h"

WebSocketManager* WebSocketManager::_instance = nullptr;

WebSocketManager::WebSocketManager()
    : _connected(false),
      _port(80),
      _onMessageCallback(nullptr),
      _onConnectedCallback(nullptr),
      _onDisconnectedCallback(nullptr),
      _onBinaryCallback(nullptr) {
    _instance = this;
}

WebSocketManager::~WebSocketManager() {
    disconnect();
    if (_instance == this) {
        _instance = nullptr;
    }
}

void WebSocketManager::init(const char* host, uint16_t port, const char* path, const char* protocol) {
    _host = host ? host : "";
    _port = port;
    _path = (path != nullptr && strlen(path) > 0) ? path : "/";
    _connected = false;

    Serial.printf("[WebSocketManager] Initializing connection to ws://%s:%u%s\n", _host.c_str(), _port, _path.c_str());

    // Connect to WebSocket server
    if (protocol != nullptr && strlen(protocol) > 0) {
        _client.begin(_host.c_str(), _port, _path.c_str(), protocol);
    } else {
        _client.begin(_host.c_str(), _port, _path.c_str());
    }

    // Register event handler
    _client.onEvent(WebSocketManager::handleWebSocketEvent);

    // Default reconnect interval: 5000ms
    _client.setReconnectInterval(5000);

    // Default heartbeat: ping every 15s, timeout 3s, disconnect after 2 missed pongs
    _client.enableHeartbeat(15000, 3000, 2);
}

void WebSocketManager::update() {
    _client.loop();
}

void WebSocketManager::onMessage(MessageCallback callback) {
    _onMessageCallback = callback;
}

void WebSocketManager::onConnected(ConnectedCallback callback) {
    _onConnectedCallback = callback;
}

void WebSocketManager::onDisconnected(DisconnectedCallback callback) {
    _onDisconnectedCallback = callback;
}

void WebSocketManager::onBinary(BinaryCallback callback) {
    _onBinaryCallback = callback;
}

bool WebSocketManager::sendText(const String& message) {
    if (!_connected) {
        Serial.println("[WebSocketManager] Cannot send text: not connected");
        return false;
    }
    return _client.sendTXT(message);
}

bool WebSocketManager::sendBinary(const uint8_t* payload, size_t length) {
    if (!_connected) {
        Serial.println("[WebSocketManager] Cannot send binary: not connected");
        return false;
    }
    return _client.sendBIN(payload, length);
}

bool WebSocketManager::isConnected() const {
    return _connected;
}

void WebSocketManager::setReconnectInterval(uint32_t ms) {
    _client.setReconnectInterval(ms);
}

void WebSocketManager::enableHeartbeat(uint32_t pingInterval, uint32_t pongTimeout, uint8_t disconnectTimeouts) {
    _client.enableHeartbeat(pingInterval, pongTimeout, disconnectTimeouts);
}

void WebSocketManager::disconnect() {
    _client.disconnect();
    _connected = false;
}

void WebSocketManager::handleWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    if (_instance != nullptr) {
        _instance->handleEvent(type, payload, length);
    }
}

void WebSocketManager::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED: {
            _connected = false;
            Serial.println("[WebSocketManager] Disconnected from server");
            if (_onDisconnectedCallback) {
                _onDisconnectedCallback();
            }
            break;
        }
        case WStype_CONNECTED: {
            _connected = true;
            Serial.printf("[WebSocketManager] Connected to server: %s\n", (payload != nullptr) ? (char*)payload : "");
            if (_onConnectedCallback) {
                _onConnectedCallback();
            }
            break;
        }
        case WStype_TEXT: {
            String msg = "";
            if (payload != nullptr && length > 0) {
                msg = String((char*)payload, length);
            }
            Serial.printf("[WebSocketManager] Received text (%u bytes): %s\n", (unsigned int)length, msg.c_str());
            if (_onMessageCallback) {
                _onMessageCallback(msg);
            }
            break;
        }
        case WStype_BIN: {
            Serial.printf("[WebSocketManager] Received binary (%u bytes)\n", (unsigned int)length);
            if (_onBinaryCallback) {
                _onBinaryCallback(payload, length);
            }
            break;
        }
        case WStype_ERROR: {
            Serial.printf("[WebSocketManager] Error: %s\n", (payload != nullptr) ? (char*)payload : "Unknown error");
            break;
        }
        case WStype_PING: {
            Serial.println("[WebSocketManager] Ping received from server");
            break;
        }
        case WStype_PONG: {
            Serial.println("[WebSocketManager] Pong received from server");
            break;
        }
        default:
            break;
    }
}
