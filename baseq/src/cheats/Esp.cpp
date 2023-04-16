#include "cheats/Esp.h"

#include <Windows.h>

#include <chrono>
#include <thread>

#include "Cheat.h"
#include "Config.h"
#include "Globals.h"
#include "Memory.h"
#include "Util.h"

#include "sdk/client.h"
#include "sdk/const.h"
#include "sdk/entity.h"
#include "sdk/math.h"
#include "sdk/misc.h"

#include <imgui.h>

using namespace std::literals;

static bool world_to_screen(const sdk::Vector &world, const sdk::VMatrix &vm, sdk::Vector &out) {
    float w = vm.m[3][0] * world.x + vm.m[3][1] * world.y + vm.m[3][2] * world.z + vm.m[3][3];
    if (w < 0.01f) {
        return false;
    }
    float x = world.x * vm[0][0] + world.y * vm[0][1] + world.z * vm[0][2] + vm[0][3];
    float y = world.x * vm[1][0] + world.y * vm[1][1] + world.z * vm[1][2] + vm[1][3];
    auto size = ImGui::GetIO().DisplaySize;
    if (size.x * size.y == 0) {
        return false;
    }
    out.x = size.x / 2.0f;
    out.y = size.y / 2.0f;
    out.x *= 1.0f + x / w;
    out.y *= 1.0f - y / w;
    return true;
}

void cheats::Esp::run() {
    g_log->dbg(L"Esp::run()");
    std::this_thread::sleep_for(100ms);

    // random ass number
    const auto k_update_time = 1s / 128;
    auto last_time = std::chrono::system_clock::now();

    // lazy hack
    uint32_t player_class_id = 0;

    while (!g_c->shutdown) {
        // make the timing consistent in the future
        // probably not a huge deal but this is just cringe
        if (!globals::local or (!g_cfg.esp.enabled and !::GetAsyncKeyState(VK_MENU))) {
            goto TIMING;
        }
        {
            auto local_team = globals::local.get_team();
            if (player_class_id == 0) [[unlikely]] {
                player_class_id = globals::local.get_class_id();
            }

            auto vm = sdk::client::get_view_matrix();

            std::lock_guard lck(g_esp.rects_mutex);
            g_esp.rects_private->clear();
            for (int i = 1; i < 64; ++i) {
                // in the case of weird memory stuff
                if (g_c->shutdown) {
                    break;
                }
                auto player = sdk::Entity(g_c->mem->read<uint32_t>(g_c->mem->client.base + hazedumper::signatures::dwEntityList + i * 0x10));
                if (!player) {
                    continue;
                }
                if (player.get() == globals::local.get()) {
                    continue;
                }
                if (player.get_class_id() != player_class_id) {
                    continue;
                }
                if (!player.is_alive()) {
                    continue;
                }
                if (player.is_dormant()) {
                    continue;
                }
                if (player.get_team() == local_team) {
                    continue;
                }
                // head bone
                auto [head, success] = player.get_bone_pos(int(sdk::Bones::Head));
                if (!success) {
                    continue;
                }
                sdk::Vector feet = player.get_origin();
                sdk::Vector s_top{}, s_bot{};
                if (!world_to_screen(head, vm, s_top) || !world_to_screen(feet, vm, s_bot)) {
                    continue;
                }
                float ratio = 0.7f;
                float h = s_bot.y - s_top.y;
                float w = h * ratio;
                g_esp.rects_private->push_back(Rect{s_top.x - w / 2.f, s_top.y, s_top.x + w / 2.f, s_bot.y});
            }
        }
    TIMING:
        auto cur_time = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration(cur_time - last_time);
        if (elapsed < k_update_time) {
            std::this_thread::sleep_for(k_update_time - elapsed);
        }
        last_time = std::chrono::system_clock::now();
    }
    g_log->dbg(L"Esp::run(): Exiting");
}
