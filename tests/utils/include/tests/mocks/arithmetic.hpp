#pragma once

#include "gmock/gmock.h"
#include "tests/mocks/arg_side.hpp"

namespace tests {

// Addition /////////////////////////////////////////////////////////////////
template <class R, class Arg>
struct AddMock {
  MOCK_METHOD(R, Addition, (Arg, ArgSide), (const));

  friend constexpr auto operator+(const AddMock& mock, Arg value) -> R {
    return mock.Addition(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator+(Arg value, const AddMock& mock) -> R {
    return mock.Addition(std::move(value), ArgSide::Left);
  }
};

// Substraction /////////////////////////////////////////////////////////////
template <class R, class Arg>
struct SubMock {
  MOCK_METHOD(R, Substraction, (Arg, ArgSide), (const));

  friend constexpr auto operator-(const SubMock& mock, Arg value) -> R {
    return mock.Substraction(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator-(Arg value, const SubMock& mock) -> R {
    return mock.Substraction(std::move(value), ArgSide::Left);
  }
};

// Multiplication ///////////////////////////////////////////////////////////
template <class R, class Arg>
struct MulMock {
  MOCK_METHOD(R, Multiplication, (Arg, ArgSide), (const));

  friend constexpr auto operator*(const MulMock& mock, Arg value) -> R {
    return mock.Multiplication(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator*(Arg value, const MulMock& mock) -> R {
    return mock.Multiplication(std::move(value), ArgSide::Left);
  }
};

// Division /////////////////////////////////////////////////////////////////
template <class R, class Arg>
struct DivMock {
  MOCK_METHOD(R, Division, (Arg, ArgSide), (const));

  friend constexpr auto operator/(const DivMock& mock, Arg value) -> R {
    return mock.Division(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator/(Arg value, const DivMock& mock) -> R {
    return mock.Division(std::move(value), ArgSide::Left);
  }
};
template <class R, class Arg>
struct ArithmeticMock : AddMock<R, Arg>,
                        SubMock<R, Arg>,
                        MulMock<R, Arg>,
                        DivMock<R, Arg> {};

}  // namespace tests
