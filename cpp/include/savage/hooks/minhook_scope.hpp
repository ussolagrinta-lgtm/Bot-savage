#pragma once

#include <atomic>

#include "savage/core/noncopyable.hpp"

namespace savage::hooks {

class MinHookScope : core::NonCopyable {
 public:
  static MinHookScope& instance();

  bool ensure_ready();
  bool enable_all();
  bool disable_all();

 private:
  MinHookScope() = default;
  std::atomic<bool> initialized_{false};
};

}  // namespace savage::hooks