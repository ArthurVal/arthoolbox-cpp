#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <limits>

#include "atb-cpp/matchers/string.hpp"
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

TEST(AtbStringTest, StrSwitch) {
  EXPECT_EQ(2, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case("Coucou", 2)
                   .Case(StrStartsWith("Cou"), 3)
                   .Case(StrContains("c"), 4)
                   .Default(-1));

  EXPECT_EQ(3, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case(chocolatine, 2)
                   .Case(coucou, 3)
                   .Case(StrContains("c"), 4)
                   .Default(-1));

  EXPECT_EQ(3, StrSwitch<int>("Coucou")
                   .Case("foo", 1)
                   .Case("Coucou ", 2)
                   .Case(StrStartsWith("Cou"), 3)
                   .Case(StrContains("c"), 4)
                   .Default(-1));

  EXPECT_EQ(4, StrSwitch<int>(coucou)
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

  EXPECT_EQ(-1, StrSwitch<int>(chocolatine)
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
