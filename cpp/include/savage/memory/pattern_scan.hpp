#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace savage::memory {

/** Parse "48 8B 05 ? ? ? ?" into bytes + mask ('x' / '?'). */
std::optional<std::pair<std::vector<std::uint8_t>, std::string>> parse_ida_pattern(std::string_view pattern);

std::optional<std::uintptr_t> scan_region(std::span<const std::uint8_t> region, std::uintptr_t region_base,
                                          std::span<const std::uint8_t> pattern, std::string_view mask);

}  // namespace savage::memory