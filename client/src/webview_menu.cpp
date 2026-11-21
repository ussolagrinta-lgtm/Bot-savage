#include "webview_menu.hpp"

#include <wrl.h>
#include <WebView2.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace rob {

namespace {

std::wstring ToWide(const std::filesystem::path& p) { return p.wstring(); }

} // namespace

WebViewMenu::~WebViewMenu() { Destroy(); }

bool WebViewMenu::Create(HWND parent, const std::filesystem::path& htmlPath) {
    Destroy();

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"RobRunWebMenu";
    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, L"Felonious Funnery — Rob Run", WS_POPUP,
                            100, 100, 920, 640, parent, nullptr, wc.hInstance, this);
    if (!hwnd_)
        return false;

    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this, htmlPath](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result))
                    return result;

                return env->CreateCoreWebView2Controller(
                    hwnd_, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                               [this, htmlPath](HRESULT res, ICoreWebView2Controller* ctrl) -> HRESULT {
                                   if (FAILED(res))
                                       return res;
                                   controller_ = ctrl;
                                   ctrl->AddRef();

                                   ICoreWebView2* webview = nullptr;
                                   ctrl->get_CoreWebView2(&webview);
                                   webview_ = webview;

                                   RECT bounds{};
                                   GetClientRect(hwnd_, &bounds);
                                   ctrl->put_Bounds(bounds);

                                   webview->add_WebMessageReceived(
                                       Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                           [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args)
                                               -> HRESULT {
                                               LPWSTR msg = nullptr;
                                               args->TryGetWebMessageAsString(&msg);
                                               if (msg) {
                                                   if (handler_) {
                                                       int len = WideCharToMultiByte(CP_UTF8, 0, msg, -1, nullptr, 0,
                                                                                     nullptr, nullptr);
                                                       std::string utf8(len, '\0');
                                                       WideCharToMultiByte(CP_UTF8, 0, msg, -1, utf8.data(), len,
                                                                           nullptr, nullptr);
                                                       handler_(utf8);
                                                   }
                                                   CoTaskMemFree(msg);
                                               }
                                               return S_OK;
                                           })
                                           .Get(),
                                       nullptr);

                                   const std::wstring uri = L"file:///" + htmlPath.lexically_normal().wstring();
                                   webview->Navigate(uri.c_str());
                                   return S_OK;
                               })
                        .Get());
            })
            .Get());

    if (FAILED(hr))
        return false;

    ShowWindow(hwnd_, SW_SHOW);
    return true;
}

void WebViewMenu::Destroy() {
    if (controller_) {
        auto* c = static_cast<ICoreWebView2Controller*>(controller_);
        c->Release();
        controller_ = nullptr;
    }
    webview_ = nullptr;
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void WebViewMenu::SetVisible(bool visible) {
    if (hwnd_)
        ShowWindow(hwnd_, visible ? SW_SHOW : SW_HIDE);
}

void WebViewMenu::PostToPage(const std::string& message) {
    if (!webview_)
        return;
    auto* wv = static_cast<ICoreWebView2*>(webview_);
    int wlen = MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, nullptr, 0);
    std::wstring wide(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, wide.data(), wlen);
    wv->PostWebMessageAsString(wide.c_str());
}

void WebViewMenu::PushState(const CoreRRConfigStore& cfg) {
    PostToPage(std::string("state:") + cfg.ToJsonState());
}

void WebViewMenu::PushTelemetry(DWORD pid, int targets, int losState) {
    char buf[128];
    snprintf(buf, sizeof(buf), "telem:{\"pid\":%lu,\"targets\":%d,\"los\":%d}", pid, targets, losState);
    PostToPage(buf);
}

LRESULT CALLBACK WebViewMenu::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* self = reinterpret_cast<WebViewMenu*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (msg == WM_SIZE && self && self->controller_) {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        static_cast<ICoreWebView2Controller*>(self->controller_)->put_Bounds(rc);
    }
    if (msg == WM_DESTROY)
        return 0;
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace rob