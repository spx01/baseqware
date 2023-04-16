#pragma once

#include <cstdint>

namespace sdk {
    enum LIFE : uint32_t {
        ALIVE = 0,
        DYING = 1,
        DEAD = 2,
        RESPAWNABLE = 3
    };// namespace LIFE

    namespace FL {
        constexpr uint32_t ONGROUND = 1 << 0;
        constexpr uint32_t DUCKING = 1 << 1;
        constexpr uint32_t WATERJUMP = 1 << 3;
        constexpr uint32_t ONTRAIN = 1 << 4;
        constexpr uint32_t INRAIN = 1 << 5;
        constexpr uint32_t FROZEN = 1 << 6;
        constexpr uint32_t ATCONTROLS = 1 << 7;
        constexpr uint32_t CLIENT = 1 << 8;
        constexpr uint32_t FAKECLIENT = 1 << 9;
        constexpr uint32_t INWATER = 1 << 10;
        constexpr uint32_t HIDEHUD_SCOPE = 1 << 11;
    };// namespace FL

    enum MOVETYPE {
        NONE = 0,
        ISOMETRIC,
        WALK,
        STEP,
        FLY,
        FLYGRAVITY,
        VPHYSICS,
        PUSH,
        NOCLIP,
        LADDER,
        OBSERVER,
        CUSTOM
    };
}// namespace sdk