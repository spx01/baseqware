#include "Memory.h"

#include "Util.h"

#include "sdk/math.h"

Memory::Memory() : ki() {
    if (this->ki.is_invalid()) {
        g_log->err(L"Kernel interface failed to initialize; is driver running?");
        this->valid = false;
    }
}

void Memory::update() {
    DWORD pid = this->ki.get_target_pid();
    if (pid == 0) {
        valid = false;
        return;
    }
    if (pid != this->game_pid) {
        valid = true;
        this->game_pid = pid;
        std::tie(this->client.base, this->client.size) = this->ki.get_module(REQUESTABLE_MODULE::CLIENT_MODULE);
        std::tie(this->engine.base, this->engine.size) = this->ki.get_module(REQUESTABLE_MODULE::ENGINE_MODULE);
    }
}