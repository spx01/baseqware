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

void duck() {
    // FIXME: scrollwheel emulation + configuration
    INPUT input{};
    input.type = INPUT_KEYBOARD;
    // TODO: implement key pool and use telnet
    input.ki.wVk = VK_CONTROL;
    input.ki.wScan = ::MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    ::SendInput(1, &input, sizeof(INPUT));
    ::Sleep(1000 / 64);
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    ::SendInput(1, &input, sizeof(INPUT));
}

void cheats::Bhop::run() {
    g_log->dbg(L"Bhop::run()");
    std::this_thread::sleep_for(200ms);

    while (!g_c->shutdown) {
        if (!g_cfg.bhop.enabled || !globals::local || !globals::local.is_alive() || util::in_menu() || !sdk::engine::in_game() || !g_c->is_focused()) {
            ::Sleep(5);
            continue;
        }

        static int last_tick;
        static int last_frame;
        static float fun_elapsed;
        static bool in_bhop;

        auto g = sdk::engine::get_global_vars();
        if (!(g.iTickCount != last_tick || g.iFrameCount != last_frame)) {
            util::sleep(g.flIntervalPerTick);
            continue;
        }

        const auto frametime = g.flAbsFrameTime;
        auto delay = fun_elapsed - (frametime < g.flIntervalPerTick ? (frametime * 0.5f) : frametime);
        auto sleep = std::min(std::max(delay, 0.f), frametime * 1000);
        util::sleep(sleep);

        auto start = std::chrono::high_resolution_clock::now();

        auto move_type = globals::local.get_move_type();
        if (move_type == sdk::MOVETYPE::LADDER || move_type == sdk::MOVETYPE::NOCLIP || move_type == sdk::MOVETYPE::OBSERVER) {
            continue;
        }

        // something something crash in debug mode something something
        static float h;
        static int land_tick;
        if (g.iTickCount != last_tick) {
            if (land_tick == g.iTickCount + 2) {
                duck();
            }
            if (land_tick == g.iTickCount) {
                g_log->info(L"land");
                land_tick = 0;
            }
            if (!(globals::local.get_flags() & sdk::FL::ONGROUND)) {
                auto vel = globals::local.get_velocity();
                if (vel.z >= -12.5f && vel.z <= 0.f) {
                    g_log->info(L"no vertical vel");
                    float dh = globals::local.get_origin().z - h;
                    if (dh > 0.f) {
                        float t = (vel.z + sqrt(vel.z * vel.z + 2 * 800 * dh)) / 800.f;
                        int dt = ceil(t * 64);
                        g_log->info(L"dt: {}", dt);
                        land_tick = g.iTickCount + dt;
                    }
                }
            } else {
                h = globals::local.get_origin().z;
            }
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
        last_tick = g.iTickCount;
        last_frame = g.iFrameCount;
        fun_elapsed = float((end - start).count());
    }
    g_log->dbg(L"Bhop::run(): Exiting");
}