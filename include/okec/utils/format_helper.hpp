///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_FORMAT_HELPER_HPP_
#define OKEC_FORMAT_HELPER_HPP_

#include <okec/common/resource.h>
#include <okec/common/response.h>
#include <okec/common/task.h>
#include <format>
#include <iostream>
#include <ns3/ipv4.h>
#include <ns3/ipv4-header.h>


namespace okec {

template <typename T>
auto unmove(T&& x) -> const T& {
    return x;
}

template <typename... Args>
inline auto format(std::format_string<Args...>&& fmt, Args&&... args)
    -> std::string {
    return std::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto print(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    std::cout << okec::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto println(std::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    print(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
    std::cout << '\n';
}

} // namespace okec


// formatting Ipv4Address
template <>
struct std::formatter<ns3::Ipv4Address> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'i' && *(++it) == 'p')) it++;
        if (it != end && *it != '}') throw std::format_error("invalid format");
        return it;
    }

    template <typename FormatContext>
    auto format(const ns3::Ipv4Address& ipv4Address, FormatContext& ctx) const {
        std::ostringstream oss;
        uint32_t address = ipv4Address.Get();
        oss << ((address >> 24) & 0xff) << "." << ((address >> 16) & 0xff) << "."
            << ((address >> 8) & 0xff) << "." << ((address >> 0) & 0xff);
        return std::vformat_to(ctx.out(), "{}", std::make_format_args(okec::unmove(oss.str())));
    }
};


// formatting okec::task
template <>
struct std::formatter<okec::task> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it == 't') it++;
        if (it != end && *it != '}') throw std::format_error("invalid task format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::task& t, FormatContext& ctx) const {
        int index = 1;
        std::string info;
        for (const auto& item : t.data())
        {
            info += std::vformat("[{:>{}}] ", std::make_format_args(
                okec::unmove(index++), okec::unmove(std::to_string(t.size()).length())));

            if (item.contains("/header"_json_pointer))
            {
                for (auto it = item["header"].begin(); it != item["header"].end(); ++it)
                {
                    info += std::vformat("{}: {} ", std::make_format_args(
                        it.key(), okec::unmove(it.value().template get<std::string>())));
                }
            }

            if (item.contains("/body"_json_pointer))
            {
                for (auto it = item["body"].begin(); it != item["body"].end(); ++it)
                {
                    info += std::vformat("{}: {} ", std::make_format_args(
                        it.key(), okec::unmove(it.value().template get<std::string>())));
                }
            }
            info += "\n";
        }

        return std::vformat_to(ctx.out(), "{}", std::make_format_args(info));
    }
};

template <>
struct std::formatter<okec::resource_container> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'r' && *(++it) == 's')) it++;
        if (it != end && *it != '}') throw std::format_error("invalid resource format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::resource_container& rs, FormatContext& ctx) const {
        int index = 1;
        std::string info;
        for (auto item = rs.cbegin(); item != rs.cend(); ++item) {
            info += std::vformat("[{:>{}}] ", std::make_format_args(
                okec::unmove(index++), okec::unmove(std::to_string(rs.size()).length())));
            for (auto it = (*item)->begin(); it != (*item)->end(); ++it)
            {
                info += std::vformat("{}: {} ", std::make_format_args(
                    it.key(), okec::unmove(it.value().template get<std::string>())));
            }

            info += "\n";
        }

        return std::vformat_to(ctx.out(), "{}", std::make_format_args(info));
    }
};

template <>
struct std::formatter<okec::response> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it == 'r') it++;
        if (it != end && *it != '}') throw std::format_error("invalid response format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::response& r, FormatContext& ctx) const {
        int index = 1;
        std::string info;
        for (const auto& item : r.data())
        {
            info += std::vformat("[{:>{}}] ", std::make_format_args(
                okec::unmove(index++), okec::unmove(std::to_string(r.size()).length())));
            for (auto it = item.begin(); it != item.end(); ++it)
            {
                info += std::vformat("{}: {} ", std::make_format_args(
                    it.key(), okec::unmove(it.value().template get<std::string>())));
            }

            info += "\n";
        }

        return std::vformat_to(ctx.out(), "{}", std::make_format_args(info));
    }
};


#endif // OKEC_FORMAT_HELPER_HPP_