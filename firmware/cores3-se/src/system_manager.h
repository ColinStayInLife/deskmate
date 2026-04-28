#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>
#include "config.h"
#include "wifi_manager.h"
#include "ui_manager.h"

class SystemManager {
public:
    SystemManager() : _initialized(false), _lastNtpSync(0) {}

    void begin() {
        if (_initialized) return;

        Serial.begin(SERIAL_BAUD);
        Serial.printf("\n\n=== DeskMate %s ===\n", FW_VERSION);

        _initSDCard();
        _initNTP();

        _initialized = true;
    }

    void loop() {
        if (!_initialized) return;

        // Periodic NTP sync
        uint32_t now = millis();
        if (now - _lastNtpSync >= NTP_UPDATE_INTERVAL && wifiManager.isConnected()) {
            _syncNTP();
            _lastNtpSync = now;
        }

        // Update time display every loop
        _updateTimeDisplay();
    }

    bool getFormattedTime(char* buf, size_t len) {
        if (!_timeSynced) {
            snprintf(buf, len, "--:--");
            return false;
        }

        time_t now = time(nullptr);
        struct tm* ti = localtime(&now);

        // Format: HH:MM
        strftime(buf, len, "%H:%M", ti);
        return true;
    }

    bool getFormattedDateTime(char* buf, size_t len) {
        if (!_timeSynced) {
            snprintf(buf, len, "--:--");
            return false;
        }

        time_t now = time(nullptr);
        struct tm* ti = localtime(&now);
        strftime(buf, len, "%m/%d %H:%M", ti);
        return true;
    }

    bool sdAvailable() const {
        return _sdAvailable;
    }

    // ── OTA (Phase 5) ───────────────────────────────────────────
    void beginOTA(const char* hostname = "deskmate") {
        // ArduinoOTA will be initialized here in Phase 5
        Serial.printf("[OTA] Reserved for Phase 5 (hostname: %s)\n", hostname);
    }

private:
    bool _initialized;
    bool _sdAvailable = false;
    bool _timeSynced = false;
    uint32_t _lastNtpSync;

    void _initSDCard() {
        // SD card on CoreS3 SE uses GPIO 4 (SPI mode)
        SPI.begin(SCK, MISO, MOSI, SD_CS_GPIO);
        if (!SD.begin(SD_CS_GPIO, SPI)) {
            Serial.println("[SD] Card mount failed");
            _sdAvailable = false;
            return;
        }

        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("[SD] Card found: %llu MB\n", cardSize);
        _sdAvailable = true;

        // Ensure directories exist
        SD.mkdir("/deskmate");
        SD.mkdir("/deskmate/logs");
        SD.mkdir("/deskmate/cache");
    }

    void _initNTP() {
        if (!wifiManager.isConnected()) return;
        _syncNTP();
    }

    void _syncNTP() {
        configTime(TZ_OFFSET_SEC, 0, NTP_SERVER1, NTP_SERVER2);

        // Wait for NTP sync (non-blocking)
        time_t now = time(nullptr);
        if (now < 100000) {
            Serial.println("[NTP] Waiting for sync...");
        } else {
            _timeSynced = true;
            char buf[32];
            getFormattedDateTime(buf, sizeof(buf));
            Serial.printf("[NTP] Synced: %s\n", buf);
            uiManager.setState(STATE_READY);
        }

        _lastNtpSync = millis();
    }

    void _updateTimeDisplay() {
        char buf[8];
        if (getFormattedTime(buf, sizeof(buf))) {
            uiManager.setTime(buf);
        }
    }
};

// Global instance
extern SystemManager systemManager;

#endif // SYSTEM_MANAGER_H
