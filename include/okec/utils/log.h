#ifndef OKEC_LOG_H_
#define OKEC_LOG_H_

#include <okec/utils/color.h>
#include <okec/utils/sys.h>
#include <algorithm>
#include <ranges>
#include <string_view>
#include <fmt/core.h>


namespace okec::log {

enum class level : uint8_t {
    debug = 0,
    info,
    warning,
    success,
    error
};

inline auto set_level(level, bool status) -> void {

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
    
    // fmt::print(fg(fmt::rgb(std::to_underlying(text_color))), "\u2588 {}\n",
    //     fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...));

}

} // namespace detail

template <typename... Args>
inline auto debug(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    detail::print(okec::color::debug, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto info(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    detail::print(okec::color::info, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto warning(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    detail::print(okec::color::warning, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto success(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    detail::print(okec::color::success, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto error(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    detail::print(okec::color::error, std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}


} // namespace okec::log

#endif // OKEC_LOG_H_