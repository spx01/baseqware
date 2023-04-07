#pragma once

#include <chrono>
#include <format>
#include <string>

#include <Windows.h>

#include <imgui.h>

namespace util {
    bool in_menu();
}

// pretend this is a singleton
class Logger {
public:
    Logger(const char *filename);
    ~Logger();

    template<typename... Args>
    inline void info(std::wformat_string<Args...> fmt, Args &&...args) {
        std::wstring str = std::format(fmt, std::forward<Args>(args)...);
        info_nofmt(str.c_str());
    }
    template<typename... Args>
    inline void err(std::wformat_string<Args...> fmt, Args &&...args) {
        std::wstring str = std::format(fmt, std::forward<Args>(args)...);
        err_nofmt(str.c_str());
    }
#ifndef NDEBUG
    template<typename... Args>
    inline void dbg(std::wformat_string<Args...> fmt, Args &&...args) {
        std::wstring str = std::format(fmt, std::forward<Args>(args)...);
        dbg_nofmt(str.c_str());
    }
    void dbg_nofmt(const WCHAR *str);
#else
    template<typename... Args>
    inline void dbg(std::wformat_string<Args...> fmt, Args &&...args) {}
#endif
    void info_nofmt(const WCHAR *str);
    void err_nofmt(const WCHAR *str);

private:
    HANDLE m_con_out;
    FILE *fp;
};

inline std::unique_ptr<Logger> g_log;

namespace gutil {
    void draw_text_border(ImDrawList *dw, const char *text, ImVec2 pos, ImU32 text_color, ImFont *font, float font_size);
}