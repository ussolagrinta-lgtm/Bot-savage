#include "game_state.hpp"

#include <cstring>

namespace rob {

GameStateReader::GameStateReader(DriverIo* io) : io_(io) {}

void GameStateReader::SetGameAssemblyBase(uint64_t base, uint64_t size) {
    ga_base_ = base;
    ga_size_ = size;
}

template <typename T>
std::optional<T> GameStateReader::ReadValue(uint64_t address) {
    auto bytes = io_->Read(address, sizeof(T));
    if (!bytes || bytes->size() < sizeof(T))
        return std::nullopt;

    T value{};
    std::memcpy(&value, bytes->data(), sizeof(T));
    return value;
}

bool GameStateReader::ReadPointerChain(uint64_t baseRva, std::vector<uint64_t>& outInstance) {
    outInstance.clear();
    if (ga_base_ == 0 || baseRva == 0)
        return false;

    uint64_t slot = ga_base_ + baseRva;
    if (slot < ga_base_ || slot >= ga_base_ + ga_size_)
        return false;

    auto klassStatic = ReadValue<uint64_t>(slot);
    if (!klassStatic || *klassStatic < 0x10000)
        return false;

    outInstance.push_back(*klassStatic);
    return true;
}

bool GameStateReader::HarvestEntities(uint64_t worldEntitiesInstance) {
    entities_.clear();

    if (worldEntitiesInstance == 0) {
        last_error_ = "WorldEntities instance is null; layout UNKNOWN";
        return false;
    }

    // Il2CPP List<T> / array layout is build-specific — gate until user supplies offsets.
    last_error_ = "Entity list traversal not configured (need Il2CppDumper + ReClass)";
    return false;
}

bool GameStateReader::Refresh() {
    entities_.clear();
    camera_.valid = false;
    last_error_.clear();

    if (!io_ || ga_base_ == 0) {
        last_error_ = "GameAssembly base not set";
        return false;
    }

    const auto& cam = statics_.BattleCameraModel;
    const auto& worldEnt = statics_.WorldEntitiesModel;

    if (cam.value == 0 || worldEnt.value == 0) {
        last_error_ = "Static RVAs unset";
        return false;
    }

    if (!game::IsTrusted(cam) || !game::IsTrusted(worldEnt))
        last_error_ = "WARNING: RVAs unverified on this build";

    std::vector<uint64_t> camInst;
    if (!ReadPointerChain(cam.value, camInst)) {
        last_error_ = "Failed to resolve BattleCameraModel static";
        return false;
    }

    std::vector<uint64_t> worldInst;
    if (!ReadPointerChain(worldEnt.value, worldInst)) {
        last_error_ = "Failed to resolve WorldEntitiesModel static";
        return false;
    }

    HarvestEntities(worldInst.empty() ? 0 : worldInst[0]);

    // Camera field offsets UNKNOWN — placeholder fail.
    last_error_ = "Camera matrix offsets UNKNOWN";
    return false;
}

} // namespace rob