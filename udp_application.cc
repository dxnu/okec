#include "udp_application.h"
#include "format_helper.hpp"
#include "message_helper.hpp"
#include "task.h"
#include "ns3/log.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"


// #define FMT_HEADER_ONLY
// #include <fmt/core.h>

#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"



namespace okec
{

NS_LOG_COMPONENT_DEFINE("udp_application");
NS_OBJECT_ENSURE_REGISTERED(udp_application);


udp_application::udp_application()
    : m_port{ 8860 },
      m_recv_socket{ nullptr },
      m_send_socket{ nullptr }
{
}

udp_application::~udp_application()
{
}

auto udp_application::GetTypeId() -> TypeId
{
    static TypeId tid = TypeId("okec::udp_application")
                        .AddConstructor<udp_application>()
                        .SetParent<Application>();
    return tid;
}

auto udp_application::GetInstanceTypeId() const -> TypeId
{
    return udp_application::GetTypeId();
}

auto udp_application::read_handler(Ptr<Socket> socket) -> void
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address remote_address;

    while ((packet = socket->RecvFrom(remote_address))) {
        // print_packet(socket, packet, remote_address);

        // 处理消息
        auto content = packet_helper::to_string(packet);
        NS_LOG_INFO(PURPLE_CODE << "At time " << fmt::format("{:.2f}", Simulator::Now().GetSeconds()) << "s "
            << fmt::format("{:ip}", this->get_address()) << " has received a packet: \"" << content << "\" size: " << content.size() << END_CODE);
        // fmt::print("handling message. begin-----------------------\n");
        if (packet) {
            auto msg_type = get_message_type(packet);
            NS_LOG_INFO(CYAN_CODE << "At time " << fmt::format("{:.2f}", Simulator::Now().GetSeconds()) << "s "
                << fmt::format("{:ip}", this->get_address()) <<  " is processing " << msg_type << " message......" << END_CODE);
            m_msg_handler.dispatch(msg_type, packet, remote_address);
            // if (m_msg_handler.dispatch(msg_type, packet, remote_address)) {
                // fmt::print("handling {} message. end------------------\n", msg_type);
            // }
        }
        

        // Ptr<Task> task = Task::GetTaskFromPacket(packet);
        // if (task->Empty())
        //     continue;

        // auto resource = socket->GetNode()->GetObject<Resource>();
        // TaskOffloading(resource, task, remoteAddress);
    }
}

auto udp_application::write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) -> void
{
    print_info(fmt::format("{:ip}:{} ---> {:ip}:{}", this->get_address(), this->get_port(), Ipv4Address::ConvertFrom(destination), port));
    NS_LOG_FUNCTION (this << packet << destination << port);
    
    // m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
    m_send_socket->Connect(InetSocketAddress(destination, port));
    m_send_socket->Send(packet);
}

auto udp_application::get_address() -> Ipv4Address const
{
    return get_socket_address(m_recv_socket);
}

auto udp_application::get_port() -> u_int16_t const
{
    return m_port;
}

auto udp_application::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    // push_message_handler<callback_type>({ msg_type, callback});
    m_msg_handler.add_handler(msg_type, callback);
}

auto udp_application::StartApplication() -> void
{
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket = Socket::CreateSocket(GetNode(), tid);

    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
    if (m_recv_socket->Bind(local) == -1) {
        NS_FATAL_ERROR("Failed to bind socket");
    }
    
    m_recv_socket->SetRecvCallback(MakeCallback(&udp_application::read_handler, this));

    m_send_socket = Socket::CreateSocket(GetNode(), tid);
}

auto udp_application::StopApplication() -> void
{
    m_recv_socket->Close();
    m_send_socket->Close();
}

inline auto udp_application::print_packet(Ptr<Socket> socket, Ptr<Packet> packet, const Address& remote_address) -> void
{
    // Ipv4Address local_address = get_socket_address(socket);

    // // 打印接收信息
    // InetSocketAddress address = InetSocketAddress::ConvertFrom(remote_address);
    // NS_LOG_INFO(PURPLE_CODE << local_address << " Received a Packet at time: " 
    //     << Now().GetSeconds() << " from " << address.GetIpv4() << END_CODE);

    // // 打印Packet
    // auto content = packet_helper::to_string(packet);
    // NS_LOG_INFO(PURPLE_CODE << "packet: \"" << content << "\" size: " << content.size() << END_CODE);

    // // 打印任务信息
    // auto t = task::from_packet(packet);
    // if (!t->empty()) {
    //     auto task_info = fmt::format("{:t}", *t);
    //     NS_LOG_INFO(CYAN_CODE << task_info << END_CODE);
    // }

    // // 获取资源信息
    // auto res = socket->GetNode()->GetObject<okec::resource>();
    // if (res == nullptr) {
    //     NS_LOG_INFO(YELLOW_CODE << " The Node " << address.GetIpv4() << " is on doesn't install any resource." << END_CODE);
    // } else {
    //     auto resourceInfo = fmt::format("{:r}", *res);
    //     NS_LOG_INFO(CYAN_CODE << resourceInfo << END_CODE);
    // }
}

auto udp_application::get_socket_address(Ptr<Socket> socket) -> Ipv4Address
{
    // 获取当前IP地址
    Ptr<Ipv4> ipv4 = socket->GetNode()->GetObject<Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}


} // namespace simeg