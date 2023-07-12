#include "base_station.h"
#include "cloud_server.h"
#include "format_helper.hpp"
#include "message.h"
#include "ns3/ipv4.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <algorithm>  // for std::ranges::for_each


namespace okec
{

base_station::base_station()
    : m_edge_devices{ nullptr },
      m_udp_application{ CreateObject<udp_application>() },
      m_node{ CreateObject<Node>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(10));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置处理回调函数
    m_udp_application->set_request_handler(message_offloading_task, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->handle_request(packet, remote_address);
    });
}

auto base_station::connect_device(edge_device_container& devices) -> void
{
    m_edge_devices = &devices;
}

auto base_station::has_free_resource(const task& t) const -> bool
{
    for (auto device : *m_edge_devices) {
        if (device->free_cpu_cycles() > t.needed_cpu_cycles() &&
            device->free_memory() > t.needed_memory() &&
            device->price() <= t.budget())
            return true;
    }

    return false;
}

auto base_station::get_address() const -> ns3::Ipv4Address
{
    auto ipv4 = m_node->GetObject<ns3::Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}

auto base_station::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto base_station::get_nodes(ns3::NodeContainer &nodes) -> void
{
    nodes.Add(m_node); // BS
    m_edge_devices->get_nodes(nodes); // EdgeDevice
}

auto base_station::get_node() -> Ptr<Node>
{
    return m_node;
}

auto base_station::link_cloud(const okec::cloud_server& cs) -> void
{
    if (!cs.get_address().IsInitialized())
    {
        fmt::print("link_cloud() Error: the network of cloud server is not initialized at this "
                    "time.\n");
        exit(0);
    }

    m_cs_address = std::make_pair(cs.get_address(), cs.get_port());
}

auto base_station::dispatch_task() -> void
{
}

auto base_station::handle_request(Ptr<Packet> packet, const Address& remote_address) -> void
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);
    fmt::print("bs[{:ip}] handles request from {:ip}, dispatching it to {:ip}.\n", 
        get_address(), inetRemoteAddress.GetIpv4(), m_edge_devices->get_device(0)->get_address());

    // 转发任务到具体的EdgeDeivce
    m_udp_application->write(packet, m_edge_devices->get_device(0)->get_address(), m_edge_devices->get_device(0)->get_port());

    // 转发任务到云端
    m_udp_application->write(packet, m_cs_address.first, m_cs_address.second);
}

} // namespace simeg