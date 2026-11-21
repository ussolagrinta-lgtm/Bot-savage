#include "process_attach.hpp"

#include <TlHelp32.h>

namespace rob {

std::optional<DWORD> FindProcessByName(const std::wstring& name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return std::nullopt;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!Process32FirstW(snap, &entry)) {
        CloseHandle(snap);
        return std::nullopt;
    }

    do {
        if (_wcsicmp(entry.szExeFile, name.c_str()) == 0) {
            DWORD pid = entry.th32ProcessID;
            CloseHandle(snap);
            return pid;
        }
    } while (Process32NextW(snap, &entry));

    CloseHandle(snap);
    return std::nullopt;
}

struct EnumWindowsCtx {
    DWORD pid;
    HWND hwnd;
};

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* ctx = reinterpret_cast<EnumWindowsCtx*>(lParam);
    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid);
    if (windowPid != ctx->pid)
        return TRUE;
    if (!IsWindowVisible(hwnd))
        return TRUE;
    ctx->hwnd = hwnd;
    return FALSE;
}

HWND FindMainWindowForPid(DWORD pid) {
    EnumWindowsCtx ctx{pid, nullptr};
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&ctx));
    return ctx.hwnd;
}

} // namespace rob