#include <array>
#include <string_view>
#include <vector>
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

struct Foo {
  float foo;
  int bar;
  bool baz;
};

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

TEST(AtbMatchersTest, IsMatching) {
  using ::testing::Return;

  ::testing::StrictMock<CallableMock<bool, int>> only_call;
  EXPECT_CALL(only_call, Call(3)).Times(1).WillOnce(Return(true));
  EXPECT_TRUE(atb::IsMatching(only_call, 3));

  ::testing::StrictMock<MatcherFreeFunctionMock<int>> call_and_free_function;
  EXPECT_CALL(call_and_free_function, FreeFunctionCall(4))
      .Times(1)
      .WillOnce(Return(false));
  EXPECT_FALSE(atb::IsMatching(call_and_free_function, 4));

  ::testing::StrictMock<MatcherMethodMock<int>>
      call_and_free_function_and_method;
  EXPECT_CALL(call_and_free_function_and_method, MethodCall(5))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false));
  EXPECT_TRUE(atb::IsMatching(call_and_free_function_and_method, 5));
  EXPECT_FALSE(atb::IsMatching(&MatcherMethodMock<int>::IsMatching,
                               call_and_free_function_and_method, 5));
}

TEST(AtbMatchersTest, Always) {
  EXPECT_TRUE(
      atb::IsMatching(Always<true>(), 1, "Foo", 3985809135u, 124.546654));
  EXPECT_FALSE(atb::IsMatching(Always<false>(), 1, 3985809135u, 124.546654));
}

TEST(AtbMatchersTest, Eq) {
  ::testing::StrictMock<tests::EqMock<bool, int>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Eq(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(Eq(3), mock));

  EXPECT_CALL(mock, Eq(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(atb::IsMatching(Eq(4), mock));

  EXPECT_CALL(mock, Eq(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(atb::Eq(std::ref(mock)), 5));

  EXPECT_CALL(mock, Eq(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(atb::IsMatching(atb::Eq(std::ref(mock)), 6));
}

TEST(AtbMatchersTest, Ne) {
  ::testing::StrictMock<tests::NeMock<bool, int>> mock;

  using testing::Return;
  EXPECT_CALL(mock, Ne(3, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(Ne(3), mock));

  EXPECT_CALL(mock, Ne(4, ArgSide::Right))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(atb::IsMatching(Ne(4), mock));

  EXPECT_CALL(mock, Ne(5, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(true))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(atb::Ne(std::ref(mock)), 5));

  EXPECT_CALL(mock, Ne(6, ArgSide::Left))
      .Times(1)
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(atb::IsMatching(atb::Ne(std::ref(mock)), 6));
}

TEST(AtbMatchersTest, Ge) {
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

TEST(AtbMatchersTest, Gt) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Gt(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(Gt(3), mock));
  EXPECT_FALSE(atb::IsMatching(Gt(3), mock));

  EXPECT_CALL(mock, Gt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(atb::Gt(std::ref(mock)), 4));
  EXPECT_FALSE(atb::IsMatching(atb::Gt(std::ref(mock)), 4));
}

TEST(AtbMatchersTest, Le) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Le(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(Le(3), mock));
  EXPECT_FALSE(atb::IsMatching(Le(3), mock));

  EXPECT_CALL(mock, Le(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(atb::Le(std::ref(mock)), 4));
  EXPECT_FALSE(atb::IsMatching(atb::Le(std::ref(mock)), 4));
}

TEST(AtbMatchersTest, Lt) {
  ::testing::StrictMock<ComparableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Lt(3, ArgSide::Right))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(Lt(3), mock));
  EXPECT_FALSE(atb::IsMatching(Lt(3), mock));

  EXPECT_CALL(mock, Lt(4, ArgSide::Left))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_TRUE(atb::IsMatching(atb::Lt(std::ref(mock)), 4));
  EXPECT_FALSE(atb::IsMatching(atb::Lt(std::ref(mock)), 4));
}

TEST(AtbMatchersTest, Near) {
  // TODO: Use mocks (Arithmetic + Comparable) instead
  EXPECT_TRUE(Near(3.14)(3.14));
  EXPECT_FALSE(Near(3.14)(3.15));

  EXPECT_TRUE(Near(3.14, 1.)(3.15));
  EXPECT_TRUE(Near(3.14, 1.)(4.14));
  EXPECT_FALSE(Near(3.14, 1.)(5.14));
}

TEST(AtbMatchersTest, Not) {
  ::testing::StrictMock<CallableMock<bool, int>> mock;
  using testing::Return;
  EXPECT_CALL(mock, Call(3))
      .Times(2)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .RetiresOnSaturation();
  EXPECT_FALSE(atb::IsMatching(atb::Not(std::ref(mock)), 3));
  EXPECT_TRUE(atb::IsMatching(atb::Not(std::ref(mock)), 3));
}

TEST(AtbMatchersTest, AllOf) {
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
    EXPECT_TRUE(atb::IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                           std::ref(mock), std::ref(mock)),
                                "Foo"sv));

    EXPECT_CALL(mock, Call("Bar"))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(atb::IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                            std::ref(mock), std::ref(mock)),
                                 "Bar"sv));

    EXPECT_CALL(mock, Call("Baz"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(atb::IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
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
    EXPECT_FALSE(atb::IsMatching(atb::AllOf(std::ref(mock), std::ref(mock),
                                            std::ref(mock), std::ref(mock)),
                                 "Foo"sv, 42));
  }
}

TEST(AtbMatchersTest, AnyOf) {
  {
    ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
    using testing::Return;

    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    EXPECT_TRUE(atb::IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                           std::ref(mock), std::ref(mock)),
                                "Foo"sv));

    EXPECT_CALL(mock, Call("Bar"))
        .Times(3)
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    EXPECT_TRUE(atb::IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                           std::ref(mock), std::ref(mock)),
                                "Bar"sv));

    EXPECT_CALL(mock, Call("Baz"))
        .Times(4)
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    EXPECT_FALSE(atb::IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
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
    EXPECT_TRUE(atb::IsMatching(atb::AnyOf(std::ref(mock), std::ref(mock),
                                           std::ref(mock), std::ref(mock)),
                                "Foo"sv, 42));
  }
}

TEST(AtbMatchersTest, SelectArgs) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  EXPECT_CALL(mock, Call("Foo"))
      .Times(3)
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .WillOnce(Return(true))
      .RetiresOnSaturation();

  EXPECT_TRUE(
      atb::IsMatching(SelectArgs<0>(std::ref(mock)), "Foo"sv, 1, 4, "Bar"sv));
  EXPECT_FALSE(
      atb::IsMatching(SelectArgs<3>(std::ref(mock)), 1, 4, "Bar"sv, "Foo"sv));
  EXPECT_TRUE(
      atb::IsMatching(SelectArgs<1>(std::ref(mock)), 1, "Foo"sv, 4, "Bar"sv));

  EXPECT_TRUE(atb::IsMatching(AllOf(SelectArgs<0>(Ge(0)), SelectArgs<2>(Eq(4)),
                                    SelectArgs<1, 3>([](auto... s) {
                                      return ((s.size() == 3) && ...);
                                    })),
                              1, "Foo"sv, 4, "Bar"sv));
}

TEST(AtbMatchersTest, EveryArg) {
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

    EXPECT_TRUE(
        atb::IsMatching(atb::EveryArg(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(
        atb::IsMatching(atb::EveryArg(std::ref(mock)), "Foo"sv, "Bar"sv));
  }

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(
        atb::IsMatching(atb::EveryArg(std::ref(mock)), "Foo"sv, "Bar"sv));
  }
}

TEST(AtbMatchersTest, AnyArg) {
  ::testing::StrictMock<CallableMock<bool, std::string_view>> mock;
  using testing::Return;

  {
    ::testing::Sequence seq;
    EXPECT_CALL(mock, Call("Foo"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_TRUE(atb::IsMatching(AnyArg(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_TRUE(atb::IsMatching(AnyArg(std::ref(mock)), "Foo"sv, "Bar"sv));
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

    EXPECT_FALSE(atb::IsMatching(AnyArg(std::ref(mock)), "Foo"sv, "Bar"sv));
  }
}

TEST(AtbMatchersTest, MapArg) {
  using testing::Return;

  {
    ::testing::StrictMock<CallableMock<bool, std::string_view>> matcher;
    ::testing::StrictMock<CallableMock<std::string_view, int>> map;
    ::testing::Sequence seq;
    EXPECT_CALL(map, Call(42))
        .Times(1)
        .WillOnce(Return("Coucou"))
        .RetiresOnSaturation();

    EXPECT_CALL(matcher, Call("Coucou"))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    EXPECT_TRUE(atb::IsMatching(MapArg(std::ref(map), std::ref(matcher)), 42));
  }

  {
    ::testing::StrictMock<CallableMock<bool, int, int, int>> matcher;
    using arg_t = typename decltype(matcher)::arg_t;

    ::testing::Sequence seq;
    EXPECT_CALL(matcher, Call(arg_t(333, 222, 111)))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();

    EXPECT_FALSE(atb::IsMatching(MapArg(&Foo::bar, std::ref(matcher)),
                                 Foo{42.f, 333, true}, Foo{3.14f, 222, true},
                                 Foo{2.7f, 111, false}));
  }
}

TEST(AtbMatchersTest, AnyMatcher) {
  using matcher_t = AnyMatcher<int, std::string_view>;

  matcher_t any;
  EXPECT_FALSE(any.IsInitialized());
  EXPECT_THROW(atb::IsMatching(any, 1, "Coucou"sv), std::runtime_error);

  any = Always<false>();
  EXPECT_TRUE(any.IsInitialized());
  EXPECT_FALSE(atb::IsMatching(any, 1, "Coucou"sv));

  any = Always<true>();
  EXPECT_TRUE(any.IsInitialized());
  EXPECT_TRUE(atb::IsMatching(any, 42, "Chocolatine"sv));

  any = AllOf(SelectArgs<0>(Eq(10)), SelectArgs<1>(Eq("Toto"sv)));
  EXPECT_TRUE(any.IsInitialized());
  EXPECT_TRUE(atb::IsMatching(any, 10, "Toto"sv));
  EXPECT_FALSE(atb::IsMatching(any, 10, "Tata"sv));

  matcher_t other;
  EXPECT_FALSE(other.IsInitialized());
  EXPECT_TRUE(any.IsInitialized());

  // Copy
  other = any;
  EXPECT_TRUE(other.IsInitialized());
  EXPECT_TRUE(any.IsInitialized());

  for (const auto& m : std::array{
           static_cast<const matcher_t&>(any),
           static_cast<const matcher_t&>(other),
       }) {
    EXPECT_TRUE(atb::IsMatching(m, 10, "Toto"sv));
    EXPECT_FALSE(atb::IsMatching(m, 10, "Tata"sv));
  }

  // Move
  other = std::move(any);
  EXPECT_FALSE(any.IsInitialized());
  EXPECT_TRUE(other.IsInitialized());
  EXPECT_TRUE(atb::IsMatching(other, 10, "Toto"sv));
  EXPECT_FALSE(atb::IsMatching(other, 10, "Tata"sv));

  // Showcase container feature of type erased
  std::vector<AnyMatcher<int>> matchers;
  matchers.emplace_back(Always<true>());
  matchers.emplace_back(Ge(20));
  matchers.emplace_back(Lt(100));
  for (const auto& m : matchers) EXPECT_TRUE(atb::IsMatching(m, 42));
}

TEST(AtbMatchersTest, UnsafeAnyMatcher) {
  UnsafeAnyMatcher<int> unsafe;
  EXPECT_FALSE(unsafe.IsInitialized());

  unsafe = Ge(500);
  EXPECT_TRUE(unsafe.IsInitialized());
  EXPECT_FALSE(atb::IsMatching(unsafe, 10));
  EXPECT_TRUE(atb::IsMatching(unsafe, 10000));

  unsafe = Le(500);
  EXPECT_TRUE(unsafe.IsInitialized());
  EXPECT_TRUE(atb::IsMatching(unsafe, 10));
  EXPECT_FALSE(atb::IsMatching(unsafe, 10000));

  UnsafeAnyMatcher<int> other;
  EXPECT_FALSE(other.IsInitialized());
  EXPECT_TRUE(unsafe.IsInitialized());

  // Copy
  other = unsafe;  // Le(500)
  EXPECT_TRUE(other.IsInitialized());
  EXPECT_TRUE(unsafe.IsInitialized());

  for (const auto& m : std::array{
           static_cast<const UnsafeAnyMatcher<int>&>(unsafe),
           static_cast<const UnsafeAnyMatcher<int>&>(other),
       }) {
    EXPECT_FALSE(atb::IsMatching(m, 1234567));
    EXPECT_TRUE(atb::IsMatching(m, 10));
  }

  // Move
  other = std::move(unsafe);
  EXPECT_FALSE(unsafe.IsInitialized());

  EXPECT_TRUE(other.IsInitialized());
  EXPECT_FALSE(atb::IsMatching(other, 5000));
  EXPECT_TRUE(atb::IsMatching(other, 10));
}

TEST(AtbMatchersDeathTest, UnsafeAnyMatcher) {
  UnsafeAnyMatcher<int> unsafe;
  EXPECT_FALSE(unsafe.IsInitialized());
  EXPECT_DEBUG_DEATH(
      { EXPECT_FALSE(atb::IsMatching(unsafe, 42)); }, "AnyMatcher is empty");
}

}  // namespace

}  // namespace atb
