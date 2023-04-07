#include "KeInterface.h"

#include "Util.h"

KeInterface::KeInterface() {
    this->h_driver = ::CreateFileW(
            L"\\\\.\\pased",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);
    if (this->h_driver == NULL || this->h_driver == INVALID_HANDLE_VALUE) {
        // not very clean design but ok
        ::PostQuitMessage(0);
        this->invalid = true;
    }
}

KeInterface::~KeInterface() {
    ::CloseHandle(this->h_driver);
}

DWORD KeInterface::get_target_pid() {
    ULONG pid{};
    DWORD bytes;
    if (!::DeviceIoControl(this->h_driver, IO_GET_ID_REQUEST, &pid, sizeof(pid), &pid, sizeof(pid), &bytes, NULL))
        return NULL;
    return static_cast<DWORD>(pid);
}

std::pair<DWORD, DWORD> KeInterface::get_module(REQUESTABLE_MODULE mod) {
    KERNEL_GET_MODULE_REQUEST req = {
            .mod = mod,
            .addr = 0,
            .size = 0};
    DWORD bytes;
    if (!::DeviceIoControl(this->h_driver, IO_GET_MODULE_REQUEST, &req, sizeof(req), &req, sizeof(req), &bytes, NULL)) {
        g_log->err(L"Failed to get module (type {}): kernel driver error", int(mod));
        ::PostQuitMessage(0);
    }
    return {req.addr, req.size};
}
