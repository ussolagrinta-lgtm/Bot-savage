#include "driver_io.hpp"

#include <cstring>

namespace rob {

DriverIo::~DriverIo() { Close(); }

bool DriverIo::Open() {
    if (device_ != INVALID_HANDLE_VALUE)
        return true;

    device_ = CreateFileW(
        ROB_RUN_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    return device_ != INVALID_HANDLE_VALUE;
}

void DriverIo::Close() {
    if (device_ != INVALID_HANDLE_VALUE) {
        CloseHandle(device_);
        device_ = INVALID_HANDLE_VALUE;
    }
}

bool DriverIo::Attach(DWORD pid) {
    if (device_ == INVALID_HANDLE_VALUE && !Open())
        return false;

    ROB_RUN_ATTACH_REQUEST req{};
    req.ProcessId = pid;

    DWORD returned = 0;
    BOOL ok = DeviceIoControl(
        device_, IOCTL_ROB_RUN_ATTACH, &req, sizeof(req), &req, sizeof(req), &returned, nullptr);

    return ok && req.Status == 0;
}

std::optional<uint64_t> DriverIo::GetModuleBase(const std::wstring& moduleName, uint64_t* moduleSizeOut) {
    if (device_ == INVALID_HANDLE_VALUE && !Open())
        return std::nullopt;

    ROB_RUN_MODULE_REQUEST req{};
    wcsncpy_s(req.ModuleName, moduleName.c_str(), _TRUNCATE);

    DWORD returned = 0;
    BOOL ok = DeviceIoControl(
        device_, IOCTL_ROB_RUN_GET_MODULE, &req, sizeof(req), &req, sizeof(req), &returned, nullptr);

    if (!ok || req.Status != 0 || req.BaseAddress == 0)
        return std::nullopt;

    if (moduleSizeOut)
        *moduleSizeOut = req.ModuleSize;

    return req.BaseAddress;
}

std::optional<std::vector<uint8_t>> DriverIo::Read(uint64_t address, uint32_t size) {
    if (size == 0 || size > ROB_RUN_MAX_RW_SIZE)
        return std::nullopt;

    if (device_ == INVALID_HANDLE_VALUE && !Open())
        return std::nullopt;

    std::vector<uint8_t> buffer(sizeof(ROB_RUN_RW_REQUEST) + size, 0);
    auto* header = reinterpret_cast<ROB_RUN_RW_REQUEST*>(buffer.data());
    header->Address = address;
    header->Size = size;

    DWORD returned = 0;
    BOOL ok = DeviceIoControl(
        device_, IOCTL_ROB_RUN_READ, buffer.data(), static_cast<DWORD>(buffer.size()), buffer.data(),
        static_cast<DWORD>(buffer.size()), &returned, nullptr);

    if (!ok || header->Status != 0)
        return std::nullopt;

    return std::vector<uint8_t>(buffer.begin() + sizeof(ROB_RUN_RW_REQUEST), buffer.end());
}

bool DriverIo::Write(uint64_t address, const void* data, uint32_t size) {
    if (!data || size == 0 || size > ROB_RUN_MAX_RW_SIZE)
        return false;

    if (device_ == INVALID_HANDLE_VALUE && !Open())
        return false;

    std::vector<uint8_t> buffer(sizeof(ROB_RUN_RW_REQUEST) + size, 0);
    auto* header = reinterpret_cast<ROB_RUN_RW_REQUEST*>(buffer.data());
    header->Address = address;
    header->Size = size;
    std::memcpy(buffer.data() + sizeof(ROB_RUN_RW_REQUEST), data, size);

    DWORD returned = 0;
    BOOL ok = DeviceIoControl(
        device_, IOCTL_ROB_RUN_WRITE, buffer.data(), static_cast<DWORD>(buffer.size()), buffer.data(),
        static_cast<DWORD>(buffer.size()), &returned, nullptr);

    return ok && header->Status == 0;
}

} // namespace rob