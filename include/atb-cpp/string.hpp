#pragma once

#include <algorithm>   // std::copy_n
#include <cstdint>     // std::uintmax_t
#include <functional>  // std::invoke
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

/// Namespace providing some default copy's policies for StrCopyInto
namespace cpy {

/// Do not do any bound checks
struct Unsafely {
  constexpr auto operator()(std::string_view str,
                            char* d_first) const noexcept -> char* {
    return std::copy_n(str.data(), str.size(), d_first);
  };
};

/**
 *  @return A CopyPolicy that limits the number of bytes written into
 *          d_first
 *
 *  @param[in] size Maximum number of bytes written into d_first
 *  @param[in] crop_list_string When true, partially copy the last string that
 *                              would have overflow the destination range up to
 *                              the max size, otherwise stop before writing the
 *                              last string
 */
constexpr auto FillUpTo(std::size_t size,
                        bool crop_last_string = false) noexcept {
  return
      [size, crop_last_string](std::string_view str,
                               char* d_first) mutable -> std::optional<char*> {
        std::optional<char*> res = std::nullopt;

        if (size >= str.size()) {
          size -= str.size();
          res = std::copy_n(str.data(), str.size(), d_first);
        } else if (crop_last_string && size > 0) {
          size = 0;
          res = std::copy_n(str.data(), size, d_first);
        }

        return res;
      };
}

}  // namespace cpy

/**
 *  @brief Copy the range of \a strings into \a d_first
 *
 *  @param[in] d_first Begin of the destination range
 *  @param[in] strings List of strings we wish to copy
 *  @param[in] do_copy Policy defining the way we copy the strings into
 *                     d_first (unsafely, stop at N bytes, etc...) (default to
 *                     cpy::Unsafely(), i.e. copy all strings)
 *
 *  @return One past the last byte written into
 */
template <class CopyPolicyType = cpy::Unsafely>
constexpr auto StrCopyInto(
    char* d_first, std::initializer_list<std::string_view> strings,
    CopyPolicyType&& do_copy = CopyPolicyType{}) -> char* {
  for (auto str : strings) {
    if (std::optional<char*> res = std::invoke(do_copy, str, d_first)) {
      d_first = std::move(*res);
    } else {
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
