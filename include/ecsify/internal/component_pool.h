#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_

#include <cstddef>
#include <memory>
#include <unordered_map>

#include "ecsify/component.h"
#include "ecsify/internal/archetype.h"
#include "ecsify/internal/data_pool.h"

namespace ecsify::internal {

template <std::size_t N>
struct ComponentPoolBase {
  virtual ~ComponentPoolBase() = default;

  virtual const ComponentBase &Get(const Archetype<N> &archetype,
                                   std::size_t handle) const = 0;
  virtual ComponentBase &Get(const Archetype<N> &archetype,
                             std::size_t handle) = 0;
  virtual std::size_t Add(const Archetype<N> &archetype) = 0;
  virtual void Remove(const Archetype<N> &old_archetype,
                      std::size_t handle) = 0;
  virtual std::size_t Move(const Archetype<N> &old_archetype,
                           std::size_t handle,
                           const Archetype<N> &new_archetype) = 0;
};

template <std::size_t N>
using ComponentPoolRef = std::unique_ptr<ComponentPoolBase<N>>;

template <class T, std::size_t N>
class ComponentPool final : public ComponentPoolBase<N> {
 public:
  T &Get(const Archetype<N> &archetype, std::size_t handle) override {
    return components_[archetype][handle];
  }

  const T &Get(const Archetype<N> &archetype,
               std::size_t handle) const override {
    return components_.at(archetype)[handle];
  }

  std::size_t Add(const Archetype<N> &archetype) override {
    return components_[archetype].Insert();
  }

  void Remove(const Archetype<N> &archetype, std::size_t handle) override {
    if (!components_.contains(archetype)) {
      return;
    }
    components_[archetype].Erase(handle);
  }

  std::size_t Move(const Archetype<N> &old_archetype, std::size_t handle,
                   const Archetype<N> &new_archetype) override {
    T tmp = std::move(components_[old_archetype][handle]);
    std::size_t new_handle = components_[new_archetype].Insert();
    components_[new_archetype][new_handle] = std::move(tmp);
    return new_handle;
  }

 private:
  std::unordered_map<Archetype<N>, DataPool<T>> components_;
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_
