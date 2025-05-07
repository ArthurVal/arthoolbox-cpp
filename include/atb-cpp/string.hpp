#pragma once

#include <algorithm>  // std::copy_n
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

struct BoundedBuffer {
  char* d_first;
  std::size_t size;

  bool crop_last_string = false;
};

/**
 *  @brief Safely Copy the range of \a strings into \a dest
 *
 *  @param[in] dest A bound char buffer
 *  @param[in] strings List of strings we wish to copy
 *
 *  @return One past the last byte written into
 */
constexpr auto StrCopyInto(BoundedBuffer dest,
                           std::initializer_list<std::string_view> strings)
    -> char* {
  auto [d_first, remaining, crop] = dest;

  for (auto str : strings) {
    if (remaining >= str.size()) {
      d_first = std::copy_n(str.data(), str.size(), d_first);
      remaining -= str.size();
    } else {
      if (crop && (remaining > 0)) {
        d_first = std::copy_n(str.data(), remaining, d_first);
      }
      break;
    }
  }

  return d_first;
}

/**
 *  @brief Append the list of strings into \a out by doing only one resize.
 *
 *  @param[out] out Destination string we wish to append into
 *  @param[in] strings List of strings we wish to copy
 *
 *  @return The number of bytes added to str when successfull. Otherwise
 *          nullopt when the new computed size is bigger than `.max_size()`
 */
constexpr auto StrAppendTo(std::string& out,
                           std::initializer_list<std::string_view>
                               strings) noexcept -> std::optional<std::size_t> {
  const std::uintmax_t old_size = out.size();
  const std::uintmax_t added = StrSize(strings);
  const std::uintmax_t new_size = old_size + added;

  if (new_size > out.max_size()) return std::nullopt;

  // WARNING: bad_alloc
  out.resize(new_size);

  StrCopyInto(std::addressof(out[old_size]), std::move(strings));
  return static_cast<std::size_t>(added);
}

/**
 *  @return std::string Containing the concatenating of the range of
 *          strings when successfull. Otherwise nullopt when the new computed
 *          size is bigger than `.max_size()`
 *
 *  @note Optimize the 'resize()' operation by doing it once instead of doing
 *        it for each new '.append()' or 'operator+=()' call
 */
inline auto StrConcat(std::initializer_list<std::string_view> strings)
    -> std::optional<std::string> {
  std::string out;
  if (StrAppendTo(out, strings)) {
    return out;
  } else {
    return std::nullopt;
  }
}

}  // namespace atb
