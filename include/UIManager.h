#pragma once
#include <M5Dial.h>
#include "DisplayDefs.h"
#include "Config.h"
#include "PWMCore.h"

// Forward declarations
class PageBase;

// ─── UI Manager ────────────────────────────────────────────────────────────
class UIManager {
public:
    static constexpr int PAGE_COUNT = 3;

    void begin(M5Canvas* canvas, PWMCore* pwm, ConfigManager* cfgMgr);
    void update(float encoderDelta, bool shortClick, bool longClick);
    void render();

    // Access current page index (0=Standard, 1=Manual, 2=Params)
    int currentPage() const { return activePage; }

private:
    M5Canvas*      canvas    = nullptr;
    PWMCore*       pwmCore   = nullptr;
    ConfigManager* cfgMgr    = nullptr;

    PageBase* pages[PAGE_COUNT] = {};
    int       activePage        = 0;
    bool      pageActive        = false; // user entered the page with a click

    // Sliding transition state
    int   slideFromPage  = 0;
    float slideProgress  = 1.0f; // 1.0 = done, <1.0 = animating
    bool  slideLeft      = true;
    static constexpr float SLIDE_SPEED = 0.15f; // per render call

    void drawHalfRing();
    void drawPageSelector(int pageIndex, int xOffset);
    void drawPageDots(int pageIndex);
    void renderSlide();
};
