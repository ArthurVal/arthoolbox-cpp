#include "atb-cpp/scope_exit.hpp"
#include "gtest/gtest.h"

namespace atb {
namespace {

TEST(AtbScopeExitTest, Nominal) {
  int value = 10;
  {
    auto _ = ScopeExit([&]() { value = 20; });
    EXPECT_EQ(value, 10);
  }
  EXPECT_EQ(value, 20);
}

TEST(AtbScopeExitTest, Abort) {
  int value = 10;
  {
    auto scope_exit = ScopeExit([&]() { value = 20; });
    scope_exit.Abort();
    EXPECT_EQ(value, 10);
  }
  EXPECT_EQ(value, 10);
}

TEST(AtbScopeExitTest, Resume) {
  int value = 10;
  {
    auto scope_exit = ScopeExit([&]() { value = 20; });
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
    auto scope_exit = ScopeExit([&]() {
      value += 10;
      str += " +1";
    });
    EXPECT_EQ(value, 10);
    EXPECT_EQ(str, "Coucou");

    scope_exit.Execute();
    EXPECT_EQ(value, 20);
    EXPECT_EQ(str, "Coucou +1");

    scope_exit.Execute();
    EXPECT_EQ(value, 20);
    EXPECT_EQ(str, "Coucou +1");
  }
  EXPECT_EQ(value, 20);
  EXPECT_EQ(str, "Coucou +1");

  {
    auto scope_exit = ScopeExit([&]() {
      value += 10;
      str += " +1";
    });

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
