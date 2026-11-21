# HANDOFF — ROB RUN / RobRunAIO recreation

## Request

Recreate source from user-uploaded `RobRunAIO.exe` + `FFLoader.exe` (public build), better structured than first scaffold.

## Measured in repo

- Steam metadata: process `ROB RUN.exe`, module `GameAssembly.dll`, Unity IL2CPP, EAC.
- Driver IOCTL protocol implemented in `shared/communication.h`.

## Assumptions

- External architecture (no injection into `ROB RUN.exe`).
- User loads a signed/test-signed `RobRunMem.sys` themselves.
- Aim/trigger via usermode mouse input.

## From RobRunAIO.exe strings (user upload)

- Device: `\\.\WmPort` (not included — map your own `kernel-mode.sys`).
- World RVA `0x81B7148`, offsets `0x38` / `0x18`, transform `0x58` / `0x60`.
- Full WebView menu HTML extracted to `client/resources/menu.html`.
- `CoreRR.cfg` keys mirrored in `core_rr_config.hpp`.

## Still UNKNOWN (blocks live ESP/aim)

1. Il2CPP dictionary/list `prim`/`dict` offsets for `_battleCharacters` enumeration.
2. `BattleCameraModel` matrix fields + yaw calibration (`aa_aim_cal`).
3. Whether your driver is `WmPort` or `RobRunMem.sys`.

## Files

- `driver/src/driver.c` — kernel attach/read/write/module walk
- `client/*` — overlay menu + features
- `CMakeLists.txt` — `RobRunClient` target

## Next verification

1. Load driver, run client, confirm attach + `GameAssembly` base in System tab error clearing to layout errors only.
2. Paste ReClass layout → fill `EntityLayout` + camera reader in `game_state.cpp`.
3. Set RVAs to `UserConfirmed`, rebuild, test ESP in a private/offline context.