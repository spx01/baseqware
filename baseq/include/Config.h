#pragma once

#include "cheats/Esp.h"

class Config {
public:
    cfg::Esp esp;
    bool should_open_menu();
};

inline Config g_cfg;