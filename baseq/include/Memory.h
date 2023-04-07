#pragma once

#include <algorithm>
#include <memory>

#include "Cheat.h"
#include "KeInterface.h"
#include "Util.h"

#include "sdk/math.h"

#include "csgo.hpp"

// i love windows btw
#undef min
#undef max

class Memory {
public:
    Memory();
    ~Memory() {}
    void update();
    inline bool is_valid() const { return this->valid; }
    template<typename T>
    inline T read(uint32_t addr) const {
        if constexpr (std::is_same_v<T, sdk::Vector>) {
            return this->read_vec(addr);
        } else if constexpr (sizeof(T) > 4) {
            // c++20 approved!!!!
            []<bool f = false>() {
                static_assert(f);
            }
            ();
        }
#ifdef _DEBUG
        // FIXME
        if (addr < std::min(this->client.base, this->engine.base)) {
            g_log->err(L"Attempt to read memory at low address: {:04X}", addr);
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
    struct ModuleInfo {
        DWORD base;
        DWORD size;
    };
    ModuleInfo client{};
    ModuleInfo engine{};

private:
    sdk::Vector read_vec(uint32_t addr) const;
    KeInterface ki;
    bool valid = true;
};