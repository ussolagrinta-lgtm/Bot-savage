# HANDOFF — ROB RUN mod menu scaffold

## Request

Mod menu for ROB RUN: kernel driver memory I/O, ESP, aimbot, triggerbot, external DXGI overlay.

## Measured in repo

- Steam metadata: process `ROB RUN.exe`, module `GameAssembly.dll`, Unity IL2CPP, EAC.
- Driver IOCTL protocol implemented in `shared/communication.h`.

## Assumptions

- External architecture (no injection into `ROB RUN.exe`).
- User loads a signed/test-signed `RobRunMem.sys` themselves.
- Aim/trigger via usermode mouse input.

## Still UNKNOWN (blocks in-game ESP/aim)

1. Confirm static RVAs in `game_config.hpp` on live `GameAssembly.dll` (Il2CppDumper).
2. `WorldEntitiesModel` list layout + count + element stride.
3. `BattleCameraModel` view/proj matrix field offsets + local team id.
4. Mouse sensitivity calibration for aimbot.

## Files

- `driver/src/driver.c` — kernel attach/read/write/module walk
- `client/*` — overlay menu + features
- `CMakeLists.txt` — `RobRunClient` target

## Next verification

1. Load driver, run client, confirm attach + `GameAssembly` base in System tab error clearing to layout errors only.
2. Paste ReClass layout → fill `EntityLayout` + camera reader in `game_state.cpp`.
3. Set RVAs to `UserConfirmed`, rebuild, test ESP in a private/offline context.