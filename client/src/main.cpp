#include "core_rr_config.hpp"
#include "driver_io.hpp"
#include "features.hpp"
#include "game_config.hpp"
#include "process_attach.hpp"
#include "status_log.hpp"
#include "webview_menu.hpp"
#include "world_chain.hpp"

#include "overlay.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <thread>

namespace {

std::filesystem::path ExeDir() {
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
}

MenuConfig FromCoreRR(const rob::CoreRRConfig& c) {
    rob::MenuConfig m{};
    m.espEnabled = c.enabled;
    m.espBox = c.show_box;
    m.espHealth = c.show_health;
    m.espDistance = c.show_distance;
    m.espMaxDistance = c.max_distance / 100.f;

    m.aimbotEnabled = c.aa_on;
    m.aimbotKey = c.aa_key;
    m.aimbotFov = c.aa_fov_px / 15.f;
    m.aimbotSmooth = static_cast<float>(c.aa_smooth);
    m.aimbotTeamCheck = !c.aa_target_clan;
    m.mouseSensitivity = c.aa_aim_cal;
    return m;
}

} // namespace

int main() {
    const auto dir = ExeDir();
    rob::CoreRRConfigStore cfgStore(dir / "CoreRR.cfg");
    if (!cfgStore.Load())
        cfgStore.Load(); // defaults

    rob::DriverIo driver;
    if (!driver.Open()) {
        std::fprintf(stderr,
                     "Kernel backend unavailable — map kernel-mode.sys (WmPort) or load RobRunMem.sys first.\n");
        return 1;
    }

    DWORD pid = 0;
    for (int i = 0; i < 300 && !pid; ++i) {
        if (auto p = rob::FindProcessByName(rob::game::ProcessInfo::kProcessName))
            pid = *p;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    if (!pid) {
        std::fprintf(stderr, "ROB RUN.exe not found.\n");
        return 2;
    }

    if (!driver.Attach(pid)) {
        std::fprintf(stderr, "Driver attach failed.\n");
        return 3;
    }

    uint64_t gaSize = 0;
    auto gaBase = driver.GetModuleBase(rob::game::ProcessInfo::kGameModule, &gaSize);
    if (!gaBase) {
        std::fprintf(stderr, "GameAssembly.dll not found.\n");
        return 4;
    }

    HWND gameHwnd = rob::FindMainWindowForPid(pid);
    rob::WorldChain world(&driver);
    world.SetGameAssembly(*gaBase, gaSize);

    rob::Overlay overlay;
    if (gameHwnd && !overlay.Create(gameHwnd, true)) {
        std::fprintf(stderr, "Overlay init failed.\n");
        return 5;
    }

    rob::WebViewMenu menu;
    const auto menuBesideExe = dir / "menu.html";
    const auto menuInTree = dir / "client" / "resources" / "menu.html";
    const auto menuPath =
        std::filesystem::exists(menuBesideExe) ? menuBesideExe :
        (std::filesystem::exists(menuInTree) ? menuInTree : menuBesideExe);

    menu.SetMessageHandler([&](const std::string& msg) {
        const auto cut = msg.find(':');
        const std::string kind = cut == std::string::npos ? msg : msg.substr(0, cut);
        const std::string payload = cut == std::string::npos ? "" : msg.substr(cut + 1);

        if (kind == "set") {
            const auto cut2 = payload.find(':');
            if (cut2 != std::string::npos) {
                cfgStore.ApplyKey(payload.substr(0, cut2), std::stoi(payload.substr(cut2 + 1)));
            }
        } else if (kind == "col") {
            const auto cut2 = payload.find(':');
            if (cut2 != std::string::npos) {
                const std::string name = payload.substr(0, cut2);
                const std::string rgb = payload.substr(cut2 + 1);
                unsigned r = 0, g = 0, b = 0;
                sscanf(rgb.c_str(), "%u,%u,%u", &r, &g, &b);
                cfgStore.ApplyColor(name, static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
            }
        } else if (kind == "act" && payload == "save") {
            if (!cfgStore.Save())
                menu.PostToPage("toast:Could not save CoreRR.cfg");
        } else if (kind == "act" && payload == "load") {
            if (!cfgStore.Load())
                menu.PostToPage("toast:Could not load CoreRR.cfg");
            else
                menu.PushState(cfgStore);
        } else if (kind == "close") {
            menu.SetVisible(false);
        } else if (kind == "hello") {
            menu.PushState(cfgStore);
        }
    });

    if (std::filesystem::exists(menuPath))
        menu.Create(gameHwnd, menuPath);

    bool menuVisible = true;
    std::vector<rob::EspDrawCommand> esp;

    const auto statusPath = dir / "status.log";

    while (true) {
        if (GetAsyncKeyState(VK_F1) & 1) {
            menuVisible = !menuVisible;
            menu.SetVisible(menuVisible);
        }
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            menuVisible = !menuVisible;
            menu.SetVisible(menuVisible);
        }
        if (GetAsyncKeyState(VK_END) & 1)
            break;

        world.RefreshPlayers();
        auto worldPtr = world.ResolveWorldModel();

        rob::WriteStatusLog(statusPath, pid, static_cast<int>(world.Players().size()),
                            worldPtr.value_or(0), std::string(world.LastError()));

        rob::Viewport vp{};
        if (gameHwnd) {
            RECT rc{};
            GetClientRect(gameHwnd, &rc);
            vp.width = rc.right - rc.left;
            vp.height = rc.bottom - rc.top;
        }

        const MenuConfig frameCfg = FromCoreRR(cfgStore.Get());

        if (overlay.OverlayHwnd() && overlay.BeginFrame()) {
            rob::BuildEspFromWorldPlayers(world.Players(), frameCfg, vp, esp);
            overlay.DrawEsp(esp);
            overlay.EndFrame();
        }

        if (gameHwnd)
            rob::RunAimbot({}, frameCfg, vp, gameHwnd);

        menu.PushTelemetry(pid, static_cast<int>(world.Players().size()), worldPtr ? 2 : 1);

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    menu.Destroy();
    overlay.Destroy();
    return 0;
}