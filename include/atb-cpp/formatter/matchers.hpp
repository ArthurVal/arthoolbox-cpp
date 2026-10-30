#pragma once

#include <format>
#include <tuple>
#include <utility>

#include "atb-cpp/format.hpp"
#include "atb-cpp/matchers.hpp"

namespace atb::details {

/**
 * @brief Base class for Matcher formatters to handle std::formatter parsing.
 */
struct MatcherFormatterBase {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && *it != '}') {
      throw std::format_error(
          "Invalid format specifier for Matcher. Use default {}");
    }
    return it;
  }
};

}  // namespace atb::details

namespace std {

// COMMON MATCHERS FORMATTERS //////////////////////////////////////////////////

template <bool Value, class CharT>
struct formatter<atb::Always<Value>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Always<Value>&, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "Always {}", Value);
  }
};

template <class T, class CharT>
struct formatter<atb::Eq<T>, CharT> : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Eq<T>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "== {}", atb::FmtTry{m.expected});
  }
};

template <class T, class CharT>
struct formatter<atb::Ne<T>, CharT> : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Ne<T>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "!= {}", atb::FmtTry{m.expected});
  }
};

template <class T, class CharT>
struct formatter<atb::Ge<T>, CharT> : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Ge<T>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), ">= {}", atb::FmtTry{m.expected});
  }
};

template <class T, class CharT>
struct formatter<atb::Gt<T>, CharT> : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Gt<T>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "> {}", atb::FmtTry{m.expected});
  }
};

template <class T, class CharT>
struct formatter<atb::Le<T>, CharT> : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Le<T>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "<= {}", atb::FmtTry{m.expected});
  }
};

template <class T, class CharT>
struct formatter<atb::Lt<T>, CharT> : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Lt<T>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "< {}", atb::FmtTry{m.expected});
  }
};

template <class T, class Error, class CharT>
struct formatter<atb::Near<T, Error>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Near<T, Error>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "~ {} (+/- {})", atb::FmtTry{m.expected},
                          atb::FmtTry{m.abs_error});
  }
};

// COMPOSITE MATCHERS FORMATTERS ///////////////////////////////////////////////

template <class Matcher, class CharT>
struct formatter<atb::Not<Matcher>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::Not<Matcher>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "Not({})", atb::FmtTry{m.matcher});
  }
};

template <class... Matchers, class CharT>
struct formatter<atb::AllOf<Matchers...>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::AllOf<Matchers...>& all_of, FormatContext& ctx) const {
    return std::format_to(
        ctx.out(), "[{}]",
        atb::FmtJoin{
            .glue = " AND ",
            .values = std::apply(
                [](const auto&... m) { return std::tie(atb::FmtTry(m)...); },
                all_of.matchers),
        });
  }
};

template <class... Matchers, class CharT>
struct formatter<atb::AnyOf<Matchers...>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::AnyOf<Matchers...>& any_of, FormatContext& ctx) const {
    return std::format_to(
        ctx.out(), "[{}]",
        atb::FmtJoin{
            .glue = " OR ",
            .values = std::apply(
                [](const auto&... m) { return std::tie(atb::FmtTry(m)...); },
                any_of.matchers),
        });
  }
};

// FILTER ARGS COMPOSITE MATCHERS FORMATTERS ///////////////////////////////////

template <class Matcher, std::size_t... Is, class CharT>
struct formatter<atb::SelectArgsMatcher<Matcher, Is...>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::SelectArgsMatcher<Matcher, Is...>& m,
              FormatContext& ctx) const {
    return std::format_to(ctx.out(), "If args [{}] {}",
                          atb::FmtJoin(", ", Is...), atb::FmtTry(m.matcher));
  }
};

template <class Matcher, class CharT>
struct formatter<atb::EveryArg<Matcher>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::EveryArg<Matcher>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "If EVERY args {}",
                          atb::FmtTry(m.matcher));
  }
};

template <class Matcher, class CharT>
struct formatter<atb::AnyArg<Matcher>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::AnyArg<Matcher>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "If ANY args {}", atb::FmtTry(m.matcher));
  }
};

template <class F, class Matcher, class CharT>
struct formatter<atb::MapArgs<F, Matcher>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::MapArgs<F, Matcher>& m, FormatContext& ctx) const {
    // Note: Callables (F) are generally not natively format-able in C++
    return std::format_to(ctx.out(), "Map args (f: {}) and then {}",
                          atb::FmtTry(m.transform), atb::FmtTry(m.matcher));
  }
};

template <class F, class Matcher, class CharT>
struct formatter<atb::ReduceArgs<F, Matcher>, CharT>
    : atb::details::MatcherFormatterBase {
  template <class FormatContext>
  auto format(const atb::ReduceArgs<F, Matcher>& m, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "Reduce args (f: {}) and then {}",
                          atb::FmtTry(m.reduce), atb::FmtTry(m.matcher));
  }
};

}  // namespace std
