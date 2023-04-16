#define IMGUI_DEFINE_MATH_OPERATORS

#include "Util.h"

#include <cassert>
#include <cstdio>

#include <imgui.h>


void print_art(HANDLE console) {
    // figlet kban
    static const char *art[] = {
            R"('||                                                                         )",
            R"( || ...   ....    ....    ....    ... .  ... ... ...  ....   ... ..    .... )",
            R"( ||'  || '' .||  ||. '  .|...|| .'   ||   ||  ||  |  '' .||   ||' '' .|...||)",
            R"( ||    | .|' ||  . '|.. ||      |.   ||    ||| |||   .|' ||   ||     ||     )",
            R"( '|...'  '|..'|' |'..|'  '|...' '|..'||     |   |    '|..'|' .||.     '|...')",
            R"(                                     ||                                     )",
            R"(                                    ''''                                    )"};
    const WORD colors[] = {
            FOREGROUND_RED,
            FOREGROUND_RED | FOREGROUND_GREEN,
            FOREGROUND_GREEN,
            FOREGROUND_GREEN | FOREGROUND_BLUE,
            FOREGROUND_BLUE,
            FOREGROUND_BLUE | FOREGROUND_RED,
    };
    int ncolors = sizeof(colors) / sizeof(colors[0]);
    int lines = sizeof(art) / sizeof(*art);
    int width = int(strlen(*art));
    for (int i = 0; i < lines; ++i) {
        for (int j = 0; j < width; ++j) {
            ::SetConsoleTextAttribute(console, colors[(j + 2 * i) / 2 % ncolors] | FOREGROUND_INTENSITY);
            fputc(art[i][j], stdout);
        }
        fputc('\n', stdout);
    }
}

Logger::Logger(const char *filename) {
    ::AllocConsole();
    ::SetConsoleTitleW(L"baseq (" __DATE__ " build)");
    FILE *conout;
    freopen_s(&conout, "CONOUT$", "w", stdout);
    assert(conout != nullptr);
    this->m_con_out = ::GetStdHandle(STD_OUTPUT_HANDLE);

    print_art(this->m_con_out);

    if (!filename) {
        this->fp = nullptr;
    } else {
        fopen_s(&this->fp, filename, "w");
    }
}

Logger::~Logger() {
    ::SetConsoleTextAttribute(this->m_con_out, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#ifndef NDEBUG
    fputws(L"\nPress ENTER to exit\n", stdout);
    FILE *f;
    freopen_s(&f, "CONIN$", "r", stdin);
    (void) getchar();
#endif
    fclose(stdout);
    fclose(stdin);
    if (this->fp) {
        fputws(L"exit\n", this->fp);
        fclose(this->fp);
    }
    ::FreeConsole();
}

#ifndef NDEBUG
void Logger::dbg_nofmt(const WCHAR *str) {
    auto tu = time(nullptr);
    tm tl;
    localtime_s(&tl, &tu);
    std::wstring fmt = std::format(L"[{:02}:{:02}:{:02}] [DBG] {}\n", tl.tm_hour, tl.tm_min, tl.tm_sec, str);
    ::SetConsoleTextAttribute(this->m_con_out, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
    fputws(fmt.c_str(), stdout);
    if (this->fp) {
        fputws(fmt.c_str(), this->fp);
    }
}
#endif

void Logger::info_nofmt(const WCHAR *str) {
    auto tu = time(nullptr);
    tm tl;
    localtime_s(&tl, &tu);
    std::wstring fmt = std::format(L"[{:02}:{:02}:{:02}] [INF] {}\n", tl.tm_hour, tl.tm_min, tl.tm_sec, str);
    ::SetConsoleTextAttribute(this->m_con_out, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    fputws(fmt.c_str(), stdout);
    if (this->fp) {
        fputws(fmt.c_str(), this->fp);
    }
}

void Logger::err_nofmt(const WCHAR *str) {
    auto tu = time(nullptr);
    tm tl;
    localtime_s(&tl, &tu);
    std::wstring fmt = std::format(L"[{:02}:{:02}:{:02}] [ERR] {}\n", tl.tm_hour, tl.tm_min, tl.tm_sec, str);
    ::SetConsoleTextAttribute(this->m_con_out, FOREGROUND_RED | FOREGROUND_INTENSITY);
    fputws(fmt.c_str(), stdout);
    if (this->fp) {
        fputws(fmt.c_str(), this->fp);
    }
}

void gutil::draw_text_border(ImDrawList *dw, const char *text, ImVec2 pos, ImU32 text_color, ImFont *font, float font_size) {
    static constexpr ImU32 border_color = IM_COL32(0, 0, 0, 255);
    // Draw the border
    dw->AddText(font, font_size, pos + ImVec2(-1, -1), border_color, text);
    dw->AddText(font, font_size, pos + ImVec2(-1, 1), border_color, text);
    dw->AddText(font, font_size, pos + ImVec2(1, -1), border_color, text);
    dw->AddText(font, font_size, pos + ImVec2(1, 1), border_color, text);
    // Draw the text on top of the border
    dw->AddText(font, font_size, pos, text_color, text);
}

// github.com/b1scoito/external
bool util::in_menu() {
    CURSORINFO ci{};
    ci.cbSize = sizeof(CURSORINFO);
    if (!GetCursorInfo(&ci))
        return false;
    auto handle = ci.hCursor;
    if (handle > (HCURSOR) 50000 && handle < (HCURSOR) 100000)
        return true;
    return false;
}

// https://github.com/b1scoito/external/blob/main/external/utils.hpp
// precise sleep
void util::sleep(float ms) {
    static HMODULE ntdll = [] {
        return ::GetModuleHandleW(L"ntdll.dll");
    }();

    typedef NTSTATUS(WINAPI * fnNtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval);
    static fnNtDelayExecution oNtDelayExecution = NULL;

    if (!oNtDelayExecution)
        oNtDelayExecution = (fnNtDelayExecution) GetProcAddress(ntdll, "NtDelayExecution");

    typedef NTSTATUS(WINAPI * fnZwSetTimerResolution)(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution);
    static fnZwSetTimerResolution oZwSetTimerResolution = NULL;

    if (!oZwSetTimerResolution)
        oZwSetTimerResolution = (fnZwSetTimerResolution) GetProcAddress(ntdll, "ZwSetTimerResolution");

    static std::once_flag flag;
    std::call_once(flag, []() {
            ULONG current;
            oZwSetTimerResolution( (ULONG)(0.5f * 10000.f), true, &current ); });

    if (ms < 0.5f)
        ms = 0.5f;

    LARGE_INTEGER time = {};
    time.QuadPart = -1 * (LONGLONG) (ms * 10000.f);

    oNtDelayExecution(false, &time);
}
