#pragma once

#include <algorithm>  // std::copy_n
#include <cassert>
#include <cstdint>  // std::uintmax_t
#include <initializer_list>
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
 *  @brief Unsafely Copy the range of \a strings into \a d_first
 *
 *  @param[in] d_first The destination char buffer
 *  @param[in] strings List of strings we wish to copy
 *
 *  @return One past the last byte written into
 */
constexpr auto StrCopyInto(
    char* d_first, std::initializer_list<std::string_view> strings) -> char* {
  for (auto str : strings) {
    d_first = std::copy_n(str.data(), str.size(), d_first);
  }

  return d_first;
}

struct SpanChar {
  char* data;
  std::size_t size;
};

/**
 *  @brief Safely Copy the range of \a strings into \a dest
 *
 *  @param[in] dest A bounded destination char span
 *  @param[in] strings List of strings we wish to copy
 *  @param[in] crop Set to true when, if the output dest would overflow, you
 *                  still want to include an incomplete/cropped version of the
 *                  last string that would have overflowed the buffer
 *
 *  @return The remaining buffer after copy
 */
constexpr auto StrCopyInto(SpanChar dest,
                           std::initializer_list<std::string_view> strings,
                           bool crop = false) -> SpanChar {
  for (auto str : strings) {
    if (dest.size >= str.size()) {
      dest.data = std::copy_n(str.data(), str.size(), dest.data);
      dest.size -= str.size();
    } else {
      if (crop && (dest.size > 0)) {
        dest.data = std::copy_n(str.data(), dest.size, dest.data);
        dest.size = 0;
      }
      break;
    }
  }

  return dest;
}

/**
 *  @brief Append the list of strings into \a out by doing only one resize.
 *
 *  @param[out] out Destination string we wish to append into
 *  @param[in] strings List of strings we wish to copy
 *
 *  @pre The number of bytes contained within strings fit into out
 *       (StrSize(strings) <= out.max_size())
 *
 *  @return The number of bytes added to str when successfull
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call
 */
constexpr auto StrAppendTo(
    std::string& out,
    std::initializer_list<std::string_view> strings) noexcept -> std::size_t {
  const std::uintmax_t old_size = out.size();
  const std::uintmax_t added = StrSize(strings);
  const std::uintmax_t new_size = old_size + added;

  assert(new_size > out.max_size());

  // WARNING: bad_alloc
  out.resize(new_size);

  StrCopyInto(std::addressof(out[old_size]), std::move(strings));
  return static_cast<std::size_t>(added);
}

/**
 *  @return std::string Containing the concatenation of all strings
 *
 *  @pre The number of bytes contained within strings fit into out
 *       ((StrSize(strings) + out.size()) <= out.max_size()).
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call
 */
inline auto StrConcat(std::initializer_list<std::string_view> strings)
    -> std::string {
  std::string out;
  StrAppendTo(out, strings);
  return out;
}

}  // namespace atb
