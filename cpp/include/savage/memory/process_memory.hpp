#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace savage::memory {

class ProcessMemory {
 public:
  ProcessMemory() = default;
  explicit ProcessMemory(HANDLE process, DWORD pid);
  ~ProcessMemory();

  ProcessMemory(ProcessMemory&& other) noexcept;
  ProcessMemory& operator=(ProcessMemory&& other) noexcept;

  ProcessMemory(const ProcessMemory&) = delete;
  ProcessMemory& operator=(const ProcessMemory&) = delete;

  static std::optional<ProcessMemory> open_by_pid(DWORD pid, DWORD desired_access = PROCESS_VM_READ | PROCESS_QUERY_INFORMATION);
  static std::optional<ProcessMemory> open_by_process_name(std::wstring_view name);

  [[nodiscard]] bool valid() const noexcept { return process_ != nullptr; }
  [[nodiscard]] DWORD pid() const noexcept { return pid_; }
  [[nodiscard]] HANDLE handle() const noexcept { return process_; }

  std::optional<std::vector<std::uint8_t>> read_bytes(std::uintptr_t address, std::size_t size) const;
  std::optional<std::uint64_t> read_u64(std::uintptr_t address) const;

  std::optional<std::uintptr_t> scan_module(const std::wstring& module_name, std::span<const std::uint8_t> pattern,
                                            std::span<const char> mask) const;

 private:
  void close() noexcept;

  HANDLE process_{nullptr};
  DWORD pid_{0};
};

}  // namespace savage::memory