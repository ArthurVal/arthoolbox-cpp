#pragma once

#include <cassert>
#include <iterator>  // iterator_traits
#include <optional>

#include "atb-cpp/matchers.hpp"

namespace atb {

template <class It>
struct Range;

namespace details {

template <class T>
struct IsRange : std::false_type {};

template <class It>
struct IsRange<Range<It>> : std::true_type {};

template <class T>
constexpr bool IsRange_v = IsRange<T>::value;

}  // namespace details

template <class It>
struct Range {
  It first;
  It last;
};

template <class It>
constexpr auto MakeRange(It begin, It end) noexcept -> Range<It> {
  return {begin, end};
}

template <class It>
constexpr auto MakeRange(It begin, std::size_t n) noexcept -> Range<It> {
  return {begin, std::next(begin, n)};
}

template <class Container>
constexpr auto MakeRange(const Container& c) noexcept {
  return Range{std::cbegin(c), std::cend(c)};
}

template <class Container>
constexpr auto MakeRange(Container& c) noexcept {
  return Range{std::begin(c), std::end(c)};
}

template <class It>
constexpr auto begin(Range<It> range) noexcept -> It {
  return range.first;
}

template <class It>
constexpr auto end(Range<It> range) noexcept -> It {
  return range.last;
}

template <class It>
constexpr auto rbegin(Range<It> range) noexcept -> std::reverse_iterator<It> {
  return range.end;
}

template <class It>
constexpr auto rend(Range<It> range) noexcept -> std::reverse_iterator<It> {
  return range.begin;
}

template <class It>
constexpr auto size(Range<It> range) noexcept ->
    typename std::iterator_traits<It>::difference_type {
  return std::distance(range.first, range.last);
}

template <class It>
constexpr auto IsValid(Range<It> range) noexcept -> bool {
  return (size(range) >= 0);
}

template <class It, class Matcher>
constexpr auto Slice(Range<It> range, std::size_t start,
                     std::optional<std::size_t> size = std::nullopt) noexcept
    -> Range<It> {
  auto current_size = size(range);
  assert(current_size > 0);

  // Clamp start to the current size
  start = std::min(start, current_size);
  std::advance(range.first, start);

  if (size.has_value() and (current_size > 0)) {
    current_size -= start;  // Size shrinked
    range = MakeRange(range.first, std::min(*size, current_size));
  }

  return range;
}

template <class It, class Matcher>
constexpr auto ShrinkUntil(Matcher&& m, Range<It> range) noexcept -> Range<It> {
  assert(IsValid(range));

  for (; range.first != range.last; ++range.first) {
    const auto& value = *range.first;
    if (matchers::Invoke(m, value)) break;
  }

  return range;
}

template <class It>
constexpr auto Reverse(Range<It> range) noexcept {
  return MakeRange(rbegin(range), rend(range));
}

}  // namespace atb
