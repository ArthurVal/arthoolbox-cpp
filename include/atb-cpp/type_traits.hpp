#pragma once

#include <concepts>
#include <type_traits>


#define ATB_FWD(v) std::forward<decltype(v)>(v)

namespace atb {

/// Always evaluates to false
template <typename...>
struct AlwaysFalse : std::false_type {};

template <typename... T>
constexpr bool AlwaysFalse_v = AlwaysFalse<T...>::value;

/// Always evaluates to true
template <typename...>
struct AlwaysTrue : std::true_type {};

template <typename... T>
constexpr bool AlwaysTrue_v = AlwaysTrue<T...>::value;

namespace details {

template <class, template <class...> class Trait, class... Args>
struct HasTraitImpl : std::false_type {};

template <template <class...> class Trait, class... Args>
struct HasTraitImpl<std::void_t<Trait<Args...>>, Trait, Args...>
    : std::true_type {};

}  // namespace details

/// Returns TRUE when the given type Trait<Args...> is a valid type (SFINAE)
///
/// Use it like this:
///
/// // Create a trait...
/// template <class ...T>
/// using FooFunction = decltype(Foo(std::declval<T>()...));
///
/// // ...Then use HasTrait:
/// static_assert(HasTrait_v<FooFunction, int, float, char>);
/// // -> True if `Foo(int, float, char)` exists
template <template <class...> class Trait, class... Args>
struct HasTrait : details::HasTraitImpl<void, Trait, Args...> {};

template <template <class...> class Trait, class... Args>
constexpr bool HasTrait_v = HasTrait<Trait, Args...>::value;


/// Meta programmation use to check if T is a templated instanciation of U<...>
template <class T, template <class...> class U>
inline constexpr bool is_specialization_of_v = false;

template <template <class...> class U, class... Args>
inline constexpr bool is_specialization_of_v<U<Args...>, U> = true;

template <class T, template <class...> class U>
concept specialization_of = is_specialization_of_v<std::remove_cvref_t<T>, U>;

/// Transparently unwraps std::reference_wrapper to an actual C++ reference.
/// If it's a normal value, it just perfectly forwards it.
template <class T>
constexpr auto unwrap_reference(T&& value) noexcept -> decltype(auto) {
  using RawT = std::remove_cvref_t<T>;
  using UnwrappedT = std::unwrap_reference_t<RawT>;
  if constexpr (!std::is_same_v<RawT, UnwrappedT>) {
    return value.get();
  } else {
    return FWD(value);
  }
}

}  // namespace atb
