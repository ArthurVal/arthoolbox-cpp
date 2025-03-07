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
    EXPECT_TRUE(Always<true>()(std::forward<decltype(v)>(v)));
  };

  constexpr auto ExpectFalse = [](auto&& v) {
    EXPECT_FALSE(Always<false>()(std::forward<decltype(v)>(v)));
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
  auto matcher_with_mock = atb::matchers::Eq(mock);

  using testing::Return;
  EXPECT_CALL(mock, Eq(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Eq(3)(mock));

  EXPECT_CALL(mock, Eq(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Eq(4)(mock));

  EXPECT_CALL(mock, Eq(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(matcher_with_mock(5));

  EXPECT_CALL(mock, Eq(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(matcher_with_mock(6));
}

TEST(TestMatchers, Ne) {
  ::testing::StrictMock<tests::NeMock<bool, int>> mock;
  auto matcher_with_mock = atb::matchers::Ne(mock);

  using testing::Return;
  EXPECT_CALL(mock, Ne(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Ne(3)(mock));

  EXPECT_CALL(mock, Ne(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Ne(4)(mock));

  EXPECT_CALL(mock, Ne(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(matcher_with_mock(5));

  EXPECT_CALL(mock, Ne(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(matcher_with_mock(6));
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
  EXPECT_TRUE(Gt(3)(mock));
  EXPECT_FALSE(Gt(3)(mock));

  EXPECT_CALL(mock, Gt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::matchers::Gt(mock)(4));
  EXPECT_FALSE(atb::matchers::Gt(mock)(4));
}

TEST(TestMatchers, Le) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Le(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Le(3)(mock));
  EXPECT_FALSE(Le(3)(mock));

  EXPECT_CALL(mock, Le(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::matchers::Le(mock)(4));
  EXPECT_FALSE(atb::matchers::Le(mock)(4));
}

TEST(TestMatchers, Lt) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Lt(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Lt(3)(mock));
  EXPECT_FALSE(Lt(3)(mock));

  EXPECT_CALL(mock, Lt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::matchers::Lt(mock)(4));
  EXPECT_FALSE(atb::matchers::Lt(mock)(4));
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
  EXPECT_FALSE(atb::matchers::Not(mock)(3));
  EXPECT_TRUE(atb::matchers::Not(mock)(3));
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
  EXPECT_TRUE(All(mock, mock, mock, mock)("Foo"sv));

  EXPECT_CALL(mock, Call("Bar"))
      .Times(3)
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(All(mock, mock, mock, mock)("Bar"sv));

  EXPECT_CALL(mock, Call("Baz"))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(All(mock, mock, mock, mock)("Baz"sv));
}

TEST(TestMatchers, Any) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Call("Foo"))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Any(mock, mock, mock, mock)("Foo"sv));

  EXPECT_CALL(mock, Call("Bar"))
      .Times(3)
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Any(mock, mock, mock, mock)("Bar"sv));

  EXPECT_CALL(mock, Call("Baz"))
      .Times(4)
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Any(mock, mock, mock, mock)("Baz"sv));
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

  EXPECT_TRUE(OnArg<0>(mock)("Foo"sv, 1, 4, "Bar"sv));
  EXPECT_FALSE(OnArg<3>(mock)(1, 4, "Bar"sv, "Foo"sv));
  EXPECT_TRUE(OnArg<1>(mock)(1, "Foo"sv, 4, "Bar"sv));
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

    EXPECT_TRUE(AllArgs(mock)("Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(AllArgs(mock)("Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(AllArgs(mock)("Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(AnyArgs(mock)("Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(AnyArgs(mock)("Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(AnyArgs(mock)("Foo"sv, "Bar"sv));
  }
}

}  // namespace

}  // namespace atb::matchers
