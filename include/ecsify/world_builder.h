#ifndef ECSIFY_INCLUDE_ECSIFY_WORLD_BUILDER_H_
#define ECSIFY_INCLUDE_ECSIFY_WORLD_BUILDER_H_

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "ecsify/entity.h"
#include "ecsify/internal/component_pool.h"
#include "ecsify/internal/world_impl.h"

namespace ecsify {

namespace internal {

template <class... Components>
auto GetComponentIDSequence() {
  return std::integer_sequence<std::size_t, Components::TypeID()...>{};
}

template <class Seq1, class Seq2>
struct SequencesEqual : std::false_type {};

template <std::size_t... Ints>
struct SequencesEqual<std::integer_sequence<std::size_t, Ints...>,
                      std::integer_sequence<std::size_t, Ints...>>
    : std::true_type {};

template <class... Components>
constexpr bool sequential_components_v =
    SequencesEqual<decltype(GetComponentIDSequence<Components...>()),
                   std::index_sequence_for<Components...>>::value;

template <class... Components>
auto MakeComponentPools() -> std::array<ComponentPoolRef<sizeof...(Components)>,
                                        sizeof...(Components)> {
  return {
      std::make_unique<ComponentPool<Components, sizeof...(Components)>>()...};
}

}  // namespace internal

class World;

template <class... Components>
  requires(internal::sequential_components_v<Entity, Components...>)
class WorldBuilder final {
 public:
  WorldBuilder() {}
  explicit WorldBuilder(std::vector<internal::SystemFunctionType> systems)
      : systems_{std::move(systems)} {}

  template <class T>
  WorldBuilder<Components..., T> Component() noexcept {
    return WorldBuilder<Components..., T>{std::move(systems_)};
  }

  template <class T>
  WorldBuilder &System(T &&system) {
    systems_.emplace_back(std::forward<T>(system));
    return *this;
  }

  std::unique_ptr<World> Build() {
    return std::make_unique<internal::WorldImpl<1 + sizeof...(Components)>>(
        internal::MakeComponentPools<Entity, Components...>(),
        std::move(systems_));
  }

 private:
  std::vector<internal::SystemFunctionType> systems_;
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_WORLD_BUILDER_H_
