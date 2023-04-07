#pragma once

#include "sdk/math.h"

#include "Cheat.h"

#include "csgo.hpp"

// TODO move shit into cpp files

namespace sdk {
    namespace client {
        sdk::VMatrix get_view_matrix() {
            uint32_t start = g_c->mem->client_base + hazedumper::signatures::dwViewMatrix;
            sdk::VMatrix res{};
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    res[i][j] = g_c->mem->read<float>(start + (i * 4 + j) * 4);
                }
            }
            return res;
        }

        uint32_t get_local_player() {
            auto lp = g_c->mem->read<uint32_t>(g_c->mem->client_base + hazedumper::signatures::dwLocalPlayer);
            // test out the other method (through clientstate)
            return lp;
        }
    }// namespace client
}// namespace sdk