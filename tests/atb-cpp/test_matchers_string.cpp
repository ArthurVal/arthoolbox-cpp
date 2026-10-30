#include <cstddef>
#include <limits>

#include "atb-cpp/matchers/string.hpp"
#include "gtest/gtest.h"

using namespace std::literals::string_view_literals;

namespace atb {
namespace {

constexpr auto foo = "foo"sv;
constexpr auto coucou = "Coucou"sv;
constexpr auto chocolatine = "Chocolatine"sv;

TEST(AtbMatchersStringTest, StrStartsWith) {
  // Empty inputs
  EXPECT_TRUE(atb::IsMatching(StrStartsWith(""), ""));
  EXPECT_TRUE(atb::IsMatching(StrStartsWith(""), coucou));
  EXPECT_FALSE(atb::IsMatching(StrStartsWith(coucou), ""));

  // Same input
  EXPECT_TRUE(atb::IsMatching(StrStartsWith(coucou), coucou));

  // Nominal
  constexpr auto starts_with_foo = StrStartsWith("foo");
  EXPECT_TRUE(atb::IsMatching(starts_with_foo, "foobar"));
  EXPECT_TRUE(atb::IsMatching(starts_with_foo, "foo bar"));
  EXPECT_FALSE(atb::IsMatching(starts_with_foo, "fobar"));
  EXPECT_FALSE(atb::IsMatching(starts_with_foo, " foo"));

  std::string str = "toto";
  EXPECT_FALSE(atb::IsMatching(StrStartsWith(str), chocolatine));

  str = "Choco";
  EXPECT_TRUE(atb::IsMatching(StrStartsWith(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(atb::IsMatching(StrStartsWith("Choco"), str));
}

TEST(AtbMatchersStringTest, StrEndsWith) {
  // Empty inputs
  EXPECT_TRUE(atb::IsMatching(StrEndsWith(""), ""));
  EXPECT_TRUE(atb::IsMatching(StrEndsWith(""), coucou));
  EXPECT_FALSE(atb::IsMatching(StrEndsWith(coucou), ""));

  // Same input
  EXPECT_TRUE(atb::IsMatching(StrEndsWith(coucou), coucou));

  // Nominal
  constexpr auto ends_with_foo = StrEndsWith("foo");
  EXPECT_FALSE(atb::IsMatching(ends_with_foo, "foobar"));
  EXPECT_FALSE(atb::IsMatching(ends_with_foo, "foo bar"));
  EXPECT_FALSE(atb::IsMatching(ends_with_foo, "fobar"));
  EXPECT_TRUE(atb::IsMatching(ends_with_foo, " foo"));

  std::string str = "toto";
  EXPECT_FALSE(atb::IsMatching(StrEndsWith(str), chocolatine));

  str = "ine";
  EXPECT_TRUE(atb::IsMatching(StrEndsWith(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(atb::IsMatching(StrEndsWith("tine"), str));
}

TEST(AtbMatchersStringTest, StrContains) {
  // Empty inputs
  EXPECT_TRUE(atb::IsMatching(StrContains(""), ""));
  EXPECT_TRUE(atb::IsMatching(StrContains(""), coucou));
  EXPECT_FALSE(atb::IsMatching(StrContains(coucou), ""));

  // Same input
  EXPECT_TRUE(atb::IsMatching(StrContains(coucou), coucou));

  // Nominal
  constexpr auto constains_foo = StrContains(foo);
  EXPECT_TRUE(atb::IsMatching(constains_foo, "foobar"));
  EXPECT_TRUE(atb::IsMatching(constains_foo, "foo bar"));
  EXPECT_FALSE(atb::IsMatching(constains_foo, "fobar"));
  EXPECT_TRUE(atb::IsMatching(constains_foo, " foo bar"));
  EXPECT_TRUE(atb::IsMatching(constains_foo, " foo bar foo"));

  std::string str = "toto";
  EXPECT_FALSE(atb::IsMatching(StrContains(str), chocolatine));

  str = "ine";
  EXPECT_TRUE(atb::IsMatching(StrContains(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(atb::IsMatching(StrContains("tine"), str));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(atb::IsMatching(StrContains("baz", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(atb::IsMatching(StrContains("foo", &where), " foo bar foo"));
  EXPECT_EQ(where, 1);

  EXPECT_TRUE(atb::IsMatching(StrContains("bar", &where), " foo bar foo"));
  EXPECT_EQ(where, 5);
}

TEST(AtbMatchersStringTest, StrContainsR) {
  // Empty input
  EXPECT_TRUE(atb::IsMatching(StrContainsR(""), ""));
  EXPECT_TRUE(atb::IsMatching(StrContainsR(""), coucou));
  EXPECT_FALSE(atb::IsMatching(StrContainsR(coucou), ""));

  // Same input
  EXPECT_TRUE(atb::IsMatching(StrContainsR(coucou), coucou));

  // Nominal
  constexpr auto constains_r_foo = StrContainsR(foo);
  EXPECT_TRUE(atb::IsMatching(constains_r_foo, "foobar"));
  EXPECT_TRUE(atb::IsMatching(constains_r_foo, "foo bar"));
  EXPECT_FALSE(atb::IsMatching(constains_r_foo, "fobar"));
  EXPECT_TRUE(atb::IsMatching(constains_r_foo, " foo bar"));
  EXPECT_TRUE(atb::IsMatching(constains_r_foo, " foo bar foo"));

  std::string str = "toto";
  EXPECT_FALSE(atb::IsMatching(StrContainsR(str), chocolatine));

  str = "ine";
  EXPECT_TRUE(atb::IsMatching(StrContainsR(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(atb::IsMatching(StrContainsR("tine"), str));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(atb::IsMatching(StrContainsR("baz", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(atb::IsMatching(StrContainsR("foo", &where), " foo bar foo"));
  EXPECT_EQ(where, 9);

  EXPECT_TRUE(atb::IsMatching(StrContainsR("bar", &where), " foo bar foo"));
  EXPECT_EQ(where, 5);
}

TEST(AtbMatchersStringTest, StrContainsOneOf) {
  // Empty inputs
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOf(""), ""));
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOf(""), coucou));
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOf(coucou), ""));

  // Same input
  EXPECT_TRUE(atb::IsMatching(StrContainsOneOf(coucou), coucou));

  // Nominal
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOf("ab"), coucou));
  EXPECT_TRUE(atb::IsMatching(StrContainsOneOf("abu"), coucou));
  EXPECT_TRUE(atb::IsMatching(StrContainsOneOf("aabyu"), coucou));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(
      atb::IsMatching(StrContainsOneOf("zxy", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(atb::IsMatching(StrContainsOneOf("akc", &where), " foo bar foo"));
  EXPECT_EQ(where, 6);

  EXPECT_TRUE(atb::IsMatching(StrContainsOneOf("hko", &where), " foo bar foo"));
  EXPECT_EQ(where, 2);
}

TEST(AtbMatchersStringTest, StrContainsOneOfR) {
  // Empty inputs
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOfR(""), ""));
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOfR(""), coucou));
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOfR(coucou), ""));

  // Same input
  EXPECT_TRUE(atb::IsMatching(StrContainsOneOfR(coucou), coucou));

  // Nominal
  EXPECT_FALSE(atb::IsMatching(StrContainsOneOfR("ab"), coucou));
  EXPECT_TRUE(atb::IsMatching(StrContainsOneOfR("abu"), coucou));
  EXPECT_TRUE(atb::IsMatching(StrContainsOneOfR("aabyu"), coucou));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(
      atb::IsMatching(StrContainsOneOfR("zxy", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(
      atb::IsMatching(StrContainsOneOfR("acd", &where), " foo bar foo"));
  EXPECT_EQ(where, 6);

  EXPECT_TRUE(
      atb::IsMatching(StrContainsOneOfR("hko", &where), " foo bar foo"));
  EXPECT_EQ(where, 11);
}

}  // namespace

}  // namespace atb
