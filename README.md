# ROB RUN toolkit (recreated from RobRunAIO public build)

Open-source scaffolding aligned with the **Felonious Funnery** layout you uploaded:

| Binary (reference) | This repo |
|--------------------|-----------|
| `FFLoader.exe` | `FFSupervisor.exe` + `configs/RobRunAIO.ini` |
| `RobRunAIO.exe` | **`RobRunESP.exe`** (`robrun/`, dump-driven) or legacy `RobRunPort.exe` |
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

## Static dump (clean-room rebuild)

Reference `RobRunAIO.exe` (SHA256 in `robrun/include/robrun/dump_schema.hpp`) is analyzed under **`analysis/`**:

- [`analysis/DUMP.md`](analysis/DUMP.md) — PE layout, CLI flags, world chain, WebView2 bridge, IOCTL strings.
- `analysis/dump/` — extracted `menu.html`, string tables, PE summary (no mapper shipped).

Build target **`RobRunESP`** uses the same `rob::` core as `RobRunPort` but entry/CLI match the dumped tool (`--probe`, `--chain`, default menu loop).

## Build (Windows)

1. Install [WebView2 SDK](https://developer.microsoft.com/microsoft-edge/webview2/) and set `WEBVIEW2_SDK`.
2. WDK for `RobRunMem.sys` (optional if you use your own `WmPort` backend).

```powershell
$env:WEBVIEW2_SDK = "C:\path\to\WebView2SDK"
cmake -S . -B build -A x64
cmake --build build --config Release
```

POST_BUILD copies `menu.html` + example `CoreRR.cfg` next to `RobRunESP.exe` / `RobRunPort.exe`.

## Deploy layout (like README upload)

```
FFSupervisor.exe
RobRunESP.exe
RobRunAIO.ini
CoreRR.cfg
menu.html
```

Run **FFSupervisor** as admin after mapping your kernel driver.