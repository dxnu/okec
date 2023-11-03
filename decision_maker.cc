#include "decision_maker.h"
#include "base_station.h"
#include "cloud_server.h"
#include "format_helper.hpp"
#include "message.h"
#include <algorithm>
#include <charconv>

namespace okec
{

auto okec::decision_maker::make_decision(const task_header& header) -> std::pair<std::string, uint16_t>
{
    for (const auto& cache : m_device_cache) {
        auto [ip, port] = cache.first;
        auto res = cache.second;
        // fmt::print("{}:{}\n{:r}\n", ip, port, cache.second);
        if(can_handle(header, res)) {
            return cache.first; // offload to edge
        }
    }

    auto [cs_ip, cs_port] = m_cs_address;
    return std::make_pair(fmt::format("{:ip}", cs_ip), cs_port); // offload to cloud
}

auto decision_maker::initialize_device(
    base_station_container* bs_container, cloud_server* cs) -> void
{
    // Obtain the address of cloud server.
    m_cs_address.first = cs->get_address();
    m_cs_address.second = cs->get_port();
    // fmt::print("云服务器 {:ip}:{}\n", m_cs_address.first, m_cs_address.second);

    // Obtain the addresses and resources(current is null) of all edge devices.
    std::for_each(bs_container->begin(), bs_container->end(),
        [this](const base_station_container::pointer_t bs) {
        // fmt::print("bs: {:ip}:{}\n", bs->get_address(), bs->get_port());

        for (auto& device : bs->get_edge_devices()) {
            // fmt::print("device: {:ip}:{}\n", device->get_address(), device->get_port());
            m_device_cache.emplace(std::make_pair(fmt::format("{:ip}", device->get_address())
                , device->get_port()), resource{});
        }
    });

    fmt::print("size of device cache: {}\n", m_device_cache.size());

    // Save a base station so we can utilize its communication component.
    m_bs_socket = bs_container->get(0);
    // 资源更新(外部所指定的BS不一定是第0个，所以要为所有BS设置消息以确保捕获)
    bs_container->set_request_handler("resource_changed",
        [this](okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) {
            message msg(packet);
            auto ip = msg.get_value("ip");
            auto port = msg.get_value("port");
            auto cpu_cycles = msg.get_value("cpu_cycles");
            fmt::print("\n\n>>>>>>>>>>>>>resource changed value: {}:{}, {}.\n", ip, port, cpu_cycles);
        });
    

    // Initial information retrieval.
    query_information();
}

auto decision_maker::query_information() -> void
{
    // Query resource information of all devices.
    auto address = std::views::keys(m_device_cache);
    auto launch_time = 0.1;
    for (const auto& [ip, port] : address) {
        fmt::print("device: {}:{}\n", ip, port);
        // 发送时间如果是0s，因为UdpApplication的StartTime也是0s，所以m_socket可能尚未初始化，此时Write将无法发送
        ns3::Simulator::Schedule(Seconds(launch_time), [&, this] {
            message msg {
                { "msgtype", "get_resource_information" }
            };
            m_bs_socket->write(msg.to_packet(), Ipv4Address{ip.c_str()}, port); });
        launch_time += 0.1;
    }
    
    m_bs_socket->set_request_handler("resource_information", 
        [this](okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) {
            fmt::print("resource information: {}\n", okec::packet_helper::to_string(packet));
            auto msg = message::from_packet(packet);
            auto ip = msg.get_value("ip");
            auto port = msg.get_value("port");
            auto cycles = msg.get_value("cycles");
            auto memory = msg.get_value("memory");
            resource res{};
            res.cpu_cycles(std::stoi(cycles));
            res.memory(std::stoi(memory));

            auto pos = m_device_cache.find(std::make_pair(ip, std::stoi(port)));
            if (pos != m_device_cache.end())
                // (*pos).second = std::move(res); // 需要优化
                (*pos).second = res;
        } );

    // 资源更新
    // m_bs_socket->set_request_handler("resource_changed",
    //     [this](okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) {
    //         message msg(packet);
    //         auto ip = msg.get_value("ip");
    //         auto port = msg.get_value("port");
    //         auto cpu_cycles = msg.get_value("cpu_cycles");
    //         fmt::print("\n\n>>>>>>>>>>>>>resource changed value: {}:{}, {}.\n", ip, port, cpu_cycles);
    //     });
}

auto decision_maker::can_handle(const task_header& header, const resource& res) -> bool
{
    double time = (double)header.needed_cpu_cycles / res.cpu_cycles();
    fmt::print("can handle time: {} ---- {} (tolorable time)\n", time, header.deadline);
    return time < header.deadline;
}

} // namespace okec