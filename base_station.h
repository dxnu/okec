#ifndef SIMEG_BASE_STATION_H
#define SIMEG_BASE_STATION_H

#include "cloud_server.h"
#include "edge_device.h"
#include "task.h"
#include "fmt/color.h"
#include <functional> // for std::reference_wrapper


namespace okec
{

class base_station_container;


class base_station
{
    using value_type   = std::reference_wrapper<edge_device>;

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

    auto push_base_stations(base_station_container* base_stations) -> void;
    
private:
 
    // default version
    auto on_dispatching_message(Ptr<Packet>, const Address& remoteAddress) -> void;
    auto on_offloading_message(Ptr<Packet>, const Address& remoteAddress) -> void;


public:
    edge_device_container* m_edge_devices;
    Ptr<udp_application> m_udp_application;
    Ptr<Node> m_node;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
    base_station_container* m_base_stations;
};


class base_station_container
{
    using pointer_t = std::shared_ptr<base_station>;
public:
    base_station_container(std::size_t n);

    template <typename... EdgeDeviceContainers>
    auto connect_device(EdgeDeviceContainers&... containers) -> bool {
        if (sizeof...(containers) != size()) {
            fmt::print(fg(fmt::color::red), "Error base_station_container::connect_device. Arguments size does not match the container size!\n");
            return false;
        }

        auto iter = std::begin(m_base_stations);
        ((*iter++)->connect_device(containers), ...);
        return true;
    }

    auto link_cloud(const cloud_server& cs) -> void;

    auto operator[](std::size_t index) -> pointer_t;
    auto operator()(std::size_t index) -> pointer_t;

    auto get(std::size_t index) -> pointer_t;

    auto begin() {
        return m_base_stations.begin();
    }

    auto end() {
        return m_base_stations.end();
    }

    auto size() -> std::size_t;

    // 记录分发处理情况
    auto dispatching_record(const std::string& task_id, const std::string& bs_ip) -> void;

    // 查询分发处理情况
    auto dispatched(const std::string& task_id, const std::string& bs_ip) -> bool;

    // 擦除分发记录
    auto erase_dispatching_record(const std::string& task_id) -> void;

private:
    std::vector<pointer_t> m_base_stations;
    std::multimap<std::string, std::string> m_dispatching_record; // [task_id, bs_ip] 
};

} // namespace okec

#endif // SIMEG_BASE_STATION_H