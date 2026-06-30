#pragma once

#include <algorithm>  // std::copy_n
#include <cstddef>    // std::size_t
#include <initializer_list>
#include <limits>  // std::numeric_limits
#include <optional>
#include <string>
#include <string_view>

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
 *  @param[in] strings List of strings we wish to copy
 *  @param[inout] d_str Destination string we wish to append into
 *
 *  @return The number of bytes added to str when successfull. Otherwise
 *  std::nullopt if the operation would overflows.
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call at the cost of
 *        iterating over the strings 2 times
 */
inline auto StrAppend(std::initializer_list<std::string_view> strings,
                      std::string& d_str) -> std::optional<std::size_t> {
  const std::size_t old_size = d_str.size();
  const std::size_t added = StrSize(strings);

  std::optional<std::size_t> res = std::nullopt;

  // std::size_t overflows
  if (old_size > std::numeric_limits<std::size_t>::max() - added) {
    return res;
  }

  const std::size_t new_size = (old_size + added);

  // std::string mem overflows
  if (new_size > d_str.max_size()) {
    return res;
  }

  d_str.resize(new_size);
  StrCopyUnsafe(std::move(strings), std::addressof(d_str[old_size]));

  res = added;
  return res;
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
  std::optional<std::string> res = std::nullopt;

  std::string str;
  if (StrAppend(strings, str)) res = std::move(str);

  return res;
}

}  // namespace atb
