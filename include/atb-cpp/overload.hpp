#pragma once

#include "atb-cpp/type_traits.hpp"

namespace atb {

/// Implementation of the overloads pattern
///(see https://en.cppreference.com/w/cpp/utility/variant/visit)
template <typename... Matchers>
struct Overload : Matchers... {
  using Matchers::operator()...;
};

/// CTAD (Not needed as of c++20)
template <typename... Matchers>
Overload(Matchers...) -> Overload<Matchers...>;

/// Implementation of the overloads pattern
///(see https://en.cppreference.com/w/cpp/utility/variant/visit)
template <typename... Matchers>
struct StrictOverload : Matchers... {
  using Matchers::operator()...;

  /// Default overload use to triggers a compilation error when an operator<T>()
  /// is missing
  template <typename... T>
  constexpr auto operator()(T...) const {
    static_assert(AlwaysFalse_v<T...>, "operator()(T...) is not implemented");
  }
};

/// CTAD (Not needed as of c++20)
template <typename... Matchers>
StrictOverload(Matchers...) -> StrictOverload<Matchers...>;

}  // namespace atb
