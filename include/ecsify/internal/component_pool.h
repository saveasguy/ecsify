#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_

#include <cstddef>
#include <type_traits>

#include "ecsify/component.h"

namespace ecsify::internal {

template <class T>
struct DerivedFromComponentMixin {
 private:
  template <std::size_t N>
  static std::is_base_of<ecsify::ComponentMixin<N>, T> Test(
      const ecsify::ComponentMixin<N> &);

  static std::false_type Test(...);

 public:
  static constexpr bool value = decltype(Test(std::declval<T>()))::value;
};

template <class T>
constexpr inline bool derived_from_component_mixin_v =
    DerivedFromComponentMixin<T>::value;

struct ComponentPoolBase {
  virtual ~ComponentPoolBase() = default;
};

template <class T> requires(derived_from_component_mixin_v<T>)
struct ComponentPoolTemplateBase {
    virtual 
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_COMPONENT_POOL_H_
