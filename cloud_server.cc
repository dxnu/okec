#include "cloud_server.h"
#include "base_station.h"
#include "message.h"
#include "format_helper.hpp"
#include "ns3/mobility-module.h"


namespace okec
{

cloud_server::cloud_server()
    : m_node{ ns3::CreateObject<Node>() },
      m_udp_application{ ns3::CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(10000));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置默认回调函数
    m_udp_application->set_request_handler(message_get_resource_information, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_get_resource_information(packet, remote_address);
    });
}

auto cloud_server::get_node() -> Ptr<Node>
{
    return m_node;
}

auto cloud_server::get_nodes(NodeContainer& nodes) -> void
{
    nodes.Add(m_node);
}

auto cloud_server::get_address() const -> Ipv4Address
{
    auto ipv4 = m_node->GetObject<Ipv4>();
    return ipv4 ? ipv4->GetAddress(1, 0).GetLocal() : Ipv4Address{};
}

auto cloud_server::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto cloud_server::get_resource() -> Ptr<resource>
{
    return m_node->GetObject<resource>();
}

auto cloud_server::install_resource(Ptr<resource> res) -> void
{
    res->install(m_node);
}

auto cloud_server::set_position(double x, double y, double z) -> void
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

auto cloud_server::get_position() -> Vector
{
    Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel>();
    return mobility ? mobility->GetPosition() : Vector();
}

auto cloud_server::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_udp_application->set_request_handler(msg_type, 
        [callback, this](Ptr<Packet> packet, const Address& remote_address) {
            callback(this, packet, remote_address);
        });
}

auto cloud_server::write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) const -> void
{
    m_udp_application->write(packet, destination, port);
}

auto cloud_server::on_get_resource_information(Ptr<Packet> packet, const Address& remote_address) -> void
{
    auto device_resource = get_resource();
    if (!device_resource || device_resource->empty())
        return; // 没有安装资源，或资源为空，都不返回任何消息


    message msg {
        { "msgtype", "resource_information" },
        { "device_type", "cs" },
        { "pos_x", fmt::format("{}", get_position().x) },
        { "pos_y", fmt::format("{}", get_position().y) },
        { "pos_z", fmt::format("{}", get_position().z) }
    };
    msg.content(*device_resource);
    m_udp_application->write(msg.to_packet(), InetSocketAddress::ConvertFrom(remote_address).GetIpv4(), 8860);
}


} // namespace okec
