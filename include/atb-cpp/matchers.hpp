#pragma once

#include <cmath>
#include <tuple>
#include <utility>

namespace atb::matchers {

namespace details {

template <class T, class... Args>
struct IsMatcher : std::is_invocable_r<bool, T, Args...> {};

template <class T, class... Args>
constexpr bool IsMatcher_v = IsMatcher<T, Args...>::value;

}  // namespace details

// COMMON MATCHERS //////////////////////////////////////////////////////////

/// Returns Value
template <bool Value>
constexpr auto Always() noexcept {
  return [](auto...) noexcept { return Value; };
}

/// Returns true when v == expected
template <class T>
constexpr auto Eq(const T& expected) noexcept {
  return [&](auto&& v) { return std::forward<decltype(v)>(v) == expected; };
}

/// Returns true when v != expected
template <class T>
constexpr auto Ne(const T& expected) noexcept {
  return [&](auto&& v) { return std::forward<decltype(v)>(v) != expected; };
}

/// Returns true when v >= expected
template <class T>
constexpr auto Ge(const T& expected) noexcept {
  return [&](auto&& v) { return std::forward<decltype(v)>(v) >= expected; };
}

/// Returns true when v > expected
template <class T>
constexpr auto Gt(const T& expected) noexcept {
  return [&](auto&& v) { return std::forward<decltype(v)>(v) > expected; };
}

/// Returns true when v <= expected
template <class T>
constexpr auto Le(const T& expected) noexcept {
  return [&](auto&& v) { return std::forward<decltype(v)>(v) <= expected; };
}

/// Returns true when v < expected
template <class T>
constexpr auto Lt(const T& expected) noexcept {
  return [&](auto&& v) { return std::forward<decltype(v)>(v) < expected; };
}

// Returns true when |v - expected| <= abs_error
template <class T, class ErrorType = double>
constexpr auto Near(const T& expected,
                    const ErrorType& abs_error = 1e-6) noexcept {
  return [&](auto&& v) {
    return (std::abs(std::forward<decltype(v)>(v) - expected) <= abs_error);
  };
}

// COMPOSITE MATCHERS ///////////////////////////////////////////////////////

/// Returns the negation of m(v)
template <class Matcher>
constexpr auto Not(Matcher&& m) noexcept {
  return [&](auto&& v) {
    static_assert(details::IsMatcher_v<Matcher, decltype(v)>);
    return not m(std::forward<decltype(v)>(v));
  };
}

/// Returns true if ALL matchers returns true
template <class... Matchers>
constexpr auto All(Matchers&&... m) noexcept {
  return [&](auto&& v) {
    static_assert((details::IsMatcher_v<Matchers, decltype(v)> and ...));
    return (m(v) and ...);
  };
}

/// Returns true if ONE OF the matchers returns true
template <class... Matchers>
constexpr auto Any(Matchers&&... m) noexcept {
  return [&](auto&& v) {
    static_assert((details::IsMatcher_v<Matchers, decltype(v)> and ...));
    return (m(v) or ...);
  };
}

// COMPOSITE ARGS MATCHERS //////////////////////////////////////////////////

/// Returns m(v) by selecting the Ith argument v from a pack of arguments
template <std::size_t I, class Matcher>
constexpr auto OnArg(Matcher&& m) noexcept {
  return [&](auto&&... v) {
    static_assert(I < sizeof...(v), "Too few arguments");
    auto values = std::forward_as_tuple(std::forward<decltype(v)>(v)...);
    return m(std::get<I>(values));
  };
}

/// Returns true if m returns true for ALL the input arguments
template <class Matcher>
constexpr auto AllArgs(Matcher&& m) noexcept {
  return [&](auto&&... v) { return (m(std::forward<decltype(v)>(v)) and ...); };
}

/// Returns true if m returns true for at least ONE argument
template <class Matcher>
constexpr auto AnyArgs(Matcher&& m) noexcept {
  return [&](auto&&... v) { return (m(std::forward<decltype(v)>(v)) or ...); };
}

}  // namespace atb::matchers
