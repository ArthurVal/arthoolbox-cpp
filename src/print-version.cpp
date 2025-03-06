#include <cstdio>

#include "atb-cpp/version.h"

int main() {
  std::printf(
      "atb-cpp:\n"
      "- Version    : %s\n"
      "- Description: %s\n",
      ATB_CPP_VERSION_STR, ATB_CPP_DESCRIPTION);
  return 0;
}
