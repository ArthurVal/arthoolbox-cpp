#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <format>  // std::format
#include <ranges>
#include <string_view>
#include <tuple>

#include "atb-cpp/type_traits.hpp"  // ATB_FWD

namespace atb {

/// Help function to test if the parse context iterator didn't reach the end
constexpr auto FmtParseContextNotOver(const auto it, const auto end) -> bool {
  return ((it != end) && (*it != '}'));
}

/// Checks if a given type T has a valie std::formatter defined
template <class T, class CharT = char>
concept formattable = std::is_default_constructible_v<
    std::formatter<std::remove_cvref_t<T>, CharT>>;

/// std::format adapter that either print value OR (when value is considered not
/// formatable) a \a fallback value (default to '<?>')
template <class _T, class _U = std::string_view>
struct FmtTry {
  using value_type = _T;
  using fallback_type = _U;

  /*!< Value we wish to print */
  value_type value;

  /*!< Backup string printed */
  fallback_type fallback = []() {
    if constexpr (std::is_constructible_v<fallback_type, const char*>) {
      return fallback_type{"<?>"};
    } else {
      return fallback_type{};
    }
  }();

  template <class T>
  constexpr auto OrElse(
      T&& other) && -> FmtTry<value_type, std::remove_cvref_t<T>> {
    return {
        .value = std::move(value),
        .fallback = ATB_FWD(other),
    };
  }

  template <class T>
  constexpr auto OrElse(
      T&& other) const& -> FmtTry<value_type, std::remove_cvref_t<T>> {
    return {
        .value = value,
        .fallback = ATB_FWD(other),
    };
  }
};

}  // namespace atb

namespace std {

/// FmtTry formatter -- Normal value
template <class T, class U, class CharT>
  requires atb::formattable<T, CharT>
class formatter<atb::FmtTry<T, U>, CharT> {
  std::formatter<T, CharT> m_fmt_value; /*!< value formatter */

 public:
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return m_fmt_value.parse(ctx);
  }

  template <class FmtContext>
  auto format(const atb::FmtTry<T, U>& fmt_or, FmtContext& ctx) const {
    return m_fmt_value.format(fmt_or.value, ctx);
  }
};

/// FmtTry formatter -- Fallback
template <class T, class U, class CharT>
  requires(!atb::formattable<T, CharT>) && atb::formattable<U, CharT>
class formatter<atb::FmtTry<T, U>, CharT> {
  std::formatter<U, CharT> m_fmt_fallback;

 public:
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    // Providing specifier for the fallback is not allowed but should cause an
    // error (in case we use fmt specifier for the value)
    return ctx.begin();
  }

  template <class FmtContext>
  auto format(const atb::FmtTry<T, U>& fmt_or, FmtContext& ctx) const {
    return m_fmt_fallback.format(fmt_or.fallback, ctx);
  }
};

}  // namespace std

namespace atb {

/// std::format adapter use to join multiple arguments around a 'glue'
///
/// Using FmtJoin correspond to:
/// - FmtJoin(GLUE, V1, V2, V3 ...) ~ "{V1}{GLUE}{V2}{GLUE}{V3}{GLUE}..."
///
/// By default, when each Vi types differs, the FmtJoin use and underlying tuple
/// as container. Because all types differs, providing a format specifier is
/// forbidden.
/// If all the types are the same, the underlying container will be an
/// std::array and giving format specifier is allowed (the same specifier will
/// be used for ALL elements).
///
/// When providing a single value which is a valid iterable (based on the
/// std::ranges::range concept), the FmtJoin will iterate over all the elements
/// of the range instead.
template <class G, class Container>
struct FmtJoin {
  G glue;           /*!< GLUE element used during the JOIN */
  Container values; /*!< Container holding ALL values to join */

  constexpr FmtJoin(G g, auto&&... args)
      : glue{std::move(g)}, values{ATB_FWD(args)...} {}
};

// CTAD - Deduce when giving any iterable
template <class G, std::ranges::range R>
FmtJoin(G, R&&) -> FmtJoin<G, std::views::all_t<R>>;

// CTAD - Deduced when multiple identical types are provided
template <class G, class T1, class T2, class... Ts>
  requires(std::same_as<std::decay_t<T1>, std::decay_t<T2>> &&
           (std::same_as<std::decay_t<T1>, std::decay_t<Ts>> && ...))
FmtJoin(G, T1&&, T2&&, Ts&&...)
    -> FmtJoin<G, std::array<std::decay_t<T1>, 2 + sizeof...(Ts)>>;

// CTAD - Tuple
template <class G, class... Ts>
FmtJoin(G, Ts&&...) -> FmtJoin<G, std::tuple<std::decay_t<Ts>...>>;

}  // namespace atb

namespace std {

/// FmtJoin formatter -- TUPLE
template <class G, class... Ts, class CharT>
  requires atb::formattable<G, CharT> && (atb::formattable<Ts, CharT> && ...)
class formatter<atb::FmtJoin<G, std::tuple<Ts...>>, CharT> {
  std::formatter<G, CharT> m_fmt_glue;

 public:
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    auto it = ctx.begin();
    if (atb::FmtParseContextNotOver(it, ctx.end())) {
      throw std::format_error{
          "FmtJoin using different value types doesn't support element format "
          "specifiers",
      };
    }
    return it;
  }

  template <class FmtContext>
  auto format(const atb::FmtJoin<G, std::tuple<Ts...>>& join,
              FmtContext& ctx) const {
    if constexpr (sizeof...(Ts) != 0) {
      std::apply(
          [&](const auto& first, const auto&... others) {
            ctx.advance_to(std::format_to(ctx.out(), "{}", first));
            ((ctx.advance_to(m_fmt_glue.format(join.glue, ctx)),
              ctx.advance_to(std::format_to(ctx.out(), "{}", others))),
             ...);
          },
          join.values);
    }

    return ctx.out();
  }
};

/// FmtJoin formatter -- RANGE / ARRAY
template <class G, class R, class CharT>
  requires std::ranges::input_range<R> && atb::formattable<G, CharT> &&
           atb::formattable<
               std::remove_cvref_t<std::ranges::range_reference_t<R>>, CharT>
class formatter<atb::FmtJoin<G, R>, CharT>
    : public std::formatter<
          std::remove_cvref_t<std::ranges::range_reference_t<R>>, CharT> {
  std::formatter<G, CharT> m_fmt_glue;

  using base_formatter =
      std::formatter<std::remove_cvref_t<std::ranges::range_reference_t<R>>,
                     CharT>;

 public:
  template <class FmtContext>
  auto format(const atb::FmtJoin<G, R>& join, FmtContext& ctx) const {
    auto it = std::ranges::begin(join.values);
    const auto end = std::ranges::end(join.values);

    if (it == end) {
      return ctx.out();
    }

    ctx.advance_to(base_formatter::format(*it, ctx));
    for (++it; it != end; ++it) {
      ctx.advance_to(m_fmt_glue.format(join.glue, ctx));
      ctx.advance_to(base_formatter::format(*it, ctx));
    }

    return ctx.out();
  }
};

}  // namespace std
