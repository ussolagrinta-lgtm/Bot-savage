#pragma once

#include <Windows.h>
#include <cstdint>
#include <optional>
#include <string>

namespace rob {

struct AttachedProcess {
    DWORD pid = 0;
    uint64_t gameAssemblyBase = 0;
    uint64_t gameAssemblySize = 0;
};

std::optional<DWORD> FindProcessByName(const std::wstring& name);
HWND FindMainWindowForPid(DWORD pid);

} // namespace rob