#pragma once

#include <type_traits>  // false_type

namespace atb::details {

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

/// Remove reference, const and volatile qualifiers.
template <class T>
struct RemoveCVRef {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using RemoveCVRef_t = typename RemoveCVRef<T>::type;

namespace details {

template <class, template <class...> class Trait, class... Args>
struct HasTraitImpl : std::false_type {};

template <template <class...> class Trait, class... Args>
struct HasTraitImpl<std::void_t<Trait<Args...>>, Trait, Args...>
    : std::true_type {};

}  // namespace details

/// Returns TRUE when the given type ...T define the traits Traits
/// Use like this:
///
/// // Create a trait...
/// template <class ...T>
/// using FooFunction = decltype(Foo(std::declval<T>()...));
///
/// // ...Then use HasTrait:
/// static_assert(HasTrait_v<FooFunction, int, float, char>);
/// // HasTrait_v<FooFunction, int, float, char> -> true when the function
/// 'Foo(int, float, char)' is defined
template <template <class...> class Trait, class... Args>
struct HasTrait : details::HasTraitImpl<void, Trait, Args...> {};

template <template <class...> class Trait, class... Args>
constexpr bool HasTrait_v = HasTrait<Trait, Args...>::value;

}  // namespace atb::details
