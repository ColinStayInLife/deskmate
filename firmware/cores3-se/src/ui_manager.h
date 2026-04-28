#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include <M5CoreS3.h>
#include "config.h"

class UIManager {
public:
    using TouchCallback = void (*)(lv_event_t* e);

    UIManager() : _state(STATE_INIT), _initialized(false) {}

    void begin() {
        if (_initialized) return;

        // LVGL init
        lv_init();

        // Display buffer
        _buf1 = (lv_color_t*)heap_caps_malloc(
            SCREEN_WIDTH * LVGL_BUF_SIZE * sizeof(lv_color_t),
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        );
        _buf2 = (lv_color_t*)heap_caps_malloc(
            SCREEN_WIDTH * LVGL_BUF_SIZE * sizeof(lv_color_t),
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        );

        if (!_buf1 || !_buf2) {
            Serial.println("[UI] Failed to allocate LVGL buffer");
            return;
        }

        // Display driver
        lv_disp_draw_buf_init(&_drawBuf, _buf1, _buf2, SCREEN_WIDTH * LVGL_BUF_SIZE);

        static lv_disp_drv_t dispDrv;
        lv_disp_drv_init(&dispDrv);
        dispDrv.hor_res = SCREEN_WIDTH;
        dispDrv.ver_res = SCREEN_HEIGHT;
        dispDrv.flush_cb = _flushDisplay;
        dispDrv.draw_buf = &_drawBuf;
        lv_disp_drv_register(&dispDrv);

        // Touch driver
        static lv_indev_drv_t indevDrv;
        lv_indev_drv_init(&indevDrv);
        indevDrv.type = LV_INDEV_TYPE_POINTER;
        indevDrv.read_cb = _readTouch;
        lv_indev_drv_register(&indevDrv);

        _initialized = true;
        _buildMainScreen();
        _buildStatusOverlay();

        Serial.println("[UI] LVGL initialized");
    }

    void loop() {
        if (!_initialized) return;
        lv_timer_handler();
    }

    void setState(DeskMateState state) {
        _state = state;
        _updateStatusBar();
    }

    void setAgentStatus(const char* agent, const char* status, float progress) {
        // Update agent status display (Phase 1: simple label, Phase 3+ full console)
        if (_agentStatusLabel) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s: %s (%.0f%%)", agent, status, progress * 100);
            lv_label_set_text(_agentStatusLabel, buf);
        }
    }

    void setTime(const char* timeStr) {
        if (_timeLabel) {
            lv_label_set_text(_timeLabel, timeStr);
        }
    }

    void setWifiStrength(int rssi) {
        // Phase 1: simple text indicator
        if (_wifiLabel) {
            if (rssi > -50)      lv_label_set_text(_wifiLabel, LV_SYMBOL_WIFI " Strong");
            else if (rssi > -70) lv_label_set_text(_wifiLabel, LV_SYMBOL_WIFI " Good");
            else if (rssi > -85) lv_label_set_text(_wifiLabel, LV_SYMBOL_WIFI " Weak");
            else                 lv_label_set_text(_wifiLabel, LV_SYMBOL_WIFI " None");
        }
    }

    // Touch callback registration (for Phase 3 approval buttons)
    void onTouch(const char* btnName, TouchCallback cb) {
        // Stored callbacks would be registered here
        (void)btnName;
        (void)cb;
    }

private:
    DeskMateState _state;
    bool _initialized;
    lv_color_t *_buf1, *_buf2;
    lv_disp_draw_buf_t _drawBuf;

    // UI elements
    lv_obj_t *_mainScreen;
    lv_obj_t *_statusBar;
    lv_obj_t *_timeLabel;
    lv_obj_t *_wifiLabel;
    lv_obj_t *_stateLabel;
    lv_obj_t *_agentStatusLabel;
    lv_obj_t *_contentArea;

    // ── Main screen ─────────────────────────────────────────────
    void _buildMainScreen() {
        _mainScreen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(_mainScreen, lv_color_hex(0x1a1b26), 0);

        // Title bar
        lv_obj_t* title = lv_label_create(_mainScreen);
        lv_label_set_text(title, "DeskMate");
        lv_obj_set_style_text_color(title, lv_color_hex(0x7aa2f7), 0);
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 4);

        // Content area (for agent status, notifications, etc.)
        _contentArea = lv_obj_create(_mainScreen);
        lv_obj_set_size(_contentArea, SCREEN_WIDTH - 16, 140);
        lv_obj_set_style_bg_color(_contentArea, lv_color_hex(0x24283b), 0);
        lv_obj_set_style_border_color(_contentArea, lv_color_hex(0x565f89), 0);
        lv_obj_align(_contentArea, LV_ALIGN_TOP_LEFT, 8, 30);

        // Agent status label
        _agentStatusLabel = lv_label_create(_contentArea);
        lv_label_set_text(_agentStatusLabel, "Waiting for Agent...");
        lv_obj_set_style_text_color(_agentStatusLabel, lv_color_hex(0xc0caf5), 0);
        lv_obj_align(_agentStatusLabel, LV_ALIGN_TOP_LEFT, 8, 8);

        // Default placeholder
        lv_obj_t* placeholder = lv_label_create(_contentArea);
        lv_label_set_text(placeholder,
            "No active tasks.\n"
            "Speak or tap to start.");
        lv_obj_set_style_text_color(placeholder, lv_color_hex(0x565f89), 0);
        lv_obj_align(placeholder, LV_ALIGN_CENTER, 0, 10);

        // Bottom status bar
        _statusBar = lv_obj_create(_mainScreen);
        lv_obj_set_size(_statusBar, SCREEN_WIDTH, 24);
        lv_obj_set_style_bg_color(_statusBar, lv_color_hex(0x1f2335), 0);
        lv_obj_set_style_border_width(_statusBar, 0);
        lv_obj_align(_statusBar, LV_ALIGN_BOTTOM_LEFT, 0, 0);

        // Time
        _timeLabel = lv_label_create(_statusBar);
        lv_label_set_text(_timeLabel, "--:--");
        lv_obj_set_style_text_color(_timeLabel, lv_color_hex(0x565f89), 0);
        lv_obj_align(_timeLabel, LV_ALIGN_LEFT_MID, 8, 0);

        // WiFi
        _wifiLabel = lv_label_create(_statusBar);
        lv_label_set_text(_wifiLabel, LV_SYMBOL_WIFI " ...");
        lv_obj_set_style_text_color(_wifiLabel, lv_color_hex(0x565f89), 0);
        lv_obj_align(_wifiLabel, LV_ALIGN_RIGHT_MID, -8, 0);

        lv_scr_load(_mainScreen);
    }

    // ── Status overlay ──────────────────────────────────────────
    void _buildStatusOverlay() {
        // Small colored indicator in the title bar area
        _stateLabel = lv_label_create(_mainScreen);
        lv_label_set_text(_stateLabel, LV_SYMBOL_STOP);
        lv_obj_set_style_text_color(_stateLabel, lv_color_hex(0xf7768e), 0);
        lv_obj_align(_stateLabel, LV_ALIGN_TOP_RIGHT, -8, 4);
    }

    // ── Update status bar ───────────────────────────────────────
    void _updateStatusBar() {
        uint32_t color;
        const char* symbol;

        switch (_state) {
            case STATE_INIT:
                color = 0x565f89; symbol = LV_SYMBOL_SETTINGS; break;
            case STATE_WIFI_CONNECTING:
            case STATE_WS_CONNECTING:
            case STATE_RECONNECTING:
                color = 0xe0af68; symbol = LV_SYMBOL_REFRESH; break;
            case STATE_WIFI_CONNECTED:
            case STATE_WS_CONNECTED:
                color = 0x7dcfff; symbol = LV_SYMBOL_CHARGE; break;
            case STATE_READY:
                color = 0x9ece6a; symbol = LV_SYMBOL_OK; break;
            case STATE_ERROR:
                color = 0xf7768e; symbol = LV_SYMBOL_CLOSE; break;
            case STATE_DEEP_SLEEP:
                color = 0x565f89; symbol = LV_SYMBOL_SLEEP; break;
            default:
                color = 0x565f89; symbol = "?"; break;
        }

        if (_stateLabel) {
            lv_label_set_text(_stateLabel, symbol);
            lv_obj_set_style_text_color(_stateLabel, lv_color_hex(color), 0);
        }
    }

    // ── Display flush ───────────────────────────────────────────
    static void _flushDisplay(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
        uint32_t w = area->x2 - area->x1 + 1;
        uint32_t h = area->y2 - area->y1 + 1;

        M5.Display.startWrite();
        M5.Display.setAddrWindow(area->x1, area->y1, w, h);
        M5.Display.writePixels((uint16_t*)color_p, w * h);
        M5.Display.endWrite();

        lv_disp_flush_ready(drv);
    }

    // ── Touch read ─────────────────────────────────────────────
    static void _readTouch(lv_indev_drv_t* drv, lv_indev_data_t* data) {
        static int16_t lastX = 0, lastY = 0;
        static bool lastPressed = false;

        bool touched = M5.Touch.getCount() > 0;
        if (touched) {
            auto t = M5.Touch.getDetail();
            lastX = t.x;
            lastY = t.y;
        }

        data->point.x = lastX;
        data->point.y = lastY;
        data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    }
};

// Global instance
extern UIManager uiManager;

#endif // UI_MANAGER_H
