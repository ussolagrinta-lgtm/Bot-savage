#pragma once

#include "features.hpp"

#include <Windows.h>
#include <d3d11.h>
#include <vector>

namespace rob {

class Overlay {
public:
    Overlay() = default;
    ~Overlay();

    bool Create(HWND gameHwnd, bool clickThrough);
    void Destroy();
    bool BeginFrame();
    void EndFrame();

    void DrawEsp(const std::vector<EspDrawCommand>& commands);
    void DrawMenu(MenuConfig& cfg, const GameStateReader& state);

    HWND OverlayHwnd() const { return hwnd_; }

private:
    bool CreateDeviceD3D(HWND hwnd);
    void CleanupDeviceD3D();
    void ApplyClickThrough(bool enabled);
    void ResizeIfNeeded();

    HWND game_hwnd_ = nullptr;
    HWND hwnd_ = nullptr;

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    IDXGISwapChain* swapchain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;

    bool imgui_init_ = false;
    int width_ = 0;
    int height_ = 0;
};

} // namespace rob