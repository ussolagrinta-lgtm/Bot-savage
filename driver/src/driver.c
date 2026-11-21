#include "../../shared/communication.h"

#define NT_DEVICE_NAME L"\\Device\\RobRunMem"

DRIVER_UNLOAD DriverUnload;
_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH DispatchCreateClose;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH DispatchCreateClose;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DispatchDeviceControl;

static PDEVICE_OBJECT g_DeviceObject = NULL;
static PEPROCESS g_TargetProcess = NULL;

static NTSTATUS CopyVirtual(PEPROCESS SourceProcess,
                            PVOID SourceAddress,
                            PEPROCESS TargetProcess,
                            PVOID TargetAddress,
                            SIZE_T Size,
                            KPROCESSOR_MODE PreviousMode,
                            PSIZE_T BytesCopied) {
    if (BytesCopied)
        *BytesCopied = 0;
    if (!SourceAddress || !TargetAddress || Size == 0)
        return STATUS_INVALID_PARAMETER;

    SIZE_T chunk = Size;
    NTSTATUS status = MmCopyVirtualMemory(
        SourceProcess, SourceAddress, TargetProcess, TargetAddress, chunk, PreviousMode, BytesCopied);
    if (BytesCopied && *BytesCopied < Size && NT_SUCCESS(status))
        status = STATUS_PARTIAL_COPY;
    return status;
}

static NTSTATUS HandleAttach(PROB_RUN_ATTACH_REQUEST req) {
    if (!req || req->ProcessId == 0)
        return STATUS_INVALID_PARAMETER;

    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)req->ProcessId, &process);
    if (!NT_SUCCESS(status))
        return status;

    if (g_TargetProcess)
        ObDereferenceObject(g_TargetProcess);

    g_TargetProcess = process;
    req->Status = STATUS_SUCCESS;
    return STATUS_SUCCESS;
}

static NTSTATUS HandleRead(PIRP Irp, ULONG outLength) {
    if (!g_TargetProcess)
        return STATUS_INVALID_HANDLE;

    if (outLength < sizeof(ROB_RUN_RW_REQUEST))
        return STATUS_BUFFER_TOO_SMALL;

    PROB_RUN_RW_REQUEST header = (PROB_RUN_RW_REQUEST)Irp->AssociatedIrp.SystemBuffer;
    if (!header || header->Size == 0 || header->Size > ROB_RUN_MAX_RW_SIZE)
        return STATUS_INVALID_PARAMETER;

    ULONG totalNeeded = sizeof(ROB_RUN_RW_REQUEST) + header->Size;
    if (outLength < totalNeeded)
        return STATUS_BUFFER_TOO_SMALL;

    PUCHAR dataOut = (PUCHAR)header + sizeof(ROB_RUN_RW_REQUEST);
    PEPROCESS current = PsGetCurrentProcess();

    SIZE_T copied = 0;
    NTSTATUS status = CopyVirtual(
        g_TargetProcess, (PVOID)header->Address, current, dataOut, header->Size, KernelMode, &copied);

    header->Status = (ULONG)status;
    if (NT_SUCCESS(status))
        Irp->IoStatus.Information = totalNeeded;
    else
        Irp->IoStatus.Information = sizeof(ROB_RUN_RW_REQUEST);

    return status;
}

static NTSTATUS HandleWrite(PIRP Irp, ULONG inLength) {
    if (!g_TargetProcess)
        return STATUS_INVALID_HANDLE;

    if (inLength < sizeof(ROB_RUN_RW_REQUEST))
        return STATUS_BUFFER_TOO_SMALL;

    PROB_RUN_RW_REQUEST header = (PROB_RUN_RW_REQUEST)Irp->AssociatedIrp.SystemBuffer;
    if (!header || header->Size == 0 || header->Size > ROB_RUN_MAX_RW_SIZE)
        return STATUS_INVALID_PARAMETER;

    ULONG totalNeeded = sizeof(ROB_RUN_RW_REQUEST) + header->Size;
    if (inLength < totalNeeded)
        return STATUS_BUFFER_TOO_SMALL;

    PUCHAR dataIn = (PUCHAR)header + sizeof(ROB_RUN_RW_REQUEST);
    PEPROCESS current = PsGetCurrentProcess();

    SIZE_T copied = 0;
    NTSTATUS status = CopyVirtual(
        current, dataIn, g_TargetProcess, (PVOID)header->Address, header->Size, KernelMode, &copied);

    header->Status = (ULONG)status;
    Irp->IoStatus.Information = sizeof(ROB_RUN_RW_REQUEST);
    return status;
}

typedef struct _PEB_LDR_DATA_PARTIAL {
    UCHAR Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InLoadOrderModuleList;
} PEB_LDR_DATA_PARTIAL, *PPEB_LDR_DATA_PARTIAL;

typedef struct _LDR_DATA_TABLE_ENTRY_PARTIAL {
    LIST_ENTRY InLoadOrderLinks;
    PVOID Reserved1[2];
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_PARTIAL, *PLDR_DATA_TABLE_ENTRY_PARTIAL;

static NTSTATUS ReadTargetMemory(PVOID Source, PVOID Dest, SIZE_T Size) {
    SIZE_T copied = 0;
    return CopyVirtual(
        g_TargetProcess, Source, PsGetCurrentProcess(), Dest, Size, KernelMode, &copied);
}

static NTSTATUS FindModuleBase(const WCHAR* moduleName, PULONG64 outBase, PULONG64 outSize) {
    *outBase = 0;
    *outSize = 0;

    PPEB peb = PsGetProcessPeb(g_TargetProcess);
    if (!peb)
        return STATUS_UNSUCCESSFUL;

    ULONG64 pebUser = (ULONG64)peb;
    ULONG64 ldrPtr = 0;
    if (!NT_SUCCESS(ReadTargetMemory((PVOID)(pebUser + 0x18), &ldrPtr, sizeof(ldrPtr))))
        return STATUS_UNSUCCESSFUL;

    LIST_ENTRY listHead = {0};
    ULONG64 listHeadAddr = ldrPtr + offsetof(PEB_LDR_DATA_PARTIAL, InLoadOrderModuleList);
    if (!NT_SUCCESS(ReadTargetMemory((PVOID)listHeadAddr, &listHead, sizeof(listHead))))
        return STATUS_UNSUCCESSFUL;

    ULONG64 entryAddr = (ULONG64)listHead.Flink;
    const ULONG64 maxWalk = 512;

    for (ULONG i = 0; i < maxWalk && entryAddr != listHeadAddr; ++i) {
        LDR_DATA_TABLE_ENTRY_PARTIAL entry = {0};
        ULONG64 entryStruct =
            entryAddr - offsetof(LDR_DATA_TABLE_ENTRY_PARTIAL, InLoadOrderLinks);

        if (!NT_SUCCESS(ReadTargetMemory((PVOID)entryStruct, &entry, sizeof(entry))))
            break;

        WCHAR nameBuf[ROB_RUN_MAX_MODULE_NAME] = {0};
        USHORT nameBytes = entry.BaseDllName.Length;
        if (nameBytes >= sizeof(nameBuf))
            nameBytes = (USHORT)(sizeof(nameBuf) - sizeof(WCHAR));

        if (entry.BaseDllName.Buffer && nameBytes > 0) {
            ReadTargetMemory(entry.BaseDllName.Buffer, nameBuf, nameBytes);
            nameBuf[nameBytes / sizeof(WCHAR)] = L'\0';

            if (_wcsicmp(nameBuf, moduleName) == 0) {
                *outBase = (ULONG64)entry.DllBase;
                *outSize = (ULONG64)entry.SizeOfImage;
                return STATUS_SUCCESS;
            }
        }

        LIST_ENTRY links = {0};
        if (!NT_SUCCESS(ReadTargetMemory((PVOID)entryStruct, &links, sizeof(links))))
            break;
        entryAddr = (ULONG64)links.Flink;
    }

    return STATUS_NOT_FOUND;
}

static NTSTATUS HandleGetModule(PROB_RUN_MODULE_REQUEST req) {
    if (!req || !g_TargetProcess)
        return g_TargetProcess ? STATUS_INVALID_PARAMETER : STATUS_INVALID_HANDLE;

    req->BaseAddress = 0;
    req->ModuleSize = 0;

    NTSTATUS status = FindModuleBase(req->ModuleName, &req->BaseAddress, &req->ModuleSize);
    req->Status = (ULONG)status;
    return status;
}

NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG inLen = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outLen = stack->Parameters.DeviceIoControl.OutputBufferLength;
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    Irp->IoStatus.Information = 0;

    switch (code) {
    case IOCTL_ROB_RUN_ATTACH: {
        if (inLen < sizeof(ROB_RUN_ATTACH_REQUEST) || outLen < sizeof(ROB_RUN_ATTACH_REQUEST)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = HandleAttach((PROB_RUN_ATTACH_REQUEST)buffer);
        if (NT_SUCCESS(status))
            Irp->IoStatus.Information = sizeof(ROB_RUN_ATTACH_REQUEST);
        break;
    }
    case IOCTL_ROB_RUN_READ:
        status = HandleRead(Irp, outLen);
        break;
    case IOCTL_ROB_RUN_WRITE:
        status = HandleWrite(Irp, inLen);
        break;
    case IOCTL_ROB_RUN_GET_MODULE: {
        if (inLen < sizeof(ROB_RUN_MODULE_REQUEST) || outLen < sizeof(ROB_RUN_MODULE_REQUEST)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        status = HandleGetModule((PROB_RUN_MODULE_REQUEST)buffer);
        if (NT_SUCCESS(status) || status == STATUS_NOT_FOUND)
            Irp->IoStatus.Information = sizeof(ROB_RUN_MODULE_REQUEST);
        break;
    }
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
    if (g_TargetProcess) {
        ObDereferenceObject(g_TargetProcess);
        g_TargetProcess = NULL;
    }
    if (g_DeviceObject) {
        UNICODE_STRING symlink;
        RtlInitUnicodeString(&symlink, ROB_RUN_DEVICE_SYMLINK);
        IoDeleteSymbolicLink(&symlink);
        IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = NULL;
    }
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    UNICODE_STRING deviceName;
    RtlInitUnicodeString(&deviceName, NT_DEVICE_NAME);

    NTSTATUS status = IoCreateDevice(
        DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_DeviceObject);

    if (!NT_SUCCESS(status))
        return status;

    UNICODE_STRING symlink;
    RtlInitUnicodeString(&symlink, ROB_RUN_DEVICE_SYMLINK);
    status = IoCreateSymbolicLink(&symlink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = NULL;
        return status;
    }

    g_DeviceObject->Flags |= DO_BUFFERED_IO;
    g_DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}