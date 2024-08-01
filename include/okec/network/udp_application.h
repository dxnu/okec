///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_UDP_APPLICATION_H_
#define OKEC_UDP_APPLICATION_H_

#include <okec/common/message_handler.hpp>
#include <ns3/application.h>
#include <ns3/socket.h>


namespace okec
{

class udp_application : public ns3::Application
{
public:
    using callback_type = std::function<void(ns3::Ptr<ns3::Packet>, const ns3::Address&)>;

public:
    udp_application();
    ~udp_application() override;

    static auto GetTypeId() -> ns3::TypeId;
    virtual auto GetInstanceTypeId() const -> ns3::TypeId;

    auto read_handler(ns3::Ptr<ns3::Socket> socket) -> void;
    auto write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) -> void;

    auto get_address() -> ns3::Ipv4Address const;
    auto get_port() -> u_int16_t const;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

    auto dispatch(std::string_view msg_type, ns3::Ptr<ns3::Packet> packet, const ns3::Address& address) -> void;

private:
    auto StartApplication() -> void override;
    auto StopApplication() -> void override;
    
    // 获取当前IPv4地址
    static auto get_socket_address(ns3::Ptr<ns3::Socket> socket) -> ns3::Ipv4Address;

private:
    uint16_t m_port;
    ns3::Ptr<ns3::Socket> m_recv_socket;
    ns3::Ptr<ns3::Socket> m_send_socket;
    message_handler<callback_type> m_msg_handler;
};


} // namespace okec


#endif // OKEC_UDP_APPLICATION_H_