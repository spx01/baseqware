#include "Cheat.h"

#include "Util.h"

#include <Windows.h>
#include <dwmapi.h>

#include <imgui.h>

Cheat::Cheat(HWND overlay) {
    this->mem = std::make_unique<Memory>();
    this->overlay = overlay;
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
        } else {
            return;
        }
    }


    // determine whether the game is in focus
    HWND foreground = ::GetForegroundWindow();
    bool last_focused = this->game_focused;
    this->game_focused = (foreground == this->game_hwnd);

    // the point of this is to only run this stuff when game is refocused
    // as it's probably not very cheap
    // callbacks might be a better solution
    // TODO: extract to a func
    if (last_focused != this->game_focused) {
        // weird shit
        Sleep(100);
        if (!last_focused) {
            // game just got focus
            g_log->dbg(L"game just got focused i suppose");
            RECT game_rect;
            ::GetClientRect(this->game_hwnd, &game_rect);
            // convert to screen coords so that we can position our overlay
            POINT pos{};
            ::ClientToScreen(this->game_hwnd, &pos);
            this->client_area = {
                    .left = pos.x,
                    .top = pos.y,
                    .right = pos.x + game_rect.right,
                    .bottom = pos.y + game_rect.bottom,
            };
            g_log->dbg(L"set area to {} {} {} {}", this->client_area.left, this->client_area.top, this->client_area.right, this->client_area.bottom);
            auto ret = ::SetWindowPos(this->overlay, HWND_TOPMOST,
                                      this->client_area.left, this->client_area.top,
                                      this->client_width(), this->client_height(),
                                      SWP_NOACTIVATE);

            MARGINS margins{
                    this->client_area.left,
                    this->client_area.top,
                    this->client_width(),
                    this->client_height(),
            };
            ::DwmExtendFrameIntoClientArea(this->overlay, &margins);
        }
    }
}

void Cheat::render_overlay() {
    if (!this->game_focused) return;
    SetWindowPos(this->overlay, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    if (::GetAsyncKeyState(VK_TAB)) {
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(0, 0));
        ImGui::Begin("baseq");
        ImGui::Text("game is focused");
        ImGui::End();
    }
}