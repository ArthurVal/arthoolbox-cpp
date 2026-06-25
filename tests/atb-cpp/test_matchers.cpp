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

namespace atb {

namespace {

template <class... Args>
struct MatcherFreeFunctionMock : public CallableMock<bool, const Args&...> {
  using Base = CallableMock<bool, const Args&...>;
  using typename Base::arg_t;

  MOCK_METHOD(bool, FreeFunctionCall, (arg_t), (const));
  friend constexpr auto IsMatching(const MatcherFreeFunctionMock& mock,
                                   const Args&... args) -> bool {
    if constexpr (sizeof...(Args) == 0) {
      return mock.FreeFunctionCall();
    } else if constexpr (sizeof...(Args) == 1) {
      return mock.FreeFunctionCall(args...);
    } else {
      return mock.FreeFunctionCall(arg_t{args...});
    }
  }
};

template <class... Args>
struct MatcherMethodMock : public MatcherFreeFunctionMock<Args...> {
  using Base = MatcherFreeFunctionMock<Args...>;
  using typename Base::arg_t;

  MOCK_METHOD(bool, MethodCall, (arg_t), (const));

  constexpr auto IsMatching(const Args&... args) const -> bool {
    if constexpr (sizeof...(Args) == 0) {
      return MethodCall();
    } else if constexpr (sizeof...(Args) == 1) {
      return MethodCall(args...);
    } else {
      return MethodCall(arg_t{args...});
    }
  }
};

TEST(TestMatchers, IsMatching) {
  using ::testing::Return;

  ::testing::StrictMock<CallableMock<bool, int>> only_call;
  EXPECT_CALL(only_call, Call(3)).Times(1).WillOnce(Return(true));
  EXPECT_TRUE(IsMatching(only_call, 3));

  ::testing::StrictMock<MatcherFreeFunctionMock<int>> call_and_free_function;
  EXPECT_CALL(call_and_free_function, FreeFunctionCall(4))
      .Times(1)
      .WillOnce(Return(false));
  EXPECT_FALSE(IsMatching(call_and_free_function, 4));

  ::testing::StrictMock<MatcherMethodMock<int>>
      call_and_free_function_and_method;
  EXPECT_CALL(call_and_free_function_and_method, MethodCall(5))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false));
  EXPECT_TRUE(IsMatching(call_and_free_function_and_method, 5));
  EXPECT_FALSE(IsMatching(&MatcherMethodMock<int>::IsMatching,
                          call_and_free_function_and_method, 5));
}

TEST(TestMatchers, Always) {
  EXPECT_TRUE(IsMatching(Always<true>(), 1, "Foo", 3985809135u, 124.546654));
  EXPECT_FALSE(IsMatching(Always<false>(), 1, 3985809135u, 124.546654));
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
  EXPECT_TRUE(IsMatching(atb::Eq(std::ref(mock)), 5));

  EXPECT_CALL(mock, Eq(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(atb::Eq(std::ref(mock)), 6));
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
  EXPECT_TRUE(IsMatching(atb::Ne(std::ref(mock)), 5));

  EXPECT_CALL(mock, Ne(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(IsMatching(atb::Ne(std::ref(mock)), 6));
}

TEST(TestMatchers, Ge) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Ge(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(Ge(3)(std::ref(mock)));
  EXPECT_FALSE(Ge(3)(std::ref(mock)));

  EXPECT_CALL(mock, Ge(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::Ge(std::ref(mock))(4));
  EXPECT_FALSE(atb::Ge(std::ref(mock))(4));
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
  EXPECT_TRUE(IsMatching(atb::Gt(std::ref(mock)), 4));
  EXPECT_FALSE(IsMatching(atb::Gt(std::ref(mock)), 4));
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
  EXPECT_TRUE(IsMatching(atb::Le(std::ref(mock)), 4));
  EXPECT_FALSE(IsMatching(atb::Le(std::ref(mock)), 4));
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
  EXPECT_TRUE(IsMatching(atb::Lt(std::ref(mock)), 4));
  EXPECT_FALSE(IsMatching(atb::Lt(std::ref(mock)), 4));
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
  EXPECT_FALSE(IsMatching(atb::Not(std::ref(mock)), 3));
  EXPECT_TRUE(IsMatching(atb::Not(std::ref(mock)), 3));
}

TEST(TestMatchers, AllOf) {
  {
    ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
    using testing::Return;

    EXPECT_CALL(mock, Call("Foo"))
        .Times(4)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    EXPECT_TRUE(IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                      std::ref(mock), std::ref(mock)),
                           "Foo"sv));

    EXPECT_CALL(mock, Call("Bar"))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                       std::ref(mock), std::ref(mock)),
                            "Bar"sv));

    EXPECT_CALL(mock, Call("Baz"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                       std::ref(mock), std::ref(mock)),
                            "Baz"sv));
  }

  {
    ::testing::StrictMock<CallableMock<bool, std::string_view, int>> mock;
    using arg_t = typename decltype(mock)::arg_t;
    using testing::Return;

    EXPECT_CALL(mock, Call(arg_t{"Foo"sv, 42}))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                       std::ref(mock), std::ref(mock)),
                            "Foo"sv, 42));
  }
}

TEST(TestMatchers, AnyOf) {
  {
    ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
    using testing::Return;

    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    EXPECT_TRUE(IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                      std::ref(mock), std::ref(mock)),
                           "Foo"sv));

    EXPECT_CALL(mock, Call("Bar"))
        .Times(3)
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    EXPECT_TRUE(IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                      std::ref(mock), std::ref(mock)),
                           "Bar"sv));

    EXPECT_CALL(mock, Call("Baz"))
        .Times(4)
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                       std::ref(mock), std::ref(mock)),
                            "Baz"sv));
  }

  {
    ::testing::StrictMock<CallableMock<bool, std::string_view, int>> mock;
    using arg_t = typename decltype(mock)::arg_t;
    using testing::Return;

    EXPECT_CALL(mock, Call(arg_t{"Foo"sv, 42}))
        .Times(3)
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    EXPECT_TRUE(IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                      std::ref(mock), std::ref(mock)),
                           "Foo"sv, 42));
  }
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

  EXPECT_TRUE(IsMatching(OnArgs<0>(std::ref(mock)), "Foo"sv, 1, 4, "Bar"sv));
  EXPECT_FALSE(IsMatching(OnArgs<3>(std::ref(mock)), 1, 4, "Bar"sv, "Foo"sv));
  EXPECT_TRUE(IsMatching(OnArgs<1>(std::ref(mock)), 1, "Foo"sv, 4, "Bar"sv));

  EXPECT_TRUE(IsMatching(
      AllOf(OnArgs<0>(Ge(0)), OnArgs<2>(Eq(4)),
            OnArgs<1, 3>([](auto... s) { return ((s.size() == 3) && ...); })),
      1, "Foo"sv, 4, "Bar"sv));
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

    EXPECT_TRUE(IsMatching(atb::AllArgs(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(IsMatching(atb::AllArgs(std::ref(mock)), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(IsMatching(atb::AllArgs(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(IsMatching(AnyArgs(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(IsMatching(AnyArgs(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(IsMatching(AnyArgs(std::ref(mock)), "Foo"sv, "Bar"sv));
  }
}

TEST(TestMatchers, AnyMatcher) {
  AnyMatcher<int, std::string_view> any_matcher{
      Always<false>(),
  };
  EXPECT_FALSE(IsMatching(any_matcher, 1, "Coucou"sv));

  any_matcher = Always<true>();
  EXPECT_TRUE(IsMatching(any_matcher, 42, "Chocolatine"sv));

  any_matcher = AllOf(OnArgs<0>(Eq(10)), OnArgs<1>(Eq("Toto"sv)));
  EXPECT_TRUE(IsMatching(any_matcher, 10, "Toto"sv));
  EXPECT_FALSE(IsMatching(any_matcher, 10, "Tata"sv));
}

}  // namespace

}  // namespace atb
