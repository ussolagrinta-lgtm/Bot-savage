# ROB RUN toolkit (recreated from RobRunAIO public build)

Open-source scaffolding aligned with the **Felonious Funnery** layout you uploaded:

| Binary (reference) | This repo |
|--------------------|-----------|
| `FFLoader.exe` | `FFSupervisor.exe` + `configs/RobRunAIO.ini` |
| `RobRunAIO.exe` | `RobRunPort.exe` |
| `CoreRR.cfg` | `CoreRRConfigStore` + `configs/CoreRR.cfg.example` |
| WebView menu HTML | `client/resources/menu.html` (extracted from your `RobRunAIO.exe`) |
| `\\.\WmPort` | Tried first in `DriverIo`; fallback `\\.\RobRunMem` (our WDK driver) |

## What the reference EXE does (from strings)

- Unity IL2CPP read via kernel backend (`map kernel-mode.sys` → `WmPort`).
- World chain: `GameAssembly+0x81B7148` → `+0x38` WorldEntities → `+0x18` `_battleCharacters`.
- BattleCharacter `Transform` @ `0x58`, `ReplicatedTransform` @ `0x60`.
- WebView2 menu bridge: `set:`, `state:`, `telem:`, `act:save`, hotkeys **F1** / **INSERT** / **END**.
- Launcher supervises overlay using `status.log` and `RobRunAIO.ini`.

We do **not** ship a vulnerable signed driver mapper or EAC bypass — only a clean `RobRunMem.sys` sample and user-mode code.

## Build (Windows)

1. Install [WebView2 SDK](https://developer.microsoft.com/microsoft-edge/webview2/) and set `WEBVIEW2_SDK`.
2. WDK for `RobRunMem.sys` (optional if you use your own `WmPort` backend).

```powershell
$env:WEBVIEW2_SDK = "C:\path\to\WebView2SDK"
cmake -S . -B build -A x64
cmake --build build --config Release
```

Copy `menu.html` next to `RobRunPort.exe` is done automatically; place `CoreRR.cfg` beside it.

## Deploy layout (like README upload)

```
FFSupervisor.exe
RobRunPort.exe
RobRunAIO.ini
CoreRR.cfg
menu.html
```

Run **FFSupervisor** as admin after mapping your kernel driver.