#ifndef ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
#define ECSIFY_INCLUDE_ECSIFY_ENTITY_H_

#include <cstddef>
#include <cstdint>

namespace ecsify {

class Entity final {
 public:
  Entity(std::int64_t unique_id, std::size_t entity_handle)
      : id_{unique_id}, handle_{entity_handle} {}

  std::int64_t id() const noexcept { return id_; }
  std::size_t handle() const noexcept { return handle_; }

 private:
  std::int64_t id_;
  std::size_t handle_;
};

}  // namespace ecsify

#endif  // ECSIFY_INCLUDE_ECSIFY_ENTITY_H_
