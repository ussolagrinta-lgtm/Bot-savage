#include "world_chain.hpp"

#include "il2cpp_layout.hpp"

#include <cstring>

namespace rob {

WorldChain::WorldChain(DriverIo* io) : io_(io) {}

void WorldChain::SetGameAssembly(uint64_t base, uint64_t size) {
    ga_base_ = base;
    ga_size_ = size;
}

template <typename T>
std::optional<T> WorldChain::Read(uint64_t address) {
    auto bytes = io_->Read(address, sizeof(T));
    if (!bytes || bytes->size() < sizeof(T))
        return std::nullopt;
    T v{};
    std::memcpy(&v, bytes->data(), sizeof(T));
    return v;
}

std::optional<uint64_t> WorldChain::ResolveWorldModel() {
    using namespace il2cpp;
    if (!ga_base_)
        return std::nullopt;

    uint64_t slot = ga_base_ + AioWorldChain::kWorldRootRva;
    return Read<uint64_t>(slot);
}

std::optional<uint64_t> WorldChain::ResolveWorldEntities(uint64_t worldModel) {
    if (!worldModel)
        return std::nullopt;
    return Read<uint64_t>(worldModel + il2cpp::AioWorldChain::kWorldToWorldEntities);
}

std::optional<uint64_t> WorldChain::ResolveBattleList(uint64_t worldEntities) {
    if (!worldEntities)
        return std::nullopt;
    return Read<uint64_t>(worldEntities + il2cpp::AioWorldChain::kWorldEntitiesToBattleList);
}

bool WorldChain::RefreshPlayers() {
    players_.clear();
    last_error_.clear();

    auto world = ResolveWorldModel();
    if (!world || *world < 0x10000) {
        last_error_ = "no world";
        return false;
    }

    auto worldEntities = ResolveWorldEntities(*world);
    if (!worldEntities || *worldEntities < 0x10000) {
        last_error_ = "WorldEntitiesModel missing";
        return false;
    }

    auto battleList = ResolveBattleList(*worldEntities);
    if (!battleList || *battleList < 0x10000) {
        last_error_ = "_battleCharacters missing";
        return false;
    }

    // Il2CPP List/Dictionary element walk is build-specific — need prim/dict offsets from probe.
    last_error_ = "battle list container layout UNKNOWN (run AIO --probe or Il2CppDumper)";
    return false;
}

} // namespace rob