#include "KeInterface.h"

KeInterface::KeInterface(const char *reg_path) {
    this->h_driver = CreateFileA(
            reg_path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);
    if (this->h_driver == NULL || this->h_driver == INVALID_HANDLE_VALUE)
        this->invalid = true;
}

KeInterface::~KeInterface() {
    CloseHandle(this->h_driver);
    this->h_driver = NULL;
}

DWORD KeInterface::get_target_pid() {
    ULONG pid{};
    DWORD bytes;
    if (!DeviceIoControl(this->h_driver, IO_GET_ID_REQUEST, &pid, sizeof(pid), &pid, sizeof(pid), &bytes, NULL))
        return NULL;
    return static_cast<DWORD>(pid);
}

DWORD KeInterface::get_module_base(const char *module_name) {
    ULONG addr{};
    DWORD bytes;
    if (!DeviceIoControl(this->h_driver, IO_GET_MODULE_REQUEST, &addr, sizeof(addr), &addr, sizeof(addr), &bytes, NULL))
        return NULL;
    return addr;
}