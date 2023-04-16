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

void jump() {
    // FIXME: scrollwheel emulation + configuration
    INPUT input{};
    input.type = INPUT_KEYBOARD;
    // TODO: implement key pool and use telnet
    input.ki.wVk = VK_END;
    input.ki.wScan = ::MapVirtualKey(VK_END, MAPVK_VK_TO_VSC);
    ::SendInput(1, &input, sizeof(INPUT));
    ::Sleep(10);
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    ::SendInput(1, &input, sizeof(INPUT));
}

void cheats::Bhop::run() {
    g_log->dbg(L"Bhop::run()");
    std::this_thread::sleep_for(100ms);

    // FIXME: timing
    // this isn't 100% consistent becuase it's not synced with the game in any way
    while (!g_c->shutdown) {
        if (!g_cfg.bhop.enabled || !globals::local || !globals::local.is_alive() || util::in_menu() || !sdk::engine::in_game() || !g_c->is_focused()) {
            ::Sleep(5);
            continue;
        }

        static int last_tick;
        static int last_frame;
        static float fun_elapsed;
        static bool in_bhop;

        auto tick_cnt = BW_ENGINE_READ_GLOBAL(iTickCount);
        auto frame_cnt = BW_ENGINE_READ_GLOBAL(iFrameCount);
        auto interval_per_tick = BW_ENGINE_READ_GLOBAL(flIntervalPerTick);
        if (!(tick_cnt != last_tick || frame_cnt != last_frame)) {
            util::sleep(interval_per_tick);
            continue;
        }

        auto frametime = BW_ENGINE_READ_GLOBAL(flAbsFrameTime);
        auto delay = fun_elapsed - (frametime < interval_per_tick ? (frametime * 0.5f) : frametime);
        auto sleep = std::min(delay, frametime * 1000);
        util::sleep(sleep);

        auto start = std::chrono::high_resolution_clock::now();

        auto move_type = globals::local.get_move_type();
        if (move_type == sdk::MOVETYPE::LADDER || move_type == sdk::MOVETYPE::NOCLIP || move_type == sdk::MOVETYPE::OBSERVER) {
            continue;
        }

        // FIXME
        // not sure why this doesn't work, but i don't think it's my fault
        // i'll just use a weirder method
        /*
        if (::GetAsyncKeyState(VK_SPACE)) {
            if (!in_bhop) {
                ::Sleep(10);
                INPUT input{};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_SPACE;
                input.ki.wScan = ::MapVirtualKey(VK_SPACE, MAPVK_VK_TO_VSC);
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                ::SendInput(1, &input, sizeof(INPUT));
            } else if (globals::local.get_flags() & sdk::FL::ONGROUND) {
                jump();
            }
            in_bhop = true;
        } else {
            in_bhop = false;
        } */

        if (::GetAsyncKeyState(VK_SPACE) && (globals::local.get_flags() & sdk::FL::ONGROUND)) {
            jump();
        }


        auto end = std::chrono::high_resolution_clock::now();
        last_tick = tick_cnt;
        last_frame = frame_cnt;
        fun_elapsed = float((end - start).count());
    }
    g_log->dbg(L"Bhop::run(): Exiting");
}