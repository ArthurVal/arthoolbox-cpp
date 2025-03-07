#include <string_view>

#include "atb-cpp/matchers.hpp"
#include "gtest/gtest.h"
#include "tests/mocks/callable.hpp"
#include "tests/mocks/comparable.hpp"

using namespace std::literals::string_view_literals;

using tests::ArgSide;
using tests::CallableMock;
using tests::ComparableMock;

namespace atb::matchers {
namespace {

TEST(TestMatchers, Always) {
  // We can't unfold parameter packs with EXPECT_ macro, hence we encapsulate
  // inside functions
  constexpr auto ExpectTrue = [](auto&& v) {
    EXPECT_TRUE(IsMatching(Always<true>(), std::forward<decltype(v)>(v)));
  };

  constexpr auto ExpectFalse = [](auto&& v) {
    EXPECT_FALSE(IsMatching(Always<false>(), std::forward<decltype(v)>(v)));
  };

  std::apply(
      [&](auto&&... v) {
        (ExpectTrue(v), ...);
        (ExpectFalse(std::forward<decltype(v)>(v)), ...);
      },
      std::forward_as_tuple(1, "Foo", 3985809135u, 124.546654));
}

TEST(TestMatchers, Eq) {
  ::testing::StrictMock<tests::EqMock<bool, int>> mock;

  using testing::Return;
  EXPECT_CALL(mock, Eq(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Eq(3), mock));

  EXPECT_CALL(mock, Eq(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(Eq(4), mock));

  EXPECT_CALL(mock, Eq(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(atb::matchers::Eq(mock), 5));

  EXPECT_CALL(mock, Eq(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(atb::matchers::Eq(mock), 6));
}

TEST(TestMatchers, Ne) {
  ::testing::StrictMock<tests::NeMock<bool, int>> mock;

  using testing::Return;
  EXPECT_CALL(mock, Ne(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Ne(3), mock));

  EXPECT_CALL(mock, Ne(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(Ne(4), mock));

  EXPECT_CALL(mock, Ne(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(atb::matchers::Ne(mock), 5));

  EXPECT_CALL(mock, Ne(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(atb::matchers::Ne(mock), 6));
}

TEST(TestMatchers, Ge) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Ge(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Ge(3)(mock));
  EXPECT_FALSE(Ge(3)(mock));

  EXPECT_CALL(mock, Ge(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::matchers::Ge(mock)(4));
  EXPECT_FALSE(atb::matchers::Ge(mock)(4));
}

TEST(TestMatchers, Gt) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Gt(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Gt(3), mock));
  EXPECT_FALSE(IsMatching(Gt(3), mock));

  EXPECT_CALL(mock, Gt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(atb::matchers::Gt(mock), 4));
  EXPECT_FALSE(IsMatching(atb::matchers::Gt(mock), 4));
}

TEST(TestMatchers, Le) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Le(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Le(3), mock));
  EXPECT_FALSE(IsMatching(Le(3), mock));

  EXPECT_CALL(mock, Le(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(atb::matchers::Le(mock), 4));
  EXPECT_FALSE(IsMatching(atb::matchers::Le(mock), 4));
}

TEST(TestMatchers, Lt) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Lt(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Lt(3), mock));
  EXPECT_FALSE(IsMatching(Lt(3), mock));

  EXPECT_CALL(mock, Lt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(atb::matchers::Lt(mock), 4));
  EXPECT_FALSE(IsMatching(atb::matchers::Lt(mock), 4));
}

TEST(TestMatchers, Near) {
  // TODO: Use mocks (Arithmetic + Comparable) instead
  EXPECT_TRUE(Near(3.14)(3.14));
  EXPECT_FALSE(Near(3.14)(3.15));

  EXPECT_TRUE(Near(3.14, 1.)(3.15));
  EXPECT_TRUE(Near(3.14, 1.)(4.14));
  EXPECT_FALSE(Near(3.14, 1.)(5.14));
}

TEST(TestMatchers, Not) {
  ::testing::StrictMock<CallableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Call(3))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(atb::matchers::Not(mock), 3));
  EXPECT_TRUE(IsMatching(atb::matchers::Not(mock), 3));
}

TEST(TestMatchers, All) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Call("Foo"))
      .Times(4)
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(All(mock, mock, mock, mock), "Foo"sv));

  EXPECT_CALL(mock, Call("Bar"))
      .Times(3)
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(All(mock, mock, mock, mock), "Bar"sv));

  EXPECT_CALL(mock, Call("Baz"))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(All(mock, mock, mock, mock), "Baz"sv));
}

TEST(TestMatchers, Any) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Call("Foo"))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Any(mock, mock, mock, mock), "Foo"sv));

  EXPECT_CALL(mock, Call("Bar"))
      .Times(3)
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(IsMatching(Any(mock, mock, mock, mock), "Bar"sv));

  EXPECT_CALL(mock, Call("Baz"))
      .Times(4)
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(Any(mock, mock, mock, mock), "Baz"sv));
}

TEST(TestMatchers, OnArgs) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Call("Foo"))
      .Times(3)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .WillOnce(Return(true))
      .RetiresOnSaturation();

  EXPECT_TRUE(IsMatching(OnArg<0>(mock), "Foo"sv, 1, 4, "Bar"sv));
  EXPECT_FALSE(IsMatching(OnArg<3>(mock), 1, 4, "Bar"sv, "Foo"sv));
  EXPECT_TRUE(IsMatching(OnArg<1>(mock), 1, "Foo"sv, 4, "Bar"sv));
}

TEST(TestMatchers, AllArgs) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_CALL(mock, Call("Bar"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_TRUE(IsMatching(AllArgs(mock), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_CALL(mock, Call("Bar"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(IsMatching(AllArgs(mock), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(IsMatching(AllArgs(mock), "Foo"sv, "Bar"sv));
  }
}

TEST(TestMatchers, AnyArgs) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_TRUE(IsMatching(AnyArgs(mock), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_CALL(mock, Call("Bar"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_TRUE(IsMatching(AnyArgs(mock), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_CALL(mock, Call("Bar"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(IsMatching(AnyArgs(mock), "Foo"sv, "Bar"sv));
  }
}

}  // namespace

}  // namespace atb::matchers
