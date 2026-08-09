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

        ledcAttach(PWM_PIN_1, PWM_FREQ_HZ, PWM_RESOLUTION);
        ledcAttach(PWM_PIN_2, PWM_FREQ_HZ, PWM_RESOLUTION);

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

        // Read volatile state into locals for calculation
        float current = currentAppliedDutyCycle;
        float tgtPct  = targetRotationSpeedPercentage;

        float targetDutyCycle = tgtPct * config->maxDutyCycle;

        if (fabsf(targetDutyCycle - current) < 1e-5f) return;

        // Kinetic energy is proportional to omega^2 (speed^2).
        // To make kinetic energy change constant, we move in sqrt-space.
        // Map current and target duty cycle magnitudes to "energy space" (sign kept),
        // step linearly in energy space, then map back.

        float sign    = (targetDutyCycle >= 0.0f) ? 1.0f : -1.0f;
        float curAbs  = fabsf(current);
        float tgtAbs  = fabsf(targetDutyCycle);

        // maxStep in duty-cycle units at the max rate (0→1 in spinupTime seconds)
        float maxStep = dt / config->zeroToMaxSpinupTime;

        // Convert magnitudes to energy space (square of magnitude)
        float curEnergy = curAbs * curAbs;
        float tgtEnergy = tgtAbs * tgtAbs;

        // maxStep in energy space: d(v^2)/dt = 2*v * dv/dt
        // At v=1 (max), maxEnergyStep = 2 * 1 * maxStep = 2*maxStep
        float maxEnergyStep = 2.0f * maxStep;

        float newEnergy;
        if (curEnergy < tgtEnergy) {
            newEnergy = curEnergy + maxEnergyStep;
            if (newEnergy > tgtEnergy) newEnergy = tgtEnergy;
        } else {
            // Decelerating — we cross zero if directions differ
            newEnergy = curEnergy - maxEnergyStep;
            if (newEnergy < 0.0f) newEnergy = 0.0f;
            if (newEnergy < tgtEnergy) newEnergy = tgtEnergy;
        }

        float newAbs = sqrtf(newEnergy);

        // Handle direction change: must pass through 0 first
        float curSign = (current >= 0.0f) ? 1.0f : -1.0f;
        float newSign;
        if (newAbs < 1e-5f) {
            // At zero – now we can start in the target direction
            newSign = sign;
        } else if (curSign != sign) {
            // Still decelerating toward zero, keep original sign
            newSign = curSign;
        } else {
            newSign = sign;
        }

        currentAppliedDutyCycle = newSign * newAbs;
        applyPWM(currentAppliedDutyCycle);
    }

    void stop() {
        targetRotationSpeedPercentage = 0.0f;
    }

private:
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
        ledcWrite(PWM_PIN_1, val1);
        ledcWrite(PWM_PIN_2, val2);
    }
};
