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
  EXPECT_EQ(std::format("{}", Eq(NotFormattable{})), "== <?>");
}

}  // namespace

}  // namespace atb
