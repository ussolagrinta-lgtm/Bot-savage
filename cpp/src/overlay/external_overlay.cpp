#include "savage/overlay/external_overlay.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>

#include "savage/config/game_profile.hpp"
#include "savage/core/logger.hpp"
#include "savage/platform/win32_util.hpp"
#include "savage/sync/state_hub.hpp"

namespace savage::overlay {

namespace {

constexpr wchar_t kOverlayClass[] = L"SavageOverlayWnd";

LRESULT CALLBACK overlay_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return TRUE;
  if (msg == WM_DESTROY) {
    savage::sync::StateHub::instance().request_shutdown();
    return 0;
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool register_overlay_class(HINSTANCE inst) {
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = overlay_wnd_proc;
  wc.hInstance = inst;
  wc.lpszClassName = kOverlayClass;
  return RegisterClassExW(&wc) != 0;
}

}  // namespace

ExternalOverlay::ExternalOverlay(std::wstring target_title_substring)
    : target_filter_(std::move(target_title_substring)) {
  if (target_filter_.empty()) {
    const auto& profile = config::GameProfile::active();
    target_filter_ = std::wstring(profile.target_window_substring.begin(), profile.target_window_substring.end());
  }
}

bool ExternalOverlay::create_overlay_window() {
  const HINSTANCE inst = GetModuleHandleW(nullptr);
  if (!register_overlay_class(inst)) return false;

  overlay_hwnd_ = CreateWindowExW(
      WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kOverlayClass, L"Savage Overlay", WS_POPUP,
      0, 0, static_cast<int>(width_), static_cast<int>(height_), nullptr, nullptr, inst, nullptr);
  if (!overlay_hwnd_) return false;

  SetLayeredWindowAttributes(overlay_hwnd_, RGB(0, 0, 0), 255, LWA_ALPHA);
  ShowWindow(overlay_hwnd_, SW_SHOWNOACTIVATE);
  return true;
}

void ExternalOverlay::sync_to_target() {
  if (!target_hwnd_ || !IsWindow(target_hwnd_)) {
    target_hwnd_ = platform::find_window_by_title_substring(target_filter_).value_or(nullptr);
    if (!target_hwnd_) return;
  }

  RECT rc{};
  if (!platform::get_client_screen_rect(target_hwnd_, rc)) return;

  const std::uint32_t w = static_cast<std::uint32_t>(std::max<LONG>(1, rc.right - rc.left));
  const std::uint32_t h = static_cast<std::uint32_t>(std::max<LONG>(1, rc.bottom - rc.top));

  if (w != width_ || h != height_) {
    width_ = w;
    height_ = h;
    SetWindowPos(overlay_hwnd_, HWND_TOPMOST, rc.left, rc.top, static_cast<int>(width_), static_cast<int>(height_),
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    gfx_.resize(width_, height_);
  } else {
    SetWindowPos(overlay_hwnd_, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
  }
}

void ExternalOverlay::render_ui() {
  const auto& profile = config::GameProfile::active();
  ImGui::SetNextWindowSize(ImVec2(360.f, 280.f), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Savage / External Overlay", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Target filter: %ls", target_filter_.c_str());
    ImGui::Text("Profile: %.*s", static_cast<int>(profile.display_name.size()), profile.display_name.data());
    ImGui::Separator();
    ImGui::Text("Frames: %llu", static_cast<unsigned long long>(sync::StateHub::instance().frame_counter()));
    if (!profile.sigs.world_pointer.has_value()) {
      ImGui::TextColored(ImVec4(1.f, 0.45f, 0.2f, 1.f), "world_pointer sig: UNKNOWN");
    }
    if (!profile.sigs.view_matrix.has_value()) {
      ImGui::TextColored(ImVec4(1.f, 0.45f, 0.2f, 1.f), "view_matrix sig: UNKNOWN");
    }
    ImGui::Separator();
    ImGui::TextUnformatted("Processo esterno — nessun hook nel client finche");
    ImGui::TextUnformatted("non misuri pattern e passi a SavageModule.");
    if (ImGui::Button("Esci")) sync::StateHub::instance().request_shutdown();
  }
  ImGui::End();
}

void ExternalOverlay::pump_messages() {
  MSG msg{};
  while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

int ExternalOverlay::run() {
  target_hwnd_ = platform::find_window_by_title_substring(target_filter_).value_or(nullptr);
  if (!target_hwnd_) {
    SAVAGE_LOG(core::LogLevel::Warn, "Target window not found — overlay starts, will retry sync");
    width_ = 1280;
    height_ = 720;
  } else {
    RECT rc{};
    platform::get_client_screen_rect(target_hwnd_, rc);
    width_ = static_cast<std::uint32_t>(std::max<LONG>(1, rc.right - rc.left));
    height_ = static_cast<std::uint32_t>(std::max<LONG>(1, rc.bottom - rc.top));
  }

  if (!create_overlay_window()) {
    SAVAGE_LOG(core::LogLevel::Error, "Overlay window creation failed");
    return 1;
  }
  if (!gfx_.create_for_hwnd(overlay_hwnd_, width_, height_)) return 2;
  if (!ui_.init(overlay_hwnd_, gfx_.device(), gfx_.context())) return 3;

  SAVAGE_LOG(core::LogLevel::Info, "External overlay running");

  while (!sync::StateHub::instance().shutdown_requested()) {
    pump_messages();
    sync_to_target();

    const float clear[4] = {0.f, 0.f, 0.f, 0.f};
    gfx_.begin_frame(clear);
    ui_.begin_frame();
    render_ui();
    const bool want_mouse = ImGui::GetIO().WantCaptureMouse;
    LONG ex_style = GetWindowLongW(overlay_hwnd_, GWL_EXSTYLE);
    if (want_mouse) {
      ex_style &= ~WS_EX_TRANSPARENT;
    } else {
      ex_style |= WS_EX_TRANSPARENT;
    }
    SetWindowLongW(overlay_hwnd_, GWL_EXSTYLE, ex_style);
    ui_.end_frame();
    ui_.render_draw_data(gfx_.context());
    gfx_.end_frame();

    if (sync::StateHub::instance().shutdown_requested()) break;
  }

  ui_.shutdown();
  if (overlay_hwnd_) DestroyWindow(overlay_hwnd_);
  return 0;
}

}  // namespace savage::overlay