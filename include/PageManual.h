#pragma once
#include "PageBase.h"
#include "PWMCore.h"
#include "Config.h"

// ─── Manual Centrifuge Page ────────────────────────────────────────────────
class PageManual : public PageBase {
public:
    void init(PWMCore* pwm, ConfigManager* cfg) {
        pwmCore = pwm;
        cfgMgr  = cfg;
    }

    void onEnter() override {
        active          = true;
        motorRunning    = false;
        selectedSpeed   = 0.0f;
        pwmCore->stop();
    }

    void onExit() override {
        active = false;
        motorRunning = false;
        pwmCore->stop();
    }

    void update(float encoderDelta, bool shortClick, bool longClick) override {
        if (!active) return;

        // Encoder changes selected speed: +encoder = clockwise = positive
        selectedSpeed += encoderDelta * 0.05f;
        if (selectedSpeed >  1.0f) selectedSpeed =  1.0f;
        if (selectedSpeed < -1.0f) selectedSpeed = -1.0f;

        if (shortClick) {
            motorRunning = !motorRunning;
            if (!motorRunning) {
                pwmCore->stop();
            }
        }

        if (motorRunning) {
            pwmCore->targetRotationSpeedPercentage = selectedSpeed;
        }
    }

    void draw(M5Canvas* canvas, int yOffset) override {
        int cx = DISP_CX, cy = DISP_CY + yOffset;

        canvas->setTextColor(COL_DIM, COL_BG);
        canvas->drawCentreString("MANUAL", cx, cy + 40, 4);

        // Speed dial arc
        int r = uiScale(50);

        float absSpd = fabsf(selectedSpeed);
        int   arcSpan = (int)(absSpd * 90.0f);
        if (arcSpan > 0) {
            uint16_t col = (selectedSpeed >= 0.0f) ? COL_ACCENT : COL_RED;
            // From top (270°) extend in direction
            if (selectedSpeed >= 0.0f) {
                canvas->fillArc(cx, cy, r, 0, 270, 270 + arcSpan, col);
            } else {
                canvas->fillArc(cx, cy, r, 0, 270 - arcSpan, 270, col);
            }
        }

        // Speed percent text
        char buf[16];
        int  pct = (int)(selectedSpeed * 100.0f);
        snprintf(buf, sizeof(buf), "%+d%%", pct);
        canvas->setTextColor(COL_TEXT);
        canvas->drawCentreString(buf, cx, cy - uiScale(10), 6);

        // State label
        canvas->setTextColor(motorRunning ? COL_GREEN : COL_DIM, COL_BG);
        canvas->drawCentreString(motorRunning ? "RUNNING" : "CLICK TO RUN", cx, cy + uiScale(50), 2);
    }

private:
    PWMCore*       pwmCore      = nullptr;
    ConfigManager* cfgMgr       = nullptr;
    bool           motorRunning = false;
    float          selectedSpeed = 0.0f;
};
