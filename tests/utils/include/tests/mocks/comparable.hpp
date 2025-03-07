#pragma once

#include "gmock/gmock.h"
#include "tests/mocks/arg_side.hpp"

namespace tests {

// == /////////////////////////////////////////////////////////////////
template <class R, class Arg>
struct EqMock {
  MOCK_METHOD(R, Eq, (Arg, ArgSide), (const));

  friend constexpr auto operator==(const EqMock& mock, Arg value) -> R {
    return mock.Eq(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator==(Arg value, const EqMock& mock) -> R {
    return mock.Eq(std::move(value), ArgSide::Left);
  }
};

// != /////////////////////////////////////////////////////////////
template <class R, class Arg>
struct NeMock {
  MOCK_METHOD(R, Ne, (Arg, ArgSide), (const));

  friend constexpr auto operator!=(const NeMock& mock, Arg value) -> R {
    return mock.Ne(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator!=(Arg value, const NeMock& mock) -> R {
    return mock.Ne(std::move(value), ArgSide::Left);
  }
};

// Greater ///////////////////////////////////////////////////////////
template <class R, class Arg>
struct GeMock {
  MOCK_METHOD(R, Ge, (Arg, ArgSide), (const));

  friend constexpr auto operator>=(const GeMock& mock, Arg value) -> R {
    return mock.Ge(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator>=(Arg value, const GeMock& mock) -> R {
    return mock.Ge(std::move(value), ArgSide::Left);
  }
};

template <class R, class Arg>
struct GtMock {
  MOCK_METHOD(R, Gt, (Arg, ArgSide), (const));

  friend constexpr auto operator>(const GtMock& mock, Arg value) -> R {
    return mock.Gt(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator>(Arg value, const GtMock& mock) -> R {
    return mock.Gt(std::move(value), ArgSide::Left);
  }
};

// Lesser ///////////////////////////////////////////////////////////
template <class R, class Arg>
struct LeMock {
  MOCK_METHOD(R, Le, (Arg, ArgSide), (const));

  friend constexpr auto operator<=(const LeMock& mock, Arg value) -> R {
    return mock.Le(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator<=(Arg value, const LeMock& mock) -> R {
    return mock.Le(std::move(value), ArgSide::Left);
  }
};

template <class R, class Arg>
struct LtMock {
  MOCK_METHOD(R, Lt, (Arg, ArgSide), (const));

  friend constexpr auto operator<(const LtMock& mock, Arg value) -> R {
    return mock.Lt(std::move(value), ArgSide::Right);
  }

  friend constexpr auto operator<(Arg value, const LtMock& mock) -> R {
    return mock.Lt(std::move(value), ArgSide::Left);
  }
};

template <class R, class Arg>
struct ComparableMock : EqMock<R, Arg>,
                        NeMock<R, Arg>,
                        GeMock<R, Arg>,
                        GtMock<R, Arg>,
                        LeMock<R, Arg>,
                        LtMock<R, Arg> {};

}  // namespace tests
