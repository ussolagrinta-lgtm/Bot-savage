#pragma once

#include "driver_io.hpp"
#include "game_config.hpp"
#include "math_util.hpp"

#include <vector>

namespace rob {

struct EntitySnapshot {
    uint64_t address = 0;
    Vec3 position{};
    float health = -1.f;
    int teamId = -1;
    bool valid = false;
};

struct CameraSnapshot {
  Matrix4x4 view{};
  Matrix4x4 projection{};
  Vec3 position{};
  Vec3 forward{0.f, 0.f, 1.f};
  bool valid = false;
};

class GameStateReader {
public:
    explicit GameStateReader(DriverIo* io);

    void SetGameAssemblyBase(uint64_t base, uint64_t size);
    void UseStatics(const game::Il2CppStatics& s) { statics_ = s; }

    bool Refresh();

    const CameraSnapshot& Camera() const { return camera_; }
    const std::vector<EntitySnapshot>& Entities() const { return entities_; }

    std::string_view LastError() const { return last_error_; }

private:
    template <typename T>
    std::optional<T> ReadValue(uint64_t address);

    bool ReadPointerChain(uint64_t baseRva, std::vector<uint64_t>& outInstance);
    bool HarvestEntities(uint64_t worldEntitiesInstance);

    DriverIo* io_ = nullptr;
    uint64_t ga_base_ = 0;
    uint64_t ga_size_ = 0;
    game::Il2CppStatics statics_{game::DefaultStatics()};
    CameraSnapshot camera_{};
    std::vector<EntitySnapshot> entities_{};
    std::string last_error_;
};

} // namespace rob