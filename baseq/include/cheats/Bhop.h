#pragma once

namespace cfg {
    struct Bhop {
        bool enabled = true;
    };
}// namespace cfg

namespace cheats {
    class Bhop {
    public:
        static void run();
    };
}// namespace cheats