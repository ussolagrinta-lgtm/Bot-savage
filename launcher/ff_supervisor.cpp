// Minimal recreation of FFLoader supervision loop (RobRunAIO.ini).

#include <Windows.h>
#include <TlHelp32.h>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

namespace {

std::wstring ExeDir() {
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path().wstring();
}

bool IniBool(const std::wstring& ini, const wchar_t* key, bool def) {
    return GetPrivateProfileIntW(L"launcher", key, def ? 1 : 0, ini.c_str()) != 0;
}

int IniInt(const std::wstring& ini, const wchar_t* key, int def) {
    return GetPrivateProfileIntW(L"launcher", key, def, ini.c_str());
}

bool ProcessRunning(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return false;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

} // namespace

int wmain() {
    const std::wstring dir = ExeDir();
    const std::wstring ini = dir + L"\\RobRunAIO.ini";

    const bool waitWindow = IniBool(ini, L"menu_wait_window", true);
    const int menuDelaySec = IniInt(ini, L"menu_delay", 5);
    const int attachTimeoutSec = IniInt(ini, L"attach_timeout", 120);
    (void)waitWindow;

    std::wprintf(L"FFSupervisor — waiting for ROB RUN.exe ...\n");
    while (!ProcessRunning(L"ROB RUN.exe"))
        std::this_thread::sleep_for(std::chrono::seconds(1));

    std::this_thread::sleep_for(std::chrono::seconds(menuDelaySec));

    const std::wstring overlay = dir + L"\\RobRunPort.exe";
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    if (!CreateProcessW(overlay.c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi)) {
        std::fwprintf(stderr, L"Failed to start RobRunPort.exe (err=%lu)\n", GetLastError());
        return 1;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(attachTimeoutSec);
    const std::wstring statusPath = dir + L"\\status.log";

    while (std::chrono::steady_clock::now() < deadline) {
        std::ifstream st(statusPath);
        std::string line;
        if (st && std::getline(st, line) && line.find("n=") != std::string::npos && line.find("n=0") == std::string::npos) {
            std::wprintf(L"Overlay reported players — supervise OK.\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}