#pragma once
#include "PageBase.h"
#include "Config.h"

// ─── Parameter Editor Page ─────────────────────────────────────────────────
struct ParamDef {
    const char* name;
    const char* unit;
    float       minVal;
    float       maxVal;
    float       step;
    float AppConfig::* field;
};

static const ParamDef PARAMS[] = {
    { "SPINUP TIME",    "s", 1.0f,  30.0f, 0.5f,  &AppConfig::zeroToMaxSpinupTime },
    { "MAX DUTY CYCLE", "%", 0.1f,   1.0f, 0.01f, &AppConfig::maxDutyCycle        },
    { "PROG DURATION",  "s", 30.0f, 300.0f, 5.0f, &AppConfig::programDuration     },
};
static constexpr int PARAM_COUNT = 3;

class PageParams : public PageBase {
public:
    void init(ConfigManager* cfg) { cfgMgr = cfg; }

    void onEnter() override {
        active     = true;
        paramIndex = 0;
        // Copy config to editing buffer
        editCfg = cfgMgr->config;
    }

    void onExit() override {
        active = false;
        // Save edited config
        cfgMgr->config = editCfg;
        cfgMgr->save();
    }

    void update(float encoderDelta, bool shortClick, bool longClick) override {
        if (!active) return;

        // Encoder changes value of current param
        const ParamDef& p = PARAMS[paramIndex];
        float& val = editCfg.*(p.field);
        val += encoderDelta * p.step;
        if (val < p.minVal) val = p.minVal;
        if (val > p.maxVal) val = p.maxVal;

        // Short click advances to next parameter
        if (shortClick) {
            paramIndex = (paramIndex + 1) % PARAM_COUNT;
        }
    }

    void draw(M5Canvas* canvas, int yOffset) override {
        int cx = DISP_CX, cy = DISP_CY + yOffset;
        int nameY  = cy - uiScale(40);
        int valueY = cy - uiScale(12);
        int dotsY  = cy + uiScale(42);
        int helpY  = cy + uiScale(66);

        // Parameter name
        const ParamDef& p = PARAMS[paramIndex];
        canvas->setTextColor(COL_ACCENT, COL_BG);
        canvas->drawCentreString(p.name, cx, nameY, 4);

        // Value
        float val = editCfg.*(p.field);
        char valueBuf[24];
        char unitBuf[12];
        if (p.field == &AppConfig::maxDutyCycle) {
            snprintf(valueBuf, sizeof(valueBuf), "%d", (int)(val * 100.0f + 0.5f));
        } else if (p.step < 0.1f) {
            snprintf(valueBuf, sizeof(valueBuf), "%.2f", val);
        } else {
            snprintf(valueBuf, sizeof(valueBuf), "%.1f", val);
        }
        snprintf(unitBuf, sizeof(unitBuf), "%s", p.unit);

        int unitGap = uiScale(8);
        canvas->setTextFont(6);
        int valueWidth = canvas->textWidth(valueBuf);
        canvas->setTextFont(4);
        int unitWidth = canvas->textWidth(unitBuf);
        int leftX = cx - (valueWidth + unitGap + unitWidth) / 2;

        canvas->setTextColor(COL_TEXT, COL_BG);
        canvas->drawString(valueBuf, leftX, valueY, 6);
        canvas->drawString(unitBuf, leftX + valueWidth + unitGap, valueY + uiScale(10), 4);

        // Param index dots
        for (int i = 0; i < PARAM_COUNT; i++) {
            uint16_t c = (i == paramIndex) ? COL_ACCENT : COL_DIM;
            canvas->fillCircle(cx - (PARAM_COUNT - 1) * uiScale(10) + i * uiScale(20), dotsY, uiScale(5), c);
        }

        canvas->setTextColor(COL_DIM, COL_BG);
        canvas->drawCentreString("LONG CLICK TO SAVE", cx, helpY, 1);
    }

private:
    ConfigManager* cfgMgr     = nullptr;
    int            paramIndex = 0;
    AppConfig      editCfg;
};
