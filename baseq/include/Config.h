#pragma once

#include "cheats/Bhop.h"
#include "cheats/Esp.h"
#include "cheats/Triggerbot.h"

class KeyBind {
public:
    enum Trigger {
        KEY_TOGGLE,
        KEY_DOWN,
        ALWAYS
    } trigger;
    inline KeyBind(Trigger trg = KEY_DOWN) : trigger(trg) {}
    operator bool();

private:
    int vkey = 0;
    bool toggled = false;
    bool invert = false;
    bool last_pressed = false;
};

class BaseOption {
public:
    inline operator bool() {
        return this->enabled && this->kb;
    }

private:
    bool enabled;
    KeyBind kb;
};

class Config {
public:
    cfg::Esp esp;
    cfg::Triggerbot trigger;
    cfg::Bhop bhop;
};

inline Config g_cfg;