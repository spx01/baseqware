#pragma once

#include <memory>

#include "KeInterface.h"

class Memory {
public:
    Memory();
    ~Memory() {}
    void update();
    inline bool is_valid() const { return this->valid; }
    DWORD game_pid = 0;
    DWORD game_base = 0;
    KeInterface ki;

private:
    bool valid = true;
};

inline std::unique_ptr<Memory> g_m;