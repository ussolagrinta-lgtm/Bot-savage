#include "savage/render/d3d11_context.hpp"

#include "savage/core/logger.hpp"

namespace savage::render {

D3D11Context::~D3D11Context() { release_rtv(); }

bool D3D11Context::create_for_hwnd(HWND hwnd, std::uint32_t width, std::uint32_t height) {
  hwnd_ = hwnd;
  width = width == 0 ? 1 : width;
  height = height == 0 ? 1 : height;

  DXGI_SWAP_CHAIN_DESC scd{};
  scd.BufferCount = 2;
  scd.BufferDesc.Width = width;
  scd.BufferDesc.Height = height;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferDesc.RefreshRate.Numerator = 0;
  scd.BufferDesc.RefreshRate.Denominator = 1;
  scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = hwnd;
  scd.SampleDesc.Count = 1;
  scd.SampleDesc.Quality = 0;
  scd.Windowed = TRUE;
  scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  constexpr UINT create_flags = 0;
  D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
  const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

  const HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_flags, levels,
                                                    static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION, &scd,
                                                    swap_chain_.GetAddressOf(), device_.GetAddressOf(), &level,
                                                    context_.GetAddressOf());
  if (FAILED(hr)) {
    SAVAGE_LOG(core::LogLevel::Error, "D3D11CreateDeviceAndSwapChain failed");
    return false;
  }
  sc_desc_ = scd;
  return create_rtv();
}

void D3D11Context::resize(std::uint32_t width, std::uint32_t height) {
  if (!swap_chain_) return;
  release_rtv();
  swap_chain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, sc_desc_.Flags);
  create_rtv();
}

void D3D11Context::release_rtv() {
  if (rtv_) {
    rtv_.Reset();
  }
}

bool D3D11Context::create_rtv() {
  Microsoft::WRL::ComPtr<ID3D11Texture2D> back;
  if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(back.GetAddressOf())))) return false;
  return SUCCEEDED(device_->CreateRenderTargetView(back.Get(), nullptr, rtv_.GetAddressOf()));
}

void D3D11Context::begin_frame(const float clear[4]) {
  if (!context_ || !rtv_) return;
  const float default_clear[4] = {0.05f, 0.05f, 0.08f, 0.55f};
  const float* c = clear ? clear : default_clear;
  context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
  context_->ClearRenderTargetView(rtv_.Get(), c);
}

void D3D11Context::end_frame() {
  if (swap_chain_) swap_chain_->Present(1, 0);
}

}  // namespace savage::render