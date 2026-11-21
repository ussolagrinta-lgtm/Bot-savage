#pragma once

// Constants extracted from RobRunAIO.exe static dump — see analysis/DUMP.md

#include <cstdint>

namespace robrun::dump {

inline constexpr wchar_t kDeviceWmPort[] = L"\\\\.\\WmPort";
inline constexpr wchar_t kProcessGame[] = L"ROB RUN.exe";
inline constexpr wchar_t kModuleIl2Cpp[] = L"GameAssembly.dll";

inline constexpr uint64_t kWorldRootRva = 0x81B7148;
inline constexpr uint32_t kWorldToWorldEntities = 0x38;
inline constexpr uint32_t kWorldEntitiesToBattleList = 0x18;
inline constexpr uint32_t kBattleCharacterTransform = 0x58;
inline constexpr uint32_t kBattleCharacterReplicatedTransform = 0x60;
inline constexpr uint32_t kIl2CppClassStaticFields = 0xB8;

// SHA-256 of uploaded RobRunAIO.exe (reference build).
inline constexpr char kReferenceExeSha256[] =
    "cd3dfd2588ca0c132fe102194a999da90cc42243d86e93d0de5b75fc7ba7dbb8";

} // namespace robrun::dump