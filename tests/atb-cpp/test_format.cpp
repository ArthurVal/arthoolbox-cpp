#include "atb-cpp/format.hpp"
#include "gtest/gtest.h"

using namespace std::string_view_literals;

namespace atb {

namespace {

struct NotFormattable {};

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
  // 1. Specifiers correctly pass through to the value's formatter
  EXPECT_EQ(std::format("{:.2f}", FmtTry{3.14159}), "3.14");
  EXPECT_EQ(std::format("{:04d}", FmtTry{42}), "0042");
  EXPECT_EQ(std::format("{:x}", FmtTry{255}), "ff");

  // 2. Specifiers correctly pass through to the fallback's formatter
  EXPECT_EQ(std::format("{:>5}", FmtTry{NotFormattable{}, "NA"}), "   NA");
  EXPECT_EQ(std::format("{:04d}", FmtTry{NotFormattable{}, 42}), "0042");
}

TEST(AtbFormatTest, FmtTryReprMode) {
  // 1. Repr mode for a formattable type
  EXPECT_EQ(std::format("{:r}", FmtTry{42, "default"}),
            "FmtTry{.value = 42, .fallback = default}");

  // 2. Repr mode for an unformattable type
  EXPECT_EQ(std::format("{:r}", FmtTry{NotFormattable{}, "default"}),
            "FmtTry{.value = <?>, .fallback = default}");

  // 3. Emergent feature: Repr mode with delegated specifiers!
  // The 'r' is consumed by FmtTry, and the 'x' is cleanly passed down to the
  // int formatter.
  EXPECT_EQ(std::format("{:rx}", FmtTry{255, "default"}),
            "FmtTry{.value = ff, .fallback = default}");
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
  // FmtTry passes 'd' to std::string's formatter, which correctly throws.
  auto try_str = FmtTry{std::string("hello")};
  auto args1 = std::make_format_args(try_str);
  EXPECT_THROW({ auto _ = std::vformat("{:d}", args1); }, std::format_error);

  // 2. Unformattable type throwing
  // We fall back to printing "NA" (a string), which also rejects the 'd'
  // specifier.
  auto try_unfmt = FmtTry{NotFormattable{}, std::string("NA")};
  auto args2 = std::make_format_args(try_unfmt);
  EXPECT_THROW({ auto _ = std::vformat("{:d}", args2); }, std::format_error);
}

TEST(AtbFormatTest, FmtJoinStandardMode) {
  // 1. Multiple identical types
  EXPECT_EQ(std::format("{}", FmtJoin(", ", 1, 2, 3)), "1, 2, 3");

  // 2. Heterogeneous types (int, string literal, double)
  EXPECT_EQ(std::format("{}", FmtJoin(" | ", 42, "hello", 3.14)),
            "42 | hello | 3.14");

  // 3. Different glue types (char instead of string_view)
  EXPECT_EQ(std::format("{}", FmtJoin('-', "a", "b", "c")), "a-b-c");

  // 4. std::string as glue
  std::string glue = " and ";
  EXPECT_EQ(std::format("{}", FmtJoin(glue, "Alice", "Bob")), "Alice and Bob");
}

TEST(AtbFormatTest, FmtJoinEdgeCases) {
  // 1. Single element (glue should not be printed)
  EXPECT_EQ(std::format("{}", FmtJoin(", ", 42)), "42");

  // 2. Zero elements (should print nothing, safe tuple iteration)
  EXPECT_EQ(std::format("{}", FmtJoin(", ")), "");

  // 3. Empty glue
  EXPECT_EQ(std::format("{}", FmtJoin("", 1, 2, 3)), "123");
}

TEST(AtbFormatTest, FmtJoinReprMode) {
  // 1. Standard repr parsing
  EXPECT_EQ(std::format("{:r}", FmtJoin("-", 1, 2, 3)),
            "FmtJoin{.glue = -, .values = {1, 2, 3}}");

  // 2. Repr mode with heterogeneous types
  EXPECT_EQ(std::format("{:r}", FmtJoin(" | ", 42, "hello")),
            "FmtJoin{.glue =  | , .values = {42, hello}}");

  // 3. Repr mode with single element
  EXPECT_EQ(std::format("{:r}", FmtJoin(":", 99)),
            "FmtJoin{.glue = :, .values = {99}}");

  // 4. Repr mode with zero elements
  EXPECT_EQ(std::format("{:r}", FmtJoin(", ")),
            "FmtJoin{.glue = , , .values = {}}");
}

TEST(AtbFormatTest, FmtJoinInvalidSpecifiersThrow) {
  auto joined = FmtJoin(", ", 1, 2, 3);

  // We must use std::vformat to test throwing.
  // If we used std::format("{:x}", joined), the compiler's consteval
  // checks would fail the build entirely, which is good in production
  // but prevents us from writing a runtime EXPECT_THROW test.

  auto args = std::make_format_args(joined);

  // 'x' (hex) is not supported by FmtJoin itself
  EXPECT_THROW({ auto _ = std::vformat("{:x}", args); }, std::format_error);

  // 'rx' (repr + trailing garbage) is not supported
  EXPECT_THROW({ auto _ = std::vformat("{:rx}", args); }, std::format_error);

  // Unclosed braces should throw (handled by std::format internals)
  EXPECT_THROW({ auto _ = std::vformat("{:", args); }, std::format_error);
}

}  // namespace

}  // namespace atb
