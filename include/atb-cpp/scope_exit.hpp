#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "atb-cpp/reference_wrapper.hpp"

namespace atb {

/**
 * @brief Generic RAII object that can be construct with any callable/args and
 *        will call the callable (with its associated args) whenever the
 *        ScopeExit goes how of scope (i.e. is destroyed).
 *
 * @tparam Callable Type of the underlying callable
 * @tparam ...Args Type of the arguments forwarded to the store callable
 */
template <class Callable, class... ArgsType>
struct [[nodiscard]] ScopeExit final {
  constexpr ScopeExit() = delete;
  constexpr ScopeExit(const ScopeExit&) = delete;
  constexpr auto operator=(const ScopeExit&) -> ScopeExit& = delete;
  constexpr auto operator=(ScopeExit&& other) -> ScopeExit& = delete;

  /// Move construct from \a other (other will be aborted)
  constexpr ScopeExit(ScopeExit&& other) noexcept
      : m_fn(std::move(other.m_fn)),
        m_args(std::move(other.m_args)),
        m_aborted(other.m_aborted) {
    other.Abort();
  }

  /// Construct a ScopeExit from the fiven callable
  template <class F, class... T>
  constexpr explicit ScopeExit(F&& f, T&&... args)
      : m_fn(std::forward<F>(f)),
        m_args(std::forward<T>(args)...),
        m_aborted(false) {}

  /// Destruct a ScopeExit and `.Execute()` its callable, if not aborted
  ~ScopeExit() noexcept { Execute(); }

  /**
   * @return bool True if the current Scope object is 'aborted' (i.e. will
   *         not execute the cleanup function). False otherwise.
   */
  [[nodiscard]] constexpr auto IsAborted() const noexcept -> bool {
    return m_aborted;
  }

  /**
   * @brief Abort the current ending scope function call
   */
  constexpr auto Abort() noexcept -> void { m_aborted = true; }

  /**
   * @brief Resume the ending scope function call
   */
  constexpr auto Resume() noexcept -> void { m_aborted = false; }

  /**
   * @brief Triggers the call of the underlying stored function (if not aborted)
   *        and then Abort()
   */
  constexpr auto Execute() -> void {
    if (!IsAborted()) {
      Abort();
      std::apply(m_fn, m_args);
    }
  }

 private:
  Callable m_fn;
  std::tuple<ArgsType...> m_args;
  bool m_aborted;
};

/// CTAD for ScopeExit
template <class F, class... T>
ScopeExit(F&&,
          T&&...) -> ScopeExit<std::decay_t<F>,
                               details::UnwrapRefWrapper_t<std::decay_t<T>>...>;

}  // namespace atb
