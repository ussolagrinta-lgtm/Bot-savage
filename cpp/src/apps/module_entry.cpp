#include <Windows.h>

#include <atomic>
#include <thread>

#include "savage/core/logger.hpp"
#include "savage/hooks/minhook_scope.hpp"
#include "savage/hooks/present_hook.hpp"
#include "savage/sync/state_hub.hpp"

namespace {

std::thread g_worker;
std::atomic<bool> g_running{false};

DWORD WINAPI worker_thread(LPVOID) {
  savage::core::set_min_level(savage::core::LogLevel::Debug);
  SAVAGE_LOG(savage::core::LogLevel::Info, "SavageModule worker start");

  if (!savage::hooks::MinHookScope::instance().ensure_ready()) {
    SAVAGE_LOG(savage::core::LogLevel::Error, "MinHook init failed");
    return 1;
  }

  /**
   * Internal Present hook attaches only after you obtain a valid IDXGISwapChain*
   * from the game device (pattern on your build). Example:
   *
   *   IDXGISwapChain* sc = ...; // measured
   *   savage::hooks::PresentHook::instance().attach(sc, [](auto* chain, UINT s, UINT f) {
   *     // ImGui on game swap chain — wire ui + render here
   *   });
   */

  while (!savage::sync::StateHub::instance().shutdown_requested()) {
    Sleep(50);
  }

  savage::hooks::PresentHook::instance().detach();
  savage::hooks::MinHookScope::instance().disable_all();
  SAVAGE_LOG(savage::core::LogLevel::Info, "SavageModule worker exit");
  return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(module);
      if (!g_running.exchange(true)) {
        g_worker = std::thread([] { worker_thread(nullptr); });
      }
      break;
    case DLL_PROCESS_DETACH:
      savage::sync::StateHub::instance().request_shutdown();
      if (g_worker.joinable()) g_worker.join();
      g_running.store(false);
      break;
    default:
      break;
  }
  return TRUE;
}