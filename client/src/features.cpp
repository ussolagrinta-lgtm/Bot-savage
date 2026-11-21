#include "features.hpp"

#include <algorithm>
#include <chrono>

namespace rob {

static bool KeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

static void SendRelativeMouse(long dx, long dy) {
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    SendInput(1, &input, sizeof(INPUT));
}

void BuildEspCommands(const GameStateReader& state, const MenuConfig& cfg, const Viewport& vp,
                      std::vector<EspDrawCommand>& out) {
    out.clear();
    if (!cfg.espEnabled || !state.Camera().valid)
        return;

    const auto& cam = state.Camera();
    for (const auto& ent : state.Entities()) {
        if (!ent.valid)
            continue;

        float dist = Distance(cam.position, ent.position);
        if (dist > cfg.espMaxDistance)
            continue;

        auto screen = WorldToScreen(ent.position, cam.view, cam.projection, vp);
        if (!screen)
            continue;

        EspDrawCommand cmd{};
        const float h = std::max(24.f, 12000.f / dist);
        const float w = h * 0.55f;
        cmd.topLeft = {screen->x - w * 0.5f, screen->y - h};
        cmd.bottomRight = {screen->x + w * 0.5f, screen->y};
        if (ent.health >= 0.f)
            cmd.health01 = std::clamp(ent.health / 100.f, 0.f, 1.f);
        cmd.distanceM = dist;
        out.push_back(cmd);
    }
}

bool RunAimbot(const GameStateReader& state, const MenuConfig& cfg, const Viewport& vp, HWND gameHwnd) {
    if (!cfg.aimbotEnabled || !state.Camera().valid)
        return false;
    if (GetForegroundWindow() != gameHwnd)
        return false;
    if (!KeyDown(cfg.aimbotKey))
        return false;

    const auto& cam = state.Camera();
    const EntitySnapshot* best = nullptr;
    float bestFov = cfg.aimbotFov;

    for (const auto& ent : state.Entities()) {
        if (!ent.valid)
            continue;
        if (cfg.aimbotTeamCheck && ent.teamId >= 0)
            continue; // need local team id

        float fov = HorizontalFovDegrees(cam.position, ent.position, cam.forward);
        if (fov < bestFov) {
            bestFov = fov;
            best = &ent;
        }
    }

    if (!best)
        return false;

    Vec3 delta = Sub(best->position, cam.position);
    float yaw = std::atan2(delta.x, delta.z) * 180.f / 3.14159265358979323846f;
    float pitch = std::atan2(-delta.y, std::sqrt(delta.x * delta.x + delta.z * delta.z)) * 180.f /
                  3.14159265358979323846f;

    Vec2 mouse = CalcMouseDeltaForAngles(yaw / cfg.aimbotSmooth, pitch / cfg.aimbotSmooth, cfg.mouseSensitivity);
    SendRelativeMouse(static_cast<long>(mouse.x), static_cast<long>(mouse.y));
    return true;
}

bool RunTriggerbot(const GameStateReader& state, const MenuConfig& cfg, const Viewport& vp, HWND gameHwnd) {
    if (!cfg.triggerEnabled || !state.Camera().valid)
        return false;
    if (GetForegroundWindow() != gameHwnd)
        return false;
    if (!KeyDown(cfg.triggerKey))
        return false;

    const auto& cam = state.Camera();
    const EntitySnapshot* cross = nullptr;
    float best = cfg.triggerFov;

    for (const auto& ent : state.Entities()) {
        if (!ent.valid)
            continue;
        float fov = HorizontalFovDegrees(cam.position, ent.position, cam.forward);
        if (fov < best) {
            best = fov;
            cross = &ent;
        }
    }

    if (!cross)
        return false;

    static auto lastShot = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShot).count() < cfg.triggerDelayMs)
        return false;

    INPUT down{};
    down.type = INPUT_MOUSE;
    down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    INPUT up = down;
    up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &down, sizeof(INPUT));
    SendInput(1, &up, sizeof(INPUT));
    lastShot = now;
    return true;
}

} // namespace rob