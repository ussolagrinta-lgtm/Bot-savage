#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace savage::config {

/** Provenance for every build-dependent value — fill from your dump / CE / IDA. */
struct MeasuredAddress {
  enum class Kind : std::uint8_t { FileOffset, Rva, RuntimeVa };

  Kind kind{Kind::Rva};
  std::uint64_t value{0};
  std::string_view module_name{};
  std::string_view measured_on_build{};  // e.g. "bXXXX" or game exe hash prefix
};

struct GameProfile {
  std::string_view display_name{"FiveM"};
  std::string_view target_window_substring{"CitizenFX"};

  /** External attach — empty optional means not configured yet. */
  std::optional<MeasuredAddress> swapchain_present_vtable_slot{};

  struct Signatures {
    /** Example: "48 8B 05 ? ? ? ? 48 85 C0" — all unset until you measure. */
    std::optional<std::string_view> world_pointer{};
    std::optional<std::string_view> view_matrix{};
  } sigs;

  static const GameProfile& active();
};

}  // namespace savage::config