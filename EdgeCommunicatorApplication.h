#ifndef NS3_EDGE_SERVER_H_
#define NS3_EDGE_SERVER_H_

#include "ns3/socket.h"
#include "ns3/application.h"
#include "PacketHelper.h"
#include "Resource.h"
#include "Task.h"


using namespace ns3;


namespace ns3
{


/**
 * \brief Communication component between Node and Node.
*/
class EdgeCommunicatorApplication : public Application
{
public:
    EdgeCommunicatorApplication(NodeContainer* nodes = nullptr);
    ~EdgeCommunicatorApplication() override;

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;

    void ReadHandler(Ptr<Socket> socket);
    void Write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port);

    /**
     * \brief 获取当前Communicator的端口，即获取了Communicator所绑定Node的端口
    */
    u_int16_t GetPort() const;

    /**
     * \brief 获取当前Communicator的地址，即获取了Communicator所绑定Node的地址
    */
    Ipv4Address GetAddress() const;


private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * \brief 打印Packet的信息
    */
    static inline void PrintPacket(Ptr<Socket> socket, Ptr<Packet> packet, const Address& remoteAddress);
    
    /**
     * \brief 获取当前IPv4地址
    */
    static Ipv4Address GetSocketAddress(Ptr<Socket> socket);
    
    /**
     * \brief 任务卸载
    */
    void TaskOffloading(Ptr<Resource> resource, Ptr<Task> task, const Address& remoteAddress) const;

private:
    Ptr<Socket> m_socket;
    u_int16_t m_port;
    NodeContainer* m_nodes; // 保存所有Nodes
};


class EdgeCommunicatorContainer
{
    using value_type = Ptr<EdgeCommunicatorApplication>;

public:
    EdgeCommunicatorContainer(std::size_t n, NodeContainer* nodes);

    value_type operator[](std::size_t index);
    value_type operator()(std::size_t index);

    value_type Get(std::size_t index);

    std::size_t GetSize() const;

    auto begin();

    auto end();

    void SetStartTime(Time start);
    void SetStopTime(Time stop);

private:
    std::vector<value_type> m_communicators;
};


} // namespace ns3


#endif // NS3_EDGE_SERVER_H_