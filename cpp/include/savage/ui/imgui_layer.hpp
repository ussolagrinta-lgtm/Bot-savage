#pragma once

#include <d3d11.h>

#include "savage/core/noncopyable.hpp"

struct ImGuiContext;

namespace savage::ui {

class ImGuiLayer : core::NonCopyable {
 public:
  bool init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
  void shutdown();

  void begin_frame();
  void end_frame();
  void render_draw_data(ID3D11DeviceContext* context);

 private:
  HWND hwnd_{nullptr};
  bool ready_{false};
};

}  // namespace savage::ui