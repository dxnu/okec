#include "edge_device.h"
#include "format_helper.hpp"
#include "message.h"
#include "task.h"
// #include "response.h"
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
    m_udp_application->set_request_handler(message_handling, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_handling_message(packet, remote_address);
    });
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

auto edge_device::get_resource() -> Ptr<resource>
{
    return m_node->GetObject<resource>();
}

auto edge_device::install_resource(Ptr<resource> res) -> void
{
    res->install(m_node);
}

void edge_device::handle_task(Ptr<task> t)
{
    // 处理任务

    // 处理完成
    auto res = this->get_resource();
    res->cpu_cycles(res->cpu_cycles() + t->needed_cpu_cycles()); // 释放资源
    res->memory(res->memory() + t->needed_memory());             // 释放资源

    // 返回响应结果
    auto [ip, port] = t->from();
    fmt::print("edge server returns response to {}:{}\n", ip, port);
    
    auto r = make_response();
    r->task_id(t->id());
    r->handling_device("cs", fmt::format("{:ip}", this->get_address()));
    r->group(t->group());
    message msg;
    msg.type(message_response);
    msg.content(r);

    // message msg {
    //     { "msgtype", message_response },
    //     { "content", "response value: 42" }
    // };
    m_udp_application->write(msg.to_packet(), Ipv4Address{ip.c_str()}, port);
}

auto edge_device::on_handling_message(ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    // InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);
    // fmt::print("edge device handle request from {:ip}, done it.\n", inetRemoteAddress.GetIpv4());
    fmt::print("edge server[{:ip}] handles the request\n", this->get_address());
    
    auto t = packet_helper::to_task(packet);

    // 能到这里，说明设备上必须有资源
    auto res = this->get_resource();
    res->cpu_cycles(res->cpu_cycles() - t->needed_cpu_cycles()); // 标记资源已使用
    res->memory(res->memory() - t->needed_memory());             // 标记资源已使用
    
    // 处理任务
    handle_task(t);
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

auto edge_device_container::size() -> std::size_t
{
    return m_devices.size();
}

auto edge_device_container::install_resources(resource_container& res, int offset) -> void
{
    for (std::size_t i = 0; i < this->size(); ++i) {
        if (i + offset < res.size())
            m_devices[i]->install_resource(res[i + offset]);
    }
}


} // namespace simeg