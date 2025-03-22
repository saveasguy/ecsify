#ifndef ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
#define ECSIFY_INCLUDE_ECSIFY_ENTITY_H_

#include <cstddef>
#include <cstdint>

#include "ecsify/component.h"

namespace ecsify {

class Entity final : public ComponentMixin<0> {
 public:
  Entity() : id_{-1}, handle_{0} {}

  Entity(std::int64_t unique_id, std::size_t entity_handle)
      : id_{unique_id}, handle_{entity_handle} {}

  std::int64_t id() const noexcept { return id_; }
  std::size_t handle() const noexcept { return handle_; }

  friend bool operator==(const Entity &lhs, const Entity &rhs);

 private:
  std::int64_t id_;
  std::size_t handle_;
};

inline bool operator==(const Entity &lhs, const Entity &rhs) {
  return lhs.id_ == rhs.id_;
}

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
