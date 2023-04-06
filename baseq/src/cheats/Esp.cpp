#include "cheats/Esp.h"

#include <Windows.h>

#include "Cheat.h"
#include "Config.h"
#include "Memory.h"
#include "Util.h"

#include "sdk/const.h"
#include "sdk/entity.h"
#include "sdk/mathlib.h"

void cheats::Esp::run() {
    g_log->dbg(L"Esp::run()");
    ::Sleep(200);
    while (!g_c->shutdown) {
        // make the timing consistent in the future
        // probably not a huge deal but this is just cringe
        ::Sleep(5);
        auto local_player = sdk::Entity(g_c->mem->get_local_player());
        if (!local_player) {
            continue;
        }

        auto vm = g_c->mem->read<sdk::VMatrix>(g_c->mem->client_base + hazedumper::signatures::dwViewMatrix);
        for (int i = 1; i < 32; ++i) {
            auto player = sdk::Entity(g_c->mem->read<uint32_t>(g_c->mem->client_base + hazedumper::signatures::dwEntityList + i * 0x10));
            if (!player) {
                continue;
            }
            if (player.get() == local_player.get()) {
                continue;
            }
            if (!player.is_alive()) {
                continue;
            }
            if (player.is_dormant()) {
                continue;
            }
            if (!player.is_enemy()) {
                continue;
            }
            g_log->dbg(L"Found player at idx {}", i);
        }
        break;
    }
    g_log->dbg(L"Esp::run(): Exiting");
}
