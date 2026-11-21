#include <Windows.h>

#include "savage/config/game_profile.hpp"
#include "savage/core/logger.hpp"
#include "savage/overlay/external_overlay.hpp"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  savage::core::set_min_level(savage::core::LogLevel::Debug);
  const auto& profile = savage::config::GameProfile::active();
  const std::wstring filter(profile.target_window_substring.begin(), profile.target_window_substring.end());
  savage::overlay::ExternalOverlay app(filter);
  return app.run();
}