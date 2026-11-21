#pragma once

#include "driver_io.hpp"
#include "math_util.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rob {

struct BattlePlayer {
    uint64_t object = 0;
    Vec3 position{};
    bool valid = false;
};

class WorldChain {
public:
    explicit WorldChain(DriverIo* io);

    void SetGameAssembly(uint64_t base, uint64_t size);

    std::optional<uint64_t> ResolveWorldModel();
    std::optional<uint64_t> ResolveWorldEntities(uint64_t worldModel);
    std::optional<uint64_t> ResolveBattleList(uint64_t worldEntities);

    bool RefreshPlayers();

    const std::vector<BattlePlayer>& Players() const { return players_; }
    std::string_view LastError() const { return last_error_; }

private:
    template <typename T>
    std::optional<T> Read(uint64_t address);

    DriverIo* io_ = nullptr;
    uint64_t ga_base_ = 0;
    uint64_t ga_size_ = 0;
    std::vector<BattlePlayer> players_;
    std::string last_error_;
};

} // namespace rob