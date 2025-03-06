#pragma once

#include <functional>  // std::reference_wrapper

namespace atb {

namespace details {
/// Transforms std::reference_wrapper<T> to T&. Any other type is untouched.
template <class T>
struct UnwrapRefWrapper {
  using type = T;
};

template <class T>
struct UnwrapRefWrapper<std::reference_wrapper<T>> {
  using type = T&;
};

template <class T>
using UnwrapRefWrapper_t = typename UnwrapRefWrapper<T>::type;

}  // namespace details

}  // namespace atb
