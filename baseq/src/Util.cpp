#include "Util.h"

#include <cassert>
#include <cstdio>

Logger::Logger(const char *filename) {
    AllocConsole();
    FILE *conout;
    freopen_s(&conout, "CONOUT$", "w", stdout);
    assert(conout != nullptr);
    if (!filename) {
        this->fp = nullptr;
    } else {
        fopen_s(&this->fp, filename, "w");
    }
    this->m_con_out = GetStdHandle(STD_OUTPUT_HANDLE);
    // print epic ascii art here
}

Logger::~Logger() {
    fclose(stdout);
    fputws(L"exit\n", this->fp);
    fclose(this->fp);
    FreeConsole();
}

#ifndef NDEBUG
void Logger::dbg_nofmt(const WCHAR *str) {
    auto tu = time(nullptr);
    tm tl;
    localtime_s(&tl, &tu);
    std::wstring fmt = std::format(L"[{:02}:{:02}:{:02}] [DBG] {}\n", tl.tm_hour, tl.tm_min, tl.tm_sec, str);
    SetConsoleTextAttribute(m_con_out, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
    fputws(fmt.c_str(), stdout);
    if (fp) {
        fputws(fmt.c_str(), fp);
    }
}
#endif

void Logger::info_nofmt(const WCHAR *str) {
    auto tu = time(nullptr);
    tm tl;
    localtime_s(&tl, &tu);
    std::wstring fmt = std::format(L"[{:02}:{:02}:{:02}] [INF] {}\n", tl.tm_hour, tl.tm_min, tl.tm_sec, str);
    SetConsoleTextAttribute(m_con_out, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    fputws(fmt.c_str(), stdout);
    if (fp) {
        fputws(fmt.c_str(), fp);
    }
}

void Logger::err_nofmt(const WCHAR *str) {
    auto tu = time(nullptr);
    tm tl;
    localtime_s(&tl, &tu);
    std::wstring fmt = std::format(L"[{:02}:{:02}:{:02}] [ERR] {}\n", tl.tm_hour, tl.tm_min, tl.tm_sec, str);
    SetConsoleTextAttribute(m_con_out, FOREGROUND_RED | FOREGROUND_INTENSITY);
    fputws(fmt.c_str(), stdout);
    if (fp) {
        fputws(fmt.c_str(), fp);
    }
}
