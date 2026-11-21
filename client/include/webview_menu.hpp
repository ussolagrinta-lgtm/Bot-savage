#pragma once

#include "core_rr_config.hpp"

#include <Windows.h>
#include <functional>
#include <filesystem>
#include <string>

namespace rob {

using WebViewMessageHandler = std::function<void(const std::string& message)>;

class WebViewMenu {
public:
    WebViewMenu() = default;
    ~WebViewMenu();

    bool Create(HWND parent, const std::filesystem::path& htmlPath);
    void Destroy();
    void SetVisible(bool visible);
    void PostToPage(const std::string& message);
    void PushState(const CoreRRConfigStore& cfg);
    void PushTelemetry(DWORD pid, int targets, int losState);

    void SetMessageHandler(WebViewMessageHandler handler) { handler_ = std::move(handler); }

    HWND Window() const { return hwnd_; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    HWND hwnd_ = nullptr;
    void* controller_ = nullptr; // ICoreWebView2Controller*
    void* webview_ = nullptr;    // ICoreWebView2*
    WebViewMessageHandler handler_;
};

} // namespace rob