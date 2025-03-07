#pragma once

#include <type_traits>

namespace atb::matchers::internal {

namespace details {

template <class T, class = void, class... Args>
struct HasIsMatchingMethodImpl : std::false_type {};

template <class T, class... Args>
struct HasIsMatchingMethodImpl<
    T,
    std::void_t<decltype(std::declval<T>().IsMatching(
        std::declval<Args>()...))>,
    Args...>
    : std::is_convertible<decltype(std::declval<T>().IsMatching(
                              std::declval<Args>()...)),
                          bool> {};

}  // namespace details

/// Meta function returning TRUE when T has a method IsMatching with the
/// following signature: 'T.IsMatching(Args...) -> bool'
template <class T, class... Args>
struct HasIsMatchingMethod
    : details::HasIsMatchingMethodImpl<T, void, Args...> {};

template <class T, class... Args>
constexpr bool HasIsMatchingMethod_v = HasIsMatchingMethod<T, Args...>::value;

/// Meta function returning TRUE when T is a callable (defined operator()) with
/// the following signature: '(Args...) -> bool'
template <class T, class... Args>
struct HasCallOperator : std::is_invocable_r<bool, T, Args...> {};

template <class T, class... Args>
constexpr bool HasCallOperator_v = HasCallOperator<T, Args...>::value;

}  // namespace atb::matchers::internal
