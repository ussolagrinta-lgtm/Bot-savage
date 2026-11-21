#pragma once

#include <cstdint>
#include <string>

#include "savage/core/noncopyable.hpp"
#include "savage/render/d3d11_context.hpp"
#include "savage/ui/imgui_layer.hpp"

namespace savage::overlay {

class ExternalOverlay : core::NonCopyable {
 public:
  explicit ExternalOverlay(std::wstring target_title_substring);

  int run();

 private:
  bool create_overlay_window();
  void sync_to_target();
  void render_ui();
  void pump_messages();

  std::wstring target_filter_;
  HWND target_hwnd_{nullptr};
  HWND overlay_hwnd_{nullptr};
  std::uint32_t width_{0};
  std::uint32_t height_{0};
  render::D3D11Context gfx_;
  ui::ImGuiLayer ui_;
};

}  // namespace savage::overlay