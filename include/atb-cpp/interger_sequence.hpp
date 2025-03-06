#pragma once

#include <utility>  // interger_sequence

namespace atb {

/**
 *  @return A new std::integer_sequence<T> offseted by Offset
 */
template <class T, T Offset, T... Ints>
constexpr auto OffsetSequence(std::integer_sequence<T, Ints...>) noexcept {
  return std::integer_sequence<T, (Offset + Ints)...>{};
}

/**
 *  @return An std::integer_sequence<> representing a continous range of Size
 *          ints T, starting at Begin
 */
template <class T, T Size, T Begin = T{}>
constexpr auto MakeSequence() noexcept {
  return OffsetSequence<T, Begin>(std::make_integer_sequence<T, Size>{});
}

/**
 *  @return An std::index_sequence<> representing a continous range of
 *          Size std::size_t, starting at Begin
 */
template <std::size_t Size, std::size_t Begin = 0>
constexpr auto MakeIndexSequence() noexcept {
  return MakeSequence<std::size_t, Size, Begin>();
}

}  // namespace atb
