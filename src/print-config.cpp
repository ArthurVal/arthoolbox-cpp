#include <cstdio>

#include "atb-cpp/config.h"

int main() {
  std::printf(
      "atb-cpp config:\n"
      "- Build type  = %s\n"
      "- Version     = %s\n"
      "- Description = %s\n",
      ATB_CPP_BUILD_TYPE, ATB_CPP_VERSION_STR, ATB_CPP_DESCRIPTION);

  return 0;
}
