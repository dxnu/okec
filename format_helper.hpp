#ifndef NS3_FORMATTING_OUTPUT_H_
#define NS3_FORMATTING_OUTPUT_H_

#include "resource.h"
#include "task.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>


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
// template <>
// struct fmt::formatter<okec::task> {
//     constexpr auto parse(format_parse_context& ctx) {
//         auto it = ctx.begin(), end = ctx.end();
//         if (it != end && *it == 't') it++;
//         if (it != end && *it != '}') throw fmt::format_error("invalid task_container format");
//         return it;
//     }

//     template <typename FormatContext>
//     auto format(const okec::task& t, FormatContext& ctx) {
//         return fmt::format_to(ctx.out(), 
//                     "{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n"
//                     "{7:<{2}}{8:<{2}}{9:<{2}}{10:>{2}}\n{0:=^{1}}\n", 
//                     "", 80, 80 / 4, "task_id", "memory", "cpu", "deadline",
//                     t.id(), t.needed_memory(), t.needed_cpu_cycles(), t.deadline());
//     }
// };

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
template <>
struct fmt::formatter<okec::resource> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it == 'r') it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid task_container format");
        return it;
    }

    template <typename FormatContext>
    auto format(const okec::resource& r, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), 
                    "{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n"
                    "{7:<{2}}{8:<{2}}{9:<{2}}{10:>{2}}\n{0:=^{1}}\n", 
                    "", 80, 80 / 4, "resource_id", "memory", "cpu", "price",
                    r.get_value("id"), r.get_value("memory"), r.get_value("cpu_cycle"), r.get_value("price"));
    }
};

// formatting okec::resource_container
template <>
struct fmt::formatter<okec::resource_container> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'r' && *(++it) == 's')) it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid task format");
        return it;
    }

    template <typename FormatContext>
    auto format(okec::resource_container& rs, FormatContext& ctx) {
        const int property_len = 80;
        auto rs_info = fmt::format("{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n",
                    "", property_len, property_len / 4, "resource_id", "memory", "cpu", "price");
        std::for_each(rs.begin(), rs.end(), [&rs_info](Ptr<okec::resource> r) {
            rs_info += fmt::format("{1:<{0}}{2:<{0}}{3:<{0}}{4:>{0}}\n",
                    property_len / 4, 
                    r->get_value("id"), r->get_value("memory"), r->get_value("cpu_cycle"), r->get_value("price"));
        });

        rs_info += fmt::format("{0:=^{1}}\n", "", property_len);
        return fmt::format_to(ctx.out(), fmt::runtime(rs_info));
    }
};

inline static void print_info(std::string_view info)
{
    fmt::print("At time {:.2f} seconds {}\n", Simulator::Now().GetSeconds(), info);
}

#endif // NS3_FORMATTING_OUTPUT_H_