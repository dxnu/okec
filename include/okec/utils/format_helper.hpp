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
#include <fmt/color.h>
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
    auto format(const ns3::Ipv4Address& ipv4Address, FormatContext& ctx) {
        std::ostringstream oss;
        uint32_t address = ipv4Address.Get();
        oss << ((address >> 24) & 0xff) << "." << ((address >> 16) & 0xff) << "."
            << ((address >> 8) & 0xff) << "." << ((address >> 0) & 0xff);
        return fmt::format_to(ctx.out(), "{}", oss.str());
    }

    // char presentation[2] = "";
};


static inline auto to_string(const ns3::Ipv4Address& ipv4Address) {
    return fmt::format("{:ip}", ipv4Address);
}


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
            info += fmt::format("[{:>3}] ", index++);
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

// formatting okec::task_container
// template <>
// struct fmt::formatter<okec::task_container> {
//     constexpr auto parse(format_parse_context& ctx) {
//         auto it = ctx.begin(), end = ctx.end();
//         if (it != end && (*it == 't' && *(++it) == 's')) it++;
//         if (it != end && *it != '}') throw fmt::format_error("invalid task format");
//         return it;
//     }

//     template <typename FormatContext>
//     auto format(okec::task_container& ts, FormatContext& ctx) {
//         const int property_len = 80;
//         auto ts_info = fmt::format("{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n",
//                     "", property_len, property_len / 4, "task_id", "memory", "cpu", "deadline");
//         std::for_each(ts.begin(), ts.end(), [&ts_info](Ptr<okec::task> t) {
//             ts_info += fmt::format("{1:<{0}}{2:<{0}}{3:<{0}}{4:>{0}}\n",
//                     property_len / 4, t->id(), t->needed_memory(), t->needed_cpu_cycles(), t->deadline());
//         });

//         ts_info += fmt::format("{0:=^{1}}\n", "", property_len);
//         return fmt::format_to(ctx.out(), fmt::runtime(ts_info));
//     }
// };

// formatting okec::resource
// template <>
// struct fmt::formatter<okec::resource> {
//     constexpr auto parse(format_parse_context& ctx) {
//         auto it = ctx.begin(), end = ctx.end();
//         if (it != end && *it == 'r') it++;
//         if (it != end && *it != '}') throw fmt::format_error("invalid task_container format");
//         return it;
//     }

//     template <typename FormatContext>
//     auto format(const okec::resource& r, FormatContext& ctx) {
//         return fmt::format_to(ctx.out(), 
//                     "{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n"
//                     "{7:<{2}}{8:<{2}}{9:<{2}}{10:>{2}}\n{0:=^{1}}\n", 
//                     "", 80, 80 / 4, "resource_id", "memory", "cpu", "price",
//                     r.get_value("id"), r.get_value("memory"), r.get_value("cpu_cycle"), r.get_value("price"));
//     }
// };

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
            info += fmt::format("[{:>3}] ", index++);
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
    auto format(const okec::response& t, FormatContext& ctx) {
        int index = 1;
        std::string info;
        for (const auto& item : t.data())
        {
            info += fmt::format("[{:>3}] ", index++);
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