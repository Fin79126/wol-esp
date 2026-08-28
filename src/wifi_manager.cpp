#include "wifi_manager.h"

WiFiManager::WiFiManager()
    : _state(State::IDLE)
    , _isConnected(false)
    , _connectTimeoutMs(WIFI_CONNECT_TIMEOUT_MS)
    , _reconnectIntervalMs(WIFI_RECONNECT_INTERVAL_MS)
    , _lastAttemptTime(0)
    , _connectingStartTime(0)
    , _onConnectedCallback(nullptr)
    , _onDisconnectedCallback(nullptr) {
}

WiFiManager::~WiFiManager() {
    disconnect();
}

void WiFiManager::init(const char* ssid, const char* password) {
    if (ssid != nullptr) {
        _ssid = ssid;
    }
    if (password != nullptr) {
        _password = password;
    }

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);

    startConnection();
}

void WiFiManager::startConnection() {
    if (_ssid.length() == 0) {
        Serial.println("[WiFi] Error: SSID is empty. Cannot initiate connection.");
        return;
    }

    Serial.printf("[WiFi] Connecting to SSID: %s ...\n", _ssid.c_str());

    WiFi.disconnect();
    if (_password.length() > 0) {
        WiFi.begin(_ssid.c_str(), _password.c_str());
    } else {
        WiFi.begin(_ssid.c_str());
    }

    _state = State::CONNECTING;
    _connectingStartTime = millis();
    _lastAttemptTime = millis();
}

void WiFiManager::update() {
    wl_status_t status = WiFi.status();

    switch (_state) {
        case State::IDLE:
            break;

        case State::CONNECTING:
            if (status == WL_CONNECTED) {
                handleConnected();
            } else if (millis() - _connectingStartTime >= _connectTimeoutMs) {
                Serial.printf("[WiFi] Connection timeout after %u ms. Retrying in %u ms...\n",
                              _connectTimeoutMs, _reconnectIntervalMs);
                WiFi.disconnect();
                _state = State::DISCONNECTED;
                _lastAttemptTime = millis();
            }
            break;

        case State::CONNECTED:
            if (status != WL_CONNECTED) {
                handleDisconnected();
            }
            break;

        case State::DISCONNECTED:
            if (millis() - _lastAttemptTime >= _reconnectIntervalMs) {
                Serial.println("[WiFi] Auto-reconnecting to Wi-Fi...");
                startConnection();
            }
            break;
    }
}

void WiFiManager::handleConnected() {
    _state = State::CONNECTED;
    _isConnected = true;

    IPAddress ip = WiFi.localIP();
    Serial.println("[WiFi] Connected successfully!");
    Serial.printf("[WiFi] IP Address : %s\n", ip.toString().c_str());
    Serial.printf("[WiFi] Signal (RSSI): %d dBm\n", WiFi.RSSI());
    Serial.printf("[WiFi] MAC Address: %s\n", WiFi.macAddress().c_str());

    if (_onConnectedCallback) {
        _onConnectedCallback(ip);
    }
}

void WiFiManager::handleDisconnected() {
    _state = State::DISCONNECTED;
    _isConnected = false;
    _lastAttemptTime = millis();

    Serial.println("[WiFi] Connection lost. Preparing to reconnect...");

    if (_onDisconnectedCallback) {
        _onDisconnectedCallback();
    }
}

bool WiFiManager::isConnected() const {
    return _isConnected && (WiFi.status() == WL_CONNECTED);
}

IPAddress WiFiManager::getIPAddress() const {
    if (isConnected()) {
        return WiFi.localIP();
    }
    return IPAddress(0, 0, 0, 0);
}

String WiFiManager::getIPAddressString() const {
    return getIPAddress().toString();
}

String WiFiManager::getSSID() const {
    return _ssid;
}

int8_t WiFiManager::getRSSI() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

String WiFiManager::getMacAddress() const {
    return WiFi.macAddress();
}

WiFiManager::State WiFiManager::getState() const {
    return _state;
}

void WiFiManager::onConnected(ConnectedCallback callback) {
    _onConnectedCallback = callback;
}

void WiFiManager::onDisconnected(DisconnectedCallback callback) {
    _onDisconnectedCallback = callback;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    _state = State::DISCONNECTED;
    _isConnected = false;
    _lastAttemptTime = millis();
    Serial.println("[WiFi] Disconnected by request.");
}

void WiFiManager::setConnectTimeout(uint32_t timeoutMs) {
    _connectTimeoutMs = timeoutMs;
}

void WiFiManager::setReconnectInterval(uint32_t intervalMs) {
    _reconnectIntervalMs = intervalMs;
}
