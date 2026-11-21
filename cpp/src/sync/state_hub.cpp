#include "savage/sync/state_hub.hpp"

namespace savage::sync {

StateHub& StateHub::instance() {
  static StateHub hub;
  return hub;
}

}  // namespace savage::sync