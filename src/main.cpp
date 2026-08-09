#include <M5Dial.h>
#include "Config.h"
#include "PWMCore.h"
#include "UIManager.h"

// ─── Globals ──────────────────────────────────────────────────────────────
ConfigManager cfgMgr;
PWMCore       pwmCore;
UIManager     ui;
M5Canvas      canvas(&M5.Lcd);

// ─── FreeRTOS PWM task ────────────────────────────────────────────────────
void pwmTask(void* pv) {
    for (;;) {
        pwmCore.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ─── Encoder state ────────────────────────────────────────────────────────
static portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;
static volatile long pendingEncoderSteps = 0;
static long encoderPrevRaw = 0;
static long encoderRemainder = 0;
static constexpr long ENCODER_COUNTS_PER_STEP = 4;
static int  encoderDominantDir = 0;
static unsigned long encoderLastMoveMs = 0;
static constexpr unsigned long ENCODER_DIR_HOLD_MS = 80;

void encoderTask(void* pv) {
    for (;;) {
        long rawPosition = M5Dial.Encoder.read();
        long rawDelta = rawPosition - encoderPrevRaw;
        encoderPrevRaw = rawPosition;

        unsigned long nowMs = millis();
        int rawDir = (rawDelta > 0) ? 1 : ((rawDelta < 0) ? -1 : 0);
        if (rawDir != 0) {
            if (encoderDominantDir == 0 || rawDir == encoderDominantDir || (nowMs - encoderLastMoveMs) > ENCODER_DIR_HOLD_MS) {
                if (rawDir != encoderDominantDir) {
                    encoderRemainder = 0;
                    encoderDominantDir = rawDir;
                }
                encoderRemainder += rawDelta;
                encoderLastMoveMs = nowMs;

                if (labs(encoderRemainder) >= ENCODER_COUNTS_PER_STEP) {
                    long steps = encoderRemainder / ENCODER_COUNTS_PER_STEP;
                    encoderRemainder -= steps * ENCODER_COUNTS_PER_STEP;

                    portENTER_CRITICAL(&encoderMux);
                    pendingEncoderSteps += steps;
                    portEXIT_CRITICAL(&encoderMux);
                }
            }
        } else if ((nowMs - encoderLastMoveMs) > ENCODER_DIR_HOLD_MS) {
            encoderDominantDir = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ─── Button state (long press detection) ──────────────────────────────────
static unsigned long btnDownMs    = 0;
static bool          btnWasDown   = false;
static bool          shortClicked = false;
static bool          longClicked  = false;

static constexpr unsigned long LONG_PRESS_MS = 800;

void handleButton() {
    bool down = M5Dial.BtnA.isPressed();

    shortClicked = false;
    longClicked  = false;

    if (down && !btnWasDown) {
        btnDownMs  = millis();
        btnWasDown = true;
    } else if (!down && btnWasDown) {
        unsigned long held = millis() - btnDownMs;
        if (held >= LONG_PRESS_MS) {
            longClicked = true;
        } else {
            shortClicked = true;
        }
        btnWasDown = false;
    }
}

// ─── Setup ────────────────────────────────────────────────────────────────
void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    Serial.begin(115200);

    // Load config from NVS
    cfgMgr.begin();

    // Init PWM core
    pwmCore.begin(&cfgMgr.config);

    // Init double-buffer canvas
    canvas.createSprite(DISP_W, DISP_H);

    // Init UI
    ui.begin(&canvas, &pwmCore, &cfgMgr);

    // Start PWM update task on core 0
    xTaskCreatePinnedToCore(pwmTask, "pwmTask", 2048, nullptr, 5, nullptr, 0);

    M5Dial.Encoder.write(0);
    encoderPrevRaw = 0;
    encoderRemainder = 0;
    encoderDominantDir = 0;
    encoderLastMoveMs = 0;
    pendingEncoderSteps = 0;

    xTaskCreatePinnedToCore(encoderTask, "encoderTask", 2048, nullptr, 6, nullptr, 0);
}

// ─── Loop ─────────────────────────────────────────────────────────────────
void loop() {
    M5Dial.update();

    handleButton();

    long steps = 0;
    portENTER_CRITICAL(&encoderMux);
    steps = pendingEncoderSteps;
    pendingEncoderSteps = 0;
    portEXIT_CRITICAL(&encoderMux);

    float delta = (float)steps;

    ui.update(delta, shortClicked, longClicked);
    ui.render();

    // ~60 fps
    delay(16);
}
