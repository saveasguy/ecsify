#ifndef ECSIFY_INCLUDE_ECSIFY_COMPONENT_H_
#define ECSIFY_INCLUDE_ECSIFY_COMPONENT_H_

#include <cstddef>

namespace ecsify {

namespace internal {

struct ComponentBase {};

}  // namespace internal

template <std::size_t kTypeID>
struct ComponentMixin : public internal::ComponentBase {
  static consteval std::size_t TypeID() { return kTypeID; }
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_COMPONENT_H_
