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
        DWORD base = this->ki.get_client_module();
        this->client_base = base;
        this->game_pid = pid;
    }
}
