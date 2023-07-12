#ifndef SIMEG_EDGE_DEVICE_H
#define SIMEG_EDGE_DEVICE_H

#include "Resource.h"
#include "udp_application.h"


namespace okec
{

class edge_device
{
public:
    edge_device();
    auto set_resource() -> void;
    auto handle_task(ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

    auto free_cpu_cycles() const -> int;

    auto free_memory() const -> int;

    auto price() const -> int;

    // 返回当前设备的IP地址
    auto get_address() const -> ns3::Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    auto get_node() -> ns3::Ptr<ns3::Node>;

public:
    ns3::Ptr<Node> m_node;
    ns3::Ptr<ns3::Resource> m_resource;
    ns3::Ptr<okec::udp_application> m_udp_application;
};


class edge_device_container
{
    using value_type   = edge_device;
    using pointer_type = std::shared_ptr<value_type>;

public:
    edge_device_container(std::size_t n);

    // 获取所有Nodes
    auto get_nodes(ns3::NodeContainer &nodes) -> void;

    auto get_device(std::size_t index) -> pointer_type;

    auto begin() -> std::vector<pointer_type>::iterator {
        return m_devices.begin();
    }

    auto end() -> std::vector<pointer_type>::iterator {
        return m_devices.end();
    }

private:
    std::vector<pointer_type> m_devices;
};


} // namespace simeg


#endif // SIMEG_EDGE_DEVICE_H