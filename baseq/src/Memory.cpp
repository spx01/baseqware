#include "Memory.h"

#include "Util.h"

Memory::Memory() : ki() {
    if (this->ki.is_invalid()) {
        g_log->err(L"Kernel interface failed to initialize; is driver running?");
        return;
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
        std::tie(this->client_base, this->client_size) = this->ki.get_module(REQUESTABLE_MODULE::CLIENT_MODULE);
        this->game_pid = pid;
    }
}
