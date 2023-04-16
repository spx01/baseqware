// https://github.com/ContionMig/Simple-Millin-Kernel

#include <limits.h>

#include <ntos.h>
#include <wdf.h>
#include <windef.h>

typedef enum _REQUESTABLE_MODULE {
    CLIENT_MODULE = 0,
    ENGINE_MODULE,
    /* VSTDLIB_MODULE,
    TIER0_MODULE, */
    _RM_MODULE_COUNT,
    _RM_FORCE_SIZE = INT_MAX,
} REQUESTABLE_MODULE;

PCWSTR kModulePaths[_RM_MODULE_COUNT] = {
        L"\\csgo\\bin\\client.dll",
        L"\\bin\\engine.dll"};

PCWSTR kExePath = L"\\csgo.exe";

ULONG ModuleAddresses[_RM_MODULE_COUNT];
ULONG ModuleSizes[_RM_MODULE_COUNT];

// Request to read virtual user memory (memory of a program) from kernel space
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0691 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Request to write virtual user memory (memory of a program) from kernel space
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0692 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Request to retrieve the process id of csgo process, from kernel space
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0693 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Request to retrieve the base address of client.dll in csgo.exe from kernel space
#define IO_GET_MODULE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0694 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define PRINT(x, ...) DbgPrintEx(0, 0, "[PASED][INF] " x, __VA_ARGS__)
#ifdef _DEBUG
#define DEBUG(x, ...) DbgPrintEx(0, 0, "[PASED][DBG] " x, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

PDEVICE_OBJECT pDeviceObject;// our driver object
UNICODE_STRING dev, dos;     // Driver registry paths

ULONG csgoId;

// datatype for read request
typedef struct _KERNEL_READ_REQUEST {
    ULONG ProcessId;
    ULONG Address;
    ULONG Size;
    ULONGLONG Response[];

} KERNEL_READ_REQUEST, *PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST {
    ULONG ProcessId;
    ULONG Address;
    ULONG Value;
    ULONG Size;

} KERNEL_WRITE_REQUEST, *PKERNEL_WRITE_REQUEST;

typedef struct _KERNEL_GET_MODULE_REQUEST {
    REQUESTABLE_MODULE Module;
    ULONG Address;
    ULONG Size;
} KERNEL_GET_MODULE_REQUEST, *PKERNEL_GET_MODULE_REQUEST;

// TODO IMPORTANT
// add module requests
// would return both the base address and size of the module
// module would be indexed by an enum

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp);

NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size) {
    PSIZE_T Bytes;
    if (NT_SUCCESS(MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
                                       TargetAddress, Size, KernelMode, &Bytes)))
        return STATUS_SUCCESS;
    else
        return STATUS_ACCESS_DENIED;
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size) {
    PSIZE_T Bytes;
    if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process,
                                       TargetAddress, Size, KernelMode, &Bytes)))
        return STATUS_SUCCESS;
    else
        return STATUS_ACCESS_DENIED;
}

// set a callback for every PE image loaded to user memory
// then find the client.dll & csgo.exe using the callback
PLOAD_IMAGE_NOTIFY_ROUTINE ImageLoadCallback(PUNICODE_STRING FullImageName,
                                             HANDLE ProcessId, PIMAGE_INFO ImageInfo) {
    // the exe gets loaded first, so we can use it to then check the process id for the other modules
    // because besides client.dll, the modules have pretty generic paths
    if (wcsstr(FullImageName->Buffer, kExePath)) {
        DEBUG("found csgo.exe\n");
        csgoId = ProcessId;
        return;
    }
    if (csgoId != ProcessId) {
        return;
    }
    for (int i = 0; i < _RM_MODULE_COUNT; ++i) {
        if (wcsstr(FullImageName->Buffer, kModulePaths[i])) {
            ModuleAddresses[i] = ImageInfo->ImageBase;
            ModuleSizes[i] = ImageInfo->ImageSize;
            DEBUG("loaded name: %ls\n", FullImageName->Buffer);
        }
    }
}

// set a callback for every process created/destroyed
// we only care about when csgo is destroyed, so that we never provide invalid data to user space
PCREATE_PROCESS_NOTIFY_ROUTINE ProcessNotifyCallback(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {
    if (!Create && csgoId == ProcessId) {
        DEBUG("process destroyed\n");
        csgoId = 0;
        for (int i = 0; i < _RM_MODULE_COUNT; ++i) {
            ModuleAddresses[i] = ModuleSizes[i] = 0;
        }
    }
}


// IOCTL Call Handler function

NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    NTSTATUS Status;
    ULONG BytesIO = 0;

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

    // Code received from user space
    ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    if (ControlCode == IO_READ_REQUEST) {
        // Get the input buffer & format it to our struct
        PKERNEL_READ_REQUEST ReadInput = (PKERNEL_READ_REQUEST) Irp->AssociatedIrp.SystemBuffer;

        PEPROCESS Process;
        // Get our process
        if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
            KeReadVirtualMemory(Process, ReadInput->Address,
                                ReadInput->Response, ReadInput->Size);

        //DbgPrintEx(0, 0, "Read Params:  %lu, %#010x \n", ReadInput->ProcessId, ReadInput->Address);
        //DbgPrintEx(0, 0, "Value: %lu \n", ReadOutput->Response);

        Status = STATUS_SUCCESS;
        BytesIO = sizeof(KERNEL_READ_REQUEST) + ReadInput->Size;
    } else if (ControlCode == IO_WRITE_REQUEST) {
        // Get the input buffer & format it to our struct
        PKERNEL_WRITE_REQUEST WriteInput = (PKERNEL_WRITE_REQUEST) Irp->AssociatedIrp.SystemBuffer;

        PEPROCESS Process;
        // Get our process
        if (NT_SUCCESS(PsLookupProcessByProcessId(WriteInput->ProcessId, &Process)))
            KeWriteVirtualMemory(Process, &WriteInput->Value,
                                 WriteInput->Address, WriteInput->Size);

        //DbgPrintEx(0, 0, "Write Params:  %lu, %#010x \n", WriteInput->Value, WriteInput->Address);

        Status = STATUS_SUCCESS;
        BytesIO = sizeof(KERNEL_WRITE_REQUEST);
    } else if (ControlCode == IO_GET_ID_REQUEST) {
        PULONG OutPut = (PULONG) Irp->AssociatedIrp.SystemBuffer;
        *OutPut = csgoId;

        // DEBUG("id get %#010x\n", csgoId);
        Status = STATUS_SUCCESS;
        BytesIO = sizeof(*OutPut);
    } else if (ControlCode == IO_GET_MODULE_REQUEST) {
        PKERNEL_GET_MODULE_REQUEST GetModuleIO = (PKERNEL_GET_MODULE_REQUEST) Irp->AssociatedIrp.SystemBuffer;
        REQUESTABLE_MODULE mod = GetModuleIO->Module;
        if (mod >= _RM_MODULE_COUNT || mod < 0) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            GetModuleIO->Address = ModuleAddresses[mod];
            GetModuleIO->Size = ModuleAddresses[mod];
            DEBUG("module get %#010x\n", ModuleAddresses[mod]);
            BytesIO = sizeof(KERNEL_GET_MODULE_REQUEST);
            Status = STATUS_SUCCESS;
        }
    } else {
        // if the code is unknown
        Status = STATUS_INVALID_PARAMETER;
        BytesIO = 0;
    }

    // Complete the request
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = BytesIO;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    WORD LoadCount;
    WORD TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct
        {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    struct _ACTIVATION_CONTEXT *EntryPointActivationContext;
    PVOID PatchInformation;
    LIST_ENTRY ForwarderLinks;
    LIST_ENTRY ServiceTagLinks;
    LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

PDEVICE_OBJECT DeviceObject;


// Driver Entrypoint
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pRegistryPath) {
    PRINT("driver loaded\n");

    PsSetLoadImageNotifyRoutine(ImageLoadCallback);
    PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, FALSE);

#ifndef _DEBUG
#if 0
    PRINT("hiding driver\n");
    PLDR_DATA_TABLE_ENTRY CurDriverEntry = (PLDR_DATA_TABLE_ENTRY) pDriverObject->DriverSection;
    PLDR_DATA_TABLE_ENTRY NextDriverEntry = (PLDR_DATA_TABLE_ENTRY) CurDriverEntry->InLoadOrderLinks.Flink;
    PLDR_DATA_TABLE_ENTRY PrevDriverEntry = (PLDR_DATA_TABLE_ENTRY) CurDriverEntry->InLoadOrderLinks.Blink;

    PrevDriverEntry->InLoadOrderLinks.Flink = CurDriverEntry->InLoadOrderLinks.Flink;
    NextDriverEntry->InLoadOrderLinks.Blink = CurDriverEntry->InLoadOrderLinks.Blink;

    CurDriverEntry->InLoadOrderLinks.Flink = (PLIST_ENTRY) CurDriverEntry;
    CurDriverEntry->InLoadOrderLinks.Blink = (PLIST_ENTRY) CurDriverEntry;
#endif
#endif

    RtlInitUnicodeString(&dev, L"\\Device\\pased");
    RtlInitUnicodeString(&dos, L"\\DosDevices\\pased");

    IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
    IoCreateSymbolicLink(&dos, &dev);

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
    pDriverObject->DriverUnload = UnloadDriver;

    if (pDeviceObject == NULL) {
        PRINT("failed to create device\n");
        return STATUS_UNSUCCESSFUL;
    }
    pDeviceObject->Flags |= DO_DIRECT_IO;
    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}


NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject) {
    PRINT("unload routine\n");
    PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);
    PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, TRUE);
    IoDeleteSymbolicLink(&dos);
    IoDeleteDevice(pDriverObject->DeviceObject);
    return STATUS_SUCCESS;
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp) {
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;

    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp) {
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;

    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
