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
static long encoderAccum = 0;
static constexpr long ENCODER_COUNTS_PER_STEP = 4;

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

    M5Dial.Encoder.readAndReset();
}

// ─── Loop ─────────────────────────────────────────────────────────────────
void loop() {
    M5Dial.update();

    // Encoder delta
    encoderAccum += M5Dial.Encoder.readAndReset();

    handleButton();

    // The encoder library reports quadrature edges, not user detents.
    // Convert to whole detent steps before handing values to the UI.
    float delta = 0.0f;
    if (labs(encoderAccum) >= ENCODER_COUNTS_PER_STEP) {
        long steps = encoderAccum / ENCODER_COUNTS_PER_STEP;
        delta = (float)steps;
        encoderAccum -= steps * ENCODER_COUNTS_PER_STEP;
    }

    ui.update(delta, shortClicked, longClicked);
    ui.render();

    // ~60 fps
    delay(16);
}
