#include "sdk/entity.h"

namespace sdk {
    std::pair<Vector, bool> Entity::get_bone_pos(int bone) const {
        Vector res{};
        auto bones = this->get_bones();
        if (!bones)
            return {res, false};
        res.x = g_c->mem->read<float>(bones + 0x30 * bone + 0xC);
        res.y = g_c->mem->read<float>(bones + 0x30 * bone + 0xC + 0x10);
        res.z = g_c->mem->read<float>(bones + 0x30 * bone + 0xC + 0x20);
        return {res, true};
    }

    uint32_t Entity::get_class_id() const {
        auto i_client_networkable = g_c->mem->read<uint32_t>(this->addr + 0x8);
        auto get_client_class = g_c->mem->read<uint32_t>(i_client_networkable + 2 * 0x4);
        auto client_class = g_c->mem->read<uint32_t>(get_client_class + 0x1);
        return g_c->mem->read<uint32_t>(client_class + 0x14);
    }
}// namespace sdk