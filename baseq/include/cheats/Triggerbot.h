#pragma once

namespace cfg {
    struct Triggerbot {
        bool enabled = false;
    };
}// namespace cfg

namespace cheats {
    class Triggerbot {
    public:
        static void run();
    };

    inline Triggerbot g_trigger;
}// namespace cheats
