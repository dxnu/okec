#include "decision_maker.h"
#include "base_station.h"
#include "cloud_server.h"
#include "format_helper.hpp"
#include <algorithm>

namespace okec
{

auto okec::decision_maker::make_decision(task_header header) -> std::pair<std::string, uint16_t>
{
    // bool handled{};
    // std::ranges::for_each(m_device_cache, [](std::pair<std::string, uint16_t> ip_port, resource res) {
    //     if(can_handle(task, resource)) {
    //         handled = true;
    //         return {}; // "offload to edge";
    //     }
    // });

    // if (!handled)
    //     return {}; // "offload to cloud";
    return {};
}

auto decision_maker::initialize_device(
    const base_station_container* bs_container, const cloud_server* cs) -> void
{
    m_cs_address.first = cs->get_address();
    m_cs_address.second = cs->get_port();
    // fmt::print("云服务器 {:ip}:{}\n", m_cs_address.first, m_cs_address.second);

    std::for_each(bs_container->cbegin(), bs_container->cend(),
        [this](const base_station_container::pointer_t bs) {
        // fmt::print("bs: {:ip}:{}\n", bs->get_address(), bs->get_port());

        for (auto& device : bs->get_edge_devices()) {
            // fmt::print("device: {:ip}:{}\n", device->get_address(), device->get_port());
            m_device_cache.emplace(std::make_pair(fmt::format("{:ip}", device->get_address())
                ,device->get_port()), resource{});
        }
    });

    fmt::print("size of device cache: {}\n", m_device_cache.size());
}

auto decision_maker::query_information() -> void
{
}

} // namespace okec