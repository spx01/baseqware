#pragma once

#include <Windows.h>

#include <memory>

#include "Memory.h"

// pretend this is a singleton
class Cheat {
public:
    Cheat();
    ~Cheat();

    // this is where most of the init code goes
    // because we support hotloading
    void update();
    void render_overlay();

private:
    bool game_focused = false;
    HWND game_hwnd = NULL;

    std::unique_ptr<Memory> mem;
};

inline std::unique_ptr<Cheat> g_c;