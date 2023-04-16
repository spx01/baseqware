#include "Config.h"

#include <Windows.h>

KeyBind::operator bool() {
    switch (this->trigger) {
        case Trigger::ALWAYS:
            return true;
        case Trigger::KEY_DOWN:
            return ::GetAsyncKeyState(this->vkey) & 0x8000;
        case Trigger::KEY_TOGGLE:
            if (::GetAsyncKeyState(this->vkey) & 0x8000) {
                if (this->last_pressed) {
                    return this->toggled;
                }
                this->last_pressed = true;
                this->toggled = !this->toggled;

            } else {
                this->last_pressed = false;
            }
            return this->invert != this->toggled;
    }
}
