#pragma once

#include <utility>

namespace atb {

/**
 * @brief Generic RAII object that can be construct with any callable and will
 *        call it whenever the ScopeExit goes out of scope (i.e. is destroyed).
 *
 * @tparam Callable Type of the underlying callable
 */
template <class Callable>
struct [[nodiscard]] ScopeExit final {
  constexpr ScopeExit() = delete;
  constexpr ScopeExit(const ScopeExit&) = delete;
  constexpr auto operator=(const ScopeExit&) -> ScopeExit& = delete;
  constexpr auto operator=(ScopeExit&& other) -> ScopeExit& = delete;

  /// Move construct from \a other (other will be aborted)
  constexpr ScopeExit(ScopeExit&& other) noexcept
      : m_fn(std::move(other.m_fn)), m_aborted(other.m_aborted) {
    other.Abort();
  }

  /// Construct a ScopeExit from the fiven callable
  template <class F>
  constexpr explicit ScopeExit(F&& f)
      : m_fn(std::forward<F>(f)), m_aborted(false) {}

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
      m_fn();
    }
  }

 private:
  Callable m_fn;
  bool m_aborted;
};

/// CTAD for ScopeExit
template <class F>
ScopeExit(F&&) -> ScopeExit<std::decay_t<F>>;

}  // namespace atb
