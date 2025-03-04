#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "ecsify/entity.h"
#include "ecsify/internal/data_pool.h"

namespace ecsify::internal {

class EntityData final {
 public:
  EntityData();
  explicit EntityData(std::size_t num_component_types);
  EntityData(const EntityData &other) = default;
  EntityData(EntityData &&other) noexcept = default;
  EntityData &operator=(const EntityData &other);
  EntityData &operator=(EntityData &&other) noexcept;
  ~EntityData() = default;

  void Link(std::size_t component_type) noexcept;
  void Unlink(std::size_t component_type) noexcept;
  void UnlinkAll() noexcept;
  bool Has(std::size_t component_type) const noexcept;

  std::int64_t id() const noexcept { return id_; }
  void id(std::int64_t new_id) noexcept { id_ = new_id; }

 private:
  friend EntityData MakeDummyEntityData();

  struct DummyTag {};

  // Constructs dummy entity data, which is ignored when copied/moved.
  explicit EntityData(DummyTag);

  std::vector<bool> component_types_;
  std::int64_t id_;
  bool is_dummy_;
};

EntityData MakeDummyEntityData();

class EntityPool {
 public:
  /**
   * @brief Constructs an EntityPool.
   *
   * @param num_component_types The number of different component types
   * that entities should handle.
   */
  explicit EntityPool(std::size_t num_component_types);

  Entity Add();
  void Remove(Entity entity);
  bool Alive(Entity entity) const noexcept;

  void Link(Entity entity, std::size_t component_type) noexcept;
  void Unlink(Entity entity, std::size_t component_type) noexcept;
  bool Has(Entity entity, std::size_t component_type) const noexcept;

 private:
  internal::DataPool<EntityData> entities_;
  std::int64_t next_entity_id_;
  const std::size_t kNumComponentTypes;
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_ENTITY_POOL_H_
