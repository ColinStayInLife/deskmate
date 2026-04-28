#ifndef CONFIG_H
#define CONFIG_H

// ── System ──────────────────────────────────────────────────────
#define FW_NAME             "DeskMate"
#define FW_VERSION          "v0.1.0"
#define SERIAL_BAUD         115200

// ── WiFi ────────────────────────────────────────────────────────
#define WIFI_RETRY_MAX      10
#define WIFI_RETRY_INTERVAL 5000  // ms
#define WIFI_LED_GPIO       37    // CoreS3 SE green LED

// ── WebSocket ───────────────────────────────────────────────────
#define WS_RECONNECT_DELAY  3000  // ms
#define WS_HEARTBEAT_INTERVAL 30000 // ms
#define WS_RECONNECT_MAX_RETRIES 0 // infinite

// ── NTP ─────────────────────────────────────────────────────────
#define NTP_SERVER1         "pool.ntp.org"
#define NTP_SERVER2         "time.nist.gov"
#define NTP_UPDATE_INTERVAL 3600000 // 1 hour (ms)
#define TZ_OFFSET_SEC       28800   // UTC+8 (Asia/Shanghai)

// ── SD Card ─────────────────────────────────────────────────────
#define SD_CS_GPIO          4
#define SD_CONFIG_FILE      "/deskmate/config.json"
#define SD_LOG_DIR          "/deskmate/logs/"
#define SD_CACHE_DIR        "/deskmate/cache/"

// ── Audio (Phase 2) ─────────────────────────────────────────────
#define I2S_BCLK            6
#define I2S_LRC             5
#define I2S_DOUT            7
#define I2S_DIN             8       // ES7210 mic
#define AUDIO_SAMPLE_RATE   16000
#define AUDIO_BITS          16

// ── Display ─────────────────────────────────────────────────────
#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240
#define LVGL_BUF_SIZE       (SCREEN_WIDTH * 40) // lines
#define LVGL_TICK_PERIOD_MS 5

// ── UI ──────────────────────────────────────────────────────────
#define UI_REFRESH_INTERVAL 100   // ms
#define TOUCH_HIT_MIN       48    // minimum touch target (px)

// ── Debug ───────────────────────────────────────────────────────
// #define DEBUG_AUDIO
// #define DEBUG_WS_RAW

// ── State machine ───────────────────────────────────────────────
enum DeskMateState : uint8_t {
    STATE_INIT          = 0,
    STATE_WIFI_CONNECTING,
    STATE_WIFI_CONNECTED,
    STATE_WS_CONNECTING,
    STATE_WS_CONNECTED,
    STATE_READY,
    STATE_RECONNECTING,
    STATE_ERROR,
    STATE_DEEP_SLEEP
};

const char* stateToString(DeskMateState s) {
    switch(s) {
        case STATE_INIT:             return "INIT";
        case STATE_WIFI_CONNECTING:  return "WiFi Connecting";
        case STATE_WIFI_CONNECTED:   return "WiFi Connected";
        case STATE_WS_CONNECTING:    return "WS Connecting";
        case STATE_WS_CONNECTED:     return "WS Connected";
        case STATE_READY:            return "READY";
        case STATE_RECONNECTING:     return "Reconnecting";
        case STATE_ERROR:            return "ERROR";
        case STATE_DEEP_SLEEP:       return "Deep Sleep";
        default:                     return "UNKNOWN";
    }
}

#endif // CONFIG_H
