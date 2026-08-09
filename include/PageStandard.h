#pragma once
#include "PageBase.h"
#include "PWMCore.h"
#include "Config.h"

// ─── Standard Centrifuge Program Page ─────────────────────────────────────
// Program steps: each step has a direction (sign) and speed fraction.
struct ProgramStep {
    float speedFraction; // 0.25 / 0.50 / 0.75 / 1.00
    float direction;     // +1 right, -1 left
};

static const ProgramStep PROGRAM_STEPS[] = {
    { 0.25f, -1.0f },
    { 0.50f,  1.0f },
    { 0.75f, -1.0f },
    { 1.00f,  1.0f },
};
static constexpr int PROGRAM_STEP_COUNT = 4;

inline float programStepRampDuration(const AppConfig& cfg, float speedFraction) {
    float clampedFraction = constrain(speedFraction, 0.0f, 1.0f);
    return cfg.zeroToMaxSpinupTime * clampedFraction * clampedFraction;
}

inline float minimumProgramDuration(const AppConfig& cfg) {
    float total = 0.0f;
    for (int i = 0; i < PROGRAM_STEP_COUNT; ++i) {
        total += 2.0f * programStepRampDuration(cfg, PROGRAM_STEPS[i].speedFraction);
    }
    return total;
}

class PageStandard : public PageBase {
public:
    void init(PWMCore* pwm, ConfigManager* cfg) {
        pwmCore = pwm;
        cfgMgr  = cfg;
    }

    void onEnter() override {
        active     = true;
        running    = false;
        stepIndex  = 0;
        stepPhase  = PHASE_SPINUP;
        progress   = 0.0f;
        lastUpdateUs = micros();
    }

    void onExit() override {
        active  = false;
        running = false;
        pwmCore->stop();
    }

    void update(float encoderDelta, bool shortClick, bool longClick) override {
        if (!active) return;

        unsigned long nowUs = micros();
        float dt = (float)(nowUs - lastUpdateUs) * 1e-6f;
        lastUpdateUs = nowUs;

        if (shortClick) {
            running = !running;
            if (!running) {
                pwmCore->stop();
                stepIndex = 0;
                stepPhase = PHASE_SPINUP;
                stepTime  = 0.0f;
                holdTimer = 0.0f;
                progress  = 0.0f;
            }
        }

        if (!running) {
            // Encoder changes program duration
            float minDur = minimumProgramDuration(cfgMgr->config);
            cfgMgr->config.programDuration += encoderDelta * 5.0f;
            if (cfgMgr->config.programDuration < minDur)
                cfgMgr->config.programDuration = minDur;
            return;
        }

        // Each step gets equal time
        float stepDur = cfgMgr->config.programDuration / (float)PROGRAM_STEP_COUNT;
        stepTime += dt;
        progress = ((float)stepIndex + stepTime / stepDur) / (float)PROGRAM_STEP_COUNT;
        if (progress > 1.0f) progress = 1.0f;

        const ProgramStep& step = PROGRAM_STEPS[stepIndex];
        float targetSpeed = step.direction * step.speedFraction;

        switch (stepPhase) {
        case PHASE_SPINUP:
            pwmCore->targetRotationSpeedPercentage = targetSpeed;
            // Move to hold once we're close to target
            if (fabsf(pwmCore->currentAppliedDutyCycle - targetSpeed * cfgMgr->config.maxDutyCycle) < 0.02f) {
                stepPhase = PHASE_HOLD;
                holdTimer = 0.0f;
            }
            break;

        case PHASE_HOLD:
            holdTimer += dt;
            {
                float rampDur = programStepRampDuration(cfgMgr->config, step.speedFraction);
                float holdDur = stepDur - 2.0f * rampDur;
                if (holdDur < 0.0f) holdDur = 0.0f;
                if (holdTimer >= holdDur) {
                    stepPhase = PHASE_SPINDOWN;
                    pwmCore->stop();
                }
            }
            break;

        case PHASE_SPINDOWN:
            if (fabsf(pwmCore->currentAppliedDutyCycle) < 0.02f) {
                stepIndex++;
                stepPhase = PHASE_SPINUP;
                stepTime  = 0.0f;
                holdTimer = 0.0f;
                if (stepIndex >= PROGRAM_STEP_COUNT) {
                    running = false;
                    stepIndex = 0;
                    progress  = 1.0f;
                }
            }
            break;
        }
    }

    void draw(M5Canvas* canvas, int yOffset) override {
        int cx = DISP_CX, cy = DISP_CY + yOffset;

        // Page label
        canvas->setTextColor(COL_DIM, COL_BG);
        canvas->setTextSize(1);
        canvas->drawCentreString("STANDARD", cx, cy - 60, 2);

        // Duration display (while stopped)
        if (!running) {
            canvas->setTextColor(COL_TEXT, COL_BG);
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0f s", cfgMgr->config.programDuration);
            canvas->drawCentreString(buf, cx, cy - 10, 4);
            canvas->setTextColor(COL_DIM, COL_BG);
            canvas->drawCentreString("CLICK TO START", cx, cy + 35, 1);
        } else {
            // Progress ring
            int pr = 50;
            canvas->drawCircle(cx, cy, pr, COL_ARC_TRACK);
            int arcEnd = (int)(progress * 360.0f);
            canvas->drawArc(cx, cy, pr, pr - 6, 270, 270 + arcEnd, COL_GREEN);

            // Percentage
            char buf[16];
            snprintf(buf, sizeof(buf), "%d%%", (int)(progress * 100.0f));
            canvas->setTextColor(COL_TEXT, COL_BG);
            canvas->drawCentreString(buf, cx, cy - 14, 4);

            // Step indicator
            snprintf(buf, sizeof(buf), "STEP %d/%d", stepIndex + 1, PROGRAM_STEP_COUNT);
            canvas->setTextColor(COL_DIM, COL_BG);
            canvas->drawCentreString(buf, cx, cy + 35, 1);
        }
    }

private:
    enum Phase { PHASE_SPINUP, PHASE_HOLD, PHASE_SPINDOWN };

    PWMCore*       pwmCore      = nullptr;
    ConfigManager* cfgMgr       = nullptr;
    bool           running      = false;
    int            stepIndex    = 0;
    Phase          stepPhase    = PHASE_SPINUP;
    float          stepTime     = 0.0f;
    float          holdTimer    = 0.0f;
    float          progress     = 0.0f;
    unsigned long  lastUpdateUs = 0;
};
