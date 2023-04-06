#pragma once

#include <memory>

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
        auto [val, success] = this->ki.read<T>(this->game_pid, addr);
#ifdef _DEBUG
        if (!success) {
            g_log->err(L"Failed to read memory at 0x%04x throgh kernel driver", addr);
        }
#endif
        return val;
    }

    inline auto get_local_player() const {
        return this->read<uint32_t>(this->client_base + hazedumper::signatures::dwLocalPlayer);
    }

    DWORD game_pid = 0;
    DWORD client_base = 0;

private:
    KeInterface ki;
    bool valid = true;
};