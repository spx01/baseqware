#include "cheats/Triggerbot.h"

#include <chrono>
#include <thread>

#include "Cheat.h"
#include "Config.h"
#include "Globals.h"

#include "sdk/client.h"
#include "sdk/entity.h"

using namespace std::literals;

// TODO: refactor everything
void cheats::Triggerbot::run() {
    while (!g_c->shutdown) {
        if (!globals::local or !g_cfg.trigger.enabled or !::GetAsyncKeyState(VK_MENU)) {
            ::Sleep(2);
            continue;
        }

        if (!globals::local || !globals::local.is_alive()) {
            ::Sleep(2);
            continue;
        }

        auto local_team = globals::local.get_team();
        auto cid = globals::local.get_crosshair_id();
        auto player = sdk::Entity(g_c->mem->read<uint32_t>(g_c->mem->client.base + hazedumper::signatures::dwEntityList + (cid - 1) * 0x10));
        if (!player) {
            ::Sleep(2);
            continue;
        }
        if (player.get_team() == local_team) {
            ::Sleep(2);
            continue;
        }
        // TODO: refactor
        // TODO: check for immunity
        // TODO: raycasting
        if (!(cid > 0 && cid <= 64)) {
            ::Sleep(2);
            continue;
        }

        sdk::client::set_force_attack(6);
        ::Sleep(100);
    }
    g_log->dbg(L"Esp::run(): Exiting");
}
