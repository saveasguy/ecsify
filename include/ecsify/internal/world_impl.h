#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_WORLD_IMPL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_WORLD_IMPL_H_

#include <array>
#include <cstddef>
#include <memory>
#include <ranges>

#include "ecsify/entity.h"
#include "ecsify/internal/archetype.h"
#include "ecsify/internal/component_pool.h"
#include "ecsify/internal/entity_pool.h"
#include "ecsify/world.h"

namespace ecsify::internal {

template <std::size_t N>
class WorldImpl : public World {
 public:
  explicit WorldImpl(std::array<ComponentPoolRef<N>, N> pools)
      : components_{std::move(pools)} {}

 protected:
  Entity Add() override {
    Entity entity = entities_.Add();
    EntityData<N> &entity_data = entities_[entity];
    entity_data.Link(Entity::TypeID());
    std::size_t handle =
        components_[Entity::TypeID()]->Add(entity_data.archetype());
    entity_data.component_handle(handle);
    return entity;
  }

  void Remove(Entity entity) override {
    const Archetype<N> &archetype = entities_[entity].archetype();
    std::size_t handle = entities_[entity].component_handle();
    for (auto [has, pool] : std::views::zip(archetype, components_)) {
      if (has) {
        pool->Remove(archetype, handle);
      }
    }
    entities_.Remove(entity);
  }

  bool Alive(Entity entity) const override { return entities_.Alive(entity); }

  void Add(Entity entity, std::size_t component_type) override {
    EntityData<N> &entity_data = entities_[entity];
    std::size_t handle = entity_data.component_handle();
    if (entity_data.Has(component_type)) {
      return;
    }
    Archetype<N> old_archetype = entity_data.archetype();
    entity_data.Link(component_type);
    const Archetype<N> &archetype = entity_data.archetype();
    for (auto [has, pool] : std::views::zip(old_archetype, components_)) {
      if (has) {
        pool->Move(old_archetype, handle, archetype);
      }
    }
    std::size_t new_handle = components_[component_type]->Add(archetype);
    entity_data.component_handle(new_handle);
  }

  void Remove(Entity entity, std::size_t component_type) override {
    EntityData<N> &entity_data = entities_[entity];
    std::size_t handle = entity_data.component_handle();
    if (!entity_data.Has(component_type)) {
      return;
    }
    Archetype<N> old_archetype = entity_data.archetype();
    entity_data.Unlink(component_type);
    const Archetype<N> &archetype = entity_data.archetype();
    components_[component_type]->Remove(archetype,
                                        entity_data.component_handle());
    std::size_t new_handle;
    for (auto [has, pool] : std::views::zip(old_archetype, components_)) {
      if (has) {
        new_handle = pool->Move(old_archetype, handle, archetype);
      }
    }
    entity_data.component_handle(new_handle);
  }

  ComponentBase &Get(Entity entity, std::size_t component_type) override {
    EntityData<N> &entity_data = entities_[entity];
    return components_[component_type]->Get(entity_data.archetype(),
                                            entity_data.component_handle());
  }

  const ComponentBase &Get(Entity entity,
                           std::size_t component_type) const override {
    const EntityData<N> &entity_data = entities_[entity];
    return components_[component_type]->Get(entity_data.archetype(),
                                            entity_data.component_handle());
  }

  bool Has(Entity entity, std::size_t component_type) const override {
    if (!entities_.Alive(entity)) {
      return false;
    }
    return entities_[entity].Has(component_type);
  }

 private:
  EntityPool<N> entities_{};
  std::array<std::unique_ptr<ComponentPoolBase<N>>, N> components_;
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_WORLD_IMPL_H_
