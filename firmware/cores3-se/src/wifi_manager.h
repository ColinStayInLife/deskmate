#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"
#include "secrets.h"

class WiFiManager {
public:
    using StateCallback = void (*)(DeskMateState);

    WiFiManager() : _stateCb(nullptr), _lastRetry(0), _retryCount(0) {}

    void begin(StateCallback cb = nullptr) {
        _stateCb = cb;
        WiFi.onEvent(_wifiEventHandler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
        WiFi.onEvent(_wifiEventHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        WiFi.onEvent(_wifiEventHandler, ARDUINO_EVENT_WIFI_STA_GOT_IP);
        connect();
    }

    void connect() {
        if (_isConnected) return;
        if (_stateCb) _stateCb(STATE_WIFI_CONNECTING);
        _retryCount = 0;
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    void loop() {
        if (_isConnected) return;

        // Auto-reconnect with backoff
        uint32_t now = millis();
        if (now - _lastRetry >= WIFI_RETRY_INTERVAL) {
            if (WIFI_RETRY_MAX == 0 || _retryCount < WIFI_RETRY_MAX) {
                Serial.printf("[WiFi] Retry %d...\n", _retryCount + 1);
                if (_stateCb) _stateCb(STATE_RECONNECTING);
                WiFi.reconnect();
                _lastRetry = now;
                _retryCount++;
            } else {
                if (_stateCb) _stateCb(STATE_ERROR);
                Serial.println("[WiFi] Max retries reached");
            }
        }
    }

    bool isConnected() const { return _isConnected; }
    IPAddress localIP() const { return WiFi.localIP(); }
    int rssi() const { return WiFi.RSSI(); }

private:
    StateCallback _stateCb;
    uint32_t _lastRetry;
    uint8_t _retryCount;
    bool _isConnected = false;

    static void _wifiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
        switch (event) {
            case ARDUINO_EVENT_WIFI_STA_CONNECTED:
                Serial.println("[WiFi] Connected to AP");
                break;
            case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
                Serial.printf("[WiFi] Got IP: %s\n", WiFi.localIP().toString().c_str());
                // Will be set via callback in loop
                break;
            }
            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
                Serial.println("[WiFi] Disconnected");
                // _isConnected cleared in loop
                break;
            default:
                break;
        }
    }

    // Non-static update called from loop
    void updateStatus() {
        _isConnected = (WiFi.status() == WL_CONNECTED);
    }

    friend void _wifiManagerLoopHelper(WiFiManager* self) {
        self->updateStatus();
    }
};

// Global instance
extern WiFiManager wifiManager;

#endif // WIFI_MANAGER_H
