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

}  // namespace atb::details
