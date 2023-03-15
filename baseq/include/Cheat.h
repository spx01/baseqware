#pragma once

#include <Windows.h>

#include <memory>

#include "Memory.h"

// pretend this is a singleton
class Cheat {
public:
    Cheat();
    ~Cheat();
    // there is probably no point in having a constructor
    // as we want to reinitialize whenever the game is reopened
    void update();
    void render_overlay();

private:
    bool game_focused = false;
    HWND game_hwnd = NULL;

    std::unique_ptr<Memory> mem;
};

inline std::unique_ptr<Cheat> g_c;