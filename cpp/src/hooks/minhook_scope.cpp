#include "savage/hooks/minhook_scope.hpp"

#include <MinHook.h>

#include "savage/core/logger.hpp"

namespace savage::hooks {

MinHookScope& MinHookScope::instance() {
  static MinHookScope inst;
  return inst;
}

bool MinHookScope::ensure_ready() {
  if (initialized_.load(std::memory_order_acquire)) return true;
  const MH_STATUS st = MH_Initialize();
  if (st != MH_OK && st != MH_ERROR_ALREADY_INITIALIZED) {
    SAVAGE_LOG(core::LogLevel::Error, "MH_Initialize failed");
    return false;
  }
  initialized_.store(true, std::memory_order_release);
  return true;
}

bool MinHookScope::enable_all() {
  if (!ensure_ready()) return false;
  return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
}

bool MinHookScope::disable_all() {
  if (!initialized_.load()) return true;
  return MH_DisableHook(MH_ALL_HOOKS) == MH_OK;
}

}  // namespace savage::hooks