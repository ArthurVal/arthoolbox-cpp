#include "atb-cpp/scope_exit.hpp"
#include "gtest/gtest.h"

namespace atb {
namespace {

TEST(AtbScopeExitTest, Nominal) {
  int value = 10;
  {
    auto _ = ScopeExit([](auto& v) { v = 20; }, std::ref(value));
    EXPECT_EQ(value, 10);
  }
  EXPECT_EQ(value, 20);

  // Not using std::ref -> doesn't work
  {
    auto _ = ScopeExit([](auto& v) { v = 30; }, value);
    EXPECT_EQ(value, 20);
  }
  EXPECT_EQ(value, 20);
}

TEST(AtbScopeExitTest, Abort) {
  int value = 10;
  {
    auto scope_exit = ScopeExit([](auto& v) { v = 20; }, std::ref(value));
    scope_exit.Abort();
    EXPECT_EQ(value, 10);
  }
  EXPECT_EQ(value, 10);
}

TEST(AtbScopeExitTest, Resume) {
  int value = 10;
  {
    auto scope_exit = ScopeExit([](auto& v) { v = 20; }, std::ref(value));
    scope_exit.Abort();
    scope_exit.Resume();
    EXPECT_EQ(value, 10);
  }
  EXPECT_EQ(value, 20);
}

TEST(AtbScopeExitTest, Execute) {
  int value = 10;
  std::string str = "Coucou";

  {
    auto scope_exit = ScopeExit(
        [](auto& v, auto& s) {
          v += 10;
          s += " +1";
        },
        std::ref(value), std::ref(str));
    EXPECT_EQ(value, 10);
    EXPECT_EQ(str, "Coucou");

    scope_exit.Execute();
    EXPECT_EQ(value, 20);
    EXPECT_EQ(str, "Coucou +1");
  }
  EXPECT_EQ(value, 20);
  EXPECT_EQ(str, "Coucou +1");

  {
    auto scope_exit = ScopeExit(
        [](auto& v, auto& s) {
          v += 10;
          s += " +1";
        },
        std::ref(value), std::ref(str));

    scope_exit.Execute();
    EXPECT_EQ(value, 30);
    EXPECT_EQ(str, "Coucou +1 +1");

    scope_exit.Resume();
  }
  EXPECT_EQ(value, 40);
  EXPECT_EQ(str, "Coucou +1 +1 +1");
}

}  // namespace

}  // namespace atb
