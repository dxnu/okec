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


class cloud_server {
    using bs_ref_type = std::reference_wrapper<base_station>;
    using model_type  = std::function<std::pair<Ipv4Address, uint16_t>(const task&, const std::vector<bs_ref_type>&)>;


public:
    cloud_server();

    auto get_node() -> Ptr<Node>;

    auto get_nodes(NodeContainer &nodes) -> void;

    // 返回当前设备的IP地址
    auto get_address() const -> Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    auto push_base_station(bs_ref_type bs) -> void;

    auto set_offload_model(model_type model) -> void;

    auto offload_task(const task&) const -> std::pair<Ipv4Address, uint16_t>;

private:
    // 处理请求回调函数
    auto handle_request(Ptr<Packet> packet, const Address& remote_address) -> void;

private:
    Ptr<Node> m_node;
    Ptr<udp_application> m_udp_application;
    model_type m_model;
    std::vector<bs_ref_type> m_base_stations;
};


} // namespace okec


#endif // OKEC_CLOUD_DEVICE_H