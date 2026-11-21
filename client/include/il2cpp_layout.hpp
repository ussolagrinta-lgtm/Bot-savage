#pragma once

#include <cstdint>
#include <optional>

namespace rob::il2cpp {

// Field layout provenance: strings extracted from user-provided RobRunAIO.exe (Nov 2026).
// Still validate on your live GameAssembly.dll before trusting in match.

struct AioWorldChain {
    static constexpr uint64_t kWorldRootRva = 0x81B7148; // GameAssembly.dll + RVA -> WorldModel*
    static constexpr uint32_t kWorldToWorldEntities = 0x38;
    static constexpr uint32_t kWorldEntitiesToBattleList = 0x18; // _battleCharacters
};

struct BattleCharacterFields {
    static constexpr uint32_t kTransform = 0x58;
    static constexpr uint32_t kReplicatedTransform = 0x60;
};

struct DictionaryLayout {
    // Filled when you run the binary's --probe and paste prim/dict offsets.
    std::optional<uint32_t> entries_offset{std::nullopt};
    std::optional<uint32_t> count_offset{std::nullopt};
};

} // namespace rob::il2cpp