#ifndef SIMEG_BASE_STATION_H
#define SIMEG_BASE_STATION_H

#include "cloud_server.h"
#include "edge_device.h"
#include "task.h"
#include <functional> // for std::reference_wrapper

namespace okec
{


class base_station
{
    using value_type = std::reference_wrapper<edge_device>;

public:
    base_station();
    auto connect_device(edge_device_container& devices) -> void;
    
    auto has_free_resource(const task& t) const -> bool;
    
    auto get_address() const ->  ns3::Ipv4Address;
    auto get_port() const -> uint16_t;
    
    // 获取包含EdgeDevice在内的所有Nodes
    auto get_nodes(ns3::NodeContainer& nodes) -> void;
    
    auto get_node() -> Ptr<Node>;
    
    auto link_cloud(const cloud_server& cs) -> void;
    
private:
    auto dispatch_task() -> void;
 
    // default version
    auto handle_request(Ptr<Packet>, const Address& remoteAddress) -> void;


public:
    edge_device_container* m_edge_devices;
    Ptr<udp_application> m_udp_application;
    Ptr<Node> m_node;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
};

} // namespace okec

#endif // SIMEG_BASE_STATION_H