#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>

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
  EXPECT_EQ(StrSize({"0123", "456", "7", "89"}), 10);
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
  EXPECT_EQ("ChocolatineCoucou",
            std::string_view(begin, chocolatine.size() + coucou.size()));

  // Multiple strings
  std::fill(begin, end, '\0');
  res = StrCopyUnsafe({coucou, sep, chocolatine}, begin);
  EXPECT_EQ(res,
            std::next(begin, coucou.size() + sep.size() + chocolatine.size()));
  EXPECT_EQ(
      "Coucou Chocolatine",
      std::string_view(begin, coucou.size() + sep.size() + chocolatine.size()));
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
  EXPECT_EQ("ChocolatineCoucou",
            std::string_view(begin, chocolatine.size() + coucou.size()));

  // Multiple strings
  std::fill(begin, end, '\0');
  res = StrCopy({coucou, sep, chocolatine}, begin, buffer.size());
  EXPECT_EQ(res,
            std::next(begin, coucou.size() + sep.size() + chocolatine.size()));
  EXPECT_EQ("Coucou Chocolatine",
            std::string_view(
                begin, static_cast<std::size_t>(std::distance(begin, res))));

  // Overflows - No crop
  std::fill(begin, end, '\0');
  res = StrCopy({coucou, sep, chocolatine, chocolatine}, begin, buffer.size());
  EXPECT_NE(res, std::next(begin, coucou.size() + sep.size() +
                                      2 * chocolatine.size()));
  EXPECT_EQ(res,
            std::next(begin, coucou.size() + sep.size() + chocolatine.size()));

  EXPECT_EQ("Coucou Chocolatine",
            std::string_view(
                begin, static_cast<std::size_t>(std::distance(begin, res))));

  // Overflows - crop
  std::fill(begin, end, '\0');
  res = StrCopy({coucou, sep, chocolatine, chocolatine}, begin, buffer.size(),
                true);
  EXPECT_EQ(res, end);
  EXPECT_EQ("Coucou ChocolatineCh",
            std::string_view(
                begin, static_cast<std::size_t>(std::distance(begin, res))));
}

}  // namespace

}  // namespace atb
