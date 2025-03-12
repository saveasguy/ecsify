#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "ecsify/entity.h"
#include "ecsify/internal/archetype.h"
#include "ecsify/internal/data_pool.h"

namespace ecsify::internal {

template <std::size_t N>
class EntityData final {
 public:
  EntityData() : archetype_{}, id_{-1} {}
  explicit EntityData(std::int64_t unique_id) : archetype_{}, id_{unique_id} {}

  bool Has(std::size_t component_type) const noexcept {
    assert(component_type < archetype_.Size() && "Unknown component type");
    return archetype_.At(component_type);
  }

  void Link(std::size_t component_type) noexcept {
    assert(component_type < archetype_.Size() && "Unknown component type");
    return archetype_.Set(component_type);
  }

  void Unlink(std::size_t component_type) noexcept {
    assert(component_type < archetype_.Size() && "Unknown component type");
    return archetype_.Unset(component_type);
  }

  std::int64_t id() const noexcept { return id_; }

  std::size_t component_handle() const noexcept { return component_handle_; }

  void component_handle(std::size_t new_component_handle) noexcept {
    component_handle_ = new_component_handle;
  }

  const Archetype<N> &archetype() const noexcept { return archetype_; }

 private:
  Archetype<N> archetype_;
  std::int64_t id_;
  std::size_t component_handle_;
};

template <std::size_t N>
class EntityPool {
 public:
  Entity Add() {
    std::int64_t unique_id = next_entity_id_++;
    std::size_t handle = entities_.Insert();
    entities_[handle] = EntityData<N>(unique_id);
    return Entity{unique_id, handle};
  }

  void Remove(Entity entity) { entities_.Erase(entity.handle()); }

  const EntityData<N> &operator[](Entity entity) const {
    return entities_[entity.handle()];
  }

  EntityData<N> &operator[](Entity entity) {
    return entities_[entity.handle()];
  }

  bool Alive(Entity entity) const noexcept {
    if (!entities_.Contains(entity.handle())) {
      return false;
    }
    return entities_[entity.handle()].id() == entity.id();
  }

 private:
  internal::DataPool<EntityData<N>> entities_{};
  std::int64_t next_entity_id_{0};
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_
