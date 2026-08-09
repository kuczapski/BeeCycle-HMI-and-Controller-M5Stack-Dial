#pragma once
#include <M5Dial.h>
#include "DisplayDefs.h"

// Base class for all UI pages.
class PageBase {
public:
    virtual ~PageBase() = default;

    // Called once when page becomes active (entered with a click)
    virtual void onEnter() {}

    // Called once when page is exited (long click)
    virtual void onExit() {}

    // Called every frame.
    // encoderDelta: accumulated encoder steps since last call
    // shortClick / longClick: edge-triggered button events
    virtual void update(float encoderDelta, bool shortClick, bool longClick) = 0;

    // Draw the page content into the canvas (excluding the header half-ring).
    // yOffset: vertical pixel offset for sliding transition (0 = normal).
    virtual void draw(M5Canvas* canvas, int yOffset) = 0;

    // Returns true if this page has been "entered" and handles input itself.
    bool isActive() const { return active; }
    void setActive(bool v) { active = v; }

protected:
    bool active = false;
};
