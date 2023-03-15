#include "KeInterface.h"

KeInterface::KeInterface() {
    this->h_driver = ::CreateFileW(
            L"\\\\.\\pased",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);
    if (this->h_driver == NULL || this->h_driver == INVALID_HANDLE_VALUE) {
        // not very clean design but ok
        PostQuitMessage(0);
        this->invalid = true;
    }
}

KeInterface::~KeInterface() {
    ::CloseHandle(this->h_driver);
    this->h_driver = NULL;
}

DWORD KeInterface::get_target_pid() {
    ULONG pid{};
    DWORD bytes;
    if (!::DeviceIoControl(this->h_driver, IO_GET_ID_REQUEST, &pid, sizeof(pid), &pid, sizeof(pid), &bytes, NULL))
        return NULL;
    return static_cast<DWORD>(pid);
}

DWORD KeInterface::get_client_module() {
    ULONG addr{};
    DWORD bytes;
    if (!::DeviceIoControl(this->h_driver, IO_GET_MODULE_REQUEST, &addr, sizeof(addr), &addr, sizeof(addr), &bytes, NULL))
        return NULL;
    return addr;
}
