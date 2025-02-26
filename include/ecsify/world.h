#ifndef ECSIFY_INCLUDE_ECSIFY_WORLD_H_
#define ECSIFY_INCLUDE_ECSIFY_WORLD_H_

#include <cstddef>

#include "internal/utils.h"

namespace ecsify {

template <class... Components>
class ComponentsxEntities {
 public:
  constexpr static std::size_t kNumComponentTypes = sizeof...(Components);
};

template <class... Ts>
struct Components {
  template <class... MoreTs>
  using x = Components<Ts..., MoreTs...>;

  using xEntities = ComponentsxEntities<Ts...>;
};

template <class T>
concept ComponentsxEntitiesSpecialization =
    internal::is_template_specialization_v<T, ComponentsxEntities>;

template <class T>
concept ComponentsxEntitiesLike = ComponentsxEntitiesSpecialization<T> ||
    internal::is_base_of_template_v<ComponentsxEntities, std::decay_t<T>>;

template <ComponentsxEntitiesSpecialization CxE>
class World : public CxE {};

template <ComponentsxEntitiesLike CxE>
constexpr std::size_t numComponentTypes(CxE&& /* unused */) {
  return std::decay_t<CxE>::kNumComponentTypes;
}

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_WORLD_H_
