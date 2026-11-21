#include "savage/hooks/present_hook.hpp"

#include "savage/core/logger.hpp"
#include "savage/sync/state_hub.hpp"

namespace savage::hooks {

namespace {
constexpr std::size_t kPresentVmtIndex = 8;
}

PresentHook& PresentHook::instance() {
  static PresentHook inst;
  return inst;
}

HRESULT STDMETHODCALLTYPE PresentHook::detour_present(IDXGISwapChain* self, UINT sync, UINT flags) {
  auto& hook = instance();
  if (!hook.hook_.has_value()) return E_FAIL;
  auto* orig = hook.hook_->original();
  if (orig == nullptr) return E_FAIL;

  PresentCallback cb;
  {
    std::lock_guard lock(hook.mutex_);
    cb = hook.callback_;
  }
  if (cb) cb(self, sync, flags);

  sync::StateHub::instance().set_frame_counter(sync::StateHub::instance().frame_counter() + 1);
  return orig(self, sync, flags);
}

bool PresentHook::attach(IDXGISwapChain* swap_chain, PresentCallback on_present) {
  if (swap_chain == nullptr) return false;
  std::lock_guard lock(mutex_);
  if (attached_.load()) return false;

  hook_.emplace(swap_chain);
  if (!hook_->valid()) {
    hook_.reset();
    SAVAGE_LOG(core::LogLevel::Error, "PresentHook: invalid swap chain vtable");
    return false;
  }
  callback_ = std::move(on_present);
  if (!hook_->install(kPresentVmtIndex, &PresentHook::detour_present)) {
    hook_.reset();
    SAVAGE_LOG(core::LogLevel::Error, "PresentHook: VMT install failed");
    callback_ = nullptr;
    return false;
  }
  attached_.store(true);
  SAVAGE_LOG(core::LogLevel::Info, "PresentHook attached");
  return true;
}

void PresentHook::detach() {
  std::lock_guard lock(mutex_);
  if (!attached_.load()) return;
  if (hook_.has_value()) hook_->remove();
  hook_.reset();
  callback_ = nullptr;
  attached_.store(false);
  SAVAGE_LOG(core::LogLevel::Info, "PresentHook detached");
}

void PresentHook::set_callback(PresentCallback cb) {
  std::lock_guard lock(mutex_);
  callback_ = std::move(cb);
}

}  // namespace savage::hooks