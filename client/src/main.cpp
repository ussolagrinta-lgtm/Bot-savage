#include "driver_io.hpp"
#include "features.hpp"
#include "game_config.hpp"
#include "game_state.hpp"
#include "overlay.hpp"
#include "process_attach.hpp"

#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    rob::DriverIo driver;
    if (!driver.Open()) {
        std::fprintf(stderr, "Driver not reachable. Load RobRunMem.sys first (see driver/README.md).\n");
        return 1;
    }

    auto pid = rob::FindProcessByName(rob::game::ProcessInfo::kProcessName);
    if (!pid) {
        std::fprintf(stderr, "Process \"%ls\" not found. Start ROB RUN via Steam first.\n",
                     rob::game::ProcessInfo::kProcessName);
        return 2;
    }

    if (!driver.Attach(*pid)) {
        std::fprintf(stderr, "Driver attach failed for PID %lu\n", *pid);
        return 3;
    }

    uint64_t gaSize = 0;
    auto gaBase = driver.GetModuleBase(rob::game::ProcessInfo::kGameModule, &gaSize);
    if (!gaBase) {
        std::fprintf(stderr, "GameAssembly.dll not found in target process.\n");
        return 4;
    }

    HWND gameHwnd = rob::FindMainWindowForPid(*pid);
    if (!gameHwnd) {
        std::fprintf(stderr, "Game window not found yet — waiting...\n");
        for (int i = 0; i < 50 && !gameHwnd; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            gameHwnd = rob::FindMainWindowForPid(*pid);
        }
    }
    if (!gameHwnd) {
        std::fprintf(stderr, "Could not resolve game window.\n");
        return 5;
    }

    rob::GameStateReader state(&driver);
    state.SetGameAssemblyBase(*gaBase, gaSize);

    rob::MenuConfig cfg{};
    rob::Overlay overlay;
    if (!overlay.Create(gameHwnd, cfg.overlayClickThrough)) {
        std::fprintf(stderr, "Overlay init failed.\n");
        return 6;
    }

    bool menuFocus = true;
    std::vector<rob::EspDrawCommand> esp;

    while (true) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            menuFocus = !menuFocus;
            cfg.overlayClickThrough = !menuFocus;
            // toggled in DrawMenu when editing; force now:
        }

        state.Refresh();

        rob::Viewport vp{};
        RECT rc{};
        GetClientRect(gameHwnd, &rc);
        vp.width = rc.right - rc.left;
        vp.height = rc.bottom - rc.top;

        if (overlay.BeginFrame()) {
            rob::BuildEspCommands(state, cfg, vp, esp);
            overlay.DrawEsp(esp);
            if (menuFocus)
                overlay.DrawMenu(cfg, state);

            overlay.EndFrame();
        }

        rob::RunAimbot(state, cfg, vp, gameHwnd);
        rob::RunTriggerbot(state, cfg, vp, gameHwnd);

        if (GetAsyncKeyState(VK_END) & 1)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    overlay.Destroy();
    return 0;
}