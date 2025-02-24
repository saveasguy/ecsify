#ifndef ECSIFY_INCLUDE_ECSIFY_WORLD_H_
#define ECSIFY_INCLUDE_ECSIFY_WORLD_H_

#include <cstddef>

namespace ecsify {

template <class... Components>
class World final {
 public:
  template <class... WithComponents>
  using With = World<Components..., WithComponents...>;
};

template <>
class World<> final {
 public:
  template <class... WithComponents>
  using With = World<WithComponents...>;
};

template <class... Components>
constexpr std::size_t numSupportedComponents(
    const World<Components...>& /*unused*/) {
  return sizeof...(Components);
}

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_WORLD_H_
