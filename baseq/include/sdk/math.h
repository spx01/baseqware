#pragma once

namespace sdk {
    struct VMatrix {
        float m[4][4];
        const float *operator[](int i) const { return m[i]; }
        float *operator[](int i) { return m[i]; }
    };
    struct Vector {
        Vector() {
            x = y = z = 0.0f;
        }
        float x, y, z;
    };
}// namespace sdk