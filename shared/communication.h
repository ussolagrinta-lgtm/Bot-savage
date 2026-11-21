#pragma once

// Shared between RobRunMem.sys (kernel) and RobRunClient.exe (user).

#ifdef _KERNEL_MODE
#include <ntddk.h>
#else
#include <Windows.h>
#include <cstdint>
#endif

#define ROB_RUN_DEVICE_NAME L"\\\\.\\RobRunMem"
#define ROB_RUN_DEVICE_SYMLINK L"\\DosDevices\\RobRunMem"

#define ROB_RUN_IOCTL_BASE 0x8A42

#define IOCTL_ROB_RUN_ATTACH \
    CTL_CODE(ROB_RUN_IOCTL_BASE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ROB_RUN_READ \
    CTL_CODE(ROB_RUN_IOCTL_BASE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ROB_RUN_WRITE \
    CTL_CODE(ROB_RUN_IOCTL_BASE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ROB_RUN_GET_MODULE \
    CTL_CODE(ROB_RUN_IOCTL_BASE, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ROB_RUN_MAX_RW_SIZE (64 * 1024)
#define ROB_RUN_MAX_MODULE_NAME 260

#pragma pack(push, 1)

typedef struct _ROB_RUN_ATTACH_REQUEST {
    ULONG ProcessId;
    ULONG Status; // out: NTSTATUS cast to ULONG
} ROB_RUN_ATTACH_REQUEST, *PROB_RUN_ATTACH_REQUEST;

typedef struct _ROB_RUN_RW_REQUEST {
    ULONG64 Address;
    ULONG Size;
    ULONG Status; // out
    // trailing buffer: Size bytes (in/out depending on IOCTL)
} ROB_RUN_RW_REQUEST, *PROB_RUN_RW_REQUEST;

typedef struct _ROB_RUN_MODULE_REQUEST {
    WCHAR ModuleName[ROB_RUN_MAX_MODULE_NAME];
    ULONG64 BaseAddress; // out
    ULONG64 ModuleSize;  // out
    ULONG Status;        // out
} ROB_RUN_MODULE_REQUEST, *PROB_RUN_MODULE_REQUEST;

#pragma pack(pop)