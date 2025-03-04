#ifndef ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
#define ECSIFY_INCLUDE_ECSIFY_ENTITY_H_

#include <cstdint>
#include <type_traits>
#include <vector>

#include "internal/data_pool.h"

namespace ecsify {

struct Entity final {
  const std::int64_t kID;
  const std::size_t kHandle;
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
