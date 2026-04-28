#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Phase 2 placeholder — audio pipeline will be implemented here

class AudioManager {
public:
    AudioManager() : _initialized(false), _isRecording(false) {}

    void begin() {
        if (_initialized) return;

        // ── Speaker I2S config (MAX98357) ───────────────────────
        // Pin mapping:
        //   I2S_BCLK  = 6
        //   I2S_LRC   = 5
        //   I2S_DOUT  = 7   (data out to speaker)
        //
        // ESP8266Audio library will be used in Phase 2:
        //   AudioOutputI2S* out = new AudioOutputI2S();
        //   out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

        // ── Microphone I2S config (ES7210) ──────────────────────
        // ES7210 is the dual-mic ADC on CoreS3 SE
        //   I2S_DIN   = 8   (data in from mic)
        //
        // M5CoreS3 library provides:
        //   M5.Mic.begin() — initializes ES7210
        //   M5.Mic.record() — captures audio

        Serial.println("[Audio] I2S pins reserved for Phase 2");
        _initialized = true;
    }

    // Phase 2 methods (stubs)
    bool startRecording() {
        if (_isRecording) return true;
        Serial.println("[Audio] Record start — Phase 2");
        _isRecording = true;
        return true;
    }

    bool stopRecording() {
        if (!_isRecording) return true;
        Serial.println("[Audio] Record stop — Phase 2");
        _isRecording = false;
        return true;
    }

    bool isRecording() const { return _isRecording; }

    bool playTTS(const char* text) {
        // Phase 2: receive TTS audio data and play via I2S
        Serial.printf("[Audio] TTS play (stub): %s\n", text);
        return true;
    }

    void loop() {
        // Phase 2: stream microphone data to WebSocket
        if (_isRecording) {
            // TODO: read from I2S mic buffer, send as voice:query
        }
    }

private:
    bool _initialized;
    bool _isRecording;
};

// Global instance
extern AudioManager audioManager;

#endif // AUDIO_MANAGER_H
