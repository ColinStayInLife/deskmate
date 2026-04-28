#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "websocket_client.h"
#include "ui_manager.h"
#include "audio_manager.h"

class MessageHandler {
public:
    using ApprovalCallback = void (*)(const char* taskId, bool approved);

    MessageHandler() : _approvalCb(nullptr) {}

    void begin(ApprovalCallback cb = nullptr) {
        _approvalCb = cb;
    }

    // Called when a WebSocket message arrives
    void handleMessage(const char* type, JsonDocument& doc) {
        if (strcmp(type, "agent:status") == 0) {
            _handleAgentStatus(doc);
        } else if (strcmp(type, "agent:approval") == 0) {
            _handleApproval(doc);
        } else if (strcmp(type, "agent:result") == 0) {
            _handleResult(doc);
        } else if (strcmp(type, "voice:response") == 0) {
            _handleVoiceResponse(doc);
        } else if (strcmp(type, "notification:alert") == 0) {
            _handleNotification(doc);
        } else if (strcmp(type, "system:config") == 0) {
            _handleConfig(doc);
        } else if (strcmp(type, "pong") == 0) {
            // Heartbeat response, ignore
        } else {
            Serial.printf("[MSG] Unknown type: %s\n", type);
        }
    }

    // ── UI callbacks (called from main.cpp on touch events) ─────
    void onApprovalResponse(const char* taskId, bool approved) {
        StaticJsonDocument<256> payload;
        payload["task_id"] = taskId;
        payload["approved"] = approved;
        wsManager.send("agent:approval_response", payload);
    }

private:
    ApprovalCallback _approvalCb;

    void _handleAgentStatus(JsonDocument& doc) {
        JsonObject payload = doc["payload"];
        const char* agent = payload["agent"] | "unknown";
        const char* status = payload["status"] | "unknown";
        float progress = payload["progress"] | 0.0f;

        Serial.printf("[MSG] Agent %s: %s (%.0f%%)\n", agent, status, progress * 100);
        uiManager.setAgentStatus(agent, status, progress);
    }

    void _handleApproval(JsonDocument& doc) {
        JsonObject payload = doc["payload"];
        const char* taskId = payload["task_id"] | "unknown";
        const char* title = payload["title"] | "Approve?";
        const char* agent = payload["agent"] | "unknown";

        Serial.printf("[MSG] Approval needed: [%s] %s\n", taskId, title);

        // Phase 3: display approval dialog on screen
        // For now, log it
        if (_approvalCb) {
            _approvalCb(taskId, false); // default: don't auto-approve
        }
    }

    void _handleResult(JsonDocument& doc) {
        JsonObject payload = doc["payload"];
        const char* taskId = payload["task_id"] | "unknown";
        const char* summary = payload["summary"] | "Task completed";
        bool success = payload["success"] | true;

        Serial.printf("[MSG] Result [%s]: %s (%s)\n",
            taskId, summary, success ? "OK" : "FAIL");

        // Phase 3: show result notification
    }

    void _handleVoiceResponse(JsonDocument& doc) {
        JsonObject payload = doc["payload"];
        const char* text = payload["text"] | "";
        const char* audioUrl = payload["audio_url"] | "";

        Serial.printf("[MSG] Voice response: %s\n", text);

        if (strlen(audioUrl) > 0) {
            // Phase 2: download and play audio
        } else if (strlen(text) > 0) {
            // Phase 2: use TTS to speak the text
            audioManager.playTTS(text);
        }
    }

    void _handleNotification(JsonDocument& doc) {
        JsonObject payload = doc["payload"];
        const char* source = payload["source"] | "system";
        const char* message = payload["message"] | "";
        int severity = payload["severity"] | 0; // 0=info, 1=warning, 2=critical

        Serial.printf("[MSG] Notification [%s] (%d): %s\n", source, severity, message);

        // Phase 4: display notification popup + TTS
    }

    void _handleConfig(JsonDocument& doc) {
        JsonObject payload = doc["payload"];
        Serial.printf("[MSG] Config update received\n");

        // Apply configuration changes
        if (payload.containsKey("wifi_ssid")) {
            // TODO: save to NVS and reconnect
        }
        if (payload.containsKey("ws_url")) {
            // TODO: reconnect with new URL
        }
    }
};

// Global instance
extern MessageHandler messageHandler;

#endif // MESSAGE_HANDLER_H
