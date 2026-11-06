#include <vector>

#include "atb-cpp/format/matchers.hpp"
#include "gtest/gtest.h"

using namespace std::string_view_literals;

namespace atb {

namespace {

struct NotFormattable {};

TEST(AtbFormatMatchersTest, Always) {
  EXPECT_EQ(std::format("{}", Always<true>{}), "Always true");
  EXPECT_EQ(std::format("{}", Always<false>{}), "Always false");
}

TEST(AtbFormatMatchersTest, Eq) {
  EXPECT_EQ(std::format("{}", Eq(true)), "== true");
  EXPECT_EQ(std::format("{}", Eq(10)), "== 10");
  EXPECT_EQ(std::format("{:X}", Eq(0xFF)), "== FF");
  static_assert(!atb::formattable<Eq<NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Ne) {
  EXPECT_EQ(std::format("{}", Ne(true)), "!= true");
  EXPECT_EQ(std::format("{}", Ne(10)), "!= 10");
  EXPECT_EQ(std::format("{:X}", Ne(0xFF)), "!= FF");
  static_assert(!atb::formattable<Ne<NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Ge) {
  EXPECT_EQ(std::format("{}", Ge(true)), ">= true");
  EXPECT_EQ(std::format("{}", Ge(10)), ">= 10");
  EXPECT_EQ(std::format("{:X}", Ge(0xFF)), ">= FF");
  static_assert(!atb::formattable<Ge<NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Gt) {
  EXPECT_EQ(std::format("{}", Gt(true)), "> true");
  EXPECT_EQ(std::format("{}", Gt(10)), "> 10");
  EXPECT_EQ(std::format("{:X}", Gt(0xFF)), "> FF");
  static_assert(!atb::formattable<Gt<NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Le) {
  EXPECT_EQ(std::format("{}", Le(true)), "<= true");
  EXPECT_EQ(std::format("{}", Le(10)), "<= 10");
  EXPECT_EQ(std::format("{:X}", Le(0xFF)), "<= FF");
  static_assert(!atb::formattable<Le<NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Lt) {
  EXPECT_EQ(std::format("{}", Lt(true)), "< true");
  EXPECT_EQ(std::format("{}", Lt(10)), "< 10");
  EXPECT_EQ(std::format("{:X}", Lt(0xFF)), "< FF");
  static_assert(!atb::formattable<Lt<NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Near) {
  EXPECT_EQ(std::format("{}", Near(true)), "~= true (+/- 1e-06)");
  EXPECT_EQ(std::format("{}", Near(10)), "~= 10 (+/- 1e-06)");
  EXPECT_EQ(std::format("{:X}", Near(0xFF)), "~= FF (+/- 1e-06)");
  static_assert(!atb::formattable<Near<NotFormattable>>);

  EXPECT_EQ(std::format("{}", Near(10, 20)), "~= 10 (+/- 20)");
  static_assert(!atb::formattable<Near<int, NotFormattable>>);
}

TEST(AtbFormatMatchersTest, Not) {
  EXPECT_EQ(std::format("{}", Not(Eq(true))), "NOT (== true)");

  const auto m = Not(Eq(255));
  EXPECT_EQ(std::format("{}", m), "NOT (== 255)");
  EXPECT_EQ(std::format("{:X}", m), "NOT (== FF)");
  static_assert(!atb::formattable<Not<Eq<NotFormattable>>>);
}

TEST(AtbFormatMatchersTest, AllOf) {
  EXPECT_EQ(std::format("{}", AllOf(Eq(10))), "[== 10]");
  EXPECT_EQ(std::format("{}", AllOf(Eq(420), Ne(true))),
            "[== 420 AND != true]");
  static_assert(!atb::formattable<AllOf<Eq<NotFormattable>>>);
  static_assert(!atb::formattable<AllOf<Eq<int>, Ne<NotFormattable>>>);

  // Using the same matchers enables format specifiers
  EXPECT_EQ(std::format("{:04X}", AllOf(Eq(0xFF), Eq(0xAA))),
            "[== 00FF AND == 00AA]");
}

}  // namespace

}  // namespace atb
