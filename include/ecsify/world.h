#ifndef ECSIFY_INCLUDE_ECSIFY_WORLD_H_
#define ECSIFY_INCLUDE_ECSIFY_WORLD_H_

#include <cstddef>

namespace ecsify {

template <class... Components>
class World final {
 public:
  template <class... WithComponents>
  using With = World<Components..., WithComponents...>;

  constexpr static std::size_t kNumComponentTypes = sizeof...(Components);
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_WORLD_H_
