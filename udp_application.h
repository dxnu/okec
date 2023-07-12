#ifndef OKEC_UDP_APPLICATION_H
#define OKEC_UDP_APPLICATION_H

#include "message_handler.hpp"
#include "ns3/socket.h"
#include "ns3/application.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>


namespace okec
{

using ns3::Address;
using ns3::Ipv4Address;
using ns3::Ptr;
using ns3::Packet;
using ns3::Socket;


class udp_application : public ns3::Application
{
public:
    using callback_type = std::function<void(Ptr<Packet>, const Address&)>;

public:
    udp_application();
    ~udp_application() override;

    static auto GetTypeId() -> ns3::TypeId;
    virtual auto GetInstanceTypeId() const -> ns3::TypeId;

    auto read_handler(Ptr<Socket> socket) -> void;
    auto write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) -> void;

    auto get_address() -> Ipv4Address const;
    auto get_port() -> u_int16_t const;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

private:
    auto StartApplication() -> void override;
    auto StopApplication() -> void override;

    // 打印Packet的信息
    static inline auto print_packet(Ptr<Socket> socket, Ptr<Packet> packet, const Address& remoteAddress) -> void;
    
    // 获取当前IPv4地址
    static auto get_socket_address(Ptr<Socket> socket) -> Ipv4Address;

private:
    uint16_t m_port;
    Ptr<Socket> m_recv_socket;
    Ptr<Socket> m_send_socket;
    message_handler<callback_type> m_msg_handler;
};


} // namespace okec


#endif // OKEC_UDP_APPLICATION_H