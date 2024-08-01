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

#include <okec/common/simulator.h>
#include <okec/utils/color.h>
#include <okec/utils/sys.h>
#include <algorithm>
#include <ranges>
#include <string_view>
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

inline auto print(okec::color c, std::string_view content) -> void {
    std::cout << fg(c) << content << end_color();
}

template <typename... Args>
inline auto print(okec::color text_color, std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    constexpr std::string_view time_format = "[+{:.8f}s] ";
    constexpr std::string_view solid_square_format = "\u2588 ";
    auto indent = std::formatted_size(time_format, okec::now::seconds()) +
                  std::formatted_size(solid_square_format);
    print(okec::color::gray, std::format(time_format, okec::now::seconds()));
    print(text_color, solid_square_format);

    auto content = std::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
    auto winsize = okec::get_winsize();
    if (winsize.col > indent) {
        auto data = content
            | std::views::chunk(winsize.col - indent)
            | std::views::join_with(std::format("\n{:{}}", "", indent - 1))
            | std::views::common;
        print(text_color, std::format("{}\n", std::string(data.begin(), data.end())));
    } else {
        print(text_color, "{}\n", content);
    }
}

} // namespace detail

template <typename... Args>
inline auto debug(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_debug_enabled)
        detail::print(okec::color::debug, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto info(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_info_enabled)
        detail::print(okec::color::info, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto warning(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_warning_enabled)
        detail::print(okec::color::warning, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto success(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_success_enabled)
        detail::print(okec::color::success, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto error(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    if (level_error_enabled)
        detail::print(okec::color::error, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}


inline auto indent_size() -> std::size_t {
    constexpr std::string_view time_format = "[+{:.8f}s] ";
    constexpr std::string_view solid_square_format = "\u2588 ";
    return std::formatted_size(time_format, ns3::Simulator::Now().GetSeconds()) +
           std::formatted_size(solid_square_format);;
}

} // namespace okec::log

#endif // OKEC_LOG_H_