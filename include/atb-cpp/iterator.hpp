#pragma once

#include <iterator>  // advance/next
#include <optional>
#include <utility>  //pair

namespace atb {

/**
 *  @return A slice of [first, last) starting at `first + begin`, of size
 *          `new_size`
 *
 *  @param[in] first, last A range of iterators
 *  @param[in] begin Offset of the new range
 *  @param[in] new_size Optionally, the new size of the slice
 *
 *  @note This function is safe i.e. begin and new_size are clamped accordingly
 *        to the initial [first, last) range and any wrong iterator range
 *        (`last < first`) returns [first, last) unchanged
 */
template <class It>
constexpr auto Slice(It first, It last, std::size_t begin,
                     std::optional<std::size_t> new_size =
                         std::nullopt) noexcept -> std::pair<It, It> {
  const auto dist = std::distance(first, last);
  if (dist > 0) {
    const auto initial_size = static_cast<std::size_t>(dist);

    // Clamp begin to the initial_size
    begin = std::min(begin, initial_size);
    std::advance(first, begin);

    if (new_size.has_value() and (first != last)) {
      const auto remaining = (initial_size - begin);
      last = std::next(first, std::min(*new_size, remaining));
    }
  }

  return std::make_pair(first, last);
}

}  // namespace atb
