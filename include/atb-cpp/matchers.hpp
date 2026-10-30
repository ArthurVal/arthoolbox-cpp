#pragma once

#include <cassert>
#include <cmath>       // std::abs()
#include <functional>  // std::invoke()/is_invokable_r/...
#include <memory>      // std::unique_ptr
#include <stdexcept>   // std::runtime_error
#include <tuple>       // std::make_tuple
#include <utility>     // std::forward/...

#include "atb-cpp/type_traits.hpp"

namespace atb {

namespace details {

/// Poison for the CPO - Prevent fall backs when the function is undefined for
/// the concepts below
void IsMatching() = delete;

/// Checks if m.IsMatching(...) exists and returns a type convertible to bool
template <class M, class... Args>
concept HasIsMatchingMethod = requires(M&& m, Args&&... args) {
  { ATB_FWD(m).IsMatching(ATB_FWD(args)...) } -> std::convertible_to<bool>;
};

/// Checks if m.IsMatching(...) exists and returns a type convertible to bool
template <class M, class... Args>
concept HasIsMatchingFreeFunction = requires(M&& m, Args&&... args) {
  { IsMatching(ATB_FWD(m), ATB_FWD(args)...) } -> std::convertible_to<bool>;
};

/// Checks if std::invoke(m, ...) is valid and returns a type convertible to
/// bool
template <class M, class... Args>
concept IsInvokableMatcher = requires(M&& m, Args&&... args) {
  { std::invoke(ATB_FWD(m), ATB_FWD(args)...) } -> std::convertible_to<bool>;
};

/// Checks that the given type M is a matcher overall
template <class M, class... Args>
concept IsMatcherFor =
    HasIsMatchingMethod<M, Args...> || HasIsMatchingFreeFunction<M, Args...> ||
    IsInvokableMatcher<M, Args...>;

/// IsMatching Customisation Point Object (CPO) dispatcher
struct IsMatching_fn {
  /// Nominal dispatcher call
  template <class M, class... Args>
    requires IsMatcherFor<M, Args...> &&
             (!specialization_of<M, std::reference_wrapper>)
  constexpr auto operator()(M&& m, Args&&... args) const -> bool {
    // Dispatch prioritizing Method > Free Function > Invokable
    if constexpr (HasIsMatchingMethod<M, Args...>) {
      return ATB_FWD(m).IsMatching(ATB_FWD(args)...);
    } else if constexpr (HasIsMatchingFreeFunction<M, Args...>) {
      return IsMatching(ATB_FWD(m), ATB_FWD(args)...);
    } else {
      return std::invoke(ATB_FWD(m), ATB_FWD(args)...);
    }
  }

  /// Handles std::reference_wrapper
  template <class... Args>
  constexpr auto operator()(specialization_of<std::reference_wrapper> auto ref,
                            Args&&... args) const -> bool {
    return (*this)(ref.get(), ATB_FWD(args)...);
  }
};

}  // namespace details

/**
 * @brief Checks for a match between ANY matcher \a m and a set of \a args
 *
 * This is a CPO (Customization Point Object) call dispatcher, using traits to
 * choose which matcher's function to call accordingly. Its interface is the
 * following (from `IsMatching_fn` definition):
 *
 * template<class M, class... Args>
 * auto IsMatching(M&& m, Args&&...args) -> bool;
 *
 * If the matcher defines multiple interfaces, the dispatcher prioritizes them
 * as follow:
 * 1. Method call (i.e. `m.IsMatching(args...)`)
 * 2. FreeFunction using ADL (i.e. `IsMatching(m, args...)`)
 * 3. std::invoke (i.e. `std::invoke(m, args...)`)
 *
 * This asserts (static_assert) when the provided M/Args are not valid matchers.
 *
 * @param[in] m Matcher object
 * @param[in] ...args Set of arguments forwarded to the matcher
 *
 * @return bool True whenever the matcher succesfully matches against the
 *              provided arguments, false otherwise.
 */
inline constexpr details::IsMatching_fn IsMatching{};

/// Concept indicating if a type T is 'Printable' into Sink, ie one of the
/// following interfaces exist (t of type T and sink of type Sink):
/// - Method call (i.e. `t.PrintInto(sink)`)
/// - FreeFunction using ADL (i.e. `PrintInto(sink, t)`)
/// - std::invoke (i.e. `std::invoke(t, sink)`)
/// - Stream overload (i.e. `sink << t`)
template <class M, class... Args>
concept MatcherFor = requires(M&& matcher, Args&&... args) {
  { atb::IsMatching(matcher, ATB_FWD(args)...) } -> std::convertible_to<bool>;
};

// COMMON MATCHERS //////////////////////////////////////////////////////////

/**
 * @brief Matcher that always returns Value, independently of the inputs
 */
template <bool Value>
struct Always {
  constexpr auto operator()(auto&&...) const noexcept -> bool { return Value; }
};

/**
 * @brief Matcher that returns true when v == expected
 */
template <class T>
struct Eq {
  T expected;
  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return ATB_FWD(v) == expected;
  }
};

/**
 * @brief Matcher that returns true when v != expected
 */
template <class T>
struct Ne {
  T expected;
  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return ATB_FWD(v) != expected;
  }
};

/**
 * @brief Matcher that returns true when v >= expected
 */
template <class T>
struct Ge {
  T expected;
  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return ATB_FWD(v) >= expected;
  }
};

/**
 * @brief Matcher that returns true when v > expected
 */
template <class T>
struct Gt {
  T expected;
  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return ATB_FWD(v) > expected;
  }
};

/**
 * @brief Matcher that returns true when v <= expected
 */
template <class T>
struct Le {
  T expected;
  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return ATB_FWD(v) <= expected;
  }
};

/**
 * @brief Matcher that returns true when v < expected
 */
template <class T>
struct Lt {
  T expected;
  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return ATB_FWD(v) < expected;
  }
};

/**
 * @brief Matcher that returns true when |v - expected| <= abs_error
 */
template <class T, class Error = double>
struct Near {
  T expected;
  Error abs_error = 1e-6;

  constexpr auto operator()(auto&& v) const noexcept -> bool {
    return (std::abs(ATB_FWD(v) - expected) <= abs_error);
  }
};

// COMPOSITE MATCHERS ///////////////////////////////////////////////////////

/**
 * @brief Matcher that negates a given matcher
 */
template <class Matcher>
struct Not {
  Matcher matcher;

  constexpr auto operator()(auto&&... v) const noexcept -> bool {
    return !atb::IsMatching(matcher, ATB_FWD(v)...);
  }
};

/**
 * @return true if ALL matchers returns true
 */
template <class... Matchers>
struct AllOf {
  std::tuple<Matchers...> matchers;
  static_assert(sizeof...(Matchers) > 0);

  constexpr explicit AllOf(Matchers... m) noexcept
      : matchers(std::move(m)...) {}

  constexpr auto operator()(const auto&... v) const noexcept -> bool {
    return std::apply(
        [&](const auto&... m) -> bool {
          return (atb::IsMatching(m, v...) && ...);
        },
        matchers);
  }
};

/**
 * @return true if ONE OF the matchers returns true
 */
template <class... Matchers>
struct AnyOf {
  std::tuple<Matchers...> matchers;
  static_assert(sizeof...(Matchers) > 0);

  constexpr explicit AnyOf(Matchers... m) noexcept
      : matchers(std::move(m)...) {}

  constexpr auto operator()(const auto&... v) const noexcept -> bool {
    return std::apply(
        [&](const auto&... m) -> bool {
          return (atb::IsMatching(m, v...) || ...);
        },
        matchers);
  }
};

// FILTER ARGS COMPOSITE MATCHERS //////////////////////////////////////////////

/**
 * @return m(v[i]...) by selecting the Ith argument(s) from v...
 */
template <class Matcher, std::size_t... I>
struct ArgsAtMatcher {
  Matcher matcher;

  constexpr auto operator()(auto&&... v) const noexcept -> bool {
    auto values = std::forward_as_tuple(ATB_FWD(v)...);
    return atb::IsMatching(matcher, std::get<I>(values)...);
  }
};

template <std::size_t... I, class Matcher>
constexpr auto ArgsAt(Matcher&& m)
    -> ArgsAtMatcher<std::remove_cvref_t<Matcher>, I...> {
  return {ATB_FWD(m)};
}

/**
 * @return true if m returns true for ALL the arguments individually
 */
template <class Matcher>
struct EveryArg {
  Matcher matcher;

  constexpr auto operator()(auto&&... v) const noexcept -> bool {
    return (atb::IsMatching(matcher, ATB_FWD(v)) && ...);
  }
};

/**
 * @return true if m returns true for at least ONE of the arguments
 */
template <class Matcher>
struct AnyArg {
  Matcher matcher;

  constexpr auto operator()(auto&&... v) const noexcept -> bool {
    return (atb::IsMatching(matcher, ATB_FWD(v)) || ...);
  }
};

/**
 * @brief Matcher that invoke \a func before calling the underlying \a matcher
 */
template <class F, class Matcher>
struct MapArg {
  F transform;
  Matcher matcher;

  constexpr auto operator()(auto&&... v) const noexcept -> bool {
    return atb::IsMatching(matcher, std::invoke(transform, ATB_FWD(v))...);
  }
};

// TYPE ERASED MATCHER /////////////////////////////////////////////////////////

namespace details {

/// TODO
template <class T>
concept UnInitializedPolicy = requires(T policy) {
  { policy.Assert() } -> std::convertible_to<bool>;
};

/**
 * @brief Type erased polymorphic Matcher for a fix set of input arguments
 *
 * This matcher can be assign to ANY matcher M (i.e. type that are 'valid
 * matcher' according to the MatcherTraits) at runtime.
 *
 * It can be used when needing to change Matcher at runtime (vs compile time)
 * dynamically or store matchers in a container (std::vector<AnyMatcher> for
 * example), etc..
 *
 * Example:
 *
 * @code{.cpp}
 *
 * // Initialze the AnyMatcher
 * AnyMatcher<int, int> m;
 *
 * // Un-initialized by default, calling IsMatching here will THROW
 * assert(m.IsInitialized() == false);
 *
 * m = Always<false>();
 * assert(m.IsInitialized() == true);
 *
 * assert(atb::IsMatching(m, 42, 24) == false);
 * assert(atb::IsMatching(m, -42, 0) == false);
 *
 * // Change the matcher dynamically
 * m = AllArgs(Eq(10));
 * assert(atb::IsMatching(m, -42, 0) == false);
 * assert(atb::IsMatching(m, 10, 10) == true);
 *
 * m = All(AllArgs(Gt(0)), OnArgs<0>(Eq(20)));
 * assert(atb::IsMatching(m, 20, -10) == false);
 * assert(atb::IsMatching(m, 19, 10) == false);
 * assert(atb::IsMatching(m, 20, 10) == true);
 *
 * std::vector<AnyMatcher<int>> matchers;
 * matchers.emplace_back(Ge(10));
 * matchers.emplace_back(Le(50));
 *
 * for (const auto& m : matchers) assert(::IsMatching(m, 42) == true);
 *
 * @endcode
 *
 * By default, this type erased class is default constructible. In this case,
 * it doesn't own any matcher and it is considered 'uninitialized'.
 * When calling `IsMatching(...)` while the matcher is uninitialized, it
 * behaves based on the UninitializedPolicy provided.
 *
 * This policy is expected to have a `::Assert(void) -> bool` interface that
 * dictates what to do whenever the internal matcher is NULL (return false,
 * throw an exception, assert, ...).
 *
 * @tparam UninitializedPolicy Policy use when the AnyMatcher is uninitialized
 * @tparam ...Args Arguments type for the given matcher
 */
template <UnInitializedPolicy Policy, class... Args>
class AnyMatcherImpl final {
  /// The virtual interface defining a Matcher
  struct Interface;

  /// Wrapper class wrapping any matcher
  template <MatcherFor<Args...> Matcher>
  struct Wrapper;

  /// Opaque ptr pointing to the matcher interface
  std::unique_ptr<Interface> m_itf;

 public:
  /**
   * @brief Default ctor (not initialized)
   */
  constexpr AnyMatcherImpl() noexcept : m_itf(nullptr) {}

  /**
   * @brief Copy ctor: clone the underlying interface
   */
  constexpr AnyMatcherImpl(const AnyMatcherImpl& other)
      : m_itf(other.m_itf != nullptr ? other.m_itf->Clone() : nullptr) {}

  /**
   * @brief Move ctor: move the underlying interface
   */
  constexpr AnyMatcherImpl(AnyMatcherImpl&& other) noexcept
      : m_itf(std::move(other.m_itf)) {}

  /**
   * @brief Copy assignment: clone the underlying interface
   */
  constexpr auto operator=(const AnyMatcherImpl& other) -> AnyMatcherImpl& {
    m_itf = other.m_itf->Clone();
    return *this;
  }

  /**
   * @brief Move assignment: move the underlying interface
   */
  constexpr auto operator=(AnyMatcherImpl&& other) noexcept -> AnyMatcherImpl& {
    m_itf = std::move(other.m_itf);
    return *this;
  }

  /**
   * @brief Dtor
   */
  ~AnyMatcherImpl() noexcept = default;

  /**
   * @brief Construct AnyMatcher from any other matcher like object
   *
   * @param[in] m Any matcher defining a 'matcher' interface based on the
   *              MatcherTraits
   */
  template <class M>
    requires MatcherFor<M, Args...> &&
             (!std::same_as<std::remove_cvref_t<M>, AnyMatcherImpl>)
  constexpr explicit AnyMatcherImpl(M&& m)
      : m_itf(std::make_unique<Wrapper<std::remove_cvref_t<M>>>(ATB_FWD(m))) {}

  /**
   * @brief Re-Assign the AnyMatcher to point to an other matcher
   *
   * @param[in] m Any matcher defining a 'matcher' interface based on the
   *              MatcherTraits
   */
  template <class M>
    requires MatcherFor<M, Args...> &&
             (!std::same_as<std::remove_cvref_t<M>, AnyMatcherImpl>)
  constexpr auto operator=(M&& m) -> AnyMatcherImpl& {
    *this = AnyMatcherImpl(ATB_FWD(m));
    return *this;
  }

  /**
   * @return true whenever the current matcher has be initialized/assigned
   *         (i.e. IsMatching can be called safely). False otherwise.
   */
  constexpr auto IsInitialized() const -> bool { return m_itf != nullptr; }

  /**
   * @brief Mandatory matcher `IsMatching(Args...) -> bool` interface.
   *
   * @return true whenever the current stored matcher matches, false otherwise.
   *         When the AnyMatcher is NOT ASSIGNED (uninitialized), the behavior
   *         depends on the AssertPolicy selected.
   */
  constexpr auto IsMatching(Args... args) const -> bool {
    if (!IsInitialized()) {
      return Policy{}.Assert();
    }

    return m_itf->IsMatching(ATB_FWD(args)...);
  }

 private:
  struct Interface {
    virtual ~Interface() noexcept = default;
    virtual auto Clone() const -> std::unique_ptr<Interface> = 0;
    virtual auto IsMatching(Args... args) const -> bool = 0;
  };

  template <MatcherFor<Args...> Matcher>
  struct Wrapper final : public Interface {
    constexpr explicit Wrapper(auto&&... args) : m_matcher(ATB_FWD(args)...) {}

    auto Clone() const -> std::unique_ptr<Interface> override {
      return std::make_unique<Wrapper>(m_matcher);
    }

    auto IsMatching(Args... args) const -> bool override {
      return atb::IsMatching(m_matcher, ATB_FWD(args)...);
    }

   private:
    Matcher m_matcher;
  };
};

/**
 * @brief UninitializedPolicy that throws an exception
 */
struct Throws {
  static auto Assert() -> bool {
    throw std::runtime_error{
        "AnyMatcher is empty (i.e. not matcher assigned to it)",
    };
  }
};

/**
 * @brief UninitializedPolicy that assert() and ALWAYS return false
 */
struct Asserts {
  static auto Assert() noexcept -> bool {
    assert(false && "AnyMatcher is empty (i.e. not matcher assigned to it)");
    return false;
  }
};

}  // namespace details

/**
 * @brief Default AnyMatcher implementation that THROWS when uninitialized
 *
 * See details::AnyMatcherImpl for more details.
 *
 * @tparam ...Args Arguments type for the given matcher
 */
template <class... Args>
using AnyMatcher = details::AnyMatcherImpl<details::Throws, Args...>;

/**
 * @brief Unsafe AnyMatcher implementation that assert() when uninitialized
 *
 * See details::AnyMatcherImpl for more details.
 *
 * @note If compiled in release (i.e. with NDEBUG) it always returns false
 *
 * @tparam ...Args Arguments type for the given matcher
 */
template <class... Args>
using UnsafeAnyMatcher = details::AnyMatcherImpl<details::Asserts, Args...>;

}  // namespace atb
