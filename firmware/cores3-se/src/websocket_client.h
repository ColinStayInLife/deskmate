#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "secrets.h"

class WebSocketManager {
public:
    using MessageCallback = void (*)(const char* type, JsonDocument& payload);
    using StateCallback = void (*)(DeskMateState);

    WebSocketManager() : _msgCb(nullptr), _stateCb(nullptr), _lastHeartbeat(0) {}

    void begin(MessageCallback msgCb, StateCallback stateCb) {
        _msgCb = msgCb;
        _stateCb = stateCb;
        connect();
    }

    void connect() {
        if (_ws.isConnected()) return;
        if (_stateCb) _stateCb(STATE_WS_CONNECTING);

        _ws.begin(WS_SERVER, WS_PORT, WS_PATH);
        _ws.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
            _onEvent(type, payload, length);
        });
        _ws.setReconnectInterval(WS_RECONNECT_DELAY);
        Serial.printf("[WS] Connecting to ws://%s:%d%s\n", WS_SERVER, WS_PORT, WS_PATH);
    }

    void loop() {
        _ws.loop();

        // Heartbeat
        uint32_t now = millis();
        if (_ws.isConnected() && now - _lastHeartbeat >= WS_HEARTBEAT_INTERVAL) {
            sendHeartbeat();
            _lastHeartbeat = now;
        }
    }

    bool isConnected() const {
        return _ws.isConnected();
    }

    void send(const char* type, JsonDocument& payload) {
        if (!_ws.isConnected()) {
            Serial.println("[WS] Not connected, can't send");
            return;
        }

        StaticJsonDocument<512> doc;
        doc["type"] = type;
        doc["id"] = _nextMsgId();
        doc["payload"] = payload;
        doc["timestamp"] = _timestamp();

        char buffer[1024];
        size_t len = serializeJson(doc, buffer);
        _ws.sendTXT(buffer, len);

#ifdef DEBUG_WS_RAW
        Serial.printf("[WS] TX (%s): %s\n", type, buffer);
#endif
    }

    void sendCommand(const char* agent, const char* action, const char* params = "") {
        StaticJsonDocument<256> payload;
        payload["agent"] = agent;
        payload["action"] = action;
        payload["params"] = params;
        send("agent:command", payload);
    }

private:
    WebSocketsClient _ws;
    MessageCallback _msgCb;
    StateCallback _stateCb;
    uint32_t _lastHeartbeat;
    uint32_t _msgCounter = 0;

    void _onEvent(WStype_t type, uint8_t* payload, size_t length) {
        switch (type) {
            case WStype_DISCONNECTED:
                Serial.println("[WS] Disconnected");
                if (_stateCb) _stateCb(STATE_RECONNECTING);
                break;

            case WStype_CONNECTED:
                Serial.printf("[WS] Connected to: %s\n", (char*)payload);
                if (_stateCb) _stateCb(STATE_WS_CONNECTED);
                _lastHeartbeat = millis();
                break;

            case WStype_TEXT: {
#ifdef DEBUG_WS_RAW
                Serial.printf("[WS] RX: %s\n", (char*)payload);
#endif
                _handleMessage((char*)payload);
                break;
            }

            case WStype_ERROR:
                Serial.printf("[WS] Error: %s\n", (char*)payload);
                break;

            case WStype_PING:
                // WebSockets library handles pong automatically
                break;

            case WStype_PONG:
                break;

            default:
                break;
        }
    }

    void _handleMessage(const char* data) {
        StaticJsonDocument<2048> doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err) {
            Serial.printf("[WS] JSON parse error: %s\n", err.c_str());
            return;
        }

        const char* type = doc["type"];
        if (!type) {
            Serial.println("[WS] Message missing type");
            return;
        }

        if (_msgCb) {
            _msgCb(type, doc);
        }
    }

    void sendHeartbeat() {
        _ws.sendTXT("{\"type\":\"ping\"}");
    }

    String _nextMsgId() {
        char buf[16];
        snprintf(buf, sizeof(buf), "dm_%04u", _msgCounter++);
        return String(buf);
    }

    String _timestamp() {
        // Simplified — NTP time will be used when available
        return String(millis() / 1000);
    }
};

// Global instance
extern WebSocketManager wsManager;

#endif // WEBSOCKET_CLIENT_H
