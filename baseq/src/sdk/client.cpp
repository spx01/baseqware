#include "sdk/client.h"

namespace sdk::client {
    sdk::VMatrix get_view_matrix() {
        uint32_t start = g_c->mem->client.base + hazedumper::signatures::dwViewMatrix;
        sdk::VMatrix res{};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res[i][j] = g_c->mem->read<float>(start + (i * 4 + j) * 4);
            }
        }
        return res;
    }
}// namespace sdk::client