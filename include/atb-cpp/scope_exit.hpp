#pragma once

#include <type_traits>
#include <utility>

namespace utils {

template <typename Callable>
struct [[nodiscard]] ScopeExit final {
  constexpr ScopeExit() = delete;

  constexpr ScopeExit(const ScopeExit&) = delete;
  constexpr auto operator=(const ScopeExit&) -> ScopeExit& = delete;

  constexpr ScopeExit(ScopeExit&& other) : m_exit(std::move(other.m_exit)) {
    other.Abort();
  }

  constexpr auto operator=(ScopeExit&& other) -> ScopeExit& {
    m_exit = std::move(other.m_exit);
    other.Abort();
    return *this;
  }

  template <typename F>
  constexpr explicit ScopeExit(F&& f) : m_exit(std::forward<F>(f)) {}

  ~ScopeExit() { Execute(); }

  constexpr auto Abort() noexcept -> void { m_aborted = true; }

  [[nodiscard]] constexpr auto IsAborted() const noexcept -> bool {
    return m_aborted;
  }

  constexpr auto Execute() const -> void {
    if (not IsAborted()) {
      m_exit();
    }
  }

 private:
  Callable m_exit;
  bool m_aborted = false;
};

template <typename Callable>
constexpr auto WhenScopeExitDo(Callable&& f) {
  return ScopeExit<std::remove_reference_t<std::remove_cv_t<Callable>>>{
      std::forward<Callable>(f),
  };
}

}  // namespace utils
