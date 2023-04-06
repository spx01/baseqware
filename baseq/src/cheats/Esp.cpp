#include "cheats/Esp.h"

#include <Windows.h>

#include "Cheat.h"
#include "Config.h"
#include "Memory.h"
#include "Util.h"

#include "sdk/const.h"
#include "sdk/mathlib.h"

void cheats::Esp::run() {
    g_log->dbg(L"Esp::run()");
    ::Sleep(200);
    auto local_player = g_c->mem->get_local_player();
    g_log->dbg(L"player: {}", local_player);
    while (!g_c->shutdown) {
        // make the timing consistent in the future
        // probably not a huge deal but this is just cringe
        ::Sleep(5);
        local_player = g_c->mem->get_local_player();
        if (!local_player) {
            continue;
        }

        auto vm = g_c->mem->read<sdk::VMatrix>(g_c->mem->client_base + hazedumper::signatures::dwViewMatrix);
        for (int i = 1; i < 32; ++i) {
            auto player = g_c->mem->read<uint32_t>(g_c->mem->client_base + hazedumper::signatures::dwEntityList + i * 0x10);
            if (!player) {
                continue;
            }
            g_log->dbg(L"{}", player);
            continue;

            auto state = g_c->mem->read<sdk::LIFE>(player + hazedumper::netvars::m_lifeState);
            if (state != sdk::LIFE::ALIVE)
                continue;
            if (player == local_player) {
                continue;
            }
            if (g_c->mem->read<bool>(player + hazedumper::signatures::m_bDormant)) {
                continue;
            }
            g_log->dbg(L"found player at idx {}", i);
        }
        break;
    }
    g_log->dbg(L"Esp::run(): Exiting");
}
