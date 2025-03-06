#pragma once

#include <functional>  // std::invoke
#include <tuple>

namespace atb::tpl {

/**
 *  @brief Invoke f with the elements of tpl filtered by indexes I
 *
 *  @note Same as std::apply but can be called with a subset of elements
 *
 *  @tparam I... Indexes selecting elements inside input tuple
 *
 *  @param[in] f Callable taking all the Ith elements of tpl as arguments
 *  @param[in] tpl Tuple like object
 */
template <std::size_t... I, class F, class Tpl>
constexpr auto Apply(F&& f, Tpl&& tpl) -> decltype(auto) {
  if constexpr (sizeof...(I) == 0) {
    return std::apply(std::forward<F>(f), std::forward<Tpl>(tpl));
  } else {
    return std::invoke(std::forward<F>(f),
                       std::get<I>(std::forward<Tpl>(tpl))...);
  }
}

/**
 *  @brief Same as Apply<I...>(f, tpl) using an index_sequence helper as
 *         argument to automatically deduce indexes
 */
template <class F, class Tpl, std::size_t... I>
constexpr auto Apply(F&& f, Tpl&& tpl,
                     std::index_sequence<I...>) -> decltype(auto) {
  return Apply<I...>(std::forward<F>(f), std::forward<Tpl>(tpl));
}

namespace details {

template <class T>
constexpr auto min(T&& t) -> decltype(auto) {
  return std::forward<T>(t);
}

template <class T0, class T1, class... Tn>
constexpr auto min(T0&& t0, T1&& t1, Tn&&... others) -> decltype(auto) {
  if (t0 < t1) {
    return min(std::forward<T0>(t0), std::forward<Tn>(others)...);
  } else {
    return min(std::forward<T1>(t1), std::forward<Tn>(others)...);
  }
}

}  // namespace details

/**
 *  @brief Invoke v(std::get<I>(tpls)...) for each elements Ith of all tpls
 *
 *  @tparam I... Indexes selecting elements inside input tuples
 *
 *  @param[in] v Visitor call for each elements
 *  @param[in] tpls... Tuple like objects
 */
template <std::size_t I, std::size_t... Others, class Visitor,
          class... TplLikes>
constexpr auto Visit(Visitor&& v, TplLikes&&... tpls) -> void {
  static_assert(I < details::min(std::tuple_size_v<std::decay_t<TplLikes>>...),
                "Out of bound tuple index access");

  std::invoke(std::forward<Visitor>(v),
              std::get<I>(std::forward<decltype(tpls)>(tpls))...);

  if constexpr (sizeof...(Others) != 0) {
    Visit<Others...>(std::forward<Visitor>(v), std::forward<TplLikes>(tpls)...);
  }
}

/**
 *  @brief Invoke v(std::get<I>(tpls)...) for each elements Ith of all tpls
 *
 *  @note Same as VisitTuples<I...>(v, tpls...) using an index_sequence helper
 *        as argument to automatically deduce indexes
 *  @note No-op when sizeof...(I)  is 0
 *
 *  @param[in] v Visitor call for each elements
 *  @param[in] tpls... Tuple like objects
 */
template <class Visitor, class... TplLikes, std::size_t... I>
constexpr auto Visit(std::index_sequence<I...>, Visitor&& v,
                     TplLikes&&... tpls) -> void {
  if constexpr (sizeof...(I) >= 1) {
    Visit<I...>(std::forward<Visitor>(v), std::forward<TplLikes>(tpls)...);
  }
}

/**
 *  @brief Invoke v(...) for each elements of all tpls
 *
 *  @note Takes the smallest tuple as reference to stop the visit
 *
 *  @param[in] v Visitor call for each elements
 *  @param[in] tpls... Tuple like objects
 */
template <class Visitor, class... TplLikes>
constexpr auto Visit(Visitor&& v, TplLikes&&... tpls) -> void {
  constexpr auto smallest_size =
      details::min(std::tuple_size_v<std::decay_t<TplLikes>>...);

  Visit(std::make_index_sequence<smallest_size>{}, std::forward<Visitor>(v),
        std::forward<TplLikes>(tpls)...);
}

/**
 *  @brief Reduce the given tuple, applying init = f(init, e) for each elements
 *         'e' of the given input tuple
 *
 *  @param[in] init The initial value containing the reduced value
 *  @param[in] f A binary operator called with the following signature:
 *               f(T, tuple_element_t<I>) -> T for each elements I of the tuple;
 *  @param[in] tpl The tuple we wish to reduce
 *
 *  @return T Result of the reduction operation
 */
template <class T, class BinaryOp, class Tpl>
constexpr auto Reduce(T init, BinaryOp&& f, Tpl&& tpl) -> T {
  Visit(
      [&init, &f](auto&& v) {
        init = std::invoke(f, init, std::forward<decltype(v)>(v));
      },
      std::forward<Tpl>(tpl));
  return init;
}

/**
 *  @brief Create a new tuple containing the result of invoking F(...) over all
 *         input tuples for each Ith elements sequentially
 *
 *  Given:
 *
 *  - The tuples:
 *  Tuple A  Tuple B  ...  Tuple Z
 *   [A_0]    [B_0]   ...   [Z_0]
 *   [A_1]    [B_1]   ...   [Z_1]
 *    ...      ...    ...    ...
 *   [A_An]   [B_Bn]  ...   [Z_Zn]
 *
 *  - A list of m indexes [I_0, I_1, ..., I_m]
 *
 *  We create a new tuple R:
 *  Tuple R
 *   [R_0]  = f(A[I_0], B[I_0], ..., Z[I_0])
 *   [R_1]  = f(A[I_1], B[I_1], ..., Z[I_1])
 *    ...   = f(...                        )
 *   [R_Rm] = f(A[I_m], B[I_m], ..., Z[I_m])
 *
 *  With m being the size of the output tuple R, corresponding to the number of
 *  indexes given.
 *
 *  @pre Each indexes I must by inferior to the size of the smallest tuple
 *
 *  @tparam I, Others... Indexes selecting elements inside inpiut tuples
 *
 *  @param[in] f An operator called with each tuples elements
 *  @param[in] tpls... All tuples we wish to transform
 *
 *  @return std::tuple<...> Containing the result of each function call. Size
 *          matching the number of indexes.
 */
template <std::size_t I, std::size_t... Others, class F, class... TplLikes>
constexpr auto Transform(F&& f, TplLikes&&... tpls) {
  static_assert(I < details::min(std::tuple_size_v<std::decay_t<TplLikes>>...),
                "Out of bound tuple index access");

  if constexpr (sizeof...(Others) == 0) {
    return std::make_tuple(std::invoke(
        std::forward<F>(f), std::get<I>(std::forward<TplLikes>(tpls))...));
  } else {
    return std::tuple_cat(
        Transform<I>(f, tpls...),
        Transform<Others...>(std::forward<F>(f),
                             std::forward<TplLikes>(tpls)...));
  }
}

/**
 *  @brief Same as TransformTuples<I...>(f, tpls...) using an index_sequence
 *         helper as argument to automatically deduce indexes
 */
template <class F, std::size_t... I, class... TplLikes>
constexpr auto Transform(F&& f, std::index_sequence<I...>, TplLikes&&... tpls) {
  if constexpr (sizeof...(I) >= 1) {
    return Transform<I...>(std::forward<F>(f), std::forward<TplLikes>(tpls)...);
  } else {
    return std::make_tuple();
  }
}

/**
 *  @brief Same as TransformTuples<I...>(f, tpls...) with I... being a continous
 *         range, starting from 0, ending based on the size of the smallest
 *         tuple
 */
template <class F, class... TplLikes>
constexpr auto Transform(F&& f, TplLikes&&... tpls) {
  constexpr auto smallest_size =
      details::min(std::tuple_size_v<std::decay_t<TplLikes>>...);

  return Transform(std::forward<F>(f),
                   std::make_index_sequence<smallest_size>{},
                   std::forward<TplLikes>(tpls)...);
}

/**
 *  @brief Apply Transform on the input tuple, then Reduce.
 *
 *  Correspond to the following:
 *  Tuple A  Tuple B  ...  Tuple M         TRANSFORM         Tuple R (size N)
 *   [A_1]    [B_1]   ...   [M_1]  -> t(A_1, B_1, ..., M_1) -> [R_1]
 *   [A_2]    [B_2]   ...   [M_2]  -> t(A_2, B_2, ..., M_2) -> [R_2]
 *    ...      ...    ...    ...   -> t(..., ..., ..., ...) ->  ...
 *   [A_N]    [B_N]   ...   [M_N]  -> t(A_N, B_N, ..., M_N) -> [R_N]
 *   [A_N+1]          ...   [M_N+1]
 *    ...                    ...                                 |
 *    ...                    ...                                 V
 *                                                |  init = r(init, [R_1])
 *                                         REDUCE |  init = r(init, [R_2])
 *                                                |           ...
 *                                                |  init = r(init, [R_N])
 *                                                               |
 *                                                               V
 *                                                             init
 *
 *  With N being the size of the smallest tuple (Tuple B in this example).
 *
 *  @param[in] init The initial value containing the reduced value
 *  @param[in] r The reduction operator
 *  @param[in] t The transform operator
 *  @param[in] tpls... All tuples we wish to transform
 *
 *  @return T Result of the reduction operation, following the transformation
 */
template <class T, class ReduceOp, class TransformOp, class... TplLikes>
constexpr auto TransformReduce(T init, ReduceOp&& r, TransformOp&& t,
                               TplLikes&&... tpls) {
  return Reduce(
      std::move(init), std::forward<ReduceOp>(r),
      Transform(std::forward<TransformOp>(t), std::forward<TplLikes>(tpls)...));
}

}  // namespace atb::tpl
