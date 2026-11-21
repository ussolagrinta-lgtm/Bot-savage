// RobRunESP — clean-room entry recreated from static dump (see analysis/DUMP.md)

#include <Windows.h>

#include "core_rr_config.hpp"
#include "driver_io.hpp"
#include "features.hpp"
#include "overlay.hpp"
#include "process_attach.hpp"
#include "robrun/cli.hpp"
#include "robrun/dump_schema.hpp"
#include "status_log.hpp"
#include "webview_menu.hpp"
#include "world_chain.hpp"

#include "robrun/probe_fwd.hpp"

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

rob::MenuConfig FromCoreRR(const rob::CoreRRConfig& c) {
    rob::MenuConfig m{};
    m.espEnabled = c.enabled;
    m.espBox = c.show_box;
    m.espDistance = c.show_distance;
    m.espMaxDistance = c.max_distance / 100.f;
    m.aimbotEnabled = c.aa_on;
    m.aimbotKey = c.aa_key;
    m.aimbotFov = c.aa_fov_px / 15.f;
    m.aimbotSmooth = static_cast<float>(c.aa_smooth);
    m.mouseSensitivity = c.aa_aim_cal;
    return m;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    const auto opts = robrun::ParseCommandLine(argc, argv);

    if (opts.run_probe)
        return robrun::probe::RunProbe(opts.pid);
    if (opts.run_chain)
        return robrun::probe::RunChain();
    if (opts.run_writetest) {
        std::fprintf(stderr, "driver write test — use mapped WmPort; see analysis/DUMP.md\n");
        return 0;
    }
    if (opts.run_aimtest) {
        std::fprintf(stderr, "[aimtest] run in match with CoreRR.cfg aa_aim_cal — not duplicated here yet\n");
        return 0;
    }

    const auto dir = ExeDir();
    rob::CoreRRConfigStore cfg(dir / L"CoreRR.cfg");
    cfg.Load();

    rob::DriverIo driver;
    if (!driver.Open()) {
        std::fwprintf(stderr, L"[RobRunESP] kernel backend unavailable — map kernel-mode.sys or load RobRunMem.sys\n");
        return 1;
    }

    DWORD pid = opts.pid;
    if (!pid) {
        for (int i = 0; i < 120 && !pid; ++i) {
            if (auto p = rob::FindProcessByName(robrun::dump::kProcessGame))
                pid = *p;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    if (!pid)
        return 2;

    if (!driver.Attach(pid))
        return 3;

    uint64_t gaSize = 0;
    auto ga = driver.GetModuleBase(robrun::dump::kModuleIl2Cpp, &gaSize);
    if (!ga)
        return 4;

    HWND gameHwnd = rob::FindMainWindowForPid(pid);
    rob::WorldChain world(&driver);
    world.SetGameAssembly(*ga, gaSize);

    rob::Overlay overlay;
    if (gameHwnd)
        overlay.Create(gameHwnd, true);

    rob::WebViewMenu menu;
    const auto html = dir / "menu.html";
    if (std::filesystem::exists(html)) {
        menu.SetMessageHandler([&](const std::string& msg) {
            const auto cut = msg.find(':');
            const std::string kind = cut == std::string::npos ? msg : msg.substr(0, cut);
            const std::string payload = cut == std::string::npos ? "" : msg.substr(cut + 1);
            if (kind == "set") {
                const auto c2 = payload.find(':');
                if (c2 != std::string::npos)
                    cfg.ApplyKey(payload.substr(0, c2), std::stoi(payload.substr(c2 + 1)));
            } else if (kind == "act" && payload == "save")
                cfg.Save();
            else if (kind == "hello" || (kind == "act" && payload == "load"))
                menu.PushState(cfg);
            else if (kind == "close")
                menu.SetVisible(false);
        });
        menu.Create(gameHwnd, html);
        menu.SetVisible(opts.run_menu);
    }

    std::vector<rob::EspDrawCommand> esp;
    const auto statusPath = dir / "status.log";

    while (true) {
        if (GetAsyncKeyState(VK_F1) & 1)
            menu.SetVisible(true);
        if (GetAsyncKeyState(VK_INSERT) & 1)
            menu.SetVisible(!menu.IsVisible());
        if (GetAsyncKeyState(VK_END) & 1)
            break;

        world.RefreshPlayers();
        auto wp = world.ResolveWorldModel();

        rob::WriteStatusLog(statusPath, pid, static_cast<int>(world.Players().size()), wp.value_or(0),
                            std::string(world.LastError()));

        RECT rc{};
        rob::Viewport vp{};
        if (gameHwnd) {
            GetClientRect(gameHwnd, &rc);
            vp.width = rc.right - rc.left;
            vp.height = rc.bottom - rc.top;
        }

        const auto frameCfg = FromCoreRR(cfg.Get());
        if (overlay.OverlayHwnd() && overlay.BeginFrame()) {
            rob::BuildEspFromWorldPlayers(world.Players(), frameCfg, vp, esp);
            overlay.DrawEsp(esp);
            overlay.EndFrame();
        }

        if (gameHwnd)
            rob::RunAimbot({}, frameCfg, vp, gameHwnd);

        menu.PushTelemetry(pid, static_cast<int>(world.Players().size()), wp ? 2 : 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    menu.Destroy();
    overlay.Destroy();
    return 0;
}