#include "savage/ui/imgui_layer.hpp"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "savage/core/logger.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace savage::ui {

bool ImGuiLayer::init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  if (!ImGui_ImplWin32_Init(hwnd)) {
    SAVAGE_LOG(core::LogLevel::Error, "ImGui Win32 init failed");
    return false;
  }
  if (!ImGui_ImplDX11_Init(device, context)) {
    SAVAGE_LOG(core::LogLevel::Error, "ImGui DX11 init failed");
    return false;
  }
  hwnd_ = hwnd;
  ready_ = true;
  return true;
}

void ImGuiLayer::shutdown() {
  if (!ready_) return;
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  ready_ = false;
}

void ImGuiLayer::begin_frame() {
  if (!ready_) return;
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::end_frame() {
  if (!ready_) return;
  ImGui::Render();
}

void ImGuiLayer::render_draw_data(ID3D11DeviceContext* context) {
  if (!ready_) return;
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace savage::ui