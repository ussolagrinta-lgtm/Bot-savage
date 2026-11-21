#include "driver_io.hpp"
#include "process_attach.hpp"
#include "robrun/dump_schema.hpp"
#include "world_chain.hpp"

#include <cstdio>
#include <fstream>

namespace robrun::probe {

int RunProbe(unsigned pid) {
    rob::DriverIo io;
    if (!io.Open()) {
        std::fprintf(stderr, "[probe] kernel backend unavailable (map kernel-mode.sys first)\n");
        return 1;
    }

    if (pid == 0) {
        if (auto p = rob::FindProcessByName(dump::kProcessGame))
            pid = *p;
    }
    if (pid == 0) {
        std::fprintf(stderr, "[probe] '%ls' is not running\n", dump::kProcessGame);
        return 2;
    }

    std::fprintf(stderr, "[probe] pid %u\n", pid);
    if (!io.Attach(pid)) {
        std::fprintf(stderr, "[probe] attach failed\n");
        return 3;
    }

    uint64_t gaSize = 0;
    auto ga = io.GetModuleBase(dump::kModuleIl2Cpp, &gaSize);
    if (!ga) {
        std::fprintf(stderr, "[probe] GameAssembly.dll not in the module list\n");
        return 4;
    }

    std::fprintf(stderr, "[probe] GameAssembly.dll base=%llX size=%X\n", *ga, static_cast<unsigned>(gaSize));

    rob::WorldChain world(&io);
    world.SetGameAssembly(*ga, gaSize);
    auto worldPtr = world.ResolveWorldModel();
    if (worldPtr && *worldPtr)
        std::fprintf(stderr, "[probe] game_init: world found world=%llX\n", *worldPtr);
    else
        std::fprintf(stderr, "[probe] game_init: no world\n");

    std::ofstream log("probe.log", std::ios::app);
    log << "pid=" << pid << " ga=" << std::hex << *ga << " world=" << (worldPtr ? *worldPtr : 0) << "\n";
    return 0;
}

int RunChain() {
    std::fprintf(stderr, "[chain] reading robrun_chain.example.txt format\n");
    std::fprintf(stderr, "  world = GameAssembly.dll+0x%llX\n", static_cast<unsigned long long>(dump::kWorldRootRva));
    std::fprintf(stderr, "  camera = world+0x%X +0x%X\n", dump::kWorldToWorldEntities, dump::kWorldEntitiesToBattleList);
    return 0;
}

} // namespace robrun::probe