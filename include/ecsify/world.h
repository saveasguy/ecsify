#ifndef ECSIFY_INCLUDE_ECSIFY_WORLD_H_
#define ECSIFY_INCLUDE_ECSIFY_WORLD_H_

#include <cstddef>

namespace ecsify {

template <class... Components>
class ComponentsEntities {
 public:
  static constexpr std::size_t kNumSupportedComponents = sizeof...(Components);
};

template <class... Components>
class World : public ComponentsEntities<Components...> {
 public:
  using CE = ComponentsEntities<Components...>;
};

struct MakeWorld final {
  template <class... Components>
  using WithComponents = World<Components...>;
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_WORLD_H_
