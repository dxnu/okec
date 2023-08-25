#ifndef OKEC_CLOUD_DEVICE_H
#define OKEC_CLOUD_DEVICE_H

#include "task.h"
#include "udp_application.h"
#include "ns3/internet-module.h"
#include "ns3/node-container.h"


namespace okec
{

using ns3::Address;
using ns3::Ipv4;
using ns3::Ipv4Address;
using ns3::Node;
using ns3::NodeContainer;
using ns3::Ptr;
using ns3::Packet;
using ns3::Seconds;
using ns3::Socket;


class base_station;
class base_station_container;


class cloud_server {
    using bs_ref_type = std::reference_wrapper<base_station>;
    using bs_pointer_t = std::shared_ptr<base_station>;


public:
    cloud_server();

    auto get_node() -> Ptr<Node>;

    auto get_nodes(NodeContainer &nodes) -> void;

    // 返回当前设备的IP地址
    auto get_address() const -> Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    auto push_base_station(bs_pointer_t bs) -> void;
    auto push_base_stations(base_station_container* base_stations) -> void;

    auto offload_task(const task&) const -> std::pair<Ipv4Address, uint16_t>;

private:
    // 处理请求回调函数
    auto on_offloading_message(Ptr<Packet> packet, const Address& remote_address) -> void;
    auto on_dispatching_failure_message(Ptr<Packet> packet, const Address& remote_address) -> void;
    auto on_dispatching_success_message(Ptr<Packet> packet, const Address& remote_address) -> void;
    auto on_handling_message(Ptr<Packet> packet, const Address& remote_address) -> void;
    

private:
    Ptr<Node> m_node;
    Ptr<udp_application> m_udp_application;
    // std::vector<bs_ref_type> m_base_stations;
    std::vector<bs_pointer_t> m_base_stations;
    std::multimap<std::string, std::string> m_task_dispatching_record; // [task_id, bs_ip] 
};


} // namespace okec


#endif // OKEC_CLOUD_DEVICE_H