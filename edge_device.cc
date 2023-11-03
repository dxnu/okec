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

    m_udp_application->set_request_handler("get_resource_information", [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_get_resource_information(packet, remote_address);
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

    m_udp_application->write(msg.to_packet(), Ipv4Address{ip.c_str()}, port);
}

auto edge_device::on_handling_message(ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);
    // fmt::print("edge device handle request from {:ip}, done it.\n", inetRemoteAddress.GetIpv4());
    fmt::print("edge server[{:ip}] handles the request from {:ip}\n", this->get_address(), inetRemoteAddress.GetIpv4());
    
    message msg { packet };
    auto instructions = msg.content<std::string>();
    auto cpu_demand = msg.get_value("cpu_demand");
    auto task_id = msg.get_value("task_id");

    auto processing_time = std::stod(cpu_demand) / get_resource()->cpu_cycles();

    fmt::print("edge server handles instructions: {}, "
        "cpu demand: {}, processing time: {}\n", instructions, cpu_demand, processing_time);

    // 通知资源更新情况
    auto old_cpu_cycles = get_resource()->cpu_cycles(0); // 标记资源已用完
    message notify_msg {
        { "msgtype", "resource_changed" },
        { "ip", fmt::format("{:ip}", this->get_address()) },
        { "port", std::to_string(get_port()) },
        { "cpu_cycles", std::to_string(get_resource()->cpu_cycles()) }
    };
    fmt::print("<<<<<<<<<<<<packet: {}\n", notify_msg.dump());
    m_udp_application->write(notify_msg.to_packet(), inetRemoteAddress.GetIpv4(), this->get_port());
    

    // 执行任务
    Simulator::Schedule(ns3::Seconds(processing_time), +[](const ns3::Time& time, double processing_time, edge_device* self, int old_cpu_cycles, const std::string& task_id, const ns3::Ipv4Address& desination) {
        auto begin_time = time.GetMilliSeconds();
        auto end_time = Simulator::Now().GetMilliSeconds();
        fmt::print("======begin time: {}, end time: {}, processing_time: {}, duration:{}\n", 
            begin_time, end_time, processing_time,  end_time - begin_time);

        // 释放资源
        self->get_resource()->cpu_cycles(old_cpu_cycles);
        auto device_address = fmt::format("{:ip}", self->get_address());
        message notify_msg {
            { "msgtype", "resource_changed" },
            { "ip", device_address },
            { "port", std::to_string(self->get_port()) },
            { "cpu_cycles", std::to_string(old_cpu_cycles) }
        };
        self->m_udp_application->write(notify_msg.to_packet(), desination, self->get_port());

        // 返回消息
        message response {
            { "msgtype", "response" },
            { "task_id", task_id },
            { "device_type", "es" },
            { "device_address", device_address },
            { "processing_time", fmt::format("{}", processing_time) }
        };
        self->m_udp_application->write(response.to_packet(), desination, self->get_port());
    }, Simulator::Now(), processing_time, this, old_cpu_cycles, task_id, inetRemoteAddress.GetIpv4());
}

auto edge_device::on_get_resource_information(
    ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    // fmt::print("on_get_resource_information from {:ip}\n", InetSocketAddress::ConvertFrom(remote_address).GetIpv4());
    message msg {
        { "msgtype", "resource_information" },
        { "ip", fmt::format("{:ip}", this->get_address()) },
        { "port", fmt::format("{}", this->get_port()) },
        { "cycles", fmt::format("{}", this->free_cpu_cycles()) },
        { "memory", fmt::format("{}", this->free_memory()) }
    };

    // fmt::print("resource information: {}\n", msg.dump());
    m_udp_application->write(msg.to_packet(), InetSocketAddress::ConvertFrom(remote_address).GetIpv4(), 8860);
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