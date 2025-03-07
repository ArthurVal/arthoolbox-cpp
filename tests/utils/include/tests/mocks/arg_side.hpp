#pragma once

#include <ostream>
#include <string_view>

namespace tests {

// ArgSide //////////////////////////////////////////////////////////////////
enum class ArgSide {
  Left,
  Right,
};

constexpr auto ToString(ArgSide side) noexcept -> std::string_view {
  switch (side) {
    case ArgSide::Left:
      return "Left";
    case ArgSide::Right:
      return "Right";
  }

  return "";
}

inline auto operator<<(std::ostream& os, ArgSide side) -> std::ostream& {
  os << ToString(side);
  return os;
}

}  // namespace tests
