# RobRunAIO / FFLoader static dump

Source artifacts: user upload (Nov 2026). No runtime execution on analyst VM.

## Binaries

| File | SHA-256 | Size | Linker stamp |
|------|---------|------|--------------|
| `RobRunAIO.exe` | `cd3dfd2588ca0c132fe102194a999da90cc42243d86e93d0de5b75fc7ba7dbb8` | 3,008,000 | MSVC 14.51, PE32+ GUI |
| `FFLoader.exe` | `4478e022987799dbca23c2c3de61c880ce19fc92a07c19fdf5fe521309323173` | 1,200,128 | MSVC 14.51, PE32+ GUI |

`RobRunAIO.exe` is mostly **`.rsrc` (~2.7 MiB)**: embedded HTML menu, fonts, WebView2 loader DLL, and string tables. Code section ~149 KiB.

Manifest names:

- `RobRunAIO.Overlay` (overlay)
- `RobRunAIO.Launcher` (`requireAdministrator`)

## Kernel backend (user-mode API, not full driver source)

- Device: `\\.\WmPort` (also registry path `SYSTEM\WmPort`)
- APIs: `NtCreateFile`, `NtDeviceIoControlFile`
- Strings: `map kernel-mode.sys first`, NAL / `iqvw64e.pdb` (Intel vulnerable-driver style IOCTL shim compiled in)
- Session key string (hash only in binary): `ec3a09682c300747dc3029c4baca5728b01403b20e2193a802bf80dbb14166aa`
- Self-test messages: `rr_write`, batch read 4 KiB, attach / `driver write test`

**We do not ship `kernel-mode.sys` or a mapper** — only document the interface.

## Unity IL2CPP (from binary self-describe strings)

| Item | Value | Provenance |
|------|-------|------------|
| World static | `GameAssembly.dll + 0x81B7148` | exe strings |
| Chain | `world + 0x38` → WorldEntitiesModel, `+ 0x18` → `_battleCharacters` | exe strings |
| BattleCharacter transform | `+0x58` Transform, `+0x60` ReplicatedTransform | exe strings |
| Il2CppClass.static_fields | offset `0xB8` (metadata 39) | exe strings |
| Dictionary layout | `_entries` / `_count` (runtime `prim`/`dict` from `--probe`) | exe strings |
| Camera chain (chain file) | `camera = world+0x38 +0x18` | exe strings |

Static RVAs also referenced in probe output format: `static %-22s rva=%07X`.

## Overlay / render

- Brand strings: `RobRunESP`, `Felonious Funnery — Rob Run`
- GPU: `D3D11/DComp` (Desktop Composition)
- `status.log` format: `%lu pid=%lu n=%d inst=%d world=%llX snap=%lums hz=%d status=[%s]`
- Config: `CoreRR.cfg` (WebView `set:` / `state:` / `telem:` bridge)
- Hotkeys: **F1** menu, **INSERT** overlay, **END** exit

## CLI tools (same binary, flag dispatch)

```
--menu --probe --gomdiag --chain --gomlist --objlist --offsets
--chamdiag --chamtrace --selftest --inputtest --writetest
--nodescan --classscan --lootdump --lootset --itemset
--cashscan --cashfind --ctxread --aimtest --xfprobe --demo
--nohide --no-pause --exe <path> --pid <n>
```

Log files: `probe.log`, `offsets.log`, `chain.log`, `aimtest.log`, `selftest.log`, …

## WebView menu

- Full HTML extracted to `analysis/dump/menu.html` (same as `client/resources/menu.html`)
- Embedded `WebView2Loader` PE at file offset `0x2B5130` (~155 KiB) → `analysis/dump/WebView2Loader.dll`

## FFLoader

- GDI+ UI (`Gdiplus*`, custom drawing)
- `ShellExecuteExW`, `CreateFileW`
- No `WmPort` strings — launcher role only (+ deploy/masquerade per ini)

## Embedded PE offsets (RobRunAIO.exe)

| File offset | Approx size | Notes |
|-------------|-------------|--------|
| `0x0` | main image | |
| `0x36730` | 223 KiB | resource bundle |
| `0x71730` | 2.4 MiB | large rsrc blob (includes HTML) |
| `0x2B5130` | 155 KiB | WebView2Loader |

## Rebuild policy (this repo)

`robrun/` is a **clean-room** recreation of the architecture above:

- Same chain constants and config keys
- WebView menu asset from dump
- `WmPort` client stub + optional `RobRunMem` clean driver
- CLI entry points stubbed to match flags (full RE of every diagnostic not possible from strings alone)

Missing without your live game: `prim offset` / `dict offset`, camera matrix fields, `aa_aim_cal` from `--aimtest`.