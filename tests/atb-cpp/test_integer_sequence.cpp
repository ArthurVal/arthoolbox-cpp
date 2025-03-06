#include "atb-cpp/interger_sequence.hpp"
#include "gtest/gtest.h"

namespace atb {
namespace {

TEST(TestIntegerSequence, MakeSequence) {
  static_assert(std::is_same_v<std::integer_sequence<int>,
                               decltype(MakeSequence<int, 0>())>);

  static_assert(std::is_same_v<std::integer_sequence<int>,
                               decltype(MakeSequence<int, 0, 0>())>);

  static_assert(std::is_same_v<std::integer_sequence<int>,
                               decltype(MakeSequence<int, 0, 314>())>);

  static_assert(std::is_same_v<std::integer_sequence<int, 0, 1, 2>,
                               decltype(MakeSequence<int, 3>())>);

  static_assert(std::is_same_v<std::integer_sequence<int, 1, 2, 3>,
                               decltype(MakeSequence<int, 3, 1>())>);

  static_assert(std::is_same_v<std::integer_sequence<int, 3, 4, 5>,
                               decltype(MakeSequence<int, 3, 3>())>);
}

TEST(TestIntegerSequence, MakeIndexSequence) {
  static_assert(std::is_same_v<std::index_sequence<>,
                               decltype(MakeIndexSequence<0, 0>())>);

  static_assert(
      std::is_same_v<std::index_sequence<>, decltype(MakeIndexSequence<0>())>);

  static_assert(std::is_same_v<std::index_sequence<>,
                               decltype(MakeIndexSequence<0, 3>())>);

  static_assert(std::is_same_v<std::make_index_sequence<4>,
                               decltype(MakeIndexSequence<4, 0>())>);

  static_assert(std::is_same_v<std::index_sequence<3, 4, 5>,
                               decltype(MakeIndexSequence<3, 3>())>);

  static_assert(std::is_same_v<std::index_sequence<3>,
                               decltype(MakeIndexSequence<1, 3>())>);
}

}  // namespace

}  // namespace atb
