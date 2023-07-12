#ifndef NS3_FORMATTING_OUTPUT_H_
#define NS3_FORMATTING_OUTPUT_H_

#include "task.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "Resource.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/color.h>


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


// formatting Task
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
        return fmt::format_to(ctx.out(), 
                    "{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n"
                    "{7:<{2}}{8:<{2}}{9:<{2}}{10:>{2}}\n{0:=^{1}}\n", 
                    "", 80, 80 / 4, "task_id", "memory", "cpu", "budget",
                    t.id(), t.needed_memory(), t.needed_cpu_cycles(), t.budget());
    }
};

// formatting Resource
template <>
struct fmt::formatter<ns3::Resource> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it == 'r') it++;
        if (it != end && *it != '}') throw fmt::format_error("invalid format");
        return it;
    }

    template <typename FormatContext>
    auto format(const Resource& resource, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), 
                    "{0:=^{1}}\n{3:<{2}}{4:<{2}}{5:<{2}}{6:>{2}}\n{0:=^{1}}\n"
                    "{7:<{2}}{8:<{2}}{9:<{2}}{10:>{2}}\n{0:=^{1}}\n", 
                    "", 80, 80 / 4, "resource provider", "memory", "cpu", "price",
                    resource.GetProvider(), resource.GetMemory(), resource.GetCpu(), resource.GetPrice());
    }
};


#endif // NS3_FORMATTING_OUTPUT_H_