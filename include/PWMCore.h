#pragma once
#include <Arduino.h>
#include "Config.h"

// PWM Control Core
// currentAppliedDutyCycle is in range [-1, 1].
//   positive -> PWM_PIN_1 active (forward)
//   negative -> PWM_PIN_2 active (reverse)
// targetRotationSpeedPercentage is in range [-1, 1].
//   sign determines direction, magnitude is speed fraction.

class PWMCore {
public:
    // State (read-only externally for display; written from PWM task)
    volatile float currentAppliedDutyCycle       = 0.0f; // [-1, 1]
    volatile float targetRotationSpeedPercentage = 0.0f; // [-1, 1]

    void begin(const AppConfig* cfg) {
        config = cfg;

        ledcSetup(PWM_CHANNEL_1, PWM_FREQ_HZ, PWM_RESOLUTION);
        ledcSetup(PWM_CHANNEL_2, PWM_FREQ_HZ, PWM_RESOLUTION);
        ledcAttachPin(PWM_PIN_1, PWM_CHANNEL_1);
        ledcAttachPin(PWM_PIN_2, PWM_CHANNEL_2);

        applyPWM(0.0f);
        lastUpdateUs = micros();
    }

    void setConfig(const AppConfig* cfg) {
        config = cfg;
    }

    // Call periodically (e.g. every 10 ms) from the PWM task/loop.
    void update() {
        unsigned long nowUs = micros();
        float dt = (float)(nowUs - lastUpdateUs) * 1e-6f;
        lastUpdateUs = nowUs;

        if (dt <= 0.0f || dt > 1.0f) return; // sanity
        if (config == nullptr || config->zeroToMaxSpinupTime <= 0.0f || config->maxDutyCycle <= 0.0f) return;

        // Read volatile state into locals for calculation
        float current = currentAppliedDutyCycle;
        float tgtPct  = targetRotationSpeedPercentage;

        float targetDutyCycle = tgtPct * config->maxDutyCycle;

        if (fabsf(targetDutyCycle - current) < 1e-5f) return;

        // Kinetic energy is proportional to omega^2. We ramp in normalized
        // energy space so 0 -> maxDutyCycle takes exactly zeroToMaxSpinupTime.
        float maxDuty = config->maxDutyCycle;
        float curSign = (current < 0.0f) ? -1.0f : 1.0f;
        float tgtSign = (targetDutyCycle < 0.0f) ? -1.0f : 1.0f;

        float effectiveTargetDuty = targetDutyCycle;
        if (fabsf(current) > 1e-5f && fabsf(targetDutyCycle) > 1e-5f && curSign != tgtSign) {
            effectiveTargetDuty = 0.0f;
            tgtSign = curSign;
        }

        float curNorm = constrain(fabsf(current) / maxDuty, 0.0f, 1.0f);
        float tgtNorm = constrain(fabsf(effectiveTargetDuty) / maxDuty, 0.0f, 1.0f);

        float curEnergy = curNorm * curNorm;
        float tgtEnergy = tgtNorm * tgtNorm;
        float maxEnergyStep = dt / config->zeroToMaxSpinupTime;

        float newEnergy = curEnergy;
        if (curEnergy < tgtEnergy) {
            newEnergy += maxEnergyStep;
            if (newEnergy > tgtEnergy) newEnergy = tgtEnergy;
        } else {
            newEnergy -= maxEnergyStep;
            if (newEnergy < tgtEnergy) newEnergy = tgtEnergy;
        }

        float newAbs = sqrtf(newEnergy) * maxDuty;
        float newSign = (newAbs < 1e-5f) ? 0.0f : tgtSign;

        currentAppliedDutyCycle = newSign * newAbs;
        applyPWM(currentAppliedDutyCycle);
    }

    void stop() {
        targetRotationSpeedPercentage = 0.0f;
    }

private:
    static constexpr uint8_t PWM_CHANNEL_1 = 0;
    static constexpr uint8_t PWM_CHANNEL_2 = 1;

    const AppConfig* config = nullptr;
    unsigned long    lastUpdateUs = 0;

    void applyPWM(float duty) {
        // duty in [-1, 1]
        int maxVal = (1 << PWM_RESOLUTION) - 1;

        int val1 = 0, val2 = 0;
        if (duty > 0.0f) {
            val1 = (int)(duty * maxVal + 0.5f);
        } else if (duty < 0.0f) {
            val2 = (int)(-duty * maxVal + 0.5f);
        }
        ledcWrite(PWM_CHANNEL_1, val1);
        ledcWrite(PWM_CHANNEL_2, val2);
    }
};
