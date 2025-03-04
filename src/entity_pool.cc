#include <ecsify/internal/entity_pool.h>

#include <algorithm>
#include <utility>

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
      next_entity_id_{0},
      kNumComponentTypes{num_component_types} {}

Entity EntityPool::Add() {
  std::size_t handle = entities_.Insert(MakeDummyEntityData());
  entities_[handle].id(next_entity_id_);
  return Entity{next_entity_id_++, handle};
}

void EntityPool::Remove(Entity entity) { entities_.Erase(entity.kHandle); }

bool EntityPool::Alive(Entity entity) const noexcept {
  const EntityData *data = entities_.At(entity.kHandle);
  return data && data->id() == entity.kID;
}

void EntityPool::Link(Entity entity, std::size_t component_type) noexcept {
  if (!Alive(entity)) {
    return;
  }
  entities_[entity.kHandle].Link(component_type);
}

void EntityPool::Unlink(Entity entity, std::size_t component_type) noexcept {
  if (!Alive(entity)) {
    return;
  }
  entities_[entity.kHandle].Unlink(component_type);
}

bool EntityPool::Has(Entity entity, std::size_t component_type) const noexcept {
  const EntityData *data = entities_.At(entity.kHandle);
  return data && data->Has(component_type);
}

EntityData MakeDummyEntityData() { return EntityData{EntityData::DummyTag{}}; }

EntityData::EntityData() : id_{-1}, is_dummy_{true} {}

EntityData::EntityData(EntityData::DummyTag /* unused */)
    : id_{-1}, is_dummy_{true} {}

EntityData::EntityData(std::size_t num_component_types)
    : component_types_(num_component_types, false), id_{-1}, is_dummy_{false} {}

EntityData &EntityData::operator=(const EntityData &other) {
  if (other.is_dummy_) {
    return *this;
  }
  EntityData copy = other;
  std::swap(*this, copy);
  return *this;
}

EntityData &EntityData::operator=(EntityData &&other) noexcept {
  if (other.is_dummy_ || this == &other) {
    return *this;
  }
  component_types_ = std::move(other.component_types_);
  id_ = other.id_;
  is_dummy_ = other.is_dummy_;
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

void EntityData::UnlinkAll() noexcept {
  std::fill(component_types_.begin(), component_types_.end(), false);
}

bool EntityData::Has(std::size_t component_type) const noexcept {
  assert(component_type < component_types_.size() && "Unknown component type");
  return component_types_[component_type];
}

}  // namespace ecsify::internal
