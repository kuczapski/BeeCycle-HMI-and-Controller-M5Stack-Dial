#pragma once
#include <Arduino.h>
#include <Preferences.h>

// PWM hardware pins on M5Dial PORT.B
// G1 = GPIO1, G2 = GPIO2 according to the M5Dial pin map.
static constexpr int PWM_PIN_1 = 1;   // G1, positive rotation
static constexpr int PWM_PIN_2 = 2;   // G2, negative rotation
static constexpr int ACTIVE_BRAKE_PIN_1 = 13; // PORT.A brake output 1
static constexpr int ACTIVE_BRAKE_PIN_2 = 15; // PORT.A brake output 2
static constexpr int PWM_FREQ_HZ = 20000;
static constexpr int PWM_RESOLUTION = 10; // 10-bit: 0-1023

// Default configuration values
static constexpr float DEFAULT_ZERO_TO_MAX_SPINUP_TIME = 5.0f;  // seconds
static constexpr float DEFAULT_MAX_DUTY_CYCLE = 0.80f;           // 80%
static constexpr bool DEFAULT_ACTIVE_BRAKING = false;

struct AppConfig {
    float zeroToMaxSpinupTime;   // seconds to go from 0% to 100% duty cycle
    float maxDutyCycle;          // 0.0 – 1.0
    float programDuration;       // seconds for the standard program total run time
    bool  activeBraking;         // true = enable active braking mode

    void loadDefaults() {
        zeroToMaxSpinupTime = DEFAULT_ZERO_TO_MAX_SPINUP_TIME;
        maxDutyCycle        = DEFAULT_MAX_DUTY_CYCLE;
        programDuration     = 60.0f;
        activeBraking       = DEFAULT_ACTIVE_BRAKING;
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
        config.activeBraking       = prefs.getBool("actBrake",     DEFAULT_ACTIVE_BRAKING);
    }

    void save() {
        prefs.putFloat("spinupTime",  config.zeroToMaxSpinupTime);
        prefs.putFloat("maxDuty",     config.maxDutyCycle);
        prefs.putFloat("progDur",     config.programDuration);
        prefs.putBool("actBrake",     config.activeBraking);
    }

private:
    Preferences prefs;
};
