#pragma once

#include "Cheat.h"

#include "sdk/const.h"
#include "sdk/math.h"

namespace sdk {
    class Entity {
    public:
        Entity(uint32_t addr) {
            this->addr = addr;
        }

        inline auto get() const {
            return this->addr;
        }

        operator bool() const {
            return bool(this->get());
        }

        inline LIFE get_lifestate() const {
            return g_c->mem->read<LIFE>(this->addr + hazedumper::netvars::m_lifeState);
        }

        inline bool is_alive() const {
            return this->get_lifestate() == LIFE::ALIVE;
        }

        inline bool is_dormant() const {
            return g_c->mem->read<bool>(this->addr + hazedumper::signatures::m_bDormant);
        }

        inline int get_team() const {
            return g_c->mem->read<int>(this->addr + hazedumper::netvars::m_iTeamNum);
        }

        inline uint32_t get_bones() const {
            return g_c->mem->read<uint32_t>(this->addr + hazedumper::netvars::m_dwBoneMatrix);
        }

        std::pair<Vector, bool> get_bone_pos(int bone) const;

        uint32_t get_class_id() const;

        inline Vector get_origin() const {
            return g_c->mem->read<Vector>(this->addr + hazedumper::netvars::m_vecOrigin);
        }

        inline Vector get_view_offset() const {
            return g_c->mem->read<Vector>(this->addr + hazedumper::netvars::m_vecViewOffset);
        }

        /* inline bool is_visible() const {
            return this->get_flags() & FL::FL_ONGROUND;
        }

        inline int get_health() const {
            return g_c->mem->read<int>(this->addr + hazedumper::netvars::m_iHealth);
        }


        inline int get_flags() const {
            return g_c->mem->read<int>(this->addr + hazedumper::netvars::m_fFlags);
        }


        inline Vector get_eye_pos() const {
            return this->get_origin() + this->get_view_offset();
        }

        inline Vector get_bone_pos(int bone, const sdk::VMatrix &vm) const {
            return this->get_bone_pos(bone).transform(vm);
        }

        inline Vector get_bone_pos(int bone, const sdk::VMatrix &vm, const Vector &origin) const {
            return this->get_bone_pos(bone, vm) + origin;
        }

        inline Vector get_bone_pos(int bone, const sdk::VMatrix &vm, const Vector &origin, const Vector &view_offset) const {
            return this->get_bone_pos(bone, vm, origin) - view_offset;
        }

        inline Vector get_bone_pos(int bone, const sdk::VMatrix &vm, const Vector &origin, const Vector &view_offset, const Vector &screen_size) const {
            return this->get_bone_pos(bone, vm, origin, view_offset).to_screen(screen_size)
        } */

    private:
        uint32_t addr;
    };
}// namespace sdk