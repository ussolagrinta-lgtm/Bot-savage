#include "savage/config/game_profile.hpp"

namespace savage::config {

const GameProfile& GameProfile::active() {
  static const GameProfile profile{};
  return profile;
}

}  // namespace savage::config