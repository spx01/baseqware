#pragma once

#include <mutex>
#include <vector>

namespace cfg {
    struct Esp {
        bool enabled = true;
    };
}// namespace cfg

namespace cheats {
    class Esp {
    public:
        struct Rect {
            // top left
            float x1, y1;
            // bottom right
            float x2, y2;
            int thickness;
        };

        template<typename F>
        inline void use_rects(F &&f) {
            std::lock_guard lck(this->rects_mutex);
            f(*this->rects_public);
            std::swap(this->rects_public, this->rects_private);
        }

        static void run();
        std::mutex rects_mutex;

    private:
        std::vector<Rect> rects[2];
        std::vector<Rect> *rects_public = &rects[0];
        std::vector<Rect> *rects_private = &rects[1];
    };

    inline Esp g_esp;
}// namespace cheats