#include "Cheat.h"

#include <thread>

#include <Windows.h>
#include <dwmapi.h>

#include <imgui.h>

#include "Util.h"

#include "cheats/Esp.h"

Cheat::Cheat(HWND overlay) {
    this->mem = std::make_unique<Memory>();
    this->overlay = overlay;
    this->dispatch_threads();
}

Cheat::~Cheat() {
    for (auto &thread: this->threads) {
        thread.join();
    }
}

void Cheat::update() {
    this->mem->update();

    // determine whether the game is still running
    if (!this->mem->is_valid()) {
        if (this->game_hwnd != NULL) {
            g_log->info(L"CSGO process destroyed");
            ::PostQuitMessage(0);
            this->shutdown = true;
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
            g_log->dbg(L"hwnd: {:8X}", uintptr_t(hwnd));
            g_log->dbg(L"pid: {:8X}", pid);
            // TODO: need engine and vstdlib as well
            g_log->info(L"client.dll -> 0x{:08X}", this->mem->client.base);
            g_log->info(L"engine.dll -> 0x{:08X}", this->mem->engine.base);
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

static void draw_esp(const std::vector<cheats::Esp::Rect> &rects) {
    auto draw_list = ImGui::GetBackgroundDrawList();
    auto size = ImGui::GetIO().DisplaySize;
    for (const auto &rect: rects) {
        /*
        ImVec2 border[4][2];
        border[0][0] = {rect.x1, rect.y1};
        border[0][1] = {rect.x2 - rect.thickness, rect.y1 + rect.thickness}; */

        // draw_list->AddRectFilled({rect.x1, rect.y1}, {rect.x2, rect.y2}, IM_COL32(255, 0, 0, 255));
        if (rect.x2 - rect.x1 > size.x / 2.f) {
            continue;
        }
        draw_list->AddRect({rect.x1, rect.y1}, {rect.x2, rect.y2}, IM_COL32(255, 0, 0, 255), 0.2f);
    }
}

void Cheat::update_overlay() {
    if (!this->game_focused) return;
    SetWindowPos(this->overlay, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

    auto &io = ImGui::GetIO();
    // io.MouseDrawCursor = true;
    auto state = ::GetAsyncKeyState(VK_LBUTTON);
    if (!(state & 0x8000) && io.MouseDown[0]) {
        io.MouseReleased[0] = true;
    }
    io.MouseDown[0] = state & 0x8000;
    POINT p;
    ::GetCursorPos(&p);
    ::ScreenToClient(overlay, &p);
    io.MousePos = {float(p.x), float(p.y)};

    auto draw_list = ImGui::GetBackgroundDrawList();
    gutil::draw_text_border(draw_list, "baseqware", ImVec2{20, 0}, IM_COL32(255, 255, 255, 255), ImGui::GetFont(), 20);
    if (::GetAsyncKeyState(VK_TAB)) {
        ImGui::Begin("baseq");
        if (ImGui::Button("quit")) {
            ::PostQuitMessage(0);
            this->shutdown = true;
            ::ShowWindow(this->overlay, SW_HIDE);
            ::UpdateWindow(this->overlay);
        }
        ImGui::End();
    }
    // just found out about background draw list
    /*
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({float(this->client_width()), float(this->client_height())});
        ImGui::Begin("something", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
    */
    if (!util::in_menu()) {
        cheats::g_esp.use_rects(draw_esp);
    }
}

void Cheat::dispatch_threads() {
    this->threads.emplace_back(cheats::Esp::run);
}
