#ifndef ECSIFY_INCLUDE_ECSIFY_WORLD_H_
#define ECSIFY_INCLUDE_ECSIFY_WORLD_H_

#include <array>
#include <cstddef>
#include <ranges>
#include <span>

#include "ecsify/component.h"
#include "ecsify/entity.h"

namespace ecsify {

class World {
 public:
  // Create new entity.
  virtual Entity Add() = 0;
  // Remove the entity.
  virtual void Remove(Entity entity) = 0;
  // Check if entity is alive.
  virtual bool Alive(Entity entity) const = 0;

  // Add component to the entity.
  template <class Component>
    requires(!std::is_same_v<Component, Entity>)
  void Add(Entity entity) {
    return Add(entity, Component::TypeID());
  }

  template <class Component>
    requires(!std::is_same_v<Component, Entity>)
  Component &Get(Entity entity) {
    return static_cast<Component &>(Get(entity, Component::TypeID()));
  }

  template <class Component>
  const Component &Get(Entity entity) const {
    return static_cast<const Component &>(Get(entity, Component::TypeID()));
  }

  // Check if an entity has the component.
  template <class Component>
  bool Has(Entity entity) {
    return Has(entity, Component::TypeID());
  }

  template <class Component>
    requires(!std::is_same_v<Component, Entity>)
  void Remove(Entity entity) {
    Remove(entity, Component::TypeID());
  }

  template <class... Components>
  auto Query() {
    std::array<std::size_t, sizeof...(Components)> component_ids = {
        Components::TypeID()...};
    return std::views::zip(CastQuery<Components>(
        QueryOne(Components::TypeID(), component_ids))...);
  }

  virtual void Update() = 0;

  virtual ~World() = default;

 protected:
  virtual void Add(Entity entity, std::size_t component_type) = 0;
  virtual void Remove(Entity entity, std::size_t component_type) = 0;
  virtual bool Has(Entity entity, std::size_t component_type) const = 0;
  virtual internal::ComponentBase &Get(Entity entity,
                                       std::size_t component_type) = 0;
  virtual const internal::ComponentBase &Get(
      Entity entity, std::size_t component_type) const = 0;

  virtual std::span<internal::ComponentBase *> QueryOne(
      std::size_t component_type, std::span<std::size_t> component_ids) = 0;

 private:
  template <class Component>
  static auto CastQuery(std::span<internal::ComponentBase *> query) {
    return query | std::views::transform(
                       [](internal::ComponentBase *component) -> Component & {
                         return *static_cast<Component *>(component);
                       });
  }
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_WORLD_H_
