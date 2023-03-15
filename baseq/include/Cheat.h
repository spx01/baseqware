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
    HWINEVENTHOOK destroy_hook = NULL;

    bool try_initialize();
    static BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lParam);
    static void CALLBACK destroy_callback(
            HWINEVENTHOOK hweh,
            DWORD event,
            HWND hwnd,
            LONG idObject,
            LONG idChild,
            DWORD dwEventThread,
            DWORD dwmsEventTime);
    std::unique_ptr<Memory> mem;
};

inline std::unique_ptr<Cheat> g_c;