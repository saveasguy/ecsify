#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_UTILS_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_UTILS_H_

#include <type_traits>

namespace ecsify::internal {

template <class Spec, template <class...> class Tmpl>
struct IsTemplateSpecialization : std::false_type {};

template <template <class...> class Tmpl, class... Args>
struct IsTemplateSpecialization<Tmpl<Args...>, Tmpl> : std::true_type {};

template <class Spec, template <class...> class Tmpl>
constexpr bool is_template_specialization_v =
    IsTemplateSpecialization<Spec, Tmpl>::value;

template <template <class...> class Base, class Derived>
struct IsBaseOfTemplate {
  template <class... Ts>
  static constexpr std::true_type test(const Base<Ts...> *);
  static constexpr std::false_type test(...);
};

template <template <class...> class Base, class Derived>
constexpr bool is_base_of_template_v =
    decltype(IsBaseOfTemplate<Base, Derived>::test(
        std::declval<Derived *>()))::value;

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_UTILS_H_
