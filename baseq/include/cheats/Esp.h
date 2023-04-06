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
        // these are screen coords, but not in pixels
        // let the caller scale them
        struct Rect {
            // top left
            float x1;
            float y1;
            // bottom right
            float x2;
            float y2;
            int thickness;
        };

        template<typename F>
        inline const std::vector<Rect> &use_rects(F &&f) const {
            f(*this->rects_public);
            std::lock_guard lck(this->rects_mutex);
            std::swap(this->rects_public, this->rects_private);
        }

        static void run();

    private:
        std::vector<Rect> rects[2];
        std::vector<Rect> *rects_public = &rects[0];
        std::vector<Rect> *rects_private = &rects[1];
        std::mutex rects_mutex;
    };

    inline Esp g_esp;
}// namespace cheats