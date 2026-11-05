#include <vector>

#include "atb-cpp/format.hpp"
#include "gtest/gtest.h"

using namespace std::string_view_literals;

namespace atb {

namespace {

struct NotFormattable {};

// ==========================================================================
// FmtTry Tests
// ==========================================================================

TEST(AtbFormatTest, FmtTryFormattable) {
  // 1. Basic formatting (default fallback "<?>" is ignored)
  EXPECT_EQ(std::format("{}", FmtTry{42}), "42");

  // 2. Explicit fallback is ignored because the value is valid
  EXPECT_EQ(std::format("{}", FmtTry{3.14, "missing"}), "3.14");
}

TEST(AtbFormatTest, FmtTryUnformattable) {
  // 1. Basic fallback (uses default "<?>")
  EXPECT_EQ(std::format("{}", FmtTry{NotFormattable{}}), "<?>");

  // 2. Custom string fallback
  EXPECT_EQ(std::format("{}", FmtTry{NotFormattable{}, "N/A"}), "N/A");

  // 3. Custom non-string fallback (e.g., falling back to an error code)
  EXPECT_EQ(std::format("{}", FmtTry{NotFormattable{}, -1}), "-1");
}

TEST(AtbFormatTest, FmtTryDelegatedSpecifiers) {
  // Specifiers correctly pass through to the value's formatter when formattable
  EXPECT_EQ(std::format("{:.2f}", FmtTry{3.14159}), "3.14");
  EXPECT_EQ(std::format("{:04d}", FmtTry{42}), "0042");
  EXPECT_EQ(std::format("{:x}", FmtTry{255}), "ff");
}

TEST(AtbFormatTest, FmtTryOrElse) {
  FmtTry<NotFormattable> try_obj{NotFormattable{}};  // fallback is "<?>"

  // OrElse returns a new FmtTry with the updated fallback type and value
  auto updated_str = try_obj.OrElse("Unknown"sv);
  auto updated_int = try_obj.OrElse(-99);

  EXPECT_EQ(std::format("{}", updated_str), "Unknown");
  EXPECT_EQ(std::format("{}", updated_int), "-99");
}

TEST(AtbFormatTest, FmtTryInvalidSpecifiersThrow) {
  // 1. Formattable type throwing
  // std::string doesn't support 'd' (decimal integer).
  auto try_str = FmtTry{std::string("hello")};
  auto args1 = std::make_format_args(try_str);
  EXPECT_THROW({ auto _ = std::vformat("{:d}", args1); }, std::format_error);

  // 2. Unformattable type throwing
  // The unformattable specialization of FmtTry returns ctx.begin() in parse().
  // If a specifier is provided, std::format will see unparsed characters and
  // throw.
  auto try_unfmt = FmtTry{NotFormattable{}, std::string("NA")};
  auto args2 = std::make_format_args(try_unfmt);
  EXPECT_THROW({ auto _ = std::vformat("{:>5}", args2); }, std::format_error);
}

// ==========================================================================
// FmtJoin Tests
// ==========================================================================

TEST(AtbFormatTest, FmtJoinStandardMode) {
  // 1. Array CTAD (Multiple identical types)
  EXPECT_EQ(std::format("{}", FmtJoin(", ", 1, 2, 3)), "1, 2, 3");

  // 2. Tuple CTAD (Heterogeneous types: int, string literal, double)
  EXPECT_EQ(std::format("{}", FmtJoin(" | ", 42, "hello", 3.14)),
            "42 | hello | 3.14");

  // 3. Different glue types (char instead of string_view)
  EXPECT_EQ(std::format("{}", FmtJoin('-', "a"sv, "b"sv, "c"sv)), "a-b-c");

  // 4. std::string as glue
  std::string glue = " and ";
  EXPECT_EQ(std::format("{}", FmtJoin(glue, "Alice"sv, "Bob"sv)),
            "Alice and Bob");
}

TEST(AtbFormatTest, FmtJoinRangeMode) {
  // Range CTAD (std::vector)
  std::vector<int> vec = {10, 20, 30};
  EXPECT_EQ(std::format("{}", FmtJoin(" - ", vec)), "10 - 20 - 30");

  std::vector<std::string> empty_vec;
  EXPECT_EQ(std::format("{}", FmtJoin(", ", empty_vec)), "");
}

TEST(AtbFormatTest, FmtJoinEdgeCases) {
  // 1. Array CTAD Single element fallback (glue should not be printed)
  EXPECT_EQ(std::format("{}", FmtJoin(", ", 42)), "42");

  // 2. Tuple CTAD Zero elements (should print nothing, safe iteration)
  EXPECT_EQ(std::format("{}", FmtJoin(", ")), "");

  // 3. Empty glue
  EXPECT_EQ(std::format("{}", FmtJoin("", 1, 2, 3)), "123");
}

TEST(AtbFormatTest, FmtJoinArrayRangeDelegatedSpecifiers) {
  // Because the Array/Range formatter inherits from std::formatter<T>,
  // element format specifiers are natively supported!

  // Array
  EXPECT_EQ(std::format("{:02}", FmtJoin("-", 1, 2, 3)), "01-02-03");

  // Range
  std::vector<int> vec = {255, 128};
  EXPECT_EQ(std::format("{:X}", FmtJoin(":", vec)), "FF:80");
}

TEST(AtbFormatTest, FmtJoinTupleSpecifiersThrow) {
  // Tuple CTAD explicitly rejects specifiers in parse() because elements
  // differ.
  auto joined = FmtJoin(", ", 1, "hello");
  auto args = std::make_format_args(joined);

  // Throws standard format error due to custom exception in parse()
  EXPECT_THROW({ auto _ = std::vformat("{:x}", args); }, std::format_error);
}

}  // namespace

}  // namespace atb
