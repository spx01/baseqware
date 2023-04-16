#include "cheats/Bhop.h"

#include <chrono>
#include <thread>

#include "Cheat.h"
#include "Config.h"
#include "Globals.h"
#include "Util.h"

#include "sdk/client.h"
#include "sdk/engine.h"

using namespace std::literals;

void cheats::Bhop::run() {
    g_log->dbg(L"Bhop::run()");
    std::this_thread::sleep_for(100ms);

    // FIXME: timing
    // this isn't 100% consistent becuase it's not synced with the game in any way
    while (!g_c->shutdown) {
        if (!g_cfg.bhop.enabled || !globals::local || !globals::local.is_alive() || util::in_menu() || !sdk::engine::in_game()) {
            ::Sleep(2);
            continue;
        }
        auto move_type = globals::local.get_move_type();
        if (move_type == sdk::MOVETYPE::LADDER || move_type == sdk::MOVETYPE::NOCLIP || move_type == sdk::MOVETYPE::OBSERVER) {
            ::Sleep(2);
            continue;
        }
        if (::GetAsyncKeyState(VK_SPACE) && globals::local.get_flags() & sdk::FL::ONGROUND) {
            sdk::client::set_force_jump(5);
        } else {
            sdk::client::set_force_jump(4);
        }
        ::Sleep(2);
    }
    g_log->dbg(L"Bhop::run(): Exiting");
}