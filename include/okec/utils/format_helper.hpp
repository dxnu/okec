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
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <ns3/ipv4.h>
#include <ns3/ipv4-header.h>


// formatting Ipv4Address
template <>
struct fmt::formatter<ns3::Ipv4Address> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'i' && *(++it) == 'p')) it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid format");
        return it;
    }

    template <typename FormatContext>
    auto format(const ns3::Ipv4Address& ipv4Address, FormatContext& ctx) const {
        std::ostringstream oss;
        uint32_t address = ipv4Address.Get();
        oss << ((address >> 24) & 0xff) << "." << ((address >> 16) & 0xff) << "."
            << ((address >> 8) & 0xff) << "." << ((address >> 0) & 0xff);
        return fmt::format_to(ctx.out(), "{}", oss.str());
    }
};


// formatting okec::task
template <>
struct fmt::formatter<okec::task> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it == 't') it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid task format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::task& t, FormatContext& ctx) {
        int index = 1;
        std::string info;
        for (const auto& item : t.data())
        {
            info += fmt::format("[{:>{}}] ", index++, std::to_string(t.size()).length());
            if (item.contains("/header"_json_pointer))
            {
                for (auto it = item["header"].begin(); it != item["header"].end(); ++it)
                {
                    info += fmt::format("{}: {} ", it.key(), it.value());
                }
            }

            if (item.contains("/body"_json_pointer))
            {
                for (auto it = item["body"].begin(); it != item["body"].end(); ++it)
                {
                    info += fmt::format("{}: {} ", it.key(), it.value());
                }
            }
            info += "\n";
        }

        return fmt::format_to(ctx.out(), fmt::runtime(info));
    }
};

// formatting okec::resource_container
template <>
struct fmt::formatter<okec::resource_container> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'r' && *(++it) == 's')) it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid resource format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::resource_container& rs, FormatContext& ctx) {
        int index = 1;
        std::string info;
        for (auto item = rs.cbegin(); item != rs.cend(); ++item) {
            info += fmt::format("[{:>{}}] ", index++, std::to_string(rs.size()).length());
            for (auto it = (*item)->begin(); it != (*item)->end(); ++it)
            {
                info += fmt::format("{}: {} ", it.key(), it.value());
            }

            info += "\n";
        }

        return fmt::format_to(ctx.out(), fmt::runtime(info));
    }
};

template <>
struct fmt::formatter<okec::response> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it == 'r') it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid response format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::response& r, FormatContext& ctx) {
        int index = 1;
        std::string info;
        for (const auto& item : r.data())
        {
            info += fmt::format("[{:>{}}] ", index++, std::to_string(r.size()).length());
            for (auto it = item.begin(); it != item.end(); ++it)
            {
                info += fmt::format("{}: {} ", it.key(), it.value());
            }

            info += "\n";
        }

        return fmt::format_to(ctx.out(), fmt::runtime(info));
    }
};

namespace okec {

template <typename... Args>
inline auto format(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> std::string {
    return fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
inline auto print(fmt::format_string<Args...>&& fmt, Args&&... args)
    -> void {
    fmt::print(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
}

} // namespace okec

#endif // OKEC_FORMAT_HELPER_HPP_