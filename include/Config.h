#pragma once
#include <Arduino.h>
#include <Preferences.h>

// PWM hardware pins
static constexpr int PWM_PIN_1 = 1;   // positive rotation
static constexpr int PWM_PIN_2 = 2;   // negative rotation
static constexpr int PWM_FREQ_HZ = 20000;
static constexpr int PWM_RESOLUTION = 10; // 10-bit: 0-1023

// Default configuration values
static constexpr float DEFAULT_ZERO_TO_MAX_SPINUP_TIME = 5.0f;  // seconds
static constexpr float DEFAULT_MAX_DUTY_CYCLE = 0.80f;           // 80%

struct AppConfig {
    float zeroToMaxSpinupTime;   // seconds to go from 0% to 100% duty cycle
    float maxDutyCycle;          // 0.0 – 1.0
    float programDuration;       // seconds for the standard program total run time

    void loadDefaults() {
        zeroToMaxSpinupTime = DEFAULT_ZERO_TO_MAX_SPINUP_TIME;
        maxDutyCycle        = DEFAULT_MAX_DUTY_CYCLE;
        programDuration     = 60.0f;
    }
};

class ConfigManager {
public:
    AppConfig config;

    void begin() {
        prefs.begin("beecycle", false);
        load();
    }

    void load() {
        config.zeroToMaxSpinupTime = prefs.getFloat("spinupTime",  DEFAULT_ZERO_TO_MAX_SPINUP_TIME);
        config.maxDutyCycle        = prefs.getFloat("maxDuty",     DEFAULT_MAX_DUTY_CYCLE);
        config.programDuration     = prefs.getFloat("progDur",     60.0f);
    }

    void save() {
        prefs.putFloat("spinupTime",  config.zeroToMaxSpinupTime);
        prefs.putFloat("maxDuty",     config.maxDutyCycle);
        prefs.putFloat("progDur",     config.programDuration);
    }

private:
    Preferences prefs;
};
