#include "EdgeCommunicatorApplication.h"
#include "ns3/log.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "format_helper.hpp"

#define FMT_HEADER_ONLY
#include <fmt/core.h>



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


#define CHECK_INDEX(index) \
if (index > GetSize()) throw std::out_of_range{"index out of range"}


namespace ns3
{


NS_LOG_COMPONENT_DEFINE("EdgeCommunicatorApplication");
NS_OBJECT_ENSURE_REGISTERED(EdgeCommunicatorApplication);


EdgeCommunicatorApplication::EdgeCommunicatorApplication(NodeContainer* nodes /*= nullptr*/)
    : m_port{8860}, m_nodes{nodes}
{
}

EdgeCommunicatorApplication::~EdgeCommunicatorApplication()
{
}

TypeId EdgeCommunicatorApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::EdgeCommunicatorApplication")
                        .AddConstructor<EdgeCommunicatorApplication>()
                        .SetParent<Application>();
    return tid;
}

TypeId EdgeCommunicatorApplication::GetInstanceTypeId() const
{
    return EdgeCommunicatorApplication::GetTypeId();
}

void EdgeCommunicatorApplication::ReadHandler(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address remoteAddress;

    while ((packet = socket->RecvFrom(remoteAddress))) {
        PrintPacket(socket, packet, remoteAddress);

        Ptr<Task> task = Task::FromPacket(packet);
        if (task->Empty())
            continue;

        auto resource = socket->GetNode()->GetObject<Resource>();
        TaskOffloading(resource, task, remoteAddress);
    }
}

void EdgeCommunicatorApplication::Write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{
    NS_LOG_FUNCTION (this << packet << destination << port);
    m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
    m_socket->Send(packet);
}

Ipv4Address EdgeCommunicatorApplication::GetAddress() const
{
    return GetSocketAddress(m_socket);
}

u_int16_t EdgeCommunicatorApplication::GetPort() const
{
    return m_port;
}

void EdgeCommunicatorApplication::StartApplication()
{
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket(GetNode(), tid);

    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
    if (m_socket->Bind(local) == -1) {
      NS_FATAL_ERROR("Failed to bind socket");
    }

    m_socket->SetRecvCallback(MakeCallback(&EdgeCommunicatorApplication::ReadHandler, this));
}

void EdgeCommunicatorApplication::StopApplication()
{
    m_socket->Close();
}

inline void EdgeCommunicatorApplication::PrintPacket(Ptr<Socket> socket, Ptr<Packet> packet, 
    const Address& remoteAddress)
{
    Ipv4Address localAddress = GetSocketAddress(socket);

    // 打印接收信息
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remoteAddress);
    NS_LOG_INFO(PURPLE_CODE << localAddress << " Received a Packet at time: " 
        << Now().GetSeconds() << " from " << inetRemoteAddress.GetIpv4() << END_CODE);

    // 打印Packet
    auto content = GetStringPacket(packet);
    NS_LOG_INFO(PURPLE_CODE << "packet: \"" << content << "\" size: " << content.size() << END_CODE);

    // 打印任务信息
    // auto taskInfo = fmt::format("{:t}", *Task::FromPacket(packet));
    // NS_LOG_INFO(CYAN_CODE << taskInfo << END_CODE);

    // 获取资源信息
    auto resource = socket->GetNode()->GetObject<Resource>();
    if (resource == nullptr) {
        NS_LOG_INFO(YELLOW_CODE << " The Node " << inetRemoteAddress.GetIpv4() << " is on doesn't install any resource." << END_CODE);
    } else {
        auto resourceInfo = fmt::format("{:r}", *resource);
        NS_LOG_INFO(CYAN_CODE << resourceInfo << END_CODE);
    }
}

Ipv4Address EdgeCommunicatorApplication::GetSocketAddress(Ptr<Socket> socket)
{
    // 获取当前IP地址
    Ptr<Ipv4> ipv4 = socket->GetNode()->GetObject<Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}


void EdgeCommunicatorApplication::TaskOffloading(Ptr<Resource> resource, Ptr<Task> task, const Address& remoteAddress) const
{
    if (resource != nullptr) {
        // 资源充足，能够处理任务
        if (resource->GetMemory() > task->GetMemory() && resource->GetCpu() > task->GetCpu() && resource->GetPrice() < task->GetBudget()) {
            resource->SetMemory(resource->GetMemory() - task->GetMemory());
            resource->SetCpu(resource->GetCpu() - task->GetCpu());
            // resource->SetPrice(resource->GetPrice() - task->GetBudget());
            
            // 处理任务，并返回结果给用户
            
        } else {
            // 资源不足，寻找其他Node处理任务
            // NS_LOG_INFO("executed at here");
            if (m_nodes != nullptr) {
                for (uint32_t i = 0; i < m_nodes->GetN(); ++i) {
                    auto node = m_nodes->Get(i);

                    // 找到了当前Node，再找
                    if (node == m_socket->GetNode())
                        continue;

                    // Node上面没有任何应用，无效Node，再找
                    if (node->GetNApplications() < 1)
                        continue;

                    // 通过Node找到Communicator，也就找到了IP地址与端口。
                    // 其实也可以直接通过Node获取到IP地址，但无法得到端口，且最终的通信依旧要使用Communicator，
                    // 因此直接使用Communicator来获取地址和端口，以保证接口的一致性。
                    Ptr<EdgeCommunicatorApplication> communicator = node->GetApplication(0)->GetObject<EdgeCommunicatorApplication>();
                    if (communicator != nullptr) {
                        // 只有边缘设备提供商的Node上面才有资源，没有资源则再找
                        auto resource = node->GetObject<Resource>();
                        if (resource == nullptr)
                            continue;

                        NS_LOG_INFO("Found ip: " << communicator->GetAddress() << " port: " << communicator->GetPort());
                        auto resourceInfo = fmt::format("{:r}", *resource);
                        // NS_LOG_INFO("Provider: " << resource->GetProvider() << " Memory: " << resource->GetMemory()
                        //     << " Cpu: " << resource->GetCpu());  
                        NS_LOG_INFO(resourceInfo); 
                    }
                }
            }
        }
    }
}



EdgeCommunicatorContainer::EdgeCommunicatorContainer(std::size_t n, NodeContainer* nodes)
{
    m_communicators.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        m_communicators.emplace_back(CreateObject<EdgeCommunicatorApplication>(nodes));
}

EdgeCommunicatorContainer::value_type EdgeCommunicatorContainer::operator[](std::size_t index)
{
    return Get(index);
}

EdgeCommunicatorContainer::value_type EdgeCommunicatorContainer::operator()(std::size_t index)
{
    return Get(index);
}

EdgeCommunicatorContainer::value_type EdgeCommunicatorContainer::Get(std::size_t index)
{
    CHECK_INDEX(index);
    return m_communicators[index];
}

std::size_t EdgeCommunicatorContainer::GetSize() const
{
    return m_communicators.size();
}

auto EdgeCommunicatorContainer::begin()
{
    return m_communicators.begin();
}

auto EdgeCommunicatorContainer::end()
{
    return m_communicators.end();
}

void EdgeCommunicatorContainer::SetStartTime(Time start)
{
    std::for_each(m_communicators.begin(), m_communicators.end(), 
                [start = std::move(start)](value_type c) mutable {
                    c->SetStartTime(std::move(start));
                });
}

void EdgeCommunicatorContainer::SetStopTime(Time stop)
{
    std::for_each(m_communicators.begin(), m_communicators.end(), 
                [stop = std::move(stop)](value_type c) mutable {
                    c->SetStopTime(std::move(stop));
                });
}

} // namespace ns3