#include <string_view>
using namespace std::literals::string_view_literals;

#include "tests/mocks/callable.hpp"
#include "tests/mocks/comparable.hpp"
using tests::ArgSide;
using tests::CallableMock;
using tests::ComparableMock;

#include "atb-cpp/matchers.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace atb::matchers {
namespace {

template <class R, class... Args>
struct MatcherFreeFunctionMock : CallableMock<R, Args...> {
  using return_t = typename CallableMock<R, Args...>::return_t;
  using arg_t = typename CallableMock<R, Args...>::arg_t;

  MOCK_METHOD(return_t, FreeFunctionCall, (arg_t), (const));
  friend constexpr auto IsMatching(const MatcherFreeFunctionMock& mock,
                                   Args... args) -> return_t {
    if constexpr (sizeof...(Args) == 0) {
      return mock.FreeFunctionCall();
    } else if constexpr (sizeof...(Args) == 1) {
      return mock.FreeFunctionCall(std::move(args)...);
    } else {
      return mock.FreeFunctionCall(std::make_tuple(std::move(args)...));
    }
  }
};

template <class R, class... Args>
struct MatcherMethodMock : MatcherFreeFunctionMock<R, Args...> {
  using return_t = typename CallableMock<R, Args...>::return_t;
  using arg_t = typename CallableMock<R, Args...>::arg_t;

  MOCK_METHOD(return_t, MethodCall, (arg_t), (const));
  constexpr auto IsMatching(Args... args) const -> return_t {
    if constexpr (sizeof...(Args) == 0) {
      return MethodCall();
    } else if constexpr (sizeof...(Args) == 1) {
      return MethodCall(std::move(args)...);
    } else {
      return MethodCall(std::make_tuple(std::move(args)...));
    }
  }
};

TEST(TestMatchers, Invoke) {
  using ::testing::Return;

  ::testing::StrictMock<CallableMock<bool, int>> only_call;
  EXPECT_CALL(only_call, Call(3)).Times(1).WillOnce(Return(true));
  EXPECT_TRUE(Invoke(only_call, 3));

  ::testing::StrictMock<MatcherFreeFunctionMock<bool, int>>
      call_and_free_function;
  EXPECT_CALL(call_and_free_function, FreeFunctionCall(4))
      .Times(1)
      .WillOnce(Return(false));
  EXPECT_FALSE(Invoke(call_and_free_function, 4));

  ::testing::StrictMock<MatcherMethodMock<bool, int>>
      call_and_free_function_and_method;
  EXPECT_CALL(call_and_free_function_and_method, MethodCall(5))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false));
  EXPECT_TRUE(Invoke(call_and_free_function_and_method, 5));
  EXPECT_FALSE(Invoke(&MatcherMethodMock<bool, int>::IsMatching,
                      call_and_free_function_and_method, 5));
}

TEST(TestMatchers, Always) {
  // We can't unfold parameter packs with EXPECT_ macro, hence we encapsulate
  // inside functions
  constexpr auto ExpectTrue = [](auto&& v) {
    EXPECT_TRUE(Invoke(Always<true>(), std::forward<decltype(v)>(v)));
  };

  constexpr auto ExpectFalse = [](auto&& v) {
    EXPECT_FALSE(Invoke(Always<false>(), std::forward<decltype(v)>(v)));
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
  EXPECT_TRUE(Invoke(Eq(3), mock));

  EXPECT_CALL(mock, Eq(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(Eq(4), mock));

  EXPECT_CALL(mock, Eq(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(atb::matchers::Eq(mock), 5));

  EXPECT_CALL(mock, Eq(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(atb::matchers::Eq(mock), 6));
}

TEST(TestMatchers, Ne) {
  ::testing::StrictMock<tests::NeMock<bool, int>> mock;

  using testing::Return;
  EXPECT_CALL(mock, Ne(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(Ne(3), mock));

  EXPECT_CALL(mock, Ne(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(Ne(4), mock));

  EXPECT_CALL(mock, Ne(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(atb::matchers::Ne(mock), 5));

  EXPECT_CALL(mock, Ne(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(atb::matchers::Ne(mock), 6));
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
  EXPECT_TRUE(Invoke(Gt(3), mock));
  EXPECT_FALSE(Invoke(Gt(3), mock));

  EXPECT_CALL(mock, Gt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(atb::matchers::Gt(mock), 4));
  EXPECT_FALSE(Invoke(atb::matchers::Gt(mock), 4));
}

TEST(TestMatchers, Le) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Le(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(Le(3), mock));
  EXPECT_FALSE(Invoke(Le(3), mock));

  EXPECT_CALL(mock, Le(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(atb::matchers::Le(mock), 4));
  EXPECT_FALSE(Invoke(atb::matchers::Le(mock), 4));
}

TEST(TestMatchers, Lt) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Lt(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(Lt(3), mock));
  EXPECT_FALSE(Invoke(Lt(3), mock));

  EXPECT_CALL(mock, Lt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(atb::matchers::Lt(mock), 4));
  EXPECT_FALSE(Invoke(atb::matchers::Lt(mock), 4));
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
  EXPECT_FALSE(Invoke(atb::matchers::Not(mock), 3));
  EXPECT_TRUE(Invoke(atb::matchers::Not(mock), 3));
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
  EXPECT_TRUE(Invoke(All(mock, mock, mock, mock), "Foo"sv));

  EXPECT_CALL(mock, Call("Bar"))
      .Times(3)
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(All(mock, mock, mock, mock), "Bar"sv));

  EXPECT_CALL(mock, Call("Baz"))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(All(mock, mock, mock, mock), "Baz"sv));
}

TEST(TestMatchers, Any) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Call("Foo"))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(Any(mock, mock, mock, mock), "Foo"sv));

  EXPECT_CALL(mock, Call("Bar"))
      .Times(3)
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(Invoke(Any(mock, mock, mock, mock), "Bar"sv));

  EXPECT_CALL(mock, Call("Baz"))
      .Times(4)
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(Invoke(Any(mock, mock, mock, mock), "Baz"sv));
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

  EXPECT_TRUE(Invoke(OnArg<0>(mock), "Foo"sv, 1, 4, "Bar"sv));
  EXPECT_FALSE(Invoke(OnArg<3>(mock), 1, 4, "Bar"sv, "Foo"sv));
  EXPECT_TRUE(Invoke(OnArg<1>(mock), 1, "Foo"sv, 4, "Bar"sv));
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

    EXPECT_TRUE(Invoke(AllArgs(mock), "Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(Invoke(AllArgs(mock), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(Invoke(AllArgs(mock), "Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(Invoke(AnyArgs(mock), "Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(Invoke(AnyArgs(mock), "Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(Invoke(AnyArgs(mock), "Foo"sv, "Bar"sv));
  }
}

}  // namespace

}  // namespace atb::matchers
