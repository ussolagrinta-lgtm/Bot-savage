#include "status_log.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>

namespace rob {

void WriteStatusLog(const std::filesystem::path& path, DWORD pid, int playerCount, uint64_t worldPtr,
                    const std::string& statusLine) {
    const auto now = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::ofstream out(path, std::ios::app);
    if (!out)
        return;

    // pid=%lu n=%d inst=%d world=%llX snap=%lums ... status=[%s]
    out << ms << " pid=" << pid << " n=" << playerCount << " inst=0"
        << " world=" << worldPtr << " snap=" << ms << "ms hz=0"
        << " status=[" << statusLine << "]\n";
}

} // namespace rob