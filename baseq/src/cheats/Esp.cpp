#include "cheats/Esp.h"

#include <Windows.h>

#include <chrono>
#include <thread>

#include "Cheat.h"
#include "Config.h"
#include "Memory.h"
#include "Util.h"

#include "sdk/client.h"
#include "sdk/const.h"
#include "sdk/entity.h"
#include "sdk/math.h"

#include <imgui.h>

using namespace std::literals;

static bool world_to_screen(const sdk::Vector &world, const sdk::VMatrix &vm, sdk::Vector &out) {
    float w = vm.m[3][0] * world.x + vm.m[3][1] * world.y + vm.m[3][2] * world.z + vm.m[3][3];
    if (w < 0.001f) {
        return false;
    }
    float x = world.x * vm[0][0] + world.y * vm[0][1] + world.z * vm[0][2] + vm[0][3];
    float y = world.x * vm[1][0] + world.y * vm[1][1] + world.z * vm[1][2] + vm[1][3];
    auto size = ImGui::GetIO().DisplaySize;
    if (size.x * size.y == 0) {
        return false;
    }
    x /= w;
    y /= w;
    out.x = size.x / 2.0f * x + x + size.x / 2.0f;
    out.y = -size.y / 2.0f * y + y + size.y / 2.0f;
    return true;
}

void cheats::Esp::run() {
    g_log->dbg(L"Esp::run()");
    std::this_thread::sleep_for(100ms);

    const auto k_update_time = 1s / 128;
    auto last_time = std::chrono::system_clock::now();

    // lazy hack
    uint32_t player_class_id = 0;

    while (!g_c->shutdown) {
        // make the timing consistent in the future
        // probably not a huge deal but this is just cringe
        auto local_player = sdk::Entity(sdk::client::get_local_player());
        if (!local_player) {
            goto TIMING;
        }
        {
            auto local_team = local_player.get_team();
            if (player_class_id == 0) [[unlikely]] {
                player_class_id = local_player.get_class_id();
            }
            auto local_player_alive = local_player.is_alive();

            auto vm = sdk::client::get_view_matrix();

            std::lock_guard lck(g_esp.rects_mutex);
            g_esp.rects_private->clear();
            for (int i = 1; i < 32; ++i) {
                // in the case of weird memory stuff
                if (g_c->shutdown) {
                    break;
                }
                auto player = sdk::Entity(g_c->mem->read<uint32_t>(g_c->mem->client_base + hazedumper::signatures::dwEntityList + i * 0x10));
                if (!player) {
                    continue;
                }
                if (player.get() == local_player.get()) {
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
                auto [head, success] = player.get_bone_pos(8);
                if (!success) {
                    continue;
                }
                sdk::Vector screen;
                if (!world_to_screen(head, vm, screen)) {
                    continue;
                }
                g_esp.rects_private->push_back(Rect{screen.x - 10.f, screen.y - 30.f, screen.x + 10.f, screen.y + 30.f, 4});
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
