#pragma once

#include <string>
#include <vector>

namespace robrun {

struct CliOptions {
    bool run_menu = false;
    bool run_probe = false;
    bool run_offsets = false;
    bool run_chain = false;
    bool run_aimtest = false;
    bool run_writetest = false;
    std::wstring exe_path;
    unsigned pid = 0;
};

CliOptions ParseCommandLine(int argc, wchar_t** argv);

} // namespace robrun