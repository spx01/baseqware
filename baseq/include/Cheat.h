#pragma once

#include <Windows.h>

#include <memory>
#include <thread>
#include <vector>

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

    // used to shut down all the cheat threads
    bool shutdown = false;

    // not entirely sure why i made this its own class
    std::unique_ptr<Memory> mem;

private:
    bool game_focused = false;
    HWND game_hwnd = NULL;
    HWND overlay = NULL;
    RECT client_area{};


    std::vector<std::thread> threads;
    void dispatch_threads();
};

inline std::unique_ptr<Cheat> g_c;