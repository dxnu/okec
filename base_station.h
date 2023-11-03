#ifndef SIMEG_BASE_STATION_H
#define SIMEG_BASE_STATION_H

#include "cloud_server.h"
#include "edge_device.h"
#include "message.h"
#include "task.h"
#include "fmt/color.h"
#include <concepts>
#include <functional> // for std::reference_wrapper
#include <ranges>


namespace okec
{

class base_station_container;

template <typename T>
concept decision_maker_t = requires (T t) { t.make_decision(task_header{}); };

class base_station
{
public:
    using value_type     = std::reference_wrapper<edge_device>;
    using callback_type  = std::function<void(base_station*, Ptr<Packet>, const Address&)>;
    using detach_predicate_type = std::function<bool(std::shared_ptr<base_station> base)>;
    using detach_result_type = std::function<void(const ns3::Ipv4Address&, uint16_t)>;

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

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;
    
    auto get_edge_devices() const -> edge_device_container;

    auto write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) const -> void;

    auto erase_dispatching_record(const std::string& task_id) const -> void;

    auto dispatched(const std::string& task_id, const std::string& bs_ip) const -> bool;

    auto dispatching_record(const std::string& task_id) const -> void;

    auto detach(detach_predicate_type pred, detach_result_type yes, detach_result_type no) const -> void;

    auto task_sequence(Ptr<task> t) -> void;

    auto make_decision(decision_maker_t auto& dmaker, task_header header) -> void
    {
        auto [target_ip, target_port] = dmaker.make_decision(header);
        fmt::print("决策完成 target ip: {}, target port: {}\n", target_ip, target_port);

        auto pred = [&header](Ptr<task> t) {
            return header.id == t->id();
        };

        if (auto it = std::ranges::find_if(m_task_sequence, pred); 
            it != m_task_sequence.end()) {
            message msg {
                { "msgtype", message_handling },
                { "content", (*it)->get_body().instructions },
                { "task_id", header.id },
                { "cpu_demand", std::to_string((*it)->needed_cpu_cycles()) }
            };
            // fmt::print("task: {}\n", msg.dump());
            m_udp_application->write(msg.to_packet(), ns3::Ipv4Address(target_ip.c_str()), target_port);
        }
    }

private:
 
    // default version
    auto on_dispatching_message(Ptr<Packet>, const Address& remoteAddress) -> void;
    auto on_offloading_message(Ptr<Packet>, const Address& remoteAddress) -> void;
    auto on_response(Ptr<Packet>, const Address& remoteAddress) -> void;


public:
    edge_device_container* m_edge_devices;
    Ptr<udp_application> m_udp_application;
    Ptr<Node> m_node;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
    base_station_container* m_base_stations;
    std::vector<Ptr<task>> m_task_sequence;
};


class base_station_container
{
public:
    using pointer_t     = std::shared_ptr<base_station>;
    using callback_type = base_station::callback_type;

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

    auto cbegin() const {
        return m_base_stations.cbegin();
    }

    auto cend() const {
        return m_base_stations.cend();
    }

    auto size() -> std::size_t;

    // 记录分发处理情况
    auto dispatching_record(const std::string& task_id, const std::string& bs_ip) -> void;

    // 查询分发处理情况
    auto dispatched(const std::string& task_id, const std::string& bs_ip) -> bool;

    // 擦除分发记录
    auto erase_dispatching_record(const std::string& task_id) -> void;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

private:
    std::vector<pointer_t> m_base_stations;
    std::multimap<std::string, std::string> m_dispatching_record; // [task_id, bs_ip] 
};

} // namespace okec

#endif // SIMEG_BASE_STATION_H