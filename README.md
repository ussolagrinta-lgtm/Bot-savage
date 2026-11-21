# ROB RUN external toolkit (kernel read + overlay menu)

Target: **ROB RUN** (Steam `4262460`), Unity IL2CPP, Easy Anti-Cheat.

| Component | Path |
|-----------|------|
| Kernel driver | `driver/src/driver.c` |
| User client + ImGui menu | `client/` |
| Shared IOCTL header | `shared/communication.h` |

## Features (client)

- ESP (box, health bar, distance) — draws when entity/camera data is configured
- Aimbot (FOV, smooth, hold key) — usermode `SendInput`, sensitivity uncalibrated
- Triggerbot (FOV gate + delay)
- Menu tabs: ESP / Aimbot / Trigger / System (`INSERT` toggles menu capture, `END` exit)

## Game data status

Static RVAs in `client/include/game_config.hpp` are **PublicLeakUnverified** until you confirm them on your build. `GameStateReader` refuses to mark the session ready until you set `Provenance::UserConfirmed` and fill entity/camera field offsets from Il2CppDumper + ReClass.

## Build client (Windows)

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Driver: see `driver/README.md` (WDK).

## Legal / scope

For offline reverse engineering on systems you own. Online use violates the game EULA and risks EAC enforcement.