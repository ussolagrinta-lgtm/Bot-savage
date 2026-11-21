#pragma once

#include <cstdint>
#include <source_location>
#include <string_view>

namespace savage::core {

enum class LogLevel : std::uint8_t { Trace, Debug, Info, Warn, Error };

void set_min_level(LogLevel level);
LogLevel min_level();

void log(LogLevel level, std::string_view message,
         std::source_location where = std::source_location::current());

}  // namespace savage::core

#define SAVAGE_LOG(level, msg) ::savage::core::log((level), (msg))