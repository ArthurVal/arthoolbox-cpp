#pragma once

#include <cstddef>  // std::size_t
#include <string_view>

#include "atb-cpp/matchers.hpp"

namespace atb {

/**
 * @return true whenever the given \a str starts by \a prefix
 *
 * @important Since \a prefix is a string_view the underlying string-like
 *            referenced NEEDS to outlive the matcher lifetime (i.e. do not
 *            give a rvalue of a std::string).
 */
struct StrStartsWith {
  std::string_view prefix;

  constexpr auto operator()(std::string_view str) const noexcept -> bool {
    return (str.substr(0, prefix.size()) == prefix);
  }
};

/**
 * @return true whenever the given \a str ends by \a suffix
 *
 * @important Since \a suffix is a string_view the underlying string-like
 *            referenced NEEDS to outlive the matcher lifetime (i.e. do not
 *            give a rvalue of a std::string).
 */
struct StrEndsWith {
  std::string_view suffix;

  constexpr auto operator()(std::string_view str) const noexcept -> bool {
    return (str.size() >= suffix.size()) &&
           (str.substr(str.size() - suffix.size(), suffix.size()) == suffix);
  }
};

namespace details {

template <class F, class... Args>
constexpr auto StrContainsImpl(std::size_t* const d_where, F&& contains,
                               Args&&... args) noexcept -> bool {
  const std::size_t pos =
      std::invoke(std::forward<F>(contains), std::forward<Args>(args)...);

  const bool found = pos != std::string_view::npos;

  if ((d_where != nullptr) && found) {
    *d_where = pos;
  }

  return found;
}

}  // namespace details

/**
 * @return true whenever the given \a str contains \a pattern in it
 *
 * @important Since \a pattern is a string_view the underlying string-like
 *            referenced NEEDS to outlive the matcher lifetime (i.e. do not
 *            give a rvalue of a std::string).
 *
 * @param[in] pattern The pattern to look for
 * @param[inout] d_where Optionally a pointer to an index that will be set to
 *                       the location of the pattern found in the input str
 *
 * @note The search for the pattern is done from the beginning of the string and
 *        will hence give the location of the FIRST pattern encountered
 */
struct StrContains {
  std::string_view pattern;
  std::size_t* d_where = nullptr;

  constexpr auto operator()(std::string_view str) const noexcept -> bool {
    return details::StrContainsImpl(
        d_where, [](auto s, auto p) { return s.find(p); }, str, pattern);
  }
};

/**
 * @return true whenever the given \a str contains \a pattern in it
 *
 * @important Since \a pattern is a string_view the underlying string-like
 *            referenced NEEDS to outlive the matcher lifetime (i.e. do not
 *            give a rvalue of a std::string).
 *
 * @param[in] pattern The pattern to look for
 * @param[inout] d_where Optionally a pointer to an index that will be set to
 *                       the location of the pattern found in the input str
 *
 * @note The search for the pattern is done from the back of the string and
 *        will hence give the location of the LAST pattern encountered
 */
struct StrContainsR {
  std::string_view pattern;
  std::size_t* d_where = nullptr;

  constexpr auto operator()(std::string_view str) const noexcept -> bool {
    return details::StrContainsImpl(
        d_where, [](auto s, auto p) { return s.rfind(p); }, str, pattern);
  }
};

/**
 * @return true whenever the given \a str contains ONE of the char contained in
 *         \a pattern
 *
 * @important Since \a pattern is a string_view the underlying string-like
 *            referenced NEEDS to outlive the matcher lifetime (i.e. do not
 *            give a rvalue of a std::string).
 *
 * @param[in] pattern The list of char to look for
 * @param[inout] d_where Optionally a pointer to an index that will be set to
 *                       the location of the char found
 *
 * @note The search for the chars is done from the beginning of the string and
 *       will hence give the location of the FIRST char encountered
 */
struct StrContainsOneOf {
  std::string_view pattern;
  std::size_t* d_where = nullptr;

  constexpr auto operator()(std::string_view str) const noexcept -> bool {
    return details::StrContainsImpl(
        d_where, [](auto s, auto p) { return s.find_first_of(p); }, str,
        pattern);
  }
};

/**
 * @return true whenever the given \a str contains ONE of the char contained in
 *         \a pattern
 *
 * @important Since \a pattern is a string_view the underlying string-like
 *            referenced NEEDS to outlive the matcher lifetime (i.e. do not
 *            give a rvalue of a std::string).
 *
 * @param[in] pattern The list of char to look for
 * @param[inout] d_where Optionally a pointer to an index that will be set to
 *                       the location of the char found
 *
 * @note The search for the chars is done from the end of the string and
 *       will hence give the location of the LAST char encountered
 */
struct StrContainsOneOfR {
  std::string_view pattern;
  std::size_t* d_where = nullptr;

  constexpr auto operator()(std::string_view str) const noexcept -> bool {
    return details::StrContainsImpl(
        d_where, [](auto s, auto p) { return s.find_last_of(p); }, str,
        pattern);
  }
};

}  // namespace atb
