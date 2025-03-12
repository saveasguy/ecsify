#ifndef ECSIFY_INCLUDE_ECSIFY_COMPONENT_H_
#define ECSIFY_INCLUDE_ECSIFY_COMPONENT_H_

#include <cstddef>

namespace ecsify {

template <std::size_t kTypeID>
struct ComponentMixin {
  static consteval std::size_t TypeID() { return kTypeID; }
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_COMPONENT_H_
