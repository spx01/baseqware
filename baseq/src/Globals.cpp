#include "Globals.h"

#include "Cheat.h"

#include "sdk/client.h"
#include "sdk/engine.h"

using namespace std::literals;
using namespace sdk;

void globals::run() {
    // we don't need this to be fast
    const auto k_delay_time = 1s / 10;
    while (!g_c) {
        ::Sleep(100);
    }
    while (!g_c->shutdown) {
        if (!engine::in_game()) {
            globals::local = Entity{};
            std::this_thread::sleep_for(k_delay_time);
            continue;
        }

        // FIXME
        globals::local = client::get_local_player();
        local_class = globals::local.get_class_id();

        if (!globals::local) {
            globals::local = client::get_local_player();
        }
        std::this_thread::sleep_for(k_delay_time);
    }
}