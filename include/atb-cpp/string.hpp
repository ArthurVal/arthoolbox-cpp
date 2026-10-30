#pragma once

#include <algorithm>  // std::copy_n
#include <cstddef>    // std::size_t
#include <initializer_list>
#include <limits>  // std::numeric_limits
#include <optional>
#include <string>
#include <string_view>

#include "atb-cpp/matchers/string.hpp"

namespace atb {

/**
 *  @return The accumulated size of all strings contained within \a strings
 *  @param[in] strings List of strings we wish compute the size of
 */
constexpr auto StrSize(std::initializer_list<std::string_view> strings) noexcept
    -> std::size_t {
  std::size_t size = 0;
  for (auto str : strings) size += str.size();
  return size;
}

/**
 *  @brief Copy the range of \a strings into \a d_first
 *
 *  @param[in] strings List of strings we wish to copy
 *  @param[in] d_first The destination char buffer
 *  @param[in] d_size The destination char buffer size
 *  @param[in] crop Indicates if we want (or not) to crop the last string
 *                  written into \a d_first when the destination buffer would
 *                  overflows. Default to false (no crop).
 *  @return One past the last byte written into
 */
constexpr auto StrCopy(std::initializer_list<std::string_view> strings,
                       char* d_first, std::size_t d_size,
                       bool crop = false) noexcept -> char* {
  for (auto str : strings) {
    if (d_size >= str.size()) {
      d_first = std::copy_n(str.data(), str.size(), d_first);
      d_size -= str.size();
    } else {
      if (crop && (d_size > 0)) {
        d_first = std::copy_n(str.data(), d_size, d_first);
        d_size = 0;
      }
      break;
    }
  }

  return d_first;
}

/**
 *  @brief Copy the range of \a strings into \a d_first
 *
 *  Same as StrCopy but unsafe (no checks for input buffer range).
 *
 *  @param[in] strings List of strings we wish to copy
 *  @param[in] d_first The destination char buffer

 *  @return One past the last byte written into
 */
constexpr auto StrCopyUnsafe(std::initializer_list<std::string_view> strings,
                             char* d_first) noexcept -> char* {
  for (auto str : strings) {
    d_first = std::copy_n(str.data(), str.size(), d_first);
  }
  return d_first;
}

/**
 *  @brief Append the list of \a strings into \a d_str by doing only one resize.
 *
 *  @important This function is safe againt self appending d_str to itself (i.e.
 *             \a strings can contain a ref to \a d_str)
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call at the cost of
 *        iterating over the strings 2 times
 *
 *  @param[in] strings List of strings we wish to copy
 *  @param[inout] d_str Destination string we wish to append into
 *
 *  @return The number of bytes added to str when successfull. Otherwise
 *          std::nullopt if the operation would overflows.
 */
inline auto StrAppend(std::initializer_list<std::string_view> strings,
                      std::string& d_str) -> std::optional<std::size_t> {
  const std::size_t old_size = d_str.size();
  const std::size_t added = StrSize(strings);

  // std::size_t overflows
  if (old_size > std::numeric_limits<std::size_t>::max() - added) {
    return std::nullopt;
  }

  const std::size_t new_size = (old_size + added);

  // std::string mem overflows
  if (new_size > d_str.max_size()) {
    return std::nullopt;
  }

  // Store initial mem range in order to protect against self assignment
  const auto base_begin = d_str.data();
  const auto base_end = base_begin + old_size;

  d_str.resize(new_size);

  // Do the copy manually (we have to check if the input strings contain a ref
  // to d_str)
  auto d_str_first = std::addressof(d_str[old_size]);
  for (auto str : strings) {
    // Protection against self assignments
    if ((base_begin <= str.data()) && (str.data() < base_end)) {
      const auto offset = static_cast<std::size_t>(str.data() - base_begin);
      d_str_first = std::copy_n(d_str.data() + offset, str.size(), d_str_first);
    } else {
      d_str_first = std::copy_n(str.data(), str.size(), d_str_first);
    }
  }

  return added;
}

/**
 *  @brief Append the list of \a strings into \a d_str by doing only one resize.
 *
 *  @important This function is NOT SAFE against self appending. \a strings MUST
 *             NOT contain a ref to d_str
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call at the cost of
 *        iterating over the strings 2 times
 *
 *  @param[in] strings List of strings we wish to copy
 *  @param[inout] d_str Destination string we wish to append into
 *
 *  @return The number of bytes added to str when successfull. Otherwise
 *          std::nullopt if the operation would overflows.
 */
inline auto StrAppendUnsafe(std::initializer_list<std::string_view> strings,
                            std::string& d_str) -> std::optional<std::size_t> {
  const std::size_t old_size = d_str.size();
  const std::size_t added = StrSize(strings);

  // std::size_t overflows
  if (old_size > std::numeric_limits<std::size_t>::max() - added) {
    return std::nullopt;
  }

  const std::size_t new_size = (old_size + added);

  // std::string mem overflows
  if (new_size > d_str.max_size()) {
    return std::nullopt;
  }

  d_str.resize(new_size);
  StrCopyUnsafe(strings, std::addressof(d_str[old_size]));

  return added;
}

/**
 *  @return std::optional<std::string> Containing the concatenation of all
 *          strings on sucess. std::nullopt when the concatenation of all
 *          strings would overflows.
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call at the cost of
 *        iterating over the strings 2 times
 */
inline auto StrCat(std::initializer_list<std::string_view> strings)
    -> std::optional<std::string> {
  std::string str;
  if (StrAppendUnsafe(strings, str)) return str;
  return std::nullopt;
}

/**
 * @brief Switch construct for string like object
 *
 * Highly inspired by LLVM' StringSwich<> class
 * (https://llvm.org/doxygen/classllvm_1_1StringSwitch.html) but use user
 * defined 'matchers' instead in order to be more versatile and customizable.
 *
 * This switch return a value of type \p R for the first Cases that matches.
 * For example:
 *
 * @code{c++}
 * int value = StrSwitch<int>(argv[0])
 *               .Case("Foo", 0)
 *               .Case(StartsWith("Bar"), 1)
 *               .Case(Contains("baz"), 2)
 *               .Case(Eq("foo"), 3)
 *               .Case(AllOf(Ne("bar"), Ne("foo"), Ne("baz")), 4)
 *               .Default(-1);
 * @endcode
 *
 * The `.Default()` state is MANDATORY. Not ending with it will result in a
 * compilation error.
 *
 * @tparam R Type of the value returned by the switch statement
 */
template <class R>
struct StrSwitch final {
  /// Not copyable/movable
  constexpr StrSwitch(const StrSwitch&) = delete;
  constexpr StrSwitch(StrSwitch&&) = delete;
  constexpr auto operator=(const StrSwitch&) -> StrSwitch& = delete;
  constexpr auto operator=(StrSwitch&&) -> StrSwitch& = delete;

  constexpr StrSwitch(std::string_view str) : m_str(str), m_res(std::nullopt) {}

  template <MatcherFor<std::string_view> Matcher, class T>
  constexpr auto Case(Matcher&& m, T&& value) -> StrSwitch& {
    if (!m_res.has_value()) {
      if (atb::IsMatching(std::forward<Matcher>(m), m_str)) {
        m_res.emplace(std::forward<T>(value));
      }
    }

    return *this;
  }

  template <std::equality_comparable_with<std::string_view> StrLike, class T>
  constexpr auto Case(StrLike&& str, T&& value) -> StrSwitch& {
    if (!m_res.has_value()) {
      if (ATB_FWD(str) == m_str) {
        m_res.emplace(ATB_FWD(value));
      }
    }

    return *this;
  }

  template <class T>
  [[nodiscard]] constexpr auto Default(T&& value) const -> R {
    return m_res.value_or(std::forward<T>(value));
  }

  template <class T = R>
  constexpr operator T() const noexcept {
    static_assert(sizeof(T) == 0, "Missing `.Default()` switch statement");
    return std::move(*m_res);
  }

 private:
  const std::string_view m_str;
  std::optional<R> m_res;
};

}  // namespace atb
