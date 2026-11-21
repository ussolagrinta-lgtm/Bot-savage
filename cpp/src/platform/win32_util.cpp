#include "savage/platform/win32_util.hpp"

#include <TlHelp32.h>
#include <string>

namespace savage::platform {

namespace {

BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lparam) {
  auto* ctx = reinterpret_cast<std::pair<std::wstring_view, HWND*>*>(lparam);
  if (!IsWindowVisible(hwnd)) return TRUE;

  wchar_t title[512]{};
  const int len = GetWindowTextW(hwnd, title, static_cast<int>(std::size(title)));
  if (len <= 0) return TRUE;

  const std::wstring_view current(title, static_cast<std::size_t>(len));
  if (current.find(ctx->first) != std::wstring_view::npos) {
    *ctx->second = hwnd;
    return FALSE;
  }
  return TRUE;
}

}  // namespace

std::optional<HWND> find_window_by_title_substring(std::wstring_view substring) {
  HWND found = nullptr;
  auto ctx = std::make_pair(substring, &found);
  EnumWindows(enum_windows_proc, reinterpret_cast<LPARAM>(&ctx));
  if (found == nullptr) return std::nullopt;
  return found;
}

bool get_client_screen_rect(HWND hwnd, RECT& out) {
  RECT client{};
  if (!GetClientRect(hwnd, &client)) return false;
  POINT tl{client.left, client.top};
  POINT br{client.right, client.bottom};
  if (!ClientToScreen(hwnd, &tl)) return false;
  if (!ClientToScreen(hwnd, &br)) return false;
  out.left = tl.x;
  out.top = tl.y;
  out.right = br.x;
  out.bottom = br.y;
  return true;
}

std::vector<ModuleInfo> list_modules(DWORD process_id) {
  std::vector<ModuleInfo> out;
  const HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
  if (snap == INVALID_HANDLE_VALUE) return out;

  MODULEENTRY32W me{};
  me.dwSize = sizeof(me);
  if (Module32FirstW(snap, &me)) {
    do {
      ModuleInfo info;
      info.name = me.szModule;
      info.base = reinterpret_cast<std::uintptr_t>(me.modBaseAddr);
      info.size = static_cast<std::size_t>(me.modBaseSize);
      out.push_back(std::move(info));
    } while (Module32NextW(snap, &me));
  }
  CloseHandle(snap);
  return out;
}

std::optional<ModuleInfo> find_module(DWORD process_id, std::wstring_view module_name) {
  for (const auto& m : list_modules(process_id)) {
    if (_wcsicmp(m.name.c_str(), std::wstring(module_name).c_str()) == 0) return m;
  }
  return std::nullopt;
}

}  // namespace savage::platform