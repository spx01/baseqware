#pragma once

#include <cstdint>

namespace sdk {
    enum class LIFE : uint32_t {
        ALIVE = 0,
        DYING = 1,
        DEAD = 2,
        RESPAWNABLE = 3
    };
}// namespace sdk