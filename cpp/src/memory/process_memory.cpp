#include "savage/memory/process_memory.hpp"

#include <TlHelp32.h>
#include <cstring>

#include "savage/memory/pattern_scan.hpp"
#include "savage/platform/win32_util.hpp"

namespace savage::memory {

ProcessMemory::ProcessMemory(HANDLE process, DWORD pid) : process_(process), pid_(pid) {}

ProcessMemory::~ProcessMemory() { close(); }

ProcessMemory::ProcessMemory(ProcessMemory&& other) noexcept : process_(other.process_), pid_(other.pid_) {
  other.process_ = nullptr;
  other.pid_ = 0;
}

ProcessMemory& ProcessMemory::operator=(ProcessMemory&& other) noexcept {
  if (this == &other) return *this;
  close();
  process_ = other.process_;
  pid_ = other.pid_;
  other.process_ = nullptr;
  other.pid_ = 0;
  return *this;
}

void ProcessMemory::close() noexcept {
  if (process_ != nullptr) {
    CloseHandle(process_);
    process_ = nullptr;
  }
  pid_ = 0;
}

std::optional<ProcessMemory> ProcessMemory::open_by_pid(DWORD pid, DWORD desired_access) {
  HANDLE h = OpenProcess(desired_access, FALSE, pid);
  if (h == nullptr) return std::nullopt;
  return ProcessMemory(h, pid);
}

std::optional<ProcessMemory> ProcessMemory::open_by_process_name(std::wstring_view name) {
  const HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return std::nullopt;

  PROCESSENTRY32W pe{};
  pe.dwSize = sizeof(pe);
  DWORD pid = 0;
  if (Process32FirstW(snap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, std::wstring(name).c_str()) == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32NextW(snap, &pe));
  }
  CloseHandle(snap);
  if (pid == 0) return std::nullopt;
  return open_by_pid(pid);
}

std::optional<std::vector<std::uint8_t>> ProcessMemory::read_bytes(std::uintptr_t address, std::size_t size) const {
  if (!valid() || size == 0) return std::nullopt;
  std::vector<std::uint8_t> buf(size);
  SIZE_T read = 0;
  if (!ReadProcessMemory(process_, reinterpret_cast<LPCVOID>(address), buf.data(), size, &read) || read != size) {
    return std::nullopt;
  }
  return buf;
}

std::optional<std::uint64_t> ProcessMemory::read_u64(std::uintptr_t address) const {
  const auto bytes = read_bytes(address, sizeof(std::uint64_t));
  if (!bytes) return std::nullopt;
  std::uint64_t v = 0;
  std::memcpy(&v, bytes->data(), sizeof(v));
  return v;
}

std::optional<std::uintptr_t> ProcessMemory::scan_module(const std::wstring& module_name,
                                                         std::span<const std::uint8_t> pattern,
                                                         std::span<const char> mask) const {
  if (!valid()) return std::nullopt;
  const auto mod = platform::find_module(pid_, module_name);
  if (!mod) return std::nullopt;

  const auto region = read_bytes(mod->base, mod->size);
  if (!region) return std::nullopt;

  std::string mask_str(mask.data(), mask.size());
  return scan_region(*region, mod->base, pattern, mask_str);
}

}  // namespace savage::memory