#pragma once
#include "PageBase.h"
#include "Config.h"

// ─── Parameter Editor Page ─────────────────────────────────────────────────
struct ParamDef {
    const char* name;
    float       minVal;
    float       maxVal;
    float       step;
    float AppConfig::* field;
};

static const ParamDef PARAMS[] = {
    { "SPINUP TIME (s)",  1.0f,  30.0f, 0.5f, &AppConfig::zeroToMaxSpinupTime },
    { "MAX DUTY CYCLE",   0.1f,   1.0f, 0.01f,&AppConfig::maxDutyCycle        },
    { "PROG DURATION(s)", 30.0f,300.0f, 5.0f, &AppConfig::programDuration     },
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

        canvas->setTextColor(COL_DIM, COL_BG);
        canvas->drawCentreString("PARAMETERS", cx, cy - 70, 2);

        // Parameter name
        const ParamDef& p = PARAMS[paramIndex];
        canvas->setTextColor(COL_ACCENT, COL_BG);
        canvas->drawCentreString(p.name, cx, cy - 40, 1);

        // Value
        float val = editCfg.*(p.field);
        char buf[32];
        if (p.step < 0.1f) {
            snprintf(buf, sizeof(buf), "%.2f", val);
        } else {
            snprintf(buf, sizeof(buf), "%.1f", val);
        }
        canvas->setTextColor(COL_TEXT, COL_BG);
        canvas->drawCentreString(buf, cx, cy - 14, 4);

        // Param index dots
        for (int i = 0; i < PARAM_COUNT; i++) {
            uint16_t c = (i == paramIndex) ? COL_ACCENT : COL_DIM;
            canvas->fillCircle(cx - (PARAM_COUNT - 1) * 8 + i * 16, cy + 45, 4, c);
        }

        canvas->setTextColor(COL_DIM, COL_BG);
        canvas->drawCentreString("LONG CLICK TO SAVE", cx, cy + 62, 1);
    }

private:
    ConfigManager* cfgMgr     = nullptr;
    int            paramIndex = 0;
    AppConfig      editCfg;
};
