#pragma once

#include <algorithm>  // std::copy_n
#include <cassert>    // assert
#include <cstdint>    // std::uintmax_t
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

namespace atb {

/**
 *  @return The accumulated size of all strings contained within \a strings
 *  @param[in] strings List of strings we wish compute the size of
 */
constexpr auto StrSize(std::initializer_list<std::string_view> strings) noexcept
    -> std::uintmax_t {
  std::uintmax_t size = 0;
  for (auto str : strings) size += str.size();
  return size;
}

/**
 *   @brief Options forwarded to the StrCopy function
 */
struct StrCopy_Opt {
  /// Size of the destination range (in bytes)
  std::optional<std::size_t> d_size = std::nullopt;

  /// Indicates if, when the dest range overflows, you still want to partially
  /// copy the last string in it (i.e. dest range will contain a cropped string
  /// in it)
  bool crop = false;
};

/**
 *  @brief Copy the range of \a strings into \a d_first
 *
 *  @param[in] strings List of strings we wish to copy
 *  @param[in] d_first The destination char buffer
 *  @param[in] opt Options use to specify the copy policy.
 *  @return One past the last byte written into
 */
constexpr auto StrCopy(std::initializer_list<std::string_view> strings,
                       char* d_first, StrCopy_Opt opt = {}) noexcept -> char* {
  if (opt.d_size.has_value()) {
    // Safe copy
    std::size_t d_size = std::move(*opt.d_size);
    for (auto str : strings) {
      if (d_size >= str.size()) {
        d_first = std::copy_n(str.data(), str.size(), d_first);
        d_size -= str.size();
      } else {
        if (opt.crop && (d_size > 0)) {
          d_first = std::copy_n(str.data(), d_size, d_first);
          d_size = 0;
        }
        break;
      }
    }
  } else {
    // Unsafe Copy
    for (auto str : strings) {
      d_first = std::copy_n(str.data(), str.size(), d_first);
    }
  }

  return d_first;
}

/**
 *  @brief Append the list of strings into \a d_str by doing only one resize.
 *
 *  @param[in] strings List of strings we wish to copy
 *  @param[out] d_str Destination string we wish to append into
 *
 *  @pre The number of bytes contained within strings fit into out
 *       (StrSize(strings) <= out.max_size())
 *
 *  @return The number of bytes added to str when successfull
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call at the cost of
 *        iterating over the strings 2 times
 */
inline auto StrAppend(std::initializer_list<std::string_view> strings,
                      std::string& d_str) -> std::size_t {
  const std::uintmax_t old_size = d_str.size();
  const std::uintmax_t added = StrSize(strings);
  const std::uintmax_t new_size = old_size + added;

  assert(new_size <= d_str.max_size());

  // WARNING: bad_alloc
  d_str.resize(new_size);

  StrCopy(std::move(strings), std::addressof(d_str[old_size]));
  return static_cast<std::size_t>(added);
}

/**
 *  @return std::string Containing the concatenation of all strings
 *
 *  @pre The number of bytes contained within strings fit into out
 *       ((StrSize(strings) + out.size()) <= out.max_size()).
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call at the cost of
 *        iterating over the strings 2 times
 */
inline auto StrCat(std::initializer_list<std::string_view> strings)
    -> std::string {
  std::string str;
  StrAppend(strings, str);
  return str;
}

}  // namespace atb
