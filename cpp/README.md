# SavageCore (C++20)

Modular Windows base for **external overlay** + **internal DLL** wiring.

| Target | Output | Role |
|--------|--------|------|
| `SavageOverlay` | EXE | Topmost D3D11 + ImGui overlay synced to target window title |
| `SavageModule` | DLL | Worker thread, MinHook ready, `PresentHook` VMT helper |
| `savage_core` | STATIC | Memory scan, hooks, render, UI |

FiveM / game offsets and signatures stay in `include/savage/config/game_profile.hpp` until you measure them on your build.

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.24+

## Build (MSVC)

```powershell
cd cpp
cmake -S . -B build -A x64
cmake --build build --config Release
```

Binaries:

- `build/Release/SavageOverlay.exe`
- `build/Release/SavageModule.dll`

## Run overlay

1. Start the game (or any window whose title contains `CitizenFX` — edit `GameProfile::target_window_substring`).
2. Run `SavageOverlay.exe`.
3. Menu captures mouse when hovered; otherwise clicks pass through (`WS_EX_TRANSPARENT`).

## Next integration steps (your build)

1. Fill `GameProfile::sigs` in `game_profile.cpp` with patterns from your dump.
2. Use `ProcessMemory::scan_module` from a small tool or extend overlay to show scan results.
3. When you have `IDXGISwapChain*`, call `PresentHook::attach` inside `SavageModule` worker (see comment in `module_entry.cpp`).

No streamproof / anti-cheat bypass code in this tree — add only in private repos if you maintain that separately.