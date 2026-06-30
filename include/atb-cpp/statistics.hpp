#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <optional>

namespace atb {

/**
 * @brief Update the provided arithmetic mean by adding a new sample xn
 *
 * @param[in] mean The previous mean computed for n-1
 * @param[in] x The new sample Xn
 * @param[in] n The sample's number
 *
 * @return M The new mean updated
 */
template <class M, class T>
constexpr auto UpdateMean(M mean, const T &x, std::size_t n) -> M {
  return (mean + ((x - mean) / n));
}

/**
 * @brief Update the provided mean and variance by adding a new sample xn,
 *        following the Welford's online algorithm
 *
 * @warning Only start to update the sampled variance's value when n > 0
 *
 * @note See https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 *
 * @param[in] var The previous variance computed for n-1
 * @param[in] mean The previous mean computed for n-1
 * @param[in] x The new sample Xn
 * @param[in] n The sample's number
 *
 * @return std::pair<V, M> A pair containing the updated variance and mean for
 *         step N
 */
template <class V, class M, class T>
constexpr auto UpdateVar(V var, M mean, const T &x,
                         std::size_t n) -> std::pair<V, M> {
  const auto delta = (x - mean);
  mean = UpdateMean(mean, x, n);

  if (n > 0) {
    var += ((delta * (x - mean) - var) / n);
  }

  return std::make_pair(var, mean);
}

/**
 * @brief Update the provided mean and sampled variance by adding a new sample
 *        xn, following the Welford's online algorithm
 *
 * @warning Only start to update the sampled variance's value when n > 1
 *
 * @note See https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 *
 * @param[in] svar The previous sampled variance computed for n-1
 * @param[in] mean The previous mean computed for n-1
 * @param[in] x The new sample Xn
 * @param[in] n The sample's number
 *
 * @return std::pair<V, M> A pair containing the updated sampled variance and
 *         mean for step N
 */
template <class V, class M, class T>
constexpr auto UpdateSVar(V svar, M mean, const T &x,
                          std::size_t n) -> std::pair<V, M> {
  const auto delta = (x - mean);
  mean = UpdateMean(mean, x, n);

  if (n > 1) {
    svar += (((delta * (x - mean)) / x.n) - (svar / (n - 1)));
  }

  return std::make_pair(svar, mean);
}

/**
 * @brief Update the provided mean and sum of square by adding a new sample xn,
 *        following the Welford's online algorithm
 *
 * The sum of squares is an alternative value that may be computed in order to
 * obtain the variance or sampled variance by simply dividing it to n (n-1
 * respectively)
 *
 * It has the advantage of not suffering from numerical instability over the
 * standard recurrent function of the variances.
 *
 * @note See https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 *
 * @param[in] sum The previous sum of square computed for n-1
 * @param[in] mean The previous mean computed for n-1
 * @param[in] x The new sample Xn
 * @param[in] n The sample's number
 *
 * @return std::pair<V, M> A pair containing the updated sum of square and
 *         mean for step N
 */
template <class V, class M, class T>
constexpr auto UpdateSumSquare(V sum, M mean, const T &x,
                               std::size_t n) -> std::pair<V, M> {
  const auto delta = (x - mean);
  mean = UpdateMean(mean, x, n);

  sum += (delta * (x - mean));

  return std::make_pair(sum, mean);
}

/**
 * @brief Build up simple online statistics (mean/variance) using Welford's
 *        online algorithm
 */
template <class _ElementType, class _Mean = double, class _Var = double>
struct OnlineStats {
  /// Expected sample's type
  using element_t = _ElementType;

  /// Underlying type using to compute the mean
  using mean_t = _Mean;

  /// Underlying type using to compute the variance
  using variance_t = _Var;

  /// Default construct a Stats (everything set to 0)
  constexpr OnlineStats() = default;

  /**
   * @brief Construct a Stats by initializing it with a range of values
   *
   * @param[in] [first, last) A range of values
   */
  template <
      class InputIt,
      std::enable_if_t<
          std::is_convertible_v<
              typename std::iterator_traits<InputIt>::reference, element_t>,
          bool> = true>
  constexpr explicit OnlineStats(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      Update(*first);
    }
  }

  /**
   * @brief Construct a Stats by initializing it with a range of values
   *
   * @param[in] values A range of values
   */
  constexpr explicit OnlineStats(std::initializer_list<element_t> values)
      : OnlineStats(std::begin(values), std::end(values)) {}

  /**
   * @return std::size_t The current number of sample
   */
  constexpr auto N() const noexcept -> std::size_t { return m_n; }

  /**
   * @return mean_t The current arithmetic mean computed for N() samples
   */
  constexpr auto Mean() const noexcept -> mean_t { return m_mean; }

  /**
   * @return variance_t The current sum of square computed for N() samples
   */
  constexpr auto Sum() const noexcept -> variance_t { return m_sum; }

  /**
   * @return std::optional<variance_t> The current variance computed for N()
   *         samples IF N() > 0, std::nullopt otherwise.
   */
  constexpr auto Var() const noexcept -> std::optional<variance_t> {
    std::optional<variance_t> var = std::nullopt;
    if (N() > 0) var = (Sum() / N());
    return var;
  }

  /**
   * @return std::optional<variance_t> The current sampled variance computed
   *         for N() samples IF N() > 1, std::nullopt otherwise.
   */
  constexpr auto SVar() const noexcept -> std::optional<variance_t> {
    std::optional<variance_t> svar = std::nullopt;
    if (N() > 1) svar = (Sum() / (N() - 1));
    return svar;
  }

  /**
   * @brief Update the stats using a new sample Xn
   *
   * @param[in] x A new sample Xn
   *
   * @return True on successfull update, false otherwise (N overflows)
   */
  constexpr auto Update(const element_t &x) -> bool {
    if (m_n == std::numeric_limits<std::size_t>::max()) return false;

    m_n += 1;

    std::tie(m_sum, m_mean) = UpdateSumSquare(m_sum, m_mean, x, m_n);

    return true;
  }

  /**
   * @brief Reset the current stats to 0
   */
  constexpr auto Reset() -> void {
    m_mean = 0;
    m_sum = 0;
    m_n = 0u;
  }

 private:
  mean_t m_mean = 0;    /*!< The recurrent arithmetic mean computed at N */
  variance_t m_sum = 0; /*!< The recurrent sum of square computed at N */
  std::size_t m_n;      /*!< The current step N */
};

}  // namespace atb
