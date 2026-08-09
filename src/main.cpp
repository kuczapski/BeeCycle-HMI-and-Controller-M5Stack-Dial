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
static long   encoderPrev = 0;
static float  encoderAccum = 0.0f;

// ─── Button state (long press detection) ──────────────────────────────────
static unsigned long btnDownMs    = 0;
static bool          btnWasDown   = false;
static bool          shortClicked = false;
static bool          longClicked  = false;

static constexpr unsigned long LONG_PRESS_MS = 800;

void handleButton() {
    bool down = (M5.BtnA.isPressed() || M5.Dial.BtnEncoder.isPressed());

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

    encoderPrev = M5Dial.Encoder.read();
}

// ─── Loop ─────────────────────────────────────────────────────────────────
void loop() {
    M5Dial.update();

    // Encoder delta
    long encNow   = M5Dial.Encoder.read();
    float encDelta = (float)(encNow - encoderPrev);
    encoderPrev   = encNow;
    encoderAccum  += encDelta;

    handleButton();

    // Pass input to UI (consume accumulator in integer steps)
    float delta = 0.0f;
    if (fabsf(encoderAccum) >= 1.0f) {
        delta        = (encoderAccum > 0.0f) ? 1.0f : -1.0f;
        encoderAccum -= delta;
    }

    ui.update(delta, shortClicked, longClicked);
    ui.render();

    // ~60 fps
    delay(16);
}
