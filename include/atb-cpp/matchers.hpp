#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>

#include "atb-cpp/type_traits.hpp"

namespace atb {

namespace details {

template <class M, class... Args>
using IsMatchingMethod =
    decltype(std::declval<M>().IsMatching(std::declval<Args>()...));

template <class M, class... Args>
using IsMatchingFreeFunction =
    decltype(IsMatching(std::declval<M>(), std::declval<Args>()...));

/// Generic Matcher Traits containing meta informations of a Matcher, for a
/// given set of arguments
template <class M, class... Args>
struct MatcherTraits {
  /// True if T has '.IsMatching(Args...) -> bool' interface
  static constexpr auto HasMethod() -> bool {
    return HasTrait_v<details::IsMatchingMethod, M, Args...>;
  }

  /// True if the function 'IsMatching(T, Args...) -> bool' is defined
  static constexpr auto HasFreeFunction() -> bool {
    return HasTrait_v<details::IsMatchingFreeFunction, M, Args...>;
  }

  /// True if T is invokable  with '(Args...) -> bool' interface
  static constexpr auto IsInvokable() -> bool {
    return std::is_invocable_r_v<bool, M, Args...>;
  }

  /// True if either HasCallOperator() or HasMethod() return true
  static constexpr auto IsValidMatcher() -> bool {
    return HasMethod() || HasFreeFunction() || IsInvokable();
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
        "\n - 'std::invoke(T, Args...)  -> bool'");
  }
};

/// Prevent fall backs for the CPO
void IsMatching() = delete;

/// IsMatching Customisation Point Object (CPO) dispatcher
struct IsMatching_fn {
  template <class M, class... Args>
  constexpr auto operator()(M&& m, Args&&... args) const -> bool {
    using Traits = MatcherTraits<std::remove_reference_t<M>,
                                 std::remove_reference_t<Args>...>;

    Traits::AssertWhenInvalid();

    if constexpr (Traits::HasMethod()) {
      return std::forward<M>(m).IsMatching(std::forward<Args>(args)...);
    } else if constexpr (Traits::HasFreeFunction()) {
      return IsMatching(std::forward<M>(m), std::forward<Args>(args)...);
    } else if constexpr (Traits::IsInvokable()) {
      return std::invoke(std::forward<M>(m), std::forward<Args>(args)...);
    } else {
      return false;  // Should never be reached due to AssertWhenInvalid
    }
  }
};

}  // namespace details
}  // namespace atb

/// Main function use to call ANY matcher
///
/// This act as call dispatcher, using traits to choose which function to call
/// accordingly.
///
/// The priority is as follow:
/// 1. Method
/// 2. FreeFunction
/// 3. Call operator
/// 4. Asserts if none available
inline constexpr atb::details::IsMatching_fn IsMatching{};

namespace atb {

// COMMON MATCHERS //////////////////////////////////////////////////////////

/// Returns Value
template <bool Value>
constexpr auto Always() noexcept {
  return [](const auto&...) noexcept -> bool { return Value; };
}

/// Returns true when v == expected
template <class T>
constexpr auto Eq(T expected) noexcept {
  return [expected = std::move(expected)](const auto& v) -> bool {
    return v == expected;
  };
}

/// Returns true when v != expected
template <class T>
constexpr auto Ne(T expected) noexcept {
  return [expected = std::move(expected)](const auto& v) -> bool {
    return v != expected;
  };
}

/// Returns true when v >= expected
template <class T>
constexpr auto Ge(T expected) noexcept {
  return [expected = std::move(expected)](const auto& v) -> bool {
    return v >= expected;
  };
}

/// Returns true when v > expected
template <class T>
constexpr auto Gt(T expected) noexcept {
  return [expected = std::move(expected)](const auto& v) -> bool {
    return v > expected;
  };
}

/// Returns true when v <= expected
template <class T>
constexpr auto Le(T expected) noexcept {
  return [expected = std::move(expected)](const auto& v) -> bool {
    return v <= expected;
  };
}

/// Returns true when v < expected
template <class T>
constexpr auto Lt(T expected) noexcept {
  return [expected = std::move(expected)](const auto& v) -> bool {
    return v < expected;
  };
}

// Returns true when |v - expected| <= abs_error
template <class T, class ErrorType = double>
constexpr auto Near(T expected, ErrorType abs_error = 1e-6) noexcept {
  return [expected = std::move(expected),
          abs_error = std::move(abs_error)](const auto& v) -> bool {
    return (std::abs(v - expected) <= abs_error);
  };
}

// COMPOSITE MATCHERS ///////////////////////////////////////////////////////

/// Returns the negation of m(v)
template <class Matcher>
constexpr auto Not(Matcher m) noexcept {
  return
      [m = std::move(m)](const auto& v) -> bool { return !IsMatching(m, v); };
}

/// Returns true if ALL matchers returns true
template <class... Matchers>
constexpr auto AllOf(Matchers... m) noexcept {
  return
      [matchers = std::make_tuple(std::move(m)...)](const auto&... v) -> bool {
        return std::apply(
            [&](const auto&... _m) { return (IsMatching(_m, v...) && ...); },
            matchers);
      };
}

/// Returns true if ONE OF the matchers returns true
template <class... Matchers>
constexpr auto AnyOf(Matchers... m) noexcept {
  return
      [matchers = std::make_tuple(std::move(m)...)](const auto&... v) -> bool {
        return std::apply(
            [&](const auto&... _m) { return (IsMatching(_m, v...) || ...); },
            matchers);
      };
}

// COMPOSITE ARGS MATCHERS //////////////////////////////////////////////////

/// Returns m(v) by selecting the Ith argument v from a pack of arguments
template <std::size_t... I, class Matcher>
constexpr auto OnArgs(Matcher m) noexcept {
  return [m = std::move(m)](const auto&... v) -> bool {
    auto values = std::tie(v...);
    return IsMatching(m, std::get<I>(values)...);
  };
}

/// Returns true if m returns true for ALL the input arguments
template <class Matcher>
constexpr auto AllArgs(Matcher m) noexcept {
  return [m = std::move(m)](const auto&... v) -> bool {
    return (IsMatching(m, v) && ...);
  };
}

/// Returns true if m returns true for at least ONE argument
template <class Matcher>
constexpr auto AnyArgs(Matcher m) noexcept {
  return [m = std::move(m)](const auto&... v) -> bool {
    return (IsMatching(m, v) || ...);
  };
}

// TYPE ERASED MATCHER /////////////////////////////////////////////////////////

/// Type erased matcher, able store ANY matcher at runtime, polymorphically
template <class... Args>
class AnyMatcher final {
  /// The virtual interface defining a Matcher
  struct Interface;

  /// Wrapper class wrapping any matcher
  template <class Matcher>
  struct Wrapper;

  /// Opaque ptr pointing to the matcher interface
  std::unique_ptr<Interface> m_interface;

 public:
  /// Default ctor disabled
  constexpr AnyMatcher() = delete;

  /// Copy ctor: clone the underlying interface
  constexpr AnyMatcher(const AnyMatcher& other)
      : m_interface(other.m_interface->Clone()) {}

  /// Move ctor: move the underlying interface
  constexpr AnyMatcher(AnyMatcher&& other)
      : m_interface(std::move(other.m_interface)) {}

  /// Copy assignment: clone the underlying interface
  constexpr auto operator=(const AnyMatcher& other) -> AnyMatcher& {
    m_interface = other.m_interface->Clone();
    return *this;
  }

  /// Move assignment: move the underlying interface
  constexpr auto operator=(AnyMatcher&& other) -> AnyMatcher& {
    m_interface = std::move(other.m_interface);
    return *this;
  }

  /// Dtor
  ~AnyMatcher() noexcept = default;

  /// Construct AnyMatcher from any ohter matcher like object
  template <class Matcher,
            std::enable_if_t<!std::is_same_v<AnyMatcher, std::decay_t<Matcher>>,
                             bool> = true>
  constexpr explicit AnyMatcher(Matcher&& m)
      : m_interface(std::make_unique<Wrapper<std::decay_t<Matcher>>>(
            std::forward<Matcher>(m))) {
    details::MatcherTraits<std::decay_t<Matcher>, Args...>::AssertWhenInvalid();
  }

  /// Assign the AnyMatcher to point to an other matcher
  template <class Matcher,
            std::enable_if_t<!std::is_same_v<AnyMatcher, std::decay_t<Matcher>>,
                             bool> = true>
  constexpr auto operator=(Matcher&& m) -> AnyMatcher& {
    *this = AnyMatcher(std::forward<Matcher>(m));
    return *this;
  }

  /// Mandatory IsMatching(Args...) -> bool interface
  constexpr auto IsMatching(const Args&... args) const -> bool {
    return m_interface->IsMatching(args...);
  }

 private:
  struct Interface {
    virtual ~Interface() noexcept = default;
    virtual auto Clone() const -> std::unique_ptr<Interface> = 0;
    virtual auto IsMatching(const Args&... args) const -> bool = 0;
  };

  template <class Matcher>
  struct Wrapper final : public Interface {
    template <class... T>
    constexpr explicit Wrapper(T&&... args)
        : m_matcher(std::forward<T>(args)...) {}

    auto Clone() const -> std::unique_ptr<Interface> override {
      return std::make_unique<Wrapper>(m_matcher);
    }

    auto IsMatching(const Args&... args) const -> bool override {
      return ::IsMatching(m_matcher, args...);
    }

   private:
    Matcher m_matcher;
  };
};

}  // namespace atb
