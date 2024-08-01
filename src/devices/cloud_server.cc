///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/message.h>
#include <okec/common/simulator.h>
#include <okec/devices/base_station.h>
#include <okec/devices/cloud_server.h>
#include <okec/utils/format_helper.hpp>
#include <ns3/mobility-module.h>


namespace okec
{

cloud_server::cloud_server(simulator& sim)
    : sim_{ sim },
      m_node{ ns3::CreateObject<ns3::Node>() },
      m_udp_application{ ns3::CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(ns3::Seconds(0));
    m_udp_application->SetStopTime(sim_.stop_time());

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置默认回调函数
    m_udp_application->set_request_handler(message_get_resource_information, [this](ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) {
        this->on_get_resource_information(packet, remote_address);
    });
}

auto cloud_server::get_node() -> ns3::Ptr<ns3::Node>
{
    return m_node;
}

auto cloud_server::get_nodes(ns3::NodeContainer& nodes) -> void
{
    nodes.Add(m_node);
}

auto cloud_server::get_address() const -> ns3::Ipv4Address
{
    auto ipv4 = m_node->GetObject<ns3::Ipv4>();
    return ipv4 ? ipv4->GetAddress(1, 0).GetLocal() : ns3::Ipv4Address{};
}

auto cloud_server::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto cloud_server::get_resource() -> ns3::Ptr<resource>
{
    return m_node->GetObject<resource>();
}

auto cloud_server::install_resource(ns3::Ptr<resource> res) -> void
{
    res->install(m_node);
}

auto cloud_server::set_position(double x, double y, double z) -> void
{
    ns3::Ptr<ns3::MobilityModel> mobility = m_node->GetObject<ns3::MobilityModel>();
    if (!mobility) {
        mobility = ns3::CreateObject<ns3::ConstantPositionMobilityModel>();
        mobility->SetPosition(ns3::Vector(x, y, z));
        m_node->AggregateObject(mobility);
    } else {
        mobility->SetPosition(ns3::Vector(x, y, z));
    }
}

auto cloud_server::get_position() -> ns3::Vector
{
    ns3::Ptr<ns3::MobilityModel> mobility = m_node->GetObject<ns3::MobilityModel>();
    return mobility ? mobility->GetPosition() : ns3::Vector();
}

auto cloud_server::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_udp_application->set_request_handler(msg_type, 
        [callback, this](ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) {
            callback(this, packet, remote_address);
        });
}

auto cloud_server::write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) const -> void
{
    m_udp_application->write(packet, destination, port);
}

auto cloud_server::on_get_resource_information(ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    auto device_resource = get_resource();
    if (!device_resource || device_resource->empty())
        return; // 没有安装资源，或资源为空，都不返回任何消息


    message msg {
        { "msgtype", "resource_information" },
        { "device_type", "cs" },
        { "pos_x", okec::format("{}", get_position().x) },
        { "pos_y", okec::format("{}", get_position().y) },
        { "pos_z", okec::format("{}", get_position().z) }
    };
    msg.content(*device_resource);
    m_udp_application->write(msg.to_packet(), ns3::InetSocketAddress::ConvertFrom(remote_address).GetIpv4(), 8860);
}


} // namespace okec
