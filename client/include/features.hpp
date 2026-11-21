#pragma once

#include "game_state.hpp"
#include "world_chain.hpp"
#include "math_util.hpp"

#include <Windows.h>

namespace rob {

struct MenuConfig {
    bool espEnabled = true;
    bool espBox = true;
    bool espSnapline = false;
    bool espHealth = true;
    bool espDistance = true;
    float espMaxDistance = 250.f;

    bool aimbotEnabled = false;
    int aimbotKey = VK_RBUTTON;
    float aimbotFov = 8.f;
    float aimbotSmooth = 6.f;
    bool aimbotTeamCheck = true;

    bool triggerEnabled = false;
    int triggerKey = VK_LBUTTON;
    int triggerDelayMs = 40;
    float triggerFov = 2.f;

    bool overlayClickThrough = true;
    float mouseSensitivity = 1.0f; // uncalibrated
};

struct EspDrawCommand {
    Vec2 topLeft;
    Vec2 bottomRight;
    float health01 = -1.f;
    float distanceM = -1.f;
};

void BuildEspCommands(const GameStateReader& state, const MenuConfig& cfg, const Viewport& vp,
                      std::vector<EspDrawCommand>& out);

void BuildEspFromWorldPlayers(const std::vector<BattlePlayer>& players, const MenuConfig& cfg, const Viewport& vp,
                              std::vector<EspDrawCommand>& out);

bool RunAimbot(const GameStateReader& state, const MenuConfig& cfg, const Viewport& vp, HWND gameHwnd);

bool RunTriggerbot(const GameStateReader& state, const MenuConfig& cfg, const Viewport& vp, HWND gameHwnd);

} // namespace rob