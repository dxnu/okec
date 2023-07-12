#include "edge_device.h"
#include "format_helper.hpp"
#include "message.h"
#include "task.h"
#include "resource.h"
#include "ns3/ipv4.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>


namespace okec
{

edge_device::edge_device()
    : m_node{ CreateObject<ns3::Node>() },
      m_udp_application{ CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(10));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置请求回调函数
    m_udp_application->set_request_handler(message_offloading_task, [this](Ptr<Packet> packet, const Address& remoteAddress) {
        this->handle_task(packet, remoteAddress);
    });
}

void edge_device::set_resource()
{
}

void edge_device::handle_task(Ptr<Packet> packet, const Address& remoteAddress)
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remoteAddress);
    fmt::print("edge device handle request from {:ip}, done it.\n", inetRemoteAddress.GetIpv4());

    // cd projects/*3.38/*3.38/ && ./ns3 run "scratch/EIP/test2"

    // 返回响应结果
    auto t = message::to_task(packet);
    auto [ip, port] = t->from();
    fmt::print("response to {}:{}\n", ip, port);
    
    message msg {
        { "msgtype", message_response },
        { "content", "response value: 10" }
    };
    m_udp_application->write(msg.to_packet(), Ipv4Address{ip.c_str()}, port);
}

auto edge_device::free_cpu_cycles() const -> int
{
    auto res = m_node->GetObject<resource>();
    if (res == nullptr)
        return -1;

    return res->cpu_cycles();
}

auto edge_device::free_memory() const -> int
{
    auto res = m_node->GetObject<resource>();
    if (res == nullptr)
        return -1;

    return res->memory();
}

auto edge_device::price() const -> int
{
    auto res = m_node->GetObject<resource>();
    if (res == nullptr)
        return -1;

    return res->price();
}

auto edge_device::get_address() const -> ns3::Ipv4Address
{
    auto ipv4 = m_node->GetObject<ns3::Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}

auto edge_device::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto edge_device::get_node() -> Ptr<Node>
{
    return m_node;
}

edge_device_container::edge_device_container(std::size_t n)
{
    m_devices.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        m_devices.emplace_back(std::make_shared<value_type>());
}

auto edge_device_container::get_nodes(ns3::NodeContainer& nodes) -> void
{
    for (auto& device : m_devices)
        nodes.Add(device->get_node());
}

auto edge_device_container::get_device(std::size_t index) -> pointer_type
{
    return m_devices[index];
}

} // namespace simeg