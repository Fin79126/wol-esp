#pragma once

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <functional>

/**
 * @brief WebSocket client manager for ESP32
 * Handles persistent connection, reconnects, heartbeat, and message dispatch.
 */
class WebSocketManager {
public:
    using MessageCallback = std::function<void(const String& message)>;
    using ConnectedCallback = std::function<void()>;
    using DisconnectedCallback = std::function<void()>;
    using BinaryCallback = std::function<void(const uint8_t* payload, size_t length)>;

    WebSocketManager();
    ~WebSocketManager();

    /**
     * @brief Initialize and start WebSocket client connection
     * @param host WebSocket server hostname or IP address
     * @param port WebSocket server port (e.g. 8080)
     * @param path WebSocket endpoint path (default: "/")
     * @param protocol Subprotocol (optional, default: nullptr)
     */
    void init(const char* host, uint16_t port, const char* path = "/", const char* protocol = nullptr);

    /**
     * @brief Update loop for WebSocket client. Should be called periodically in Arduino loop().
     */
    void update();

    /**
     * @brief Register callback for incoming text messages
     * @param callback Function accepting const String& message
     */
    void onMessage(MessageCallback callback);

    /**
     * @brief Register callback for connection established event
     * @param callback Function to call when connected
     */
    void onConnected(ConnectedCallback callback);

    /**
     * @brief Register callback for disconnection event
     * @param callback Function to call when disconnected
     */
    void onDisconnected(DisconnectedCallback callback);

    /**
     * @brief Register callback for binary message received
     * @param callback Function accepting const uint8_t* payload, size_t length
     */
    void onBinary(BinaryCallback callback);

    /**
     * @brief Send text message to WebSocket server
     * @param message Text string to send
     * @return true if successfully queued/sent, false if disconnected or failed
     */
    bool sendText(const String& message);

    /**
     * @brief Send binary data to WebSocket server
     * @param payload Pointer to byte buffer
     * @param length Buffer length in bytes
     * @return true if successfully queued/sent, false if disconnected or failed
     */
    bool sendBinary(const uint8_t* payload, size_t length);

    /**
     * @brief Check if WebSocket client is currently connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Set reconnect interval in milliseconds
     * @param ms Reconnect interval (default 5000ms)
     */
    void setReconnectInterval(uint32_t ms);

    /**
     * @brief Enable ping/pong heartbeat
     * @param pingInterval Interval in ms between ping packets (default 15000ms)
     * @param pongTimeout Timeout in ms to wait for pong response (default 3000ms)
     * @param disconnectTimeouts Number of failed timeouts before disconnecting (default 2)
     */
    void enableHeartbeat(uint32_t pingInterval = 15000, uint32_t pongTimeout = 3000, uint8_t disconnectTimeouts = 2);

    /**
     * @brief Disconnect client from server
     */
    void disconnect();

private:
    WebSocketsClient _client;
    bool _connected;
    String _host;
    uint16_t _port;
    String _path;

    MessageCallback _onMessageCallback;
    ConnectedCallback _onConnectedCallback;
    DisconnectedCallback _onDisconnectedCallback;
    BinaryCallback _onBinaryCallback;

    static WebSocketManager* _instance;
    static void handleWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    void handleEvent(WStype_t type, uint8_t* payload, size_t length);
};
