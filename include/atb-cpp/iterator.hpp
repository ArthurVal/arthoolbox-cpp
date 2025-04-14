#pragma once

#include <cassert>
#include <iterator>  // iterator_traits
#include <optional>
#include <utility>  //pair

namespace atb {

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
