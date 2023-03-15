#include "Cheat.h"

#include "DriverPlaceholder.h"
#include "Util.h"

#include <Windows.h>
#include <imgui.h>

Cheat::Cheat() {
    this->mem = std::make_unique<Memory>();
}

Cheat::~Cheat() {
}

void Cheat::update() {
    this->mem->update();

    // determine whether the game is still running
    if (!this->mem->is_valid()) {
        if (this->game_hwnd != NULL) {
            g_log->info(L"CSGO process destroyed");
        }
        // game probably closed, so invalidate the window
        this->game_hwnd = NULL;
        this->game_focused = false;
        return;
    }

    if (this->game_hwnd == NULL) {
        HWND hwnd = ::FindWindowW(L"Valve001", NULL);
        if (hwnd == NULL) {
            return;
        }
        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);
        if (pid == this->mem->game_pid) {
            g_log->info(L"Found game window");
            g_log->dbg(L"hwnd: {:0x}", uintptr_t(hwnd));
            g_log->dbg(L"pid: {:0x}", pid);
            this->game_hwnd = hwnd;
        }
    }

    // determine whether the game is in focus
    HWND foreground = ::GetForegroundWindow();
    this->game_focused = (foreground == this->game_hwnd);
}

void Cheat::render_overlay() {
    if (!this->game_focused) return;
    if (GetAsyncKeyState(VK_TAB)) {
        ImGui::Begin("baseq");
        ImGui::SetWindowSize(ImVec2(0, 0));
        ImGui::Text("game is focused");
        ImGui::End();
    }
}