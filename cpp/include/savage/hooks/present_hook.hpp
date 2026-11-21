#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>

#include <d3d11.h>
#include <dxgi.h>

#include "savage/core/noncopyable.hpp"
#include "savage/hooks/vmt_hook.hpp"

namespace savage::hooks {

using PresentCallback = std::function<void(IDXGISwapChain*, UINT, UINT)>;

/** Hooks IDXGISwapChain::Present via VMT slot 8 (D3D10/11 common). */
class PresentHook : core::NonCopyable {
 public:
  static PresentHook& instance();

  bool attach(IDXGISwapChain* swap_chain, PresentCallback on_present);
  void detach();

  void set_callback(PresentCallback cb);

 private:
  PresentHook() = default;

  static HRESULT STDMETHODCALLTYPE detour_present(IDXGISwapChain* self, UINT sync, UINT flags);

  std::mutex mutex_;
  using PresentFn = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);
  std::optional<VmtHook<PresentFn>> hook_;
  PresentCallback callback_;
  std::atomic<bool> attached_{false};
};

}  // namespace savage::hooks