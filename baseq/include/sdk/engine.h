#pragma once

#include "Cheat.h"

#include "sdk/misc.h"

#define BW_ENGINE_READ_GLOBAL(member) \
    (g_c->mem->read<decltype(sdk::GlobalVars::member)>(g_c->mem->engine.base + hazedumper::signatures::dwGlobalVars + offsetof(sdk::GlobalVars, member)))

namespace sdk::engine {
    inline uint32_t get_client_state() {
        return g_c->mem->read<uint32_t>(g_c->mem->engine.base + hazedumper::signatures::dwClientState);
    }

    inline bool in_game() {
        auto client_state = get_client_state();
        return client_state != 0 && g_c->mem->read<int>(client_state + hazedumper::signatures::dwClientState_State) == int(SIGNONSTATE::FULL);
    }
}// namespace sdk::engine
