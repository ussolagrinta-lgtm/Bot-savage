#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <Windows.h>

#include "../../shared/communication.h"

namespace rob {

class DriverIo {
public:
    DriverIo() = default;
  ~DriverIo();

    DriverIo(const DriverIo&) = delete;
    DriverIo& operator=(const DriverIo&) = delete;

    bool Open();
    void Close();

    bool Attach(DWORD pid);
    std::optional<uint64_t> GetModuleBase(const std::wstring& moduleName, uint64_t* moduleSizeOut = nullptr);
    std::optional<std::vector<uint8_t>> Read(uint64_t address, uint32_t size);
    bool Write(uint64_t address, const void* data, uint32_t size);

private:
    HANDLE device_ = INVALID_HANDLE_VALUE;
};

} // namespace rob