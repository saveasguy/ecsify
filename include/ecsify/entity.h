#ifndef ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
#define ECSIFY_INCLUDE_ECSIFY_ENTITY_H_

#include <cstdint>

namespace ecsify {

struct Entity final {
  const std::uint64_t kID;
  const std::uint32_t kHandle;

 private:
  static Entity create_(std::uint32_t handle);
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
