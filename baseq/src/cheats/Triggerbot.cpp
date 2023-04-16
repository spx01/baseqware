#include "cheats/Triggerbot.h"

#include <chrono>
#include <thread>

#include "Cheat.h"
#include "Config.h"
#include "Globals.h"

#include "sdk/client.h"
#include "sdk/entity.h"

using namespace std::literals;

void shoot() {
    // FIXME: randomized delay + maybe configuration for key
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    ::SendInput(1, &input, sizeof(INPUT));
    ::Sleep(10);
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    ::SendInput(1, &input, sizeof(INPUT));
}

// TODO: refactor everything
void cheats::Triggerbot::run() {
    g_log->dbg(L"Triggerbot::run()");
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

        // TODO: refactor
        // TODO: check for immunity
        // TODO: raycasting
        if (!(cid > 0 && cid <= 64)) {
            ::Sleep(2);
            continue;
        }

        auto player = sdk::Entity(g_c->mem->read<uint32_t>(g_c->mem->client.base + hazedumper::signatures::dwEntityList + (cid - 1) * 0x10));
        if (!player || player.get_class_id() != globals::local_class || !player.is_alive()) {
            ::Sleep(2);
            continue;
        }
        if (player.get_team() == local_team) {
            ::Sleep(2);
            continue;
        }

        shoot();
        ::Sleep(100);
    }
    g_log->dbg(L"Triggerbot::run(): Exiting");
}
