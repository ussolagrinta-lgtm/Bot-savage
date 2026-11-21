#include "savage/core/logger.hpp"

#include <chrono>
#include <cstdio>
#include <mutex>

namespace savage::core {

namespace {

std::mutex g_mutex;
LogLevel g_min = LogLevel::Info;

const char* level_name(LogLevel l) {
  switch (l) {
    case LogLevel::Trace:
      return "TRACE";
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warn:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
  }
  return "?";
}

}  // namespace

void set_min_level(LogLevel level) { g_min = level; }
LogLevel min_level() { return g_min; }

void log(LogLevel level, std::string_view message, std::source_location where) {
  if (static_cast<int>(level) < static_cast<int>(g_min)) return;

  const auto now = std::chrono::system_clock::now();
  const auto t = std::chrono::system_clock::to_time_t(now);

  std::lock_guard lock(g_mutex);
  std::fprintf(stderr, "[%s] %.*s (%s:%u)\n", level_name(level), static_cast<int>(message.size()),
               message.data(), where.file_name(), static_cast<unsigned>(where.line()));
  std::fflush(stderr);
}

}  // namespace savage::core