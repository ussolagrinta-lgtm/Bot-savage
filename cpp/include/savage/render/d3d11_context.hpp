#pragma once

#include <cstdint>

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "savage/core/noncopyable.hpp"

namespace savage::render {

class D3D11Context : core::NonCopyable {
 public:
  D3D11Context() = default;
  ~D3D11Context();

  bool create_for_hwnd(HWND hwnd, std::uint32_t width, std::uint32_t height);
  void resize(std::uint32_t width, std::uint32_t height);
  void begin_frame(const float clear[4]);
  void end_frame();

  [[nodiscard]] ID3D11Device* device() const noexcept { return device_.Get(); }
  [[nodiscard]] ID3D11DeviceContext* context() const noexcept { return context_.Get(); }
  [[nodiscard]] IDXGISwapChain* swap_chain() const noexcept { return swap_chain_.Get(); }
  [[nodiscard]] ID3D11RenderTargetView* rtv() const noexcept { return rtv_.Get(); }
  [[nodiscard]] HWND hwnd() const noexcept { return hwnd_; }

 private:
  void release_rtv();
  bool create_rtv();

  HWND hwnd_{nullptr};
  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
  DXGI_SWAP_CHAIN_DESC sc_desc_{};
};

}  // namespace savage::render