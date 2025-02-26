#include <ecsify/entity.h>

namespace ecsify {

Entity Entity::create_(std::uint32_t handle) {
  static std::uint64_t next_id = 0;
  return Entity{.kID = next_id, .kHandle = handle};
}

}  // namespace ecsify
