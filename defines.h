#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>

/*
#include <format>
#include <iostream>

#include <algorithm>
#include <numeric>
#include <ranges>

#include <concepts>
#include <type_traits>
#include <utility>

#include <string_view>

#include <array>
#include <string>
#include <string_view>


namespace detail {
template <std::size_t N> struct string_literal {
  consteval string_literal(const char (&arr)[N]) noexcept {
    std::ranges::copy_n(arr, N, buffer);
  }

  template <std::size_t LhsSize, std::size_t RhsSize>
    requires(LhsSize + RhsSize - 1 == N)
  consteval string_literal(const string_literal<LhsSize> &lhs,
                           const string_literal<RhsSize> &rhs) noexcept {
    auto it =
        std::ranges::copy(lhs.buffer | std::views::take(LhsSize - 1), buffer)
            .out;
    std::ranges::copy(rhs.buffer, it);
  }

  template <std::size_t LhsSize, std::size_t RhsSize>
    requires(LhsSize + RhsSize == N)
  consteval string_literal(char delim, const string_literal<LhsSize> &lhs,
                           const string_literal<RhsSize> &rhs) noexcept {
    auto it =
        std::ranges::copy(lhs.buffer | std::views::take(LhsSize - 1), buffer)
            .out;
    *it = delim;
    ++it;
    std::ranges::copy(rhs.buffer, it);
  }

  char buffer[N]{};
  std::size_t size = N;
};

template <std::size_t LhsSize, std::size_t RhsSize>
string_literal(string_literal<LhsSize>, string_literal<RhsSize>)
    -> string_literal<LhsSize + RhsSize - 1>;

template <std::size_t LhsSize, std::size_t RhsSize>
string_literal(char, string_literal<LhsSize>,
               string_literal<RhsSize>) -> string_literal<LhsSize + RhsSize>;

template <string_literal String>
consteval auto get_static_buffer() noexcept -> const char (&)[String.size] {
  return String.buffer;
}
} // namespace detail

template <detail::string_literal Lhs, detail::string_literal Rhs>
consteval auto static_concat() noexcept -> const
    char (&)[Lhs.size + Rhs.size - 1] {
  return detail::get_static_buffer<detail::string_literal{Lhs, Rhs}>();
}

template <detail::string_literal Lhs, detail::string_literal Rhs,
          detail::string_literal... Others>
  requires(sizeof...(Others) != 0)
consteval decltype(auto) static_concat() noexcept {
  return static_concat<detail::string_literal{Lhs, Rhs}, Others...>();
}

template <char Delim, detail::string_literal Lhs, detail::string_literal Rhs>
consteval auto static_concat() noexcept -> const char (&)[Lhs.size + Rhs.size] {
  return detail::get_static_buffer<detail::string_literal{Delim, Lhs, Rhs}>();
}

template <char Delim, detail::string_literal Lhs, detail::string_literal Rhs,
          detail::string_literal... Others>
  requires(sizeof...(Others) != 0)
consteval decltype(auto) static_concat() noexcept {
  return static_concat<Delim, detail::string_literal{Delim, Lhs, Rhs},
                       Others...>();
}
    static constexpr decltype(auto) result = static_concat<'_', "concat",
   "test", "works", "perfectly">();

    static_assert(std::size(result) == 28);
    static_assert(std::same_as<decltype(result), const char (&)[28]>);

    static constexpr std::string_view sv { result };

    // FIXME: not compiled with MSVC, but works well with GCC/Clang
    // auto with_endl = static_concat<result, "\n">();
    // but the following one compiles
    auto with_endl = static_concat<detail::string_literal(result), "\n">();

    std::cout << with_endl;

    static constexpr decltype(auto) formatter = static_concat<' ', "{}", "{}",
   "{}">();

    std::cout << std::format(formatter, 1, 2, 3);
*/

#define VERSION_MAJOR "01"
#define VERSION_MINOR "00.00"
#define VERSION VERSION_MAJOR "." VERSION_MINOR
#define APPLICATIONNAME "QtScreenShoot"

// defaults
#define defaultHelp "True"
#define defaultClientHelp "False"
#define defaultServerHelp "False"
#define defaultVersion "True"
#define defaultVerbose "False"
#define defaultDebug "False"
#define defaultGui "False"
#define defaultFilename "untitled.jpg"
#define defaultNumber "1"
#define defaultIp "127.0.0.1"        // ip:port
#define defaultIncomePort 8000       // ip:port
#define defaultIncomePortStr "8000"  // ip:port
#define defaultOutcomePort 8001      // ip:port
#define defaultOutcomePortStr "8001" // ip:port
#define defaultBindSeperator ":"     // :
#define defaultBind                                                            \
  defaultIp defaultBindSeperator defaultIncomePortStr // ip:port
#define defaultUsage "client"
#define defaultConnectionType "TCP"
#define MaxPositionalArgs 0
#define Author "* Author: Mustafa Selçuk Çağlar\n"

#include "help.h"

#endif // DEFINES_H
