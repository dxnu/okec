///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_BASE_STATION_H_
#define OKEC_BASE_STATION_H_

#include <okec/algorithms/decision_engine.h>
#include <okec/common/message.h>
#include <okec/devices/cloud_server.h>
#include <okec/devices/edge_device.h>
#include <okec/utils/log.h>
#include <concepts>
#include <functional>
#include <ranges>


namespace okec
{

class base_station_container;
class simulator;

template <typename T>
concept decision_maker_t = requires (T t) { t.make_decision(task_element{nullptr}); };

class base_station
{
public:
    using callback_type     = std::function<void(base_station*, ns3::Ptr<ns3::Packet>, const ns3::Address&)>;
    using es_callback_type  = std::function<void(edge_device*, ns3::Ptr<ns3::Packet>, const ns3::Address&)>;

public:
    base_station(simulator& sim);
    ~base_station();
    auto connect_device(edge_device_container& devices) -> void;
    
    auto get_address() const ->  ns3::Ipv4Address;
    auto get_port() const -> uint16_t;
    
    // 获取包含EdgeDevice在内的所有Nodes
    auto get_nodes(ns3::NodeContainer& nodes) -> void;

    auto get_edge_nodes(ns3::NodeContainer& nodes) -> void;
    
    auto get_node() -> ns3::Ptr<ns3::Node>;

    auto push_base_stations(base_station_container* base_stations) -> void;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

    auto set_es_request_handler(std::string_view msg_type, es_callback_type callback) -> void;

    auto set_position(double x, double y, double z) -> void;

    auto set_decision_engine(std::shared_ptr<decision_engine> engine) -> void;

    auto get_position() -> ns3::Vector;
    
    auto get_edge_devices() const -> edge_device_container;

    auto write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) const -> void;

    auto task_sequence(const task_element& item) -> void;
    auto task_sequence(task_element&& item) -> void;
    auto task_sequence() -> std::vector<task_element>&;
    auto task_sequence_status() -> std::vector<char>&;

    auto print_task_info() -> void;

    auto handle_next() -> void;

public:
    simulator& sim_;
    edge_device_container* m_edge_devices;
    ns3::Ptr<udp_application> m_udp_application;
    ns3::Ptr<ns3::Node> m_node;
    std::vector<task_element> m_task_sequence;
    std::vector<char> m_task_sequence_status; // 0: 未分发；1：已分发
    std::shared_ptr<decision_engine> m_decision_engine;
};


class base_station_container
{
public:
    using pointer_t         = std::shared_ptr<base_station>;
    using callback_type     = base_station::callback_type;
    using es_callback_type = base_station::es_callback_type;

public:
    base_station_container(simulator& sim, std::size_t n);

    template <typename... EdgeDeviceContainers>
    auto connect_device(EdgeDeviceContainers&... containers) -> bool {
        if (sizeof...(containers) != size()) {
            log::error("Error base_station_container::connect_device. Arguments size does not match the container size!");
            return false;
        }

        auto iter = std::begin(m_base_stations);
        ((*iter++)->connect_device(containers), ...);
        return true;
    }

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

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

    auto set_es_request_handler(std::string_view msg_type, es_callback_type callback) -> void;

    auto set_decision_engine(std::shared_ptr<decision_engine> engine) -> void;

private:
    std::vector<pointer_t> m_base_stations;
};

} // namespace okec

#endif // OKEC_BASE_STATION_H_