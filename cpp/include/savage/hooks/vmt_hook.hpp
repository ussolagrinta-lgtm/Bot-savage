#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "savage/core/noncopyable.hpp"

namespace savage::hooks {

template <typename Fn>
class VmtHook : core::NonCopyable {
 public:
  explicit VmtHook(void* object) : object_(object), vtable_(nullptr), original_(nullptr) {
    if (object == nullptr) return;
    auto** base = reinterpret_cast<std::uintptr_t**>(object);
    vtable_ = *base;
  }

  [[nodiscard]] bool valid() const noexcept { return vtable_ != nullptr; }

  bool install(std::size_t index, Fn detour) {
    if (!valid()) return false;
    if (installed_) return false;
    original_ = reinterpret_cast<Fn>(vtable_[index]);
    DWORD old = 0;
    if (!VirtualProtect(&vtable_[index], sizeof(std::uintptr_t), PAGE_READWRITE, &old)) return false;
    vtable_[index] = reinterpret_cast<std::uintptr_t>(detour);
    VirtualProtect(&vtable_[index], sizeof(std::uintptr_t), old, &old);
    index_ = index;
    detour_ = detour;
    installed_ = true;
    return true;
  }

  void remove() {
    if (!installed_) return;
    DWORD old = 0;
    if (VirtualProtect(&vtable_[index_], sizeof(std::uintptr_t), PAGE_READWRITE, &old)) {
      vtable_[index_] = reinterpret_cast<std::uintptr_t>(original_);
      VirtualProtect(&vtable_[index_], sizeof(std::uintptr_t), old, &old);
    }
    installed_ = false;
    detour_ = nullptr;
    original_ = nullptr;
  }

  [[nodiscard]] Fn original() const noexcept { return original_; }

  ~VmtHook() { remove(); }

 private:
  void* object_{nullptr};
  std::uintptr_t* vtable_{nullptr};
  std::size_t index_{0};
  Fn original_{nullptr};
  Fn detour_{nullptr};
  bool installed_{false};
};

}  // namespace savage::hooks