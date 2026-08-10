#pragma once
#include <M5Dial.h>

// ─── Display geometry ──────────────────────────────────────────────────────
static constexpr int   DISP_W          = 240;
static constexpr int   DISP_H          = 240;
static constexpr int   DISP_CX         = DISP_W / 2;
static constexpr int   DISP_CY         = DISP_H / 2;
static constexpr int   RING_RADIUS     = 108;
static constexpr int   ARC_WIDTH       = 4;
static constexpr int   TARGET_DOT_R    = 3;
static constexpr float UI_SCALE        = 1.5f;

inline int uiScale(int value) {
    return (int)(value * UI_SCALE + 0.5f);
}

static constexpr int   PAGE_TITLE_Y_OFFSET = -54;

static constexpr float ARC_ZERO_DEG    = 270.0f;

inline float speedToAngle(float pct) {
    pct = constrain(pct, -1.0f, 1.0f);
    return ARC_ZERO_DEG + pct * 90.0f;
}

// ─── Color palette ─────────────────────────────────────────────────────────
static constexpr uint16_t COL_BG           = TFT_BLACK;
static constexpr uint16_t COL_ARC_TRACK    = 0x2104;
static constexpr uint16_t COL_ARC_APPLIED  = TFT_ORANGE;
static constexpr uint16_t COL_TARGET_DOT   = TFT_WHITE;
static constexpr uint16_t COL_TEXT         = TFT_WHITE;
static constexpr uint16_t COL_ACCENT       = TFT_ORANGE;
static constexpr uint16_t COL_DIM          = 0x7BEF;
static constexpr uint16_t COL_GREEN        = TFT_GREEN;
static constexpr uint16_t COL_DARK_GREEN   = 0x03E0;
static constexpr uint16_t COL_RED          = 0xF800;
