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

}  // namespace

}  // namespace atb
