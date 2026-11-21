#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace rob {

// Format matched to RobRunAIO status.log line (from binary strings).
void WriteStatusLog(const std::filesystem::path& path, DWORD pid, int playerCount, uint64_t worldPtr,
                    const std::string& statusLine);

} // namespace rob