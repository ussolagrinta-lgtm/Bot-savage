#include "savage/memory/pattern_scan.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace savage::memory {

namespace {

std::optional<std::uint8_t> parse_hex_byte(std::string_view token) {
  if (token == "?" || token == "??") return std::nullopt;
  if (token.size() != 2) return std::uint8_t{0};

  auto nybble = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  };
  const int hi = nybble(token[0]);
  const int lo = nybble(token[1]);
  if (hi < 0 || lo < 0) return std::uint8_t{0};
  return static_cast<std::uint8_t>((hi << 4) | lo);
}

}  // namespace

std::optional<std::pair<std::vector<std::uint8_t>, std::string>> parse_ida_pattern(std::string_view pattern) {
  std::vector<std::uint8_t> bytes;
  std::string mask;
  std::string token;

  auto flush = [&]() {
    if (token.empty()) return;
    const auto b = parse_hex_byte(token);
    if (b) {
      bytes.push_back(*b);
      mask.push_back('x');
    } else {
      bytes.push_back(0);
      mask.push_back('?');
    }
    token.clear();
  };

  for (char c : pattern) {
    if (std::isspace(static_cast<unsigned char>(c))) {
      flush();
      continue;
    }
    token.push_back(c);
  }
  flush();

  if (bytes.empty() || bytes.size() != mask.size()) return std::nullopt;
  return std::make_pair(std::move(bytes), std::move(mask));
}

std::optional<std::uintptr_t> scan_region(std::span<const std::uint8_t> region, std::uintptr_t region_base,
                                          std::span<const std::uint8_t> pattern, std::string_view mask) {
  if (pattern.empty() || pattern.size() != mask.size() || region.size() < pattern.size()) return std::nullopt;

  const std::size_t limit = region.size() - pattern.size();
  for (std::size_t i = 0; i <= limit; ++i) {
  bool match = true;
    for (std::size_t j = 0; j < pattern.size(); ++j) {
      if (mask[j] == '?') continue;
      if (region[i + j] != pattern[j]) {
        match = false;
        break;
      }
    }
    if (match) return region_base + i;
  }
  return std::nullopt;
}

}  // namespace savage::memory