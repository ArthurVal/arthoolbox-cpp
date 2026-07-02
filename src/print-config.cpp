#include <cstdio>

#include "atb-cpp/config.h"

int main() {
  std::printf(
      "atb-cpp config:\n"
      "- Version     = %s\n",
      ATB_CPP_VERSION_STR);

  return 0;
}
