#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_

#include <cstddef>
#include <memory>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

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
  virtual std::span<ComponentBase *> Query(const Archetype<N> &archetype) = 0;
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
    query_cache_.clear();
    return components_[archetype].Insert();
  }

  void Remove(const Archetype<N> &archetype, std::size_t handle) override {
    if (!components_.contains(archetype)) {
      return;
    }
    components_[archetype].Erase(handle);
    query_cache_.clear();
  }

  std::size_t Move(const Archetype<N> &old_archetype, std::size_t handle,
                   const Archetype<N> &new_archetype) override {
    T tmp = std::move(components_[old_archetype][handle]);
    std::size_t new_handle = components_[new_archetype].Insert();
    components_[new_archetype][new_handle] = std::move(tmp);
    query_cache_.clear();
    return new_handle;
  }

  std::span<ComponentBase *> Query(const Archetype<N> &archetype) override {
    if (!query_cache_.empty()) {
      return query_cache_;
    }
    for (auto &[key_archetype, pool] : components_) {
      if (archetype.IsPrefix(key_archetype)) {
        for (T &component : pool) {
          query_cache_.push_back(&component);
        }
      }
    }
    return std::span<ComponentBase *>{query_cache_.begin(), query_cache_.end()};
  }

 private:
  std::unordered_map<Archetype<N>, DataPool<T>> components_;
  mutable std::vector<ComponentBase *> query_cache_;
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_
