#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace rob::game {

// Target: ROB RUN (Steam 4262460), Unity IL2CPP, Easy Anti-Cheat.
// Every RVA below is UNVERIFIED unless you confirm it on your live build.

enum class Provenance : uint8_t {
    Unknown,
    UserConfirmed,
    MeasuredIl2CppDumper,
    PublicLeakUnverified,
};

struct StaticRva {
    uint64_t value{};
    Provenance provenance{Provenance::Unknown};
    std::string_view note;
};

struct Il2CppStatics {
    // Public paste / scanner output — NOT confirmed in this workspace.
    StaticRva WorldEntitiesModel{.value = 0x81D0710,
                                 .provenance = Provenance::PublicLeakUnverified,
                                 .note = "GameAssembly static field RVA"};
    StaticRva WorldModel{.value = 0x81D36C8,
                         .provenance = Provenance::PublicLeakUnverified,
                         .note = "GameAssembly static field RVA"};
    StaticRva BattleCameraModel{.value = 0x81CB420,
                                .provenance = Provenance::PublicLeakUnverified,
                                .note = "GameAssembly static field RVA"};
    StaticRva BattleCharacterModel{.value = 0x8196748,
                                   .provenance = Provenance::PublicLeakUnverified,
                                   .note = "GameAssembly static field RVA"};
};

struct EntityLayout {
    // Fill from ReClass after you resolve the managed type.
    std::optional<uint32_t> position_x{std::nullopt};
    std::optional<uint32_t> position_y{std::nullopt};
    std::optional<uint32_t> position_z{std::nullopt};
    std::optional<uint32_t> health{std::nullopt};
    std::optional<uint32_t> team_id{std::nullopt};
    std::optional<uint32_t> is_dead{std::nullopt};
};

struct ProcessInfo {
    static constexpr wchar_t kProcessName[] = L"ROB RUN.exe";
    static constexpr wchar_t kGameModule[] = L"GameAssembly.dll";
    static constexpr wchar_t kUnityModule[] = L"UnityPlayer.dll";
};

inline bool IsTrusted(const StaticRva& rva) {
    return rva.value != 0 && rva.provenance == Provenance::UserConfirmed;
}

inline const Il2CppStatics& DefaultStatics() {
    static Il2CppStatics s{};
    return s;
}

} // namespace rob::game