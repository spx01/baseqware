#include "Util.h"

#include <cassert>
#include <cstdio>


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
    fputws(L"\nPress ENTER to exit\n", stdout);
    FILE *f;
    freopen_s(&f, "CONIN$", "r", stdin);
    (void) getchar();
    fclose(stdout);
    fclose(stdin);
    fputws(L"exit\n", this->fp);
    fclose(this->fp);
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
