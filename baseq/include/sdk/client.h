#pragma once

#include "sdk/math.h"

#include "Cheat.h"

#include "csgo.hpp"

namespace sdk::client {
    sdk::VMatrix get_view_matrix();

    inline uint32_t get_local_player() {
        return g_c->mem->read<uint32_t>(g_c->mem->client.base + hazedumper::signatures::dwLocalPlayer);
    }

    inline bool set_force_attack(std::uint32_t val) {
        return g_c->mem->write(g_c->mem->client.base + hazedumper::signatures::dwForceAttack, val);
    }

    inline bool set_force_jump(std::uint32_t val) {
        return g_c->mem->write(g_c->mem->client.base + hazedumper::signatures::dwForceJump, val);
    }
}// namespace sdk::client