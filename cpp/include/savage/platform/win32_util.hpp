#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace savage::platform {

struct ModuleInfo {
  std::wstring name;
  std::uintptr_t base{0};
  std::size_t size{0};
};

std::optional<HWND> find_window_by_title_substring(std::wstring_view substring);

bool get_client_screen_rect(HWND hwnd, RECT& out);

std::vector<ModuleInfo> list_modules(DWORD process_id);

std::optional<ModuleInfo> find_module(DWORD process_id, std::wstring_view module_name);

}  // namespace savage::platform