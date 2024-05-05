///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_LOG_H_
#define OKEC_LOG_H_

#include <okec/utils/color.h>
#include <okec/utils/sys.h>
#include <algorithm>
#include <ranges>
#include <string_view>
#include <fmt/core.h>
#include <ns3/core-module.h>


namespace okec::log {

enum class level : uint8_t {
    debug = 1 << 0,
    info = 1 << 1,
    warning = 1 << 2,
    success = 1 << 3,
    error = 1 << 4,
    all = debug | info | warning | success | error
};

inline constinit bool level_debug_enabled   = false;
inline constinit bool level_info_enabled    = false;
inline constinit bool level_warning_enabled = false;
inline constinit bool level_success_enabled = false;
inline constinit bool level_error_enabled   = false;

inline auto set_level(level log_level, bool enabled = true) -> void {
    if (enabled) {
        level_debug_enabled   |= std::to_underlying(log_level) & std::to_underlying(level::debug);
        level_info_enabled    |= std::to_underlying(log_level) & std::to_underlying(level::info);
        level_warning_enabled |= std::to_underlying(log_level) & std::to_underlying(level::warning);
        level_success_enabled |= std::to_underlying(log_level) & std::to_underlying(level::success);
        level_error_enabled   |= std::to_underlying(log_level) & std::to_underlying(level::error);
    } else {
        level_debug_enabled   &= ~(std::to_underlying(log_level) & std::to_underlying(level::debug));
        level_info_enabled    &= ~(std::to_underlying(log_level) & std::to_underlying(level::info));
        level_warning_enabled &= ~(std::to_underlying(log_level) & std::to_underlying(level::warning));
        level_success_enabled &= ~(std::to_underlying(log_level) & std::to_underlying(level::success));
        level_error_enabled   &= ~(std::to_underlying(log_level) & std::to_underlying(level::error));
    }
}

inline auto operator|(level lhs, level rhs) -> level {
    return static_cast<level>(std::to_underlying(lhs) | std::to_underlying(rhs));
}


namespace detail {

template <typename... Args>
inline auto print(okec::color text_color, fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    constexpr std::string_view time_format = "[+{:.8f}s] ";
    constexpr std::string_view solid_square_format = "\u2588 ";
    auto indent = fmt::formatted_size(time_format, ns3::Simulator::Now().GetSeconds()) +
                  fmt::formatted_size(solid_square_format);
    fmt::print(fg(fmt::color::gray), time_format, ns3::Simulator::Now().GetSeconds());
    fmt::print(fg(fmt::rgb(std::to_underlying(text_color))), solid_square_format);

    auto content = fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
    auto winsize = okec::get_winsize();
    if (winsize.col > indent) {
        auto data = content
            | std::views::chunk(winsize.col - indent)
            | std::views::join_with(fmt::format("\n{:{}}", "", indent - 1))
            | std::views::common;
        fmt::print(fg(fmt::rgb(std::to_underlying(text_color))), "{}\n", std::string(data.begin(), data.end()));
    } else {
        fmt::print(fg(fmt::rgb(std::to_underlying(text_color))), "{}\n", content);
    }
}

} // namespace detail

template <typename... Args>
inline auto debug(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_debug_enabled)
        detail::print(okec::color::debug, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto info(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_info_enabled)
        detail::print(okec::color::info, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto warning(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_warning_enabled)
        detail::print(okec::color::warning, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto success(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_success_enabled)
        detail::print(okec::color::success, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto error(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_error_enabled)
        detail::print(okec::color::error, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}


} // namespace okec::log

#endif // OKEC_LOG_H_