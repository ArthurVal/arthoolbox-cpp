#include "atb-cpp/string.hpp"
#include "gtest/gtest.h"

using namespace std::literals::string_view_literals;

namespace atb {
namespace {

TEST(AtbStringTest, StrSize) {
  EXPECT_EQ(0, StrSize({}));
  EXPECT_EQ(3, StrSize({"foo"sv}));
  EXPECT_EQ(10, StrSize({"abc", "def", "ghij"}));
}

}  // namespace

}  // namespace atb
