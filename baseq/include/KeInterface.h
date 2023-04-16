#pragma once

#include <Windows.h>

#include <utility>

#include "Util.h"

// Request to read virtual user memory (memory of a program) from kernel space
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0691 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Request to write virtual user memory (memory of a program) from kernel space
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0692 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Request to retrieve the process id of csgo process, from kernel space
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0693 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Request to retrieve the base address of client.dll in csgo.exe from kernel space
#define IO_GET_MODULE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0694 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

enum class REQUESTABLE_MODULE : int {
    CLIENT_MODULE = 0,
    ENGINE_MODULE,
    /* VSTDLIB_MODULE,
    TIER0_MODULE, */
    _COUNT
};

struct KERNEL_READ_REQUEST {
    ULONG pid;
    ULONG addr;
    ULONG size;
    ULONGLONG out[1];
};
using PKERNEL_READ_REQUEST = KERNEL_READ_REQUEST *;

template<typename T>
struct KernelReadWrapper {
    ULONG pid;
    ULONG addr;
    ULONG size;
    ULONGLONG out[(sizeof(T) + 7) / 8];

    KernelReadWrapper(ULONG pid, ULONG addr) {
        this->pid = pid;
        this->addr = addr;
        this->size = sizeof(T);
    }
};

struct KERNEL_WRITE_REQUEST {
    ULONG pid;
    ULONG addr;
    ULONG val;
    ULONG size;
};
using PKERNEL_WRITE_REQUEST = KERNEL_WRITE_REQUEST *;

struct KERNEL_GET_MODULE_REQUEST {
    REQUESTABLE_MODULE mod;
    ULONG addr;
    ULONG size;
};
using PKERNEL_GET_MODULE_REQUEST = KERNEL_GET_MODULE_REQUEST *;

class KeInterface {
public:
    KeInterface();
    ~KeInterface();

    template<typename T>
    std::pair<T, bool> read(ULONG pid, ULONG addr, ULONG size = sizeof(T)) const {
        KernelReadWrapper<T> req(pid, addr);
        DWORD bytes;
        if (!::DeviceIoControl(this->h_driver, IO_READ_REQUEST, &req, sizeof(req), &req, sizeof(req), &bytes, NULL))
            return {T{}, false};
        return {*reinterpret_cast<T *>(&req.out), true};
    }
    template<typename T>
    bool write(ULONG pid, ULONG addr, T val, ULONG size = sizeof(T)) const {
        KERNEL_WRITE_REQUEST req = {
                .pid = pid,
                .addr = addr,
                .val = 0,
                .size = size};
        memcpy_s(&req.val, sizeof(req.val), &val, size);
        DWORD bytes;
        return bool(::DeviceIoControl(this->h_driver, IO_WRITE_REQUEST, &req, sizeof(req), &req, sizeof(req), &bytes, NULL));
    }
    DWORD get_target_pid();
    std::pair<DWORD, DWORD> get_module(REQUESTABLE_MODULE mod);

    inline bool is_invalid() const { return this->invalid; }

private:
    HANDLE h_driver;
    bool invalid = false;
};