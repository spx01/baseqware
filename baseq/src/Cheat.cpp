#include "Cheat.h"

#include "DriverPlaceholder.h"
#include "Util.h"

#include <Windows.h>
#include <imgui.h>

Cheat::~Cheat() {
    // remove our hook
    if (this->destroy_hook) UnhookWinEvent(this->destroy_hook);
}

static DWORD g_game_tid;

BOOL CALLBACK Cheat::enum_windows_proc(HWND hwnd, LPARAM lParam) {
    DWORD pid, tid;
    tid = GetWindowThreadProcessId(hwnd, &pid);
    if (pid == static_cast<DWORD>(lParam)) {
        g_c->game_hwnd = hwnd;
        g_game_tid = tid;
        return FALSE;
    }
    return TRUE;
}

// i intially wanted to make it so that the cheat works if you close the game and reopen it
// however this is not the way to do it
// the problem is the callback gets called before the window is fully gone
// so i end up updating the hwnd to the same one as before
// working around this seems like needless complexity
// so i'll just leave this here as it is
// once kernel driver is implemented this will be easy to do
static bool g_closed = false;
void Cheat::destroy_callback(HWINEVENTHOOK hweh, DWORD event, HWND hwnd, LONG obj, LONG child, DWORD event_thread, DWORD event_time) {
    // when game window is destroyed, invalidate our hwnd
    if (event == EVENT_OBJECT_DESTROY && hwnd == g_c->game_hwnd && obj == OBJID_WINDOW && child == INDEXID_CONTAINER) {
        g_log->info(L"game window closed");
        g_c->game_hwnd = NULL;
        g_closed = true;
    }
}

void Cheat::update() {
    // TODO: determine whether the game is still running
    // we are going to to this by storing the hwnd of the game window
    // and checking if it is still valid
    if (g_closed) {
        this->game_focused = false;
        return;
    }
    if (this->game_hwnd == NULL) {
        if (!this->try_initialize()) {
            return;
        }
    }

    // determine whether the game is in focus
    HWND foreground = GetForegroundWindow();
    this->game_focused = foreground == this->game_hwnd;
}

bool Cheat::try_initialize() {
    DWORD pid = driver::get_pid();
    if (pid == 0) return false;
    EnumWindows(this->enum_windows_proc, static_cast<LPARAM>(pid));
    if (this->game_hwnd == NULL) {
        return false;
    }
    g_log->dbg(L"got a window, hwnd {:x}", reinterpret_cast<uintptr_t>(this->game_hwnd));
    // register our callback such that we know when the window is destroyed
    this->destroy_hook = SetWinEventHook(
            EVENT_OBJECT_DESTROY,
            EVENT_OBJECT_DESTROY,
            NULL,
            this->destroy_callback,
            pid,
            g_game_tid,
            WINEVENT_OUTOFCONTEXT);
    return true;
}

void Cheat::render_overlay() {
    if (!this->game_focused) return;
    if (GetAsyncKeyState(VK_TAB)) {
        ImGui::Begin("baseq");
        ImGui::Text("game is focused");
        ImGui::End();
    }
}