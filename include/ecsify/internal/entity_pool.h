#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "ecsify/entity.h"
#include "ecsify/internal/data_pool.h"

namespace ecsify::internal {

template <std::size_t N>
class EntityData final {
 public:
  EntityData() : archetype_{}, id_{-1} {}
  explicit EntityData(std::int64_t unique_id) : archetype_{}, id_{unique_id} {}

  void Link(std::size_t component_type) noexcept {
    assert(component_type < archetype_.size() && "Unknown component type");
    archetype_[component_type] = true;
  }

  void Unlink(std::size_t component_type) noexcept {
    assert(component_type < archetype_.size() && "Unknown component type");
    archetype_[component_type] = false;
  }

  bool Has(std::size_t component_type) const noexcept {
    assert(component_type < archetype_.size() && "Unknown component type");
    return archetype_[component_type];
  }

  std::int64_t id() const noexcept { return id_; }

 private:
  Archetype<N> archetype_;
  std::int64_t id_;
};

template <std::size_t N>
class EntityPool {
 public:
  Entity Add() {
    std::int64_t unique_id = next_entity_id_++;
    std::size_t handle = entities_.Insert(EntityData<N>(unique_id));
    return Entity{unique_id, handle};
  }

  void Remove(Entity entity) { entities_.Erase(entity.handle()); }

  bool Alive(Entity entity) const noexcept {
    if (!entities_.Contains(entity.handle())) {
      return false;
    }
    return entities_[entity.handle()].id() == entity.id();
  }

  void Link(Entity entity, std::size_t component_type) noexcept {
    if (!Alive(entity)) {
      return;
    }
    entities_[entity.handle()].Link(component_type);
  }

  void Unlink(Entity entity, std::size_t component_type) noexcept {
    if (!Alive(entity)) {
      return;
    }
    entities_[entity.handle()].Unlink(component_type);
  }

  bool Has(Entity entity, std::size_t component_type) const noexcept {
    return entities_[entity.handle()].Has(component_type);
  }

 private:
  internal::DataPool<EntityData<N>> entities_{};
  std::int64_t next_entity_id_{0};
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_
