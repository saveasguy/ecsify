#include <ecsify/internal/entity_pool.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "ecsify/entity.h"

namespace {

auto GetEntityDataInitializer(std::size_t num_component_types) {
  return [num_component_types]() {
    return ecsify::internal::EntityData(num_component_types);
  };
}

}  // namespace

namespace ecsify::internal {

EntityPool::EntityPool(std::size_t num_component_types)
    : entities_{GetEntityDataInitializer(num_component_types)},
      next_entity_id_{0} {}

Entity EntityPool::Add() {
  std::int64_t unique_id = next_entity_id_++;
  std::size_t handle = entities_.Insert(MakeNonAllocEntityData(unique_id));
  return Entity{unique_id, handle};
}

void EntityPool::Remove(Entity entity) { entities_.Erase(entity.handle()); }

bool EntityPool::Alive(Entity entity) const noexcept {
  if (!entities_.Contains(entity.handle())) {
    return false;
  }
  return entities_[entity.handle()].id() == entity.id();
}

void EntityPool::Link(Entity entity, std::size_t component_type) noexcept {
  if (!Alive(entity)) {
    return;
  }
  entities_[entity.handle()].Link(component_type);
}

void EntityPool::Unlink(Entity entity, std::size_t component_type) noexcept {
  if (!Alive(entity)) {
    return;
  }
  entities_[entity.handle()].Unlink(component_type);
}

bool EntityPool::Has(Entity entity, std::size_t component_type) const noexcept {
  return entities_[entity.handle()].Has(component_type);
}

EntityData MakeNonAllocEntityData(std::int64_t unique_id) {
  return EntityData{unique_id, EntityData::NonAllocTag{}};
}

EntityData::EntityData() : id_{-1}, is_non_alloc_{false} {}

EntityData::EntityData(std::int64_t unique_id,
                       EntityData::NonAllocTag /* unused */)
    : id_{unique_id}, is_non_alloc_{true} {}

EntityData::EntityData(std::size_t num_component_types)
    : component_types_(num_component_types, false),
      id_{-1},
      is_non_alloc_{false} {}

EntityData &EntityData::operator=(const EntityData &other) {
  if (this == &other) {
    return *this;
  }
  if (other.is_non_alloc_) {
    UnlinkAll();
    id_ = other.id_;
    return *this;
  }
  EntityData copy = other;
  std::swap(*this, copy);
  return *this;
}

EntityData &EntityData::operator=(EntityData &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (other.is_non_alloc_) {
    UnlinkAll();
    id_ = other.id_;
    return *this;
  }
  component_types_ = std::move(other.component_types_);
  id_ = other.id_;
  is_non_alloc_ = false;
  return *this;
}

void EntityData::Link(std::size_t component_type) noexcept {
  assert(component_type < component_types_.size() && "Unknown component type");
  component_types_[component_type] = true;
}

void EntityData::Unlink(std::size_t component_type) noexcept {
  assert(component_type < component_types_.size() && "Unknown component type");
  component_types_[component_type] = false;
}

bool EntityData::Has(std::size_t component_type) const noexcept {
  assert(component_type < component_types_.size() && "Unknown component type");
  return component_types_[component_type];
}

void EntityData::UnlinkAll() noexcept {
  std::ranges::fill(component_types_, false);
}

}  // namespace ecsify::internal
