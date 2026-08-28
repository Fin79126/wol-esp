#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>
#include "config.h"

/**
 * @brief Wi-Fi connection manager for ESP32.
 * Provides non-blocking Wi-Fi initialization, event callbacks,
 * automatic reconnection, and connection status polling.
 */
class WiFiManager {
public:
    using ConnectedCallback = std::function<void(const IPAddress& ip)>;
    using DisconnectedCallback = std::function<void()>;

    enum class State {
        IDLE,
        CONNECTING,
        CONNECTED,
        DISCONNECTED
    };

    WiFiManager();
    ~WiFiManager();

    /**
     * @brief Initialize Wi-Fi connection with given credentials.
     * @param ssid Wi-Fi network SSID (default: WIFI_SSID from config.h)
     * @param password Wi-Fi password (default: WIFI_PASSWORD from config.h)
     */
    void init(const char* ssid = WIFI_SSID, const char* password = WIFI_PASSWORD);

    /**
     * @brief Update connection monitor and manage auto-reconnects.
     * Should be called periodically inside Arduino loop().
     */
    void update();

    /**
     * @brief Check if Wi-Fi is currently connected.
     * @return true if connected and assigned an IP, false otherwise.
     */
    bool isConnected() const;

    /**
     * @brief Get assigned IP address.
     * @return IPAddress object
     */
    IPAddress getIPAddress() const;

    /**
     * @brief Get assigned IP address as a formatted string.
     * @return String representing IPv4 address
     */
    String getIPAddressString() const;

    /**
     * @brief Get connected Wi-Fi SSID.
     * @return String SSID
     */
    String getSSID() const;

    /**
     * @brief Get current Wi-Fi signal strength in dBm.
     * @return int8_t RSSI
     */
    int8_t getRSSI() const;

    /**
     * @brief Get MAC address of the ESP32 STA interface.
     * @return String MAC address
     */
    String getMacAddress() const;

    /**
     * @brief Get current internal connection state.
     * @return State enum
     */
    State getState() const;

    /**
     * @brief Register callback for when Wi-Fi connection is established.
     * @param callback Function accepting const IPAddress&
     */
    void onConnected(ConnectedCallback callback);

    /**
     * @brief Register callback for when Wi-Fi is disconnected.
     * @param callback Void function
     */
    void onDisconnected(DisconnectedCallback callback);

    /**
     * @brief Explicitly disconnect from current Wi-Fi network.
     */
    void disconnect();

    /**
     * @brief Set timeout for connection attempt.
     * @param timeoutMs Timeout in milliseconds
     */
    void setConnectTimeout(uint32_t timeoutMs);

    /**
     * @brief Set retry interval before attempting reconnection.
     * @param intervalMs Retry interval in milliseconds
     */
    void setReconnectInterval(uint32_t intervalMs);

private:
    String _ssid;
    String _password;
    State _state;
    bool _isConnected;
    uint32_t _connectTimeoutMs;
    uint32_t _reconnectIntervalMs;
    unsigned long _lastAttemptTime;
    unsigned long _connectingStartTime;

    ConnectedCallback _onConnectedCallback;
    DisconnectedCallback _onDisconnectedCallback;

    void startConnection();
    void handleConnected();
    void handleDisconnected();
};
