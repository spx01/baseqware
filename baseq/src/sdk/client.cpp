#include "sdk/client.h"

namespace sdk::client {
    sdk::VMatrix get_view_matrix() {
        return g_c->mem->read<sdk::VMatrix>(g_c->mem->client.base + hazedumper::signatures::dwViewMatrix);
    }
}// namespace sdk::client