#include "edge_device.h"
#include "format_helper.hpp"
#include "message.h"
#include "task.h"
#include "resource.h"
#include "ns3/ipv4.h"
#include "ns3/mobility-module.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>


namespace okec
{

edge_device::edge_device()
    : m_node{ CreateObject<ns3::Node>() },
      m_udp_application{ CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(300));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置请求回调函数
    m_udp_application->set_request_handler("get_resource_information", [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_get_resource_information(packet, remote_address);
    });
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

auto edge_device::set_position(double x, double y, double z) -> void
{
    Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel>();
    if (!mobility) {
        mobility = CreateObject<ConstantPositionMobilityModel>();
        mobility->SetPosition(Vector(x, y, z));
        m_node->AggregateObject(mobility);
    } else {
        mobility->SetPosition(Vector(x, y, z));
    }
}

auto edge_device::get_position() -> Vector
{
    Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel>();
    return mobility ? mobility->GetPosition() : Vector();
}

auto edge_device::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_udp_application->set_request_handler(msg_type, 
        [callback, this](Ptr<Packet> packet, const Address& remote_address) {
            callback(this, packet, remote_address);
        });
}

auto edge_device::write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) const -> void
{
    m_udp_application->write(packet, destination, port);
}

auto edge_device::on_get_resource_information(ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    // fmt::print("on_get_resource_information from {:ip}\n", InetSocketAddress::ConvertFrom(remote_address).GetIpv4());
    auto device_resource = get_resource();
    if (!device_resource || device_resource->empty())
        return; // 没有安装资源，或资源为空，都不返回任何消息


    message msg {
        { "msgtype", message_resource_information },
        { "device_type", "es" },
        { "ip", fmt::format("{:ip}", this->get_address()) },
        { "port", fmt::format("{}", this->get_port()) },
        { "pos_x", fmt::format("{}", get_position().x) },
        { "pos_y", fmt::format("{}", get_position().y) },
        { "pos_z", fmt::format("{}", get_position().z) }
    };
    msg.content(*device_resource);
    this->write(msg.to_packet(), InetSocketAddress::ConvertFrom(remote_address).GetIpv4(), 8860);
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