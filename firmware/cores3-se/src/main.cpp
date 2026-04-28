/*
 * DeskMate — CoreS3 SE Firmware Main Entry
 *
 * Phase 1: LVGL UI + WiFi + WebSocket + Agent Status Display
 * Phase 2+: Voice, Approvals, Notifications (stubs in place)
 *
 * Architecture:
 *   main.cpp orchestrates all managers via global instances.
 *   Each manager has begin() + loop() pattern.
 *   State transitions are managed centrally here.
 *
 * Pin map (CoreS3 SE):
 *   I2S_BCLK=6, I2S_LRC=5, I2S_DOUT=7, I2S_DIN=8
 *   SD_CS=4,  LED=37,  Touch=I2C (internal)
 */

#include <Arduino.h>
#include <M5CoreS3.h>

#include "config.h"
#include "wifi_manager.h"
#include "websocket_client.h"
#include "ui_manager.h"
#include "audio_manager.h"
#include "message_handler.h"
#include "system_manager.h"

// ── Global instances ────────────────────────────────────────────────
WiFiManager      wifiManager;
WebSocketManager wsManager;
UIManager        uiManager;
AudioManager     audioManager;
MessageHandler   messageHandler;
SystemManager    systemManager;

// ── Forward declarations ─────────────────────────────────────────────
void onWSMessage(const char* type, JsonDocument& payload);
void onStateChange(DeskMateState newState);
void onApprovalReceived(const char* taskId, bool approved);

// ── Setup ────────────────────────────────────────────────────────────
void setup() {
    // CoreS3 hardware init (display, I2C, touch, PMU)
    auto cfg = M5.config();
    cfg.external_spk = true;  // Enable external speaker (NS4168)
    M5.begin(cfg);

    // Screen brightness
    M5.Display.setBrightness(120);

    // Initialize managers
    systemManager.begin();
    uiManager.begin();
    audioManager.begin();
    wifiManager.begin(onStateChange);

    // WebSocket — callbacks route to messageHandler
    wsManager.begin(onWSMessage, onStateChange);
    messageHandler.begin(onApprovalReceived);

    // Initial state
    onStateChange(STATE_INIT);

    Serial.printf("[DM] DeskMate %s ready — waiting for WiFi\n", FW_VERSION);
    uiManager.setState(STATE_WIFI_CONNECTING);
}

// ── Main loop ────────────────────────────────────────────────────────
void loop() {
    static uint32_t lastUiRefresh = 0;
    uint32_t now = millis();

    // Update subsystems (always)
    wifiManager.loop();
    wsManager.loop();
    systemManager.loop();

    // Audio pipeline (Phase 2)
    audioManager.loop();

    // UI refresh (throttled)
    if (now - lastUiRefresh >= UI_REFRESH_INTERVAL) {
        uiManager.loop();
        lastUiRefresh = now;
    }

    // Periodic status updates
    static uint32_t lastStatus = 0;
    if (now - lastStatus >= 5000) {
        _updateWiFiStatus();
        lastStatus = now;
    }

    // Handle button B (middle button on CoreS3 SE)
    M5.update();
    if (M5.BtnB.wasPressed()) {
        Serial.println("[DM] Button B pressed — sending test command");
        wsManager.sendCommand("test-agent", "ping");
    }

    // Handle button A (left) — toggle status display
    if (M5.BtnA.wasPressed()) {
        Serial.println("[DM] Button A — toggle debug info");
        // Future: toggle between status views
    }
}

// ── Callbacks ────────────────────────────────────────────────────────

void onWSMessage(const char* type, JsonDocument& payload) {
    messageHandler.handleMessage(type, payload);
}

void onStateChange(DeskMateState newState) {
    uiManager.setState(newState);

    // State transition logic
    switch (newState) {
        case STATE_WIFI_CONNECTED:
            // WiFi ready, now connect WebSocket
            wsManager.connect();
            break;

        case STATE_WS_CONNECTED:
            // WebSocket ready, full system ready
            uiManager.setState(STATE_READY);
            Serial.println("[DM] System READY");
            break;

        case STATE_RECONNECTING:
            // Something disconnected, show warning
            Serial.println("[DM] Reconnecting...");
            break;

        case STATE_ERROR:
            Serial.println("[DM] ERROR — check serial for details");
            break;

        default:
            break;
    }
}

void onApprovalReceived(const char* taskId, bool approved) {
    // Phase 3: forward to messageHandler for response
    messageHandler.onApprovalResponse(taskId, approved);
}

// ── Internal helpers ─────────────────────────────────────────────────

static void _updateWiFiStatus() {
    if (wifiManager.isConnected()) {
        uiManager.setWifiStrength(wifiManager.rssi());

        char buf[8];
        if (systemManager.getFormattedTime(buf, sizeof(buf))) {
            uiManager.setTime(buf);
        }
    }
}

// ── Globals needed by Arduino framework ──────────────────────────────
// (setup() and loop() are already the entry points)
