#pragma once

namespace cfg {
    struct Bhop {
        bool enabled = false;
    };
}// namespace cfg

namespace cheats {
    class Bhop {
    public:
        static void run();
    };
}// namespace cheats