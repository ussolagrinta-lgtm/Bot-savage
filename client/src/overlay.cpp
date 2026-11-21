#include "overlay.hpp"

#include <dwmapi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace rob {

namespace {

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

} // namespace

Overlay::~Overlay() { Destroy(); }

bool Overlay::CreateDeviceD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL featureLevel{};
    const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};

    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags, levels, 1,
                                             D3D11_SDK_VERSION, &sd, &swapchain_, &device_, &featureLevel,
                                             &context_)))
        return false;

    ID3D11Texture2D* backBuffer = nullptr;
    swapchain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (backBuffer) {
        device_->CreateRenderTargetView(backBuffer, nullptr, &rtv_);
        backBuffer->Release();
    }
    return rtv_ != nullptr;
}

void Overlay::CleanupDeviceD3D() {
    if (rtv_) {
        rtv_->Release();
        rtv_ = nullptr;
    }
    if (swapchain_) {
        swapchain_->Release();
        swapchain_ = nullptr;
    }
    if (context_) {
        context_->Release();
        context_ = nullptr;
    }
    if (device_) {
        device_->Release();
        device_ = nullptr;
    }
}

void Overlay::ApplyClickThrough(bool enabled) {
    if (!hwnd_)
        return;
    LONG_PTR style = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
    if (enabled)
        style |= WS_EX_TRANSPARENT;
    else
        style &= ~WS_EX_TRANSPARENT;
    SetWindowLongPtr(hwnd_, GWL_EXSTYLE, style);
}

bool Overlay::Create(HWND gameHwnd, bool clickThrough) {
    game_hwnd_ = gameHwnd;
    if (!game_hwnd_)
        return false;

    RECT rc{};
    GetClientRect(game_hwnd_, &rc);
    width_ = rc.right - rc.left;
    height_ = rc.bottom - rc.top;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"RobRunOverlay";
    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, L"RobRun Overlay",
        WS_POPUP, 0, 0, width_, height_, nullptr, nullptr, wc.hInstance, nullptr);

    if (!hwnd_)
        return false;

    MARGINS margins{-1};
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
    SetLayeredWindowAttributes(hwnd_, RGB(0, 0, 0), 255, LWA_ALPHA);

    if (!CreateDeviceD3D(hwnd_))
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX11_Init(device_, context_);
    imgui_init_ = true;

    ApplyClickThrough(clickThrough);
    ShowWindow(hwnd_, SW_SHOW);
    return true;
}

void Overlay::Destroy() {
    if (imgui_init_) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        imgui_init_ = false;
    }
    CleanupDeviceD3D();
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void Overlay::ResizeIfNeeded() {
    if (!game_hwnd_ || !hwnd_)
        return;

    RECT rc{};
    GetWindowRect(game_hwnd_, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w == width_ && h == height_)
        return;

    width_ = w;
    height_ = h;
    SetWindowPos(hwnd_, HWND_TOPMOST, rc.left, rc.top, width_, height_, SWP_NOACTIVATE);
    if (swapchain_) {
        if (rtv_) {
            rtv_->Release();
            rtv_ = nullptr;
        }
        swapchain_->ResizeBuffers(0, width_, height_, DXGI_FORMAT_UNKNOWN, 0);
        ID3D11Texture2D* backBuffer = nullptr;
        swapchain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (backBuffer) {
            device_->CreateRenderTargetView(backBuffer, nullptr, &rtv_);
            backBuffer->Release();
        }
    }
}

bool Overlay::BeginFrame() {
    if (!hwnd_ || !imgui_init_)
        return false;
    ResizeIfNeeded();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    return true;
}

void Overlay::EndFrame() {
    ImGui::Render();
    const float clear[4] = {0, 0, 0, 0};
    context_->OMSetRenderTargets(1, &rtv_, nullptr);
    context_->ClearRenderTargetView(rtv_, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapchain_->Present(1, 0);
}

void Overlay::DrawEsp(const std::vector<EspDrawCommand>& commands) {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    for (const auto& c : commands) {
        ImU32 col = IM_COL32(80, 220, 120, 220);
        draw->AddRect(ImVec2(c.topLeft.x, c.topLeft.y), ImVec2(c.bottomRight.x, c.bottomRight.y), col, 0.f, 0, 1.8f);

        if (c.health01 >= 0.f) {
            float barW = c.bottomRight.x - c.topLeft.x;
            draw->AddRectFilled(ImVec2(c.topLeft.x, c.topLeft.y - 6.f), ImVec2(c.topLeft.x + barW * c.health01, c.topLeft.y - 2.f),
                                IM_COL32(220, 80, 80, 255));
        }
        if (c.distanceM >= 0.f) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0fm", c.distanceM);
            draw->AddText(ImVec2(c.topLeft.x, c.bottomRight.y + 2.f), IM_COL32(255, 255, 255, 220), buf);
        }
    }
}

void Overlay::DrawMenu(MenuConfig& cfg, const GameStateReader& state) {
    ImGui::Begin("ROB RUN // Ritmada", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTabBar("tabs")) {
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Checkbox("Enable", &cfg.espEnabled);
            ImGui::Checkbox("Box", &cfg.espBox);
            ImGui::Checkbox("Snapline", &cfg.espSnapline);
            ImGui::Checkbox("Health", &cfg.espHealth);
            ImGui::Checkbox("Distance", &cfg.espDistance);
            ImGui::SliderFloat("Max distance", &cfg.espMaxDistance, 50.f, 500.f, "%.0f");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Aimbot")) {
            ImGui::Checkbox("Enable", &cfg.aimbotEnabled);
            ImGui::SliderFloat("FOV", &cfg.aimbotFov, 1.f, 30.f);
            ImGui::SliderFloat("Smooth", &cfg.aimbotSmooth, 1.f, 20.f);
            ImGui::Checkbox("Team check", &cfg.aimbotTeamCheck);
            ImGui::SliderFloat("Sensitivity scale", &cfg.mouseSensitivity, 0.1f, 5.f);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Trigger")) {
            ImGui::Checkbox("Enable", &cfg.triggerEnabled);
            ImGui::SliderFloat("FOV", &cfg.triggerFov, 0.5f, 10.f);
            ImGui::SliderInt("Delay ms", &cfg.triggerDelayMs, 0, 250);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("System")) {
            ImGui::Checkbox("Click-through overlay", &cfg.overlayClickThrough);
            if (ImGui::IsItemDeactivatedAfterEdit())
                ApplyClickThrough(cfg.overlayClickThrough);
            ImGui::Text("INSERT — toggle menu focus");
            ImGui::TextWrapped("Status: %s", state.LastError().empty() ? "idle" : state.LastError().data());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace rob