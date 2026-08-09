#include "UIManager.h"
#include "PageBase.h"
#include "PageStandard.h"
#include "PageManual.h"
#include "PageParams.h"

// Page name labels shown in the page selector (not entered)
static const char* PAGE_NAMES[UIManager::PAGE_COUNT] = {
    "STANDARD",
    "MANUAL",
    "PARAMS",
};

static PageStandard pageStd;
static PageManual   pageMan;
static PageParams   pagePrm;

void UIManager::begin(M5Canvas* cvs, PWMCore* pwm, ConfigManager* cfgMgr_) {
    canvas   = cvs;
    pwmCore  = pwm;
    cfgMgr   = cfgMgr_;

    pageStd.init(pwm, cfgMgr_);
    pageMan.init(pwm, cfgMgr_);
    pagePrm.init(cfgMgr_);

    pages[0] = &pageStd;
    pages[1] = &pageMan;
    pages[2] = &pagePrm;

    activePage    = 0;
    pageActive    = false;
    slideProgress = 1.0f;
}

void UIManager::update(float encoderDelta, bool shortClick, bool longClick) {
    if (!pageActive) {
        // Navigate between pages with encoder
        if (encoderDelta != 0.0f) {
            int prev = activePage;
            int nextPage = activePage + ((encoderDelta > 0.0f) ? 1 : -1);
            if (nextPage < 0) {
                nextPage = 0;
            }
            if (nextPage >= PAGE_COUNT) {
                nextPage = PAGE_COUNT - 1;
            }
            activePage = nextPage;

            // Trigger slide only when the page actually changes.
            if (activePage != prev) {
                slideFromPage = prev;
                slideProgress = 0.0f;
                slideLeft     = (encoderDelta > 0.0f);
            }
        }

        // Short click enters the page
        if (shortClick) {
            pageActive = true;
            pages[activePage]->onEnter();
        }
    } else {
        // Long click exits
        if (longClick) {
            pages[activePage]->onExit();
            pageActive = false;
            return;
        }
        // Delegate input to active page
        pages[activePage]->update(encoderDelta, shortClick, false);
    }
}

void UIManager::render() {
    canvas->fillScreen(COL_BG);

    // Advance slide animation
    if (slideProgress < 1.0f) {
        slideProgress += SLIDE_SPEED;
        if (slideProgress > 1.0f) slideProgress = 1.0f;
    }

    if (slideProgress < 1.0f) {
        renderSlide();
    } else {
        // Normal render
        if (!pageActive) {
            drawPageSelector(activePage, 0);
        } else {
            pages[activePage]->draw(canvas, 0);
        }
    }

    drawHalfRing();

    // Push to display
    canvas->pushSprite(0, 0);
}

void UIManager::drawPageSelector(int pageIndex, int xOffset) {
    int cx = DISP_CX + xOffset;
    int cy = DISP_CY;

    canvas->setTextColor(COL_DIM, COL_BG);
    canvas->drawCentreString(PAGE_NAMES[pageIndex], cx, cy - 14, 4);
    canvas->setTextColor(COL_DIM, COL_BG);
    canvas->drawCentreString("CLICK TO ENTER", cx, cy + 30, 1);

    for (int i = 0; i < PAGE_COUNT; i++) {
        uint16_t c = (i == pageIndex) ? COL_ACCENT : COL_DIM;
        canvas->fillCircle(cx - (PAGE_COUNT - 1) * 12 + i * 24, cy + 55, 5, c);
    }
}

void UIManager::renderSlide() {
    // Page changes happen only in selector mode, so slide the selector itself.
    canvas->fillScreen(COL_BG);

    int direction = slideLeft ? 1 : -1;
    int hNewOffset = direction * (int)((1.0f - slideProgress) * DISP_W);
    int hOldOffset = -direction * (int)(slideProgress * DISP_W);

    drawPageSelector(slideFromPage, hOldOffset);
    drawPageSelector(activePage, hNewOffset);
}

void UIManager::drawHalfRing() {
    // Draw background track arc (upper half)
    canvas->drawArc(DISP_CX, DISP_CY, RING_RADIUS, RING_RADIUS - ARC_WIDTH,
                    180, 360, COL_ARC_TRACK);

    // Draw applied duty cycle arc (orange)
    float applied = pwmCore->currentAppliedDutyCycle / (cfgMgr->config.maxDutyCycle > 0.0f ? cfgMgr->config.maxDutyCycle : 1.0f);
    applied = constrain(applied, -1.0f, 1.0f);

    float appliedAngle = speedToAngle(applied);
    float zeroAngle    = ARC_ZERO_DEG;

    float arcA = min(appliedAngle, zeroAngle);
    float arcB = max(appliedAngle, zeroAngle);

    // Only draw if there's a non-trivial arc
    if (fabsf(arcB - arcA) > 0.5f) {
        canvas->drawArc(DISP_CX, DISP_CY, RING_RADIUS, RING_RADIUS - ARC_WIDTH,
                        (int)arcA, (int)arcB, COL_ARC_APPLIED);
    }

    // Draw target dot (white circle)
    float target = pwmCore->targetRotationSpeedPercentage;
    target = constrain(target, -1.0f, 1.0f);
    float targetAngle = speedToAngle(target);
    float rad         = targetAngle * DEG_TO_RAD;
    int   dotX        = DISP_CX + (int)(RING_RADIUS * cos(rad));
    int   dotY        = DISP_CY + (int)(RING_RADIUS * sin(rad));
    canvas->fillCircle(dotX, dotY, TARGET_DOT_R, COL_TARGET_DOT);
}
