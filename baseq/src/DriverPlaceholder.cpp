#include "DriverPlaceholder.h"

#include "Util.h"

DWORD driver::get_pid() {
    HWND hwnd = FindWindowW(NULL, L"Notepad");
    if (hwnd == NULL) {
        return 0;
    }
    WCHAR buf[256];
    GetClassNameW(hwnd, buf, 256);
    g_log->dbg(L"DRIVER: Found a notepad (?), class {}", buf);
    DWORD pid;
    if (GetWindowThreadProcessId(hwnd, &pid) == 0) {
        return 0;
    }
    return pid;
}