#pragma once

#include <memory>

#include "Cheat.h"
#include "KeInterface.h"
#include "Util.h"

#include "csgo.hpp"

class Memory {
public:
    Memory();
    ~Memory() {}
    void update();
    inline bool is_valid() const { return this->valid; }
    template<typename T>
    inline T read(uint32_t addr) const {
#ifdef _DEBUG
        if (addr < this->client_base /* || addr >= this->client_base + this->client_size */) {
            g_log->err(L"Attempt to read memory at {} outside of client module", addr);
            ::PostQuitMessage(0);
            ::g_c->shutdown = true;
            return T{};
        }
#endif
        auto [val, success] = this->ki.read<T>(this->game_pid, addr);
#ifdef _DEBUG
        if (!success) {
            g_log->err(L"Failed to read memory at {} throgh kernel driver", addr);
        }
#endif
        return val;
    }

    DWORD game_pid = 0;
    // FIXME
    DWORD client_base = 0;
    DWORD client_size = 0;

private:
    KeInterface ki;
    bool valid = true;
};