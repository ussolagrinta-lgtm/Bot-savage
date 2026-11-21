# HANDOFF — ROB RUN / RobRunAIO (dump + clean-room)

## Request

Dump user-uploaded `RobRunAIO.exe`, document findings, recreate a buildable tree from scratch (`robrun/` + `analysis/`).

## Measured (static dump)

- EXE SHA256 `cd3dfd2588ca0c132fe102194a999da90cc42243d86e93d0de5b75fc7ba7dbb8` (~3 MiB).
- Device `\\.\WmPort`, message `map kernel-mode.sys`, `NtDeviceIoControlFile`.
- IL2CPP world: `GameAssembly.dll+0x81B7148` → `+0x38` → `+0x18` `_battleCharacters`; transform `0x58` / `0x60`.
- WebView2 bridge: `set:`, `state:`, `telem:`, `act:save`, hotkeys F1 / INSERT / END.
- Full report: `analysis/DUMP.md`; artifacts in `analysis/dump/`.

## Built in repo

| Area | Path |
|------|------|
| Dump report | `analysis/DUMP.md`, `analysis/dump/*` |
| Dump constants | `robrun/include/robrun/dump_schema.hpp` |
| New entry binary | `RobRunESP` — `robrun/src/main.cpp`, `cli.cpp`, `probe.cpp` |
| Shared game/driver code | `client/*`, `shared/communication.h`, optional `driver/` |
| Legacy equivalent | `RobRunPort` (older monolithic entry) |

## Not shipped / UNKNOWN

1. Kernel mapper for original `kernel-mode.sys` / full `WmPort` IOCTL layout.
2. Il2CPP dictionary walk for live player list (`world_chain.cpp` incomplete without `--probe` / ReClass).
3. Camera matrices, loot/cash CLI modes, chams — mostly stubs vs original AIO.

## Build / verify

Windows MSVC + `WEBVIEW2_SDK`. See root `README.md`.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

1. Map your driver (`WmPort` or load `RobRunMem.sys`).
2. `RobRunESP.exe --probe` with game running → `probe.log`.
3. In match: `status.log` player count; fill missing Il2CPP offsets from probe/ReClass.

## Git

Branch `cursor/rob-run-mod-menu-68b4`, PR #4.