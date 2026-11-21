# RobRunMem kernel driver

Minimal IOCTL driver for attach + cross-process read/write + `GameAssembly.dll` base via PEB walk.

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 + **Windows Driver Kit (WDK)** matching your SDK
- Test signing **or** a properly code-signed certificate (EAC will block known vulnerable drivers)

## Build (Visual Studio Developer Command Prompt)

1. Create a **Kernel Mode Driver, Empty (KMDF)** project or point the WDK build at `driver/src/driver.c`.
2. Copy `shared/communication.h` into the include path.
3. Build `x64` / `Release` to produce `RobRunMem.sys`.

## Load (lab machine only)

```text
sc create RobRunMem type= kernel binPath= "C:\full\path\RobRunMem.sys"
sc start RobRunMem
```

Unload:

```text
sc stop RobRunMem
sc delete RobRunMem
```

## IOCTL surface

See `shared/communication.h`:

- `IOCTL_ROB_RUN_ATTACH` — bind to game PID (`ROB RUN.exe`)
- `IOCTL_ROB_RUN_READ` / `IOCTL_ROB_RUN_WRITE` — virtual memory copy
- `IOCTL_ROB_RUN_GET_MODULE` — resolve module base (e.g. `GameAssembly.dll`)

This repo does **not** ship EAC bypass logic. You are responsible for how the driver is loaded and signed.