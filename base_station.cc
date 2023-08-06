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


#define CHECK_INDEX(index) \
if (index > size()) throw std::out_of_range{"index out of range"}


namespace okec
{

base_station::base_station()
    : m_edge_devices{ nullptr },
      m_udp_application{ ns3::CreateObject<udp_application>() },
      m_node{ ns3::CreateObject<Node>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(10));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置处理回调函数
    m_udp_application->set_request_handler(message_dispatching, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_dispatching_message(packet, remote_address);
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

auto base_station::on_dispatching_message(Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    fmt::print("bs[{:ip}] receives the request from {:ip},", 
        get_address(), inetRemoteAddress.GetIpv4(), m_edge_devices->get_device(0)->get_address());

    bool handled{};
    auto msg = message::from_packet(packet);
    auto t = msg.to_task();

    for (auto device : *m_edge_devices) {
        if (device->free_cpu_cycles() > t->needed_cpu_cycles() &&
            device->free_memory() > t->needed_memory() &&
            device->price() <= t->budget()) {

            fmt::print(" dispatching it to {:ip} to handle the concrete tasks.\n", device->get_address());

            // 能够处理
            msg.type(message_handling);
            m_udp_application->write(msg.to_packet(), device->get_address(), device->get_port());
            handled = true;

            // 反服务器返回分发成功消息，以便服务器清除某些记录
            msg.type(message_dispatching_success);
            m_udp_application->write(msg.to_packet(), m_cs_address.first, m_cs_address.second);

            break;
        }
    }

    // 不能处理，消息需要再次转发
    if (!handled) {
        fmt::print(" dispatching it to {:ip} bacause of lacking resource.\n", m_cs_address.first);
        msg.type(message_dispatching_failure);
        m_udp_application->write(msg.to_packet(), m_cs_address.first, m_cs_address.second);
    }
}

base_station_container::base_station_container(std::size_t n)
{
    m_base_stations.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        m_base_stations.emplace_back(std::make_shared<base_station>());
}

auto base_station_container::link_cloud(const cloud_server& cs) -> void
{
    std::for_each(std::begin(m_base_stations), std::end(m_base_stations), [&cs](pointer_t bs) {
        bs->link_cloud(cs);
    });
}

auto base_station_container::operator[](std::size_t index) -> pointer_t
{
    return this->get(index);
}

auto base_station_container::operator()(std::size_t index) -> pointer_t
{
    return this->get(index);
}

auto base_station_container::get(std::size_t index) -> pointer_t
{
    CHECK_INDEX(index);
    return m_base_stations[index];
}

auto base_station_container::size() -> std::size_t
{
    return m_base_stations.size();
}

} // namespace okec