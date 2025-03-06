#pragma once

#include <tuple>
#include <type_traits>

#include "gmock/gmock.h"

namespace tests {

namespace details {

template <class... T>
struct FirstOf;

template <class T, class... Others>
struct FirstOf<T, Others...> {
  using type = T;
};

template <class... T>
using FirstOf_t = typename FirstOf<T...>::type;

}  // namespace details

template <class R, class... Args>
struct CallableMock {
  using return_t = R;
  using arg_t = std::conditional_t<
      sizeof...(Args) == 0, void,
      std::conditional_t<sizeof...(Args) == 1, details::FirstOf_t<Args...>,
                         std::tuple<Args...>>>;

  MOCK_METHOD(return_t, Call, (arg_t), (const));
  constexpr auto operator()(Args... args) const -> return_t {
    if constexpr (sizeof...(Args) == 0) {
      return Call();
    } else if constexpr (sizeof...(Args) == 1) {
      return Call(std::move(args)...);
    } else {
      return Call(std::make_tuple(std::move(args)...));
    }
  }
};

}  // namespace tests
