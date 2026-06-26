#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>

#include "atb-cpp/type_traits.hpp"

namespace atb {

namespace details {

/// Traits to check for the existence of IsMatching class method
template <class M, class... Args>
using IsMatchingMethod =
    decltype(std::declval<M>().IsMatching(std::declval<Args>()...));

/// Traits to check for the existence of IsMatching free function
template <class M, class... Args>
using IsMatchingFreeFunction =
    decltype(IsMatching(std::declval<M>(), std::declval<Args>()...));

}  // namespace details

/**
 *  \brief Matcher traits use to probe any type M for Matcher's interfaces
 *
 *  This traits can be use to check if any type M is defined as a 'Matcher' for
 *  the provided Args... arguments.
 *
 *  A 'Matcher' is an object m (of type M) that provides at least ONE of the
 *  following interface for the given args... (of type ...Args):
 *  - Method:        `m.IsMatching(args...) -> bool`
 *  - Free function: `IsMatching(m, args...) -> bool`
 *  - Invokable:     `m(args...) -> bool`
 *
 *  The traits therefore defines the static method `::IsValidMatcher() -> bool`
 *  that returns TRUE whenever the M/Args provide one of the interface listed
 *  above. False otherwise.
 *
 *  Each interface existence can be check individually by the `::HasMethod() ->
 *  bool`, `::HasFreeFunction() -> bool` and `::IsInvokable() -> bool` static
 *  methods.
 *
 *  A Static assert helper static method `::AssertWhenInvalid() -> void` is also
 *  provided, in order to `static_assert(IsValidMatcher(), ...)` with a default
 *  message listing the needed requirement on those types.
 */
template <class M, class... Args>
struct MatcherTraits {
  /// True if M has 'm.IsMatching(args...) -> bool' interface
  static constexpr auto HasMethod() -> bool {
    return HasTrait_v<details::IsMatchingMethod, M, Args...>;
  }

  /// True if the function 'IsMatching(m, args...) -> bool' is defined
  static constexpr auto HasFreeFunction() -> bool {
    return HasTrait_v<details::IsMatchingFreeFunction, M, Args...>;
  }

  /// True if M is invokable  with 'm(args...) -> bool' interface
  static constexpr auto IsInvokable() -> bool {
    return std::is_invocable_r_v<bool, M, Args...>;
  }

  /// True if the M/Args provide at least ONE of the required interface
  static constexpr auto IsValidMatcher() -> bool {
    return HasMethod() || HasFreeFunction() || IsInvokable();
  }

  /// Static assert when M and Args... are not valid
  ///
  /// \note This provides a single and clear message for assertion since Traits
  ///       is responsible for defining what interfaces we are expecting
  static constexpr auto AssertWhenInvalid() -> void {
    static_assert(
        IsValidMatcher(),
        "\nThe Matcher type M provided doesn't have the correct interfaces."
        "\nIt must defined one of the following interface:"
        "\n - 'm.IsMatching(args...)  -> bool'"
        "\n - 'IsMatching(m, args...)  -> bool' (using ADL)"
        "\n - 'std::invoke(m, args...)  -> bool'");
  }
};

namespace details {

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

/**
 *  \brief Checks for a match between ANY matcher \a m and a set of \a args
 *
 * This is a CPO (Customization Point Object) call dispatcher, using traits to
 * choose which matcher's function to call accordingly. Its interface is as
 * follow (from `IsMatching_fn` definition):
 *
 * template<class M, class... Args>
 * auto IsMatching(M&& m, Args&&...args) -> bool;
 *
 * If the matcher defines multiple interfaces, the dispatcher prioritizes them
 *
 * as follow:
 * 1. Method call (i.e. `m.IsMatching(args...)`)
 * 2. FreeFunction using ADL (i.e. `IsMatching(m, args...)`)
 * 3. std::invoke (i.e. `std::invoke(m, args...)`)
 *
 * This asserts (static_assert) when the provided M/Args are not valid matchers.
 *
 *  \param[in] m Matcher object
 *  \param[in] ...args Set of arguments forwarded to the matcher
 *
 *  \return bool True whenever the matcher succesfully matches against the
 *               provided arguments, false otherwise.
 */
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
      [m = std::move(m)](const auto& v) -> bool { return !::IsMatching(m, v); };
}

/// Returns true if ALL matchers returns true
template <class... Matchers>
constexpr auto AllOf(Matchers... m) noexcept {
  return
      [matchers = std::make_tuple(std::move(m)...)](const auto&... v) -> bool {
        return std::apply(
            [&](const auto&... _m) { return (::IsMatching(_m, v...) && ...); },
            matchers);
      };
}

/// Returns true if ONE OF the matchers returns true
template <class... Matchers>
constexpr auto AnyOf(Matchers... m) noexcept {
  return
      [matchers = std::make_tuple(std::move(m)...)](const auto&... v) -> bool {
        return std::apply(
            [&](const auto&... _m) { return (::IsMatching(_m, v...) || ...); },
            matchers);
      };
}

// FILTER ARGS COMPOSITE MATCHERS //////////////////////////////////////////////

/// Returns m(v...) by selecting the Ith argument(s) v from a pack of arguments
template <std::size_t... I, class Matcher>
constexpr auto OnArgs(Matcher m) noexcept {
  return [m = std::move(m)](const auto&... v) -> bool {
    auto values = std::tie(v...);
    return ::IsMatching(m, std::get<I>(values)...);
  };
}

/// Returns true if m returns true for ALL the arguments individually
template <class Matcher>
constexpr auto AllArgs(Matcher m) noexcept {
  return [m = std::move(m)](const auto&... v) -> bool {
    return (::IsMatching(m, v) && ...);
  };
}

/// Returns true if m returns true for at least ONE of the arguments
template <class Matcher>
constexpr auto AnyArgs(Matcher m) noexcept {
  return [m = std::move(m)](const auto&... v) -> bool {
    return (::IsMatching(m, v) || ...);
  };
}

// TYPE ERASED MATCHER /////////////////////////////////////////////////////////

/**
 *  \brief Type erased polymorphic Matcher for a fix set of input arguments
 *
 *  This matcher can be assign to ANY matcher M (i.e. type that are 'valid
 *  matcher' according to the MatcherTraits) at runtime.
 *
 *  It can be used when needing to change Matcher at runtime (vs compile time)
 *  dynamically or store matchers in a container (std::vector<AnyMatcher> for
 *  example), etc..
 *
 *  Example:
 *  \code{.cpp}
 *
 *  // Initialze the AnyMatcher
 *  AnyMatcher<int, int> m;
 *
 *  m = Always<false>();
 *  assert(::IsMatching(m, 42, 24) == false);
 *  assert(::IsMatching(m, -42, 0) == false);
 *
 *  // Change the matcher dynamically
 *  m = AllArgs(Eq(10));
 *  assert(::IsMatching(m, -42, 0) == false);
 *  assert(::IsMatching(m, 10, 10) == true);
 *
 *  m = All(AllArgs(Gt(0)), OnArgs<0>(Eq(20)));
 *  assert(::IsMatching(m, 20, -10) == false);
 *  assert(::IsMatching(m, 19, 10) == false);
 *  assert(::IsMatching(m, 20, 10) == true);
 *
 *  std::vector<AnyMatcher<int, int>> matchers;
 *  matchers.emplace_back()
 *
 *  \endcode
 *
 *
 *
 *  \tparam ...Args Arguments type for the given matcher
 */
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
  /// Default ctor - Bad state/unassigned
  constexpr AnyMatcher() : m_interface(nullptr){};

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
    MatcherTraits<std::decay_t<Matcher>, Args...>::AssertWhenInvalid();
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
  ///
  /// \pre AnyMatcher has been assigned to a valid matcher (not default constructed)
  constexpr auto IsMatching(const Args&... args) const -> bool {
    assert(m_interface != nullptr);
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
