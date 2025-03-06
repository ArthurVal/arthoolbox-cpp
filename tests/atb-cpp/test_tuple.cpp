#include <string_view>

#include "atb-cpp/interger_sequence.hpp"
#include "atb-cpp/overload.hpp"
#include "atb-cpp/tuple.hpp"
#include "gtest/gtest.h"

using namespace std::literals::string_view_literals;

namespace atb {
namespace {

constexpr auto Add = [](auto&&... v) { return (... + v); };
constexpr auto Mul = [](auto&&... v) { return (... * v); };

auto FailWhenCalled(std::unique_ptr<::testing::ScopedTrace> trace = nullptr) {
  return [trace = std::move(trace)](auto&&... args) {
    FAIL() << "Functor called with " << (sizeof...(args))
           << " arguments yet it is expected to never be called...";
  };
}

#define FailWhenCalledWithTrace(msg) \
  FailWhenCalled(                    \
      std::make_unique<::testing::ScopedTrace>(__FILE__, __LINE__, (msg)))

TEST(TestTuple, Apply) {
  EXPECT_EQ(50 + 1 + 5,
            (tpl::Apply(Add, std::make_tuple(50, 1, 5, 4, 5452, "Coucou"),
                        MakeIndexSequence<3>())));

  EXPECT_EQ(
      (1 + 5 + 4),
      (tpl::Apply<1, 2, 3>(Add, std::make_tuple(50, 1, 5, 4, 5452, "Coucou"))));

  EXPECT_EQ(50 + 1 + 5,
            (tpl::Apply(Add, std::make_tuple(50, 1, 5, 4, 5452, "Coucou"),
                        MakeIndexSequence<3>())));

  EXPECT_EQ((4 * 5452),
            (tpl::Apply(Mul, std::make_tuple(50, 1, 5, 4, 5452, "Coucou"),
                        MakeIndexSequence<2, 3>())));

  EXPECT_EQ((1 + 2 + 3), (tpl::Apply(Add, std::make_tuple(1, 2, 3))));
}

TEST(TestTuple, Visit) {
  // Empty tuple
  tpl::Visit(FailWhenCalledWithTrace(""), std::make_tuple());
  tpl::Visit(FailWhenCalledWithTrace(""), std::make_tuple(1, 2, 3),
             std::make_tuple());

  {
    int expected_v = -5;
    tpl::Visit([&](int v) { EXPECT_EQ(v, expected_v++); },
               std::make_tuple(-5, -4, -3, -2, -1));
  }

  {
    int expected_v = -5;
    tpl::Visit<0, 2, 4>(
        [&](int v) {
          EXPECT_EQ(v, expected_v);
          expected_v += 2;
        },
        std::make_tuple(-5, -4, -3, -2, -1));
  }

  {
    int expected_v = -5;
    double expected_d = 5.;
    tpl::Visit(
        [&](int v, double d) {
          EXPECT_EQ(v, expected_v++);
          EXPECT_EQ(d, expected_d);
          expected_d *= 2;
        },
        std::make_tuple(-5, -4, -3, -2, -1), std::make_tuple(5., 10.));
  }

  {
    tpl::Visit<1>(
        [&](int i, double d) {
          EXPECT_EQ(i, -4);
          EXPECT_EQ(d, 10.);
        },
        std::make_tuple(-5, -4, -3, -2, -1), std::make_tuple(5., 10.));

    tpl::Visit<1, 1, 1, 1>(
        [&](int i, double d) {
          EXPECT_EQ(i, -4);
          EXPECT_EQ(d, 10.);
        },
        std::make_tuple(-5, -4, -3, -2, -1), std::make_tuple(5., 10.));
  }

  {
    tpl::Visit(Overload{
                   [](std::string_view str) { EXPECT_EQ(str, "Coucou"); },
                   [](int v) { EXPECT_EQ(v, 1); },
                   [](double v) { EXPECT_EQ(v, 3.14); },
                   FailWhenCalledWithTrace(""),
               },
               std::make_tuple(1, 3.14, "Coucou"sv));
  }
}

template <class T>
struct Accumulator {
  T accumulated = 0;

  Accumulator& Add(T v) {
    accumulated += v;
    return *this;
  }
};

TEST(TestTuple, Reduce) {
  // Empty tuple
  EXPECT_EQ(42,
            tpl::Reduce(42, FailWhenCalledWithTrace(""), std::make_tuple()));

  EXPECT_EQ((5 + (1 + 1 + 1 + 1 + 1)),
            tpl::Reduce(5, Add, std::make_tuple(1, 1, 1, 1, 1)));

  EXPECT_EQ((5 + (1 + 1 + 1 + 1 + 1)),
            tpl::Reduce(5, Add, std::make_tuple(1, 1, 1, 1, 1)));

  EXPECT_EQ((1 + 1 + "Coucou"sv.size() + 1 + 1),
            tpl::Reduce(0,
                        Overload{
                            [](int init, std::string_view v) {
                              return init + static_cast<int>(v.size());
                            },
                            Add,
                        },
                        std::make_tuple(1, 1, "Coucou"sv, 1, 1)));

  // We can do some weird stuff ...
  auto acc = Accumulator<int>{3};

  EXPECT_EQ(tpl::Reduce(acc, &Accumulator<int>::Add,
                        std::make_tuple(5, 3, 0, 5, -100))
                .accumulated,
            (3 + (5 + 3 + 0 + 5 - 100)));

  // init is passed by copy, therefore acc is not modified by default...
  EXPECT_EQ(acc.accumulated, 3);

  // ... But we can use std::ref
  tpl::Reduce(std::ref(acc), &Accumulator<int>::Add,
              std::make_tuple(5, 3, 0, 5, -100));

  EXPECT_EQ(acc.accumulated, (3 + (5 + 3 + 0 + 5 - 100)));
}

TEST(TestTuple, Transform) {
  // Empty tuple
  EXPECT_EQ(std::make_tuple(),
            tpl::Transform(FailWhenCalledWithTrace(""), std::make_tuple(3, 4),
                           std::make_tuple(), std::make_tuple(2, 1, "Coucou")));

  EXPECT_EQ(
      std::make_tuple(3, 3, 3),
      tpl::Transform(Add, std::make_tuple(1, 1, 1), std::make_tuple(2, 2, 2)));

  EXPECT_EQ(std::make_tuple(3), tpl::Transform<1>(Add, std::make_tuple(1, 1, 1),
                                                  std::make_tuple(2, 2, 2)));

  EXPECT_EQ(std::make_tuple(6, 7, 8),
            tpl::Transform(Add, std::make_tuple(1, 2, 3),
                           std::make_tuple(2, 2, 2), std::make_tuple(3, 3, 3)));

  EXPECT_EQ(std::make_tuple(6, 6, 6, 6, 6),
            (tpl::Transform<0, 0, 0, 0, 0>(Add, std::make_tuple(1, 2, 3),
                                           std::make_tuple(2, 2, 2),
                                           std::make_tuple(3, 3, 3))));

  EXPECT_EQ(std::make_tuple(3, 3),
            tpl::Transform(Add, std::make_tuple(1, 1, 1, 1, 1, 1, 1),
                           std::make_tuple(2, 2)));

  EXPECT_EQ(std::make_tuple(3),
            tpl::Transform(Add, std::make_tuple(1),
                           std::make_tuple(2, 2, 2, 2, 2, 2)));

  EXPECT_EQ(std::make_tuple("Foo Bar"sv, 0, 3.14),
            tpl::Transform(Overload{
                               [](std::string_view lhs, std::string_view rhs) {
                                 std::string out;
                                 out.reserve(lhs.size() + 1 + rhs.size() + 1);

                                 out.append(lhs);
                                 out.append(" ");
                                 out.append(rhs);
                                 return out;
                               },
                               [](int lhs, int rhs) { return lhs - rhs; },
                               [](auto, auto) { return 3.14; },
                           },
                           std::make_tuple("Foo"sv, 1, ""),
                           std::make_tuple("Bar"sv, 1, 0.)));
}

TEST(TestTuple, TransformReduce) {
  EXPECT_EQ((10 + (2 * 1) + (4 * 3)),
            tpl::TransformReduce(10, Add, Mul, std::make_tuple(2, 4, 6),
                                 std::make_tuple(1, 3)));

  EXPECT_EQ(
      (20 * (2 + 1 - 1) * (4 + 3 - 5)),
      tpl::TransformReduce(20, Mul, Add, std::make_tuple(2, 4, 8),
                           std::make_tuple(1, 3), std::make_tuple(-1, -5)));
}

}  // namespace

}  // namespace atb
