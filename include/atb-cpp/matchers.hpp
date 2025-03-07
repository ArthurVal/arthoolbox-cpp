#pragma once

#include <cmath>
#include <functional>
#include <tuple>
#include <utility>

#include "internal/matcher-traits.hpp"

namespace atb::matchers {

/// Generic Matcher Traits containing meta informations of a Matcher, for a
/// given set of arguments
template <class T, class... Args>
struct MatcherTraits {
  /// True if T has '.IsMatching(Args...) -> bool' interface
  static constexpr auto HasMethod() -> bool {
    return internal::HasIsMatchingMethod_v<T, Args...>;
  }

  /// True if the function 'IsMatching(T, Args...) -> bool' is defined
  static constexpr auto HasFreeFunction() -> bool {
    return internal::HasIsMatchingFreeFunction_v<T, Args...>;
  }

  /// True if T is invokable  with '(Args...) -> bool' interface
  static constexpr auto HasCallOperator() -> bool {
    return internal::HasCallOperator_v<T, Args...>;
  }

  /// True if either HasCallOperator() or HasMethod() return true
  static constexpr auto IsValidMatcher() -> bool {
    return HasMethod() || HasFreeFunction() || HasCallOperator();
  }

  /// static assert when T and Args... and not valid
  static constexpr auto AssertWhenInvalid() -> void {
    // NOTE: Provide a single and clear message location for assertion since
    // Traits is responsible for defining what interfaces we are expecting
    static_assert(
        IsValidMatcher(),
        "\nThe Matcher type T provided doesn't have the correct traits."
        "\nIt must defined one of the following interface:"
        "\n - 'T.IsMatching(Args...)  -> bool'"
        "\n - 'IsMatching(T, Args...)  -> bool' (using ADL)"
        "\n - 'T.operator()(Args...)  -> bool'");
  }
};

/// Return the value of calling the matcher with the provided args.
///
/// Act as call dispatcher, using traits to choose which function to call
/// accordingly.
/// The priority is as follow:
/// 1. Method
/// 2. FreeFunction
/// 3. Call operator
/// 4. Asserts if none available
template <class Matcher, class... Args>
constexpr auto Invoke(Matcher&& m, Args&&... args) -> bool {
  using Traits = MatcherTraits<Matcher, Args...>;
  Traits::AssertWhenInvalid();

  if constexpr (Traits::HasMethod()) {
    return std::forward<Matcher>(m).IsMatching(std::forward<Args>(args)...);
  } else if constexpr (Traits::HasFreeFunction()) {
    return IsMatching(std::forward<Matcher>(m), std::forward<Args>(args)...);
  } else if constexpr (Traits::HasCallOperator()) {
    return std::invoke(std::forward<Matcher>(m), std::forward<Args>(args)...);
  } else {
    return false;
  }
}

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
    MatcherTraits<Matcher, decltype(v)>::AssertWhenInvalid();
    return ! m(std::forward<decltype(v)>(v));
  };
}

/// Returns true if ALL matchers returns true
template <class... Matchers>
constexpr auto All(Matchers&&... m) noexcept {
  return [&](auto&& v) {
    (MatcherTraits<Matchers, decltype(v)>::AssertWhenInvalid(), ...);
    return (m(v) && ...);
  };
}

/// Returns true if ONE OF the matchers returns true
template <class... Matchers>
constexpr auto Any(Matchers&&... m) noexcept {
  return [&](auto&& v) {
    (MatcherTraits<Matchers, decltype(v)>::AssertWhenInvalid(), ...);
    return (m(v) || ...);
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
  return [&](auto&&... v) {
    (MatcherTraits<Matcher, decltype(v)>::AssertWhenInvalid(), ...);
    return (m(std::forward<decltype(v)>(v)) && ...);
  };
}

/// Returns true if m returns true for at least ONE argument
template <class Matcher>
constexpr auto AnyArgs(Matcher&& m) noexcept {
  return [&](auto&&... v) {
    (MatcherTraits<Matcher, decltype(v)>::AssertWhenInvalid(), ...);
    return (m(std::forward<decltype(v)>(v)) || ...);
  };
}

}  // namespace atb::matchers
