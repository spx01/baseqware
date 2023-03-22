#pragma once

#include <Windows.h>

#include <memory>

#include "Memory.h"

// pretend this is a singleton
class Cheat {
public:
    Cheat(HWND overlay);
    ~Cheat();

    // this is where most of the init code goes
    // because we support hotloading
    void update();
    void update_overlay();
    inline LONG client_width() const {
        return this->client_area.right - this->client_area.left;
    }
    inline LONG client_height() const {
        return this->client_area.bottom - this->client_area.top;
    }
    inline POINT client_pos() const {
        return {this->client_area.left, this->client_area.top};
    }

private:
    bool game_focused = false;
    HWND game_hwnd = NULL;
    HWND overlay = NULL;
    RECT client_area{};

    std::unique_ptr<Memory> mem;
};

inline std::unique_ptr<Cheat> g_c;