#include "robrun/cli.hpp"

#include <cwchar>

namespace robrun {

CliOptions ParseCommandLine(int argc, wchar_t** argv) {
    CliOptions o{};
    for (int i = 1; i < argc; ++i) {
        const std::wstring a = argv[i];
        if (a == L"--menu")
            o.run_menu = true;
        else if (a == L"--probe")
            o.run_probe = true;
        else if (a == L"--offsets")
            o.run_offsets = true;
        else if (a == L"--chain")
            o.run_chain = true;
        else if (a == L"--aimtest")
            o.run_aimtest = true;
        else if (a == L"--writetest")
            o.run_writetest = true;
        else if (a == L"--exe" && i + 1 < argc)
            o.exe_path = argv[++i];
        else if (a == L"--pid" && i + 1 < argc)
            o.pid = static_cast<unsigned>(_wcstoui64(argv[++i], nullptr, 10));
    }
    if (!o.run_probe && !o.run_offsets && !o.run_chain && !o.run_aimtest && !o.run_writetest)
        o.run_menu = true;
    return o;
}

} // namespace robrun