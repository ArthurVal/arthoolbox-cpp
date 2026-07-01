#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <limits>

#include "atb-cpp/string.hpp"
#include "gtest/gtest.h"

using namespace std::literals::string_view_literals;

namespace atb {
namespace {

constexpr auto foo = "foo"sv;
constexpr auto coucou = "Coucou"sv;
constexpr auto sep = " "sv;
constexpr auto chocolatine = "Chocolatine"sv;

TEST(AtbStringTest, StrSize) {
  EXPECT_EQ(StrSize({}), 0);
  EXPECT_EQ(StrSize({foo}), foo.size());
  EXPECT_EQ(StrSize({"0123"sv, "456"sv, "7"sv, "89"sv}), 10);
}

TEST(AtbStringTest, StrCopy) {
  std::array<char, 20> buffer;
  const auto begin = buffer.data();
  const auto end = begin + buffer.size();

  std::fill(begin, end, '\0');
  auto res = StrCopy({}, begin, buffer.size());
  EXPECT_EQ(res, begin);

  res = StrCopy({coucou}, begin, buffer.size());
  EXPECT_EQ(res, std::next(begin, coucou.size()));
  EXPECT_EQ(coucou, std::string_view(begin, coucou.size()));

  std::fill(begin, end, '\0');
  res = StrCopy({chocolatine}, begin, buffer.size());
  EXPECT_EQ(res, std::next(begin, chocolatine.size()));
  EXPECT_EQ(chocolatine, std::string_view(begin, chocolatine.size()));

  // Not using begin
  res =
      StrCopy({coucou}, res, static_cast<std::size_t>(std::distance(res, end)));
  EXPECT_EQ(res, std::next(begin, chocolatine.size() + coucou.size()));
  EXPECT_EQ("ChocolatineCoucou"sv,
            std::string_view(begin, chocolatine.size() + coucou.size()));

  // Multiple strings
  std::fill(begin, end, '\0');
  res = StrCopy({coucou, sep, chocolatine}, begin, buffer.size());
  EXPECT_EQ(res,
            std::next(begin, coucou.size() + sep.size() + chocolatine.size()));
  EXPECT_EQ("Coucou Chocolatine"sv,
            std::string_view(
                begin, static_cast<std::size_t>(std::distance(begin, res))));

  // Overflows - No crop
  std::fill(begin, end, '\0');
  res = StrCopy({coucou, sep, chocolatine, chocolatine}, begin, buffer.size());
  EXPECT_NE(res, std::next(begin, coucou.size() + sep.size() +
                                      2 * chocolatine.size()));
  EXPECT_EQ(res,
            std::next(begin, coucou.size() + sep.size() + chocolatine.size()));

  EXPECT_EQ("Coucou Chocolatine"sv,
            std::string_view(
                begin, static_cast<std::size_t>(std::distance(begin, res))));

  // Overflows - crop
  std::fill(begin, end, '\0');
  res = StrCopy({coucou, sep, chocolatine, chocolatine}, begin, buffer.size(),
                true);
  EXPECT_EQ(res, end);
  EXPECT_EQ("Coucou ChocolatineCh"sv,
            std::string_view(
                begin, static_cast<std::size_t>(std::distance(begin, res))));
}

TEST(AtbStringTest, StrCopyUnsafe) {
  std::array<char, 256> buffer;
  const auto begin = buffer.data();
  const auto end = begin + buffer.size();

  std::fill(begin, end, '\0');
  auto res = StrCopyUnsafe({}, begin);
  EXPECT_EQ(res, begin);

  res = StrCopyUnsafe({coucou}, begin);
  EXPECT_EQ(res, std::next(begin, coucou.size()));
  EXPECT_EQ(coucou, std::string_view(begin, coucou.size()));

  std::fill(begin, end, '\0');
  res = StrCopyUnsafe({chocolatine}, begin);
  EXPECT_EQ(res, std::next(begin, chocolatine.size()));
  EXPECT_EQ(chocolatine, std::string_view(begin, chocolatine.size()));

  // Not using begin
  res = StrCopyUnsafe({coucou}, res);
  EXPECT_EQ(res, std::next(begin, chocolatine.size() + coucou.size()));
  EXPECT_EQ("ChocolatineCoucou"sv,
            std::string_view(begin, chocolatine.size() + coucou.size()));

  // Multiple strings
  std::fill(begin, end, '\0');
  res = StrCopyUnsafe({coucou, sep, chocolatine}, begin);
  EXPECT_EQ(res,
            std::next(begin, coucou.size() + sep.size() + chocolatine.size()));
  EXPECT_EQ(
      "Coucou Chocolatine"sv,
      std::string_view(begin, coucou.size() + sep.size() + chocolatine.size()));
}

TEST(AtbStringTest, StrAppend) {
  std::string str;

  auto added = StrAppend({}, str);
  EXPECT_EQ(added.value(), 0);
  EXPECT_EQ(str, ""sv);

  added = StrAppend({foo}, str);
  EXPECT_EQ(added.value(), foo.size());
  EXPECT_EQ(str, foo);

  added = StrAppend({sep, chocolatine}, str);
  EXPECT_EQ(added.value(), sep.size() + chocolatine.size());
  EXPECT_EQ(str, "foo Chocolatine"sv);

  // Append itself
  added = StrAppend({sep, str}, str);
  EXPECT_EQ(added.value(), foo.size() + (2 * sep.size()) + chocolatine.size());
  EXPECT_EQ(str, "foo Chocolatine foo Chocolatine"sv);
}

TEST(AtbStringTest, StrAppendUnsafe) {
  std::string str;

  auto added = StrAppendUnsafe({}, str);
  EXPECT_EQ(added.value(), 0);
  EXPECT_EQ(str, ""sv);

  added = StrAppendUnsafe({foo}, str);
  EXPECT_EQ(added.value(), foo.size());
  EXPECT_EQ(str, foo);

  added = StrAppendUnsafe({sep, chocolatine}, str);
  EXPECT_EQ(added.value(), sep.size() + chocolatine.size());
  EXPECT_EQ(str, "foo Chocolatine"sv);
}

TEST(AtbStringTest, StrCat) {
  EXPECT_EQ(StrCat({}).value(), ""sv);
  EXPECT_EQ(StrCat({foo, sep, chocolatine, sep, coucou}).value(),
            "foo Chocolatine Coucou"sv);
}

TEST(AtbStringTest, StrStartsWith) {
  // Empty inputs
  EXPECT_TRUE(::IsMatching(StrStartsWith(""), ""));
  EXPECT_TRUE(::IsMatching(StrStartsWith(""), coucou));
  EXPECT_FALSE(::IsMatching(StrStartsWith(coucou), ""));

  // Same input
  EXPECT_TRUE(::IsMatching(StrStartsWith(coucou), coucou));

  // Nominal
  constexpr auto starts_with_foo = StrStartsWith("foo");
  EXPECT_TRUE(::IsMatching(starts_with_foo, "foobar"));
  EXPECT_TRUE(::IsMatching(starts_with_foo, "foo bar"));
  EXPECT_FALSE(::IsMatching(starts_with_foo, "fobar"));
  EXPECT_FALSE(::IsMatching(starts_with_foo, " foo"));

  std::string str = "toto";
  EXPECT_FALSE(::IsMatching(StrStartsWith(str), chocolatine));

  str = "Choco";
  EXPECT_TRUE(::IsMatching(StrStartsWith(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(::IsMatching(StrStartsWith("Choco"), str));
}

TEST(AtbStringTest, StrEndsWith) {
  // Empty inputs
  EXPECT_TRUE(::IsMatching(StrEndsWith(""), ""));
  EXPECT_TRUE(::IsMatching(StrEndsWith(""), coucou));
  EXPECT_FALSE(::IsMatching(StrEndsWith(coucou), ""));

  // Same input
  EXPECT_TRUE(::IsMatching(StrEndsWith(coucou), coucou));

  // Nominal
  constexpr auto ends_with_foo = StrEndsWith("foo");
  EXPECT_FALSE(::IsMatching(ends_with_foo, "foobar"));
  EXPECT_FALSE(::IsMatching(ends_with_foo, "foo bar"));
  EXPECT_FALSE(::IsMatching(ends_with_foo, "fobar"));
  EXPECT_TRUE(::IsMatching(ends_with_foo, " foo"));

  std::string str = "toto";
  EXPECT_FALSE(::IsMatching(StrEndsWith(str), chocolatine));

  str = "ine";
  EXPECT_TRUE(::IsMatching(StrEndsWith(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(::IsMatching(StrEndsWith("tine"), str));
}

TEST(AtbStringTest, StrContains) {
  // Empty inputs
  EXPECT_TRUE(::IsMatching(StrContains(""), ""));
  EXPECT_TRUE(::IsMatching(StrContains(""), coucou));
  EXPECT_FALSE(::IsMatching(StrContains(coucou), ""));

  // Same input
  EXPECT_TRUE(::IsMatching(StrContains(coucou), coucou));

  // Nominal
  constexpr auto constains_foo = StrContains(foo);
  EXPECT_TRUE(::IsMatching(constains_foo, "foobar"));
  EXPECT_TRUE(::IsMatching(constains_foo, "foo bar"));
  EXPECT_FALSE(::IsMatching(constains_foo, "fobar"));
  EXPECT_TRUE(::IsMatching(constains_foo, " foo bar"));
  EXPECT_TRUE(::IsMatching(constains_foo, " foo bar foo"));

  std::string str = "toto";
  EXPECT_FALSE(::IsMatching(StrContains(str), chocolatine));

  str = "ine";
  EXPECT_TRUE(::IsMatching(StrContains(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(::IsMatching(StrContains("tine"), str));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(::IsMatching(StrContains("baz", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(::IsMatching(StrContains("foo", &where), " foo bar foo"));
  EXPECT_EQ(where, 1);

  EXPECT_TRUE(::IsMatching(StrContains("bar", &where), " foo bar foo"));
  EXPECT_EQ(where, 5);
}

TEST(AtbStringTest, StrContainsR) {
  // Empty input
  EXPECT_TRUE(::IsMatching(StrContainsR(""), ""));
  EXPECT_TRUE(::IsMatching(StrContainsR(""), coucou));
  EXPECT_FALSE(::IsMatching(StrContainsR(coucou), ""));

  // Same input
  EXPECT_TRUE(::IsMatching(StrContainsR(coucou), coucou));

  // Nominal
  constexpr auto constains_r_foo = StrContainsR(foo);
  EXPECT_TRUE(::IsMatching(constains_r_foo, "foobar"));
  EXPECT_TRUE(::IsMatching(constains_r_foo, "foo bar"));
  EXPECT_FALSE(::IsMatching(constains_r_foo, "fobar"));
  EXPECT_TRUE(::IsMatching(constains_r_foo, " foo bar"));
  EXPECT_TRUE(::IsMatching(constains_r_foo, " foo bar foo"));

  std::string str = "toto";
  EXPECT_FALSE(::IsMatching(StrContainsR(str), chocolatine));

  str = "ine";
  EXPECT_TRUE(::IsMatching(StrContainsR(str), chocolatine));

  str = chocolatine;
  EXPECT_TRUE(::IsMatching(StrContainsR("tine"), str));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(::IsMatching(StrContainsR("baz", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(::IsMatching(StrContainsR("foo", &where), " foo bar foo"));
  EXPECT_EQ(where, 9);

  EXPECT_TRUE(::IsMatching(StrContainsR("bar", &where), " foo bar foo"));
  EXPECT_EQ(where, 5);
}

TEST(AtbStringTest, StrContainsOneOf) {
  // Empty inputs
  EXPECT_FALSE(::IsMatching(StrContainsOneOf(""), ""));
  EXPECT_FALSE(::IsMatching(StrContainsOneOf(""), coucou));
  EXPECT_FALSE(::IsMatching(StrContainsOneOf(coucou), ""));

  // Same input
  EXPECT_TRUE(::IsMatching(StrContainsOneOf(coucou), coucou));

  // Nominal
  EXPECT_FALSE(::IsMatching(StrContainsOneOf("ab"), coucou));
  EXPECT_TRUE(::IsMatching(StrContainsOneOf("abu"), coucou));
  EXPECT_TRUE(::IsMatching(StrContainsOneOf("aabyu"), coucou));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(::IsMatching(StrContainsOneOf("zxy", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(::IsMatching(StrContainsOneOf("akc", &where), " foo bar foo"));
  EXPECT_EQ(where, 6);

  EXPECT_TRUE(::IsMatching(StrContainsOneOf("hko", &where), " foo bar foo"));
  EXPECT_EQ(where, 2);
}

TEST(AtbStringTest, StrContainsOneOfR) {
  // Empty inputs
  EXPECT_FALSE(::IsMatching(StrContainsOneOfR(""), ""));
  EXPECT_FALSE(::IsMatching(StrContainsOneOfR(""), coucou));
  EXPECT_FALSE(::IsMatching(StrContainsOneOfR(coucou), ""));

  // Same input
  EXPECT_TRUE(::IsMatching(StrContainsOneOfR(coucou), coucou));

  // Nominal
  EXPECT_FALSE(::IsMatching(StrContainsOneOfR("ab"), coucou));
  EXPECT_TRUE(::IsMatching(StrContainsOneOfR("abu"), coucou));
  EXPECT_TRUE(::IsMatching(StrContainsOneOfR("aabyu"), coucou));

  // Test d_where
  auto where = std::numeric_limits<std::size_t>::max();

  EXPECT_FALSE(::IsMatching(StrContainsOneOfR("zxy", &where), " foo bar foo"));
  EXPECT_EQ(where, std::numeric_limits<std::size_t>::max());

  EXPECT_TRUE(::IsMatching(StrContainsOneOfR("acd", &where), " foo bar foo"));
  EXPECT_EQ(where, 6);

  EXPECT_TRUE(::IsMatching(StrContainsOneOfR("hko", &where), " foo bar foo"));
  EXPECT_EQ(where, 11);
}

TEST(AtbStringTest, StrSwitch) {
  EXPECT_EQ(2, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case("Coucou", 2)
                   .Case(StrStartsWith("Cou"), 3)
                   .Case(StrContains("c"), 4)
                   .Default(-1));

  EXPECT_EQ(3, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case("Coucou ", 2)
                   .Case(StrStartsWith("Cou"), 3)
                   .Case(StrContains("c"), 4)
                   .Default(-1));

  EXPECT_EQ(4, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case("Coucou ", 2)
                   .Case(StrStartsWith("Cou "), 3)
                   .Case(StrContains("c"), 4)
                   .Default(-1));

  EXPECT_EQ(5, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case("Coucou ", 2)
                   .Case(StrStartsWith("Cou "), 3)
                   .Case(StrContains("k"), 4)
                   .Case(AllOf(StrStartsWith("Co"), StrEndsWith("ou")), 5)
                   .Default(-1));

  EXPECT_EQ(-1, StrSwitch<int>("Coucou")
                    .Case("foo", 1)
                    .Case("Coucou ", 2)
                    .Case(StrStartsWith("Cou "), 3)
                    .Case(StrContains("k"), 4)
                    .Case(AllOf(StrStartsWith("Kou"), StrEndsWith("ou")), 5)
                    .Default(-1));

  EXPECT_EQ(-1, StrSwitch<int>("")
                    .Case("foo", 1)
                    .Case("Coucou ", 2)
                    .Case(StrStartsWith("Cou "), 3)
                    .Case(StrContains("k"), 4)
                    .Case(AllOf(StrStartsWith("Kou"), StrEndsWith("ou")), 5)
                    .Default(-1));
}

}  // namespace

}  // namespace atb
