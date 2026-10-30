#pragma once

#include <format>  // std::format
#include <string_view>
#include <tuple>

#include "atb-cpp/type_traits.hpp"

namespace atb {

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
      T&& new_fallback) && -> FmtTry<value_type, std::remove_cvref_t<T>> {
    return {
        .value = std::move(value),
        .fallback = std::forward<T>(new_fallback),
    };
  }

  template <class T>
  constexpr auto OrElse(
      T&& new_fallback) const& -> FmtTry<value_type, std::remove_cvref_t<T>> {
    return {
        .value = value,
        .fallback = std::forward<T>(new_fallback),
    };
  }
};

/// std::format adapter that either print value OR (when value is considered not
/// formatable) a \a fallback value (default to '<?>')
template <class G, class... Ts>
struct FmtJoin {
  G glue;
  std::tuple<Ts...> values;

  constexpr explicit FmtJoin(G g, Ts... args) noexcept
      : glue(std::move(g)), values(std::move(args)...) {}
};

}  // namespace atb

namespace atb::details {

constexpr auto fmt_get_flag(char flag, bool& d_value, auto it, const auto end) {
  if (it != end && *it == flag) {
    d_value = true;
    ++it;
  }
  return it;
}

constexpr auto fmt_throw_when_invalid(
    auto it, const auto end, std::string msg = "Invalid format specifier") {
  if ((it != end) && (*it != '}')) {
    throw std::format_error(std::move(msg));
  }

  return it;
}

}  // namespace atb::details

namespace std {

/// TODO
template <class T, class U, class CharT>
  requires atb::formattable<T, CharT>
class formatter<atb::FmtTry<T, U>, CharT> {
  bool m_is_repr = false; /*!< True whenever fmt parsed a 'REPR' (i.e. :r) */
  std::formatter<T, CharT> m_fmt_value; /*!< value formatter */

 public:
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    ctx.advance_to(
        atb::details::fmt_get_flag('r', m_is_repr, ctx.begin(), ctx.end()));
    return m_fmt_value.parse(ctx);
  }

  template <class FormatContext>
  auto format(const atb::FmtTry<T, U>& fmt_or, FormatContext& ctx) const {
    if (m_is_repr) {
      ctx.advance_to(std::format_to(ctx.out(), "FmtTry{{.value = "));
      ctx.advance_to(m_fmt_value.format(fmt_or.value, ctx));
      return std::format_to(ctx.out(), ", .fallback = {}}}", fmt_or.fallback);
    } else {
      return m_fmt_value.format(fmt_or.value, ctx);
    }
  }
};

/// TODO
template <class T, class U, class CharT>
  requires(!atb::formattable<T, CharT>) && atb::formattable<U, CharT>
class formatter<atb::FmtTry<T, U>, CharT> {
  bool m_is_repr = false; /*!< True whenever fmt parsed a 'REPR' (i.e. :r) */
  std::formatter<U, CharT> m_fmt_fallback; /*!< fallback formatter */

 public:
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    ctx.advance_to(
        atb::details::fmt_get_flag('r', m_is_repr, ctx.begin(), ctx.end()));
    return m_fmt_fallback.parse(ctx);
  }

  template <class FormatContext>
  auto format(const atb::FmtTry<T, U>& fmt_or, FormatContext& ctx) const {
    if (m_is_repr) {
      return std::format_to(ctx.out(), "FmtTry{{.value = <?>, .fallback = {}}}",
                            fmt_or.fallback);
    } else {
      return m_fmt_fallback.format(fmt_or.fallback, ctx);
    }
  }
};

/// Format adapter for FmtJoin
template <class G, class... Ts, class CharT>
  requires atb::formattable<G, CharT> && (atb::formattable<Ts, CharT> && ...)
class formatter<atb::FmtJoin<G, Ts...>, CharT> {
  bool m_is_repr = false; /*!< True whenever fmt parsed a 'REPR' (i.e. :r) */

 public:
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    const auto end = ctx.end();
    auto it = atb::details::fmt_get_flag('r', m_is_repr, ctx.begin(), end);
    return atb::details::fmt_throw_when_invalid(it, end);
  }

  template <class FormatContext>
  auto format(const atb::FmtJoin<G, Ts...>& fmt_join,
              FormatContext& ctx) const {
    auto out = ctx.out();
    if (m_is_repr) {
      out = std::format_to(out, "FmtJoin{{.glue = {}, .values = {{",
                           fmt_join.glue);

      if constexpr (sizeof...(Ts) != 0) {
        std::apply(
            [&out](const auto& first, const auto&... others) {
              out = std::format_to(out, "{}", first);
              ((out = std::format_to(out, ", {}", others)), ...);
            },
            fmt_join.values);
      }

      out = std::format_to(out, "}}}}");
    } else {
      if constexpr (sizeof...(Ts) != 0) {
        std::apply(
            [&out, &fmt_join](const auto& first, const auto&... others) {
              out = std::format_to(out, "{}", first);
              ((out = std::format_to(out, "{}{}", fmt_join.glue, others)), ...);
            },
            fmt_join.values);
      }
    }
    return out;
  }
};

}  // namespace std
