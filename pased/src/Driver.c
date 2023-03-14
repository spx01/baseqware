#include <ntos.h>
#include <wdf.h>
#include <windef.h>

// Request to read virtual user memory (memory of a program) from kernel space
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0691 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to write virtual user memory (memory of a program) from kernel space
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0692 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to retrieve the process id of csgo process, from kernel space
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0693 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to retrieve the base address of client.dll in csgo.exe from kernel space
#define IO_GET_MODULE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0694 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)


PDEVICE_OBJECT pDeviceObject;// our driver object
UNICODE_STRING dev, dos;     // Driver registry paths

ULONG csgoId, ClientAddress;

// datatype for read request
typedef struct _KERNEL_READ_REQUEST {
    ULONG ProcessId;

    ULONG Address;
    ULONG Response;
    ULONG Size;

} KERNEL_READ_REQUEST, *PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST {
    ULONG ProcessId;

    ULONG Address;
    ULONG Value;
    ULONG Size;

} KERNEL_WRITE_REQUEST, *PKERNEL_WRITE_REQUEST;

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
    // Compare our string to input
    if (wcsstr(FullImageName->Buffer, L"\\csgo\\bin\\client.dll")) {
        // if it matches
        DbgPrintEx(0, 0, "Loaded Name: %ls \n", FullImageName->Buffer);
        DbgPrintEx(0, 0, "Loaded To Process: %d \n", (DWORD) ProcessId);

        ClientAddress = ImageInfo->ImageBase;
        csgoId = ProcessId;
    }
}

PCREATE_PROCESS_NOTIFY_ROUTINE ProcessNotifyCallback(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {
    if (!Create && csgoId == ProcessId) {
        csgoId = 0;
        ClientAddress = 0;
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
        PKERNEL_READ_REQUEST ReadOutput = (PKERNEL_READ_REQUEST) Irp->AssociatedIrp.SystemBuffer;

        PEPROCESS Process;
        // Get our process
        if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
            KeReadVirtualMemory(Process, ReadInput->Address,
                                &ReadInput->Response, ReadInput->Size);

        //DbgPrintEx(0, 0, "Read Params:  %lu, %#010x \n", ReadInput->ProcessId, ReadInput->Address);
        //DbgPrintEx(0, 0, "Value: %lu \n", ReadOutput->Response);

        Status = STATUS_SUCCESS;
        BytesIO = sizeof(KERNEL_READ_REQUEST);
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

        DbgPrintEx(0, 0, "id get %#010x", csgoId);
        Status = STATUS_SUCCESS;
        BytesIO = sizeof(*OutPut);
    } else if (ControlCode == IO_GET_MODULE_REQUEST) {
        PULONG OutPut = (PULONG) Irp->AssociatedIrp.SystemBuffer;
        *OutPut = ClientAddress;

        DbgPrintEx(0, 0, "Module get %#010x", ClientAddress);
        Status = STATUS_SUCCESS;
        BytesIO = sizeof(*OutPut);
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
    DbgPrintEx(0, 0, "321321321Driver Loaded\n");

    PsSetLoadImageNotifyRoutine(ImageLoadCallback);
    PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, FALSE);

    PLDR_DATA_TABLE_ENTRY CurDriverEntry = (PLDR_DATA_TABLE_ENTRY) pDriverObject->DriverSection;
    PLDR_DATA_TABLE_ENTRY NextDriverEntry = (PLDR_DATA_TABLE_ENTRY) CurDriverEntry->InLoadOrderLinks.Flink;
    PLDR_DATA_TABLE_ENTRY PrevDriverEntry = (PLDR_DATA_TABLE_ENTRY) CurDriverEntry->InLoadOrderLinks.Blink;

    PrevDriverEntry->InLoadOrderLinks.Flink = CurDriverEntry->InLoadOrderLinks.Flink;
    NextDriverEntry->InLoadOrderLinks.Blink = CurDriverEntry->InLoadOrderLinks.Blink;

    CurDriverEntry->InLoadOrderLinks.Flink = (PLIST_ENTRY) CurDriverEntry;
    CurDriverEntry->InLoadOrderLinks.Blink = (PLIST_ENTRY) CurDriverEntry;

    RtlInitUnicodeString(&dev, L"\\Device\\pased");
    RtlInitUnicodeString(&dos, L"\\DosDevices\\pased");

    IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
    IoCreateSymbolicLink(&dos, &dev);

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
    pDriverObject->DriverUnload = UnloadDriver;

    pDeviceObject->Flags |= DO_DIRECT_IO;
    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
}


NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject) {
    DbgPrintEx(0, 0, "Unload routine called.\n");
    PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);
    PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, TRUE);
    IoDeleteSymbolicLink(&dos);
    IoDeleteDevice(pDriverObject->DeviceObject);
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
