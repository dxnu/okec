///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/task.h>
#include <okec/network/udp_application.h>
#include <okec/utils/format_helper.hpp>
#include <okec/utils/log.h>
#include <okec/utils/message_helper.hpp>
#include <ns3/arp-header.h>
#include <ns3/csma-net-device.h>
#include <ns3/ethernet-header.h>
#include <ns3/ipv4.h>
#include <ns3/ipv4-header.h>
#include <ns3/simulator.h>
#include <ns3/udp-header.h>
#include <ns3/udp-socket.h>

// #define PURPLE_CODE "\033[95m"
// #define CYAN_CODE "\033[96m"
// #define TEAL_CODE "\033[36m"
// #define BLUE_CODE "\033[94m"
// #define GREEN_CODE "\033[32m"
// #define YELLOW_CODE "\033[33m"
// #define LIGHT_YELLOW_CODE "\033[93m"
// #define RED_CODE "\033[91m"
// #define BOLD_CODE "\033[1m"
// #define END_CODE "\033[0m"


namespace okec
{

// NS_LOG_COMPONENT_DEFINE("udp_application");
// NS_OBJECT_ENSURE_REGISTERED(udp_application);


udp_application::udp_application()
    : m_port{ 8860 },
      m_recv_socket{ nullptr },
      m_send_socket{ nullptr }
{
}

udp_application::~udp_application()
{
}

auto udp_application::GetTypeId() -> ns3::TypeId
{
    static ns3::TypeId tid = ns3::TypeId("okec::udp_application")
                        .AddConstructor<udp_application>()
                        .SetParent<Application>();
    return tid;
}

auto udp_application::GetInstanceTypeId() const -> ns3::TypeId
{
    return udp_application::GetTypeId();
}

auto udp_application::read_handler(ns3::Ptr<ns3::Socket> socket) -> void
{
    // NS_LOG_FUNCTION(this << socket);
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address remote_address;

    while ((packet = socket->RecvFrom(remote_address))) {
        auto content = packet_helper::to_string(packet);
        log::debug("{:ip} has received a packet: \"{}\" size: {}", this->get_address(), content, content.size());
        if (packet) {
            auto msg_type = get_message_type(packet);
            log::debug("{:ip} is processing [{}] message...", this->get_address(), msg_type);
            auto dispatched = m_msg_handler.dispatch(msg_type, packet, remote_address);
            NS_ASSERT_MSG(dispatched, "Invalid message type: " << msg_type);
        }
    }
}

auto udp_application::write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) -> void
{
    log::debug("{:ip}:{} ---> {:ip}:{}", this->get_address(), this->get_port(), ns3::Ipv4Address::ConvertFrom(destination), port);
    // NS_LOG_FUNCTION (this << packet << destination << port);
    
    m_send_socket->Connect(ns3::InetSocketAddress(destination, port));
    m_send_socket->Send(packet);
}

auto udp_application::get_address() -> ns3::Ipv4Address const
{
    return get_socket_address(m_recv_socket);
}

auto udp_application::get_port() -> u_int16_t const
{
    return m_port;
}

auto udp_application::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_msg_handler.add_handler(msg_type, callback);
}

auto udp_application::dispatch(std::string_view msg_type, ns3::Ptr<ns3::Packet> packet, const ns3::Address& address) -> void
{
    m_msg_handler.dispatch(msg_type.data(), packet, address);
}

auto udp_application::StartApplication() -> void
{
    ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket = ns3::Socket::CreateSocket(GetNode(), tid);

    ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), m_port);
    if (m_recv_socket->Bind(local) == -1) {
        log::error("Failed to build socket");
        return;
    }
    
    m_recv_socket->SetRecvCallback(MakeCallback(&udp_application::read_handler, this));

    m_send_socket = ns3::Socket::CreateSocket(GetNode(), tid);
}

auto udp_application::StopApplication() -> void
{
    m_recv_socket->Close();
    m_send_socket->Close();
}

auto udp_application::get_socket_address(ns3::Ptr<ns3::Socket> socket) -> ns3::Ipv4Address
{
    // 获取当前IP地址
    ns3::Ptr<ns3::Ipv4> ipv4 = socket->GetNode()->GetObject<ns3::Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}


} // namespace simeg