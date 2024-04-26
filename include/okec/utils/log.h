#ifndef OKEC_LOG_H_
#define OKEC_LOG_H_

#include <okec/utils/color.h>
#include <string_view>
#include <fmt/core.h>


namespace okec::log {


namespace detail {

template <typename... Args>
inline auto print(okec::color text_color, fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    fmt::print(fg(fmt::color::gray), "[+{:.8f}s] ", ns3::Simulator::Now().GetSeconds());
    fmt::print(fg(fmt::rgb(std::to_underlying(text_color))), "\u2588 {}\n",
        fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...));
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