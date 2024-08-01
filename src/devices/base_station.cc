///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/devices/base_station.h>
#include <okec/devices/cloud_server.h>
#include <okec/common/simulator.h>
#include <algorithm>  // for std::ranges::for_each
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ipv4.h>
#include <ns3/mobility-module.h>


#define CHECK_INDEX(index) \
if (index > size()) throw std::out_of_range{"index out of range"}


namespace okec
{

base_station::base_station(simulator& sim)
    : sim_{ sim },
      m_edge_devices{ nullptr },
      m_udp_application{ ns3::CreateObject<udp_application>() },
      m_node{ ns3::CreateObject<ns3::Node>() }
{
    m_udp_application->SetStartTime(ns3::Seconds(0));
    m_udp_application->SetStopTime(sim_.stop_time());

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);
}

base_station::~base_station()
{
    // auto& task_sequence = this->task_sequence();
    // if (!task_sequence.empty()) {
    //     okec::print("任务列表还余{}任务未处理\n", task_sequence.size());
    // } else {
    //     okec::print("任务已全部完成\n");
    // }
}

auto base_station::connect_device(edge_device_container& devices) -> void
{
    m_edge_devices = &devices;
}

auto base_station::get_address() const -> ns3::Ipv4Address
{
    auto ipv4 = m_node->GetObject<ns3::Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}

auto base_station::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto base_station::get_nodes(ns3::NodeContainer &nodes) -> void
{
    nodes.Add(m_node); // BS
    m_edge_devices->get_nodes(nodes); // EdgeDevice
}

auto base_station::get_edge_nodes(ns3::NodeContainer& nodes) -> void
{
    m_edge_devices->get_nodes(nodes); // EdgeDevice
}

auto base_station::get_node() -> ns3::Ptr<ns3::Node>
{
    return m_node;
}

auto base_station::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_udp_application->set_request_handler(msg_type, 
        [callback, this](ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) {
            callback(this, packet, remote_address);
        });
}

auto base_station::set_es_request_handler(std::string_view msg_type, es_callback_type callback) -> void
{
    for (auto it = m_edge_devices->begin(); it != m_edge_devices->end(); it++)
        (*it)->set_request_handler(msg_type, callback);
}

auto base_station::set_position(double x, double y, double z) -> void
{
    ns3::Ptr<ns3::MobilityModel> mobility = m_node->GetObject<ns3::MobilityModel>();
    if (!mobility) {
        mobility = ns3::CreateObject<ns3::ConstantPositionMobilityModel>();
        mobility->SetPosition(ns3::Vector(x, y, z));
        m_node->AggregateObject(mobility);
    } else {
        mobility->SetPosition(ns3::Vector(x, y, z));
    }
}

auto base_station::set_decision_engine(std::shared_ptr<decision_engine> engine) -> void
{
    m_decision_engine = engine;
}

auto base_station::get_position() -> ns3::Vector
{
    ns3::Ptr<ns3::MobilityModel> mobility = m_node->GetObject<ns3::MobilityModel>();
    return mobility ? mobility->GetPosition() : ns3::Vector();
}

auto base_station::get_edge_devices() const -> edge_device_container
{
    return *m_edge_devices;
}

auto base_station::write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) const -> void
{
    m_udp_application->write(packet, destination, port);
}

auto base_station::task_sequence(const task_element& item) -> void
{
    m_task_sequence.push_back(item);
    m_task_sequence_status.push_back(0); // 0 means not dispatched.
}

auto base_station::task_sequence(task_element&& item) -> void
{
    m_task_sequence.emplace_back(std::move(item));
    m_task_sequence_status.push_back(0); // 0 means not dispatched.
}

auto base_station::task_sequence() -> std::vector<task_element>&
{
    return m_task_sequence;
}

auto base_station::task_sequence_status() -> std::vector<char>&
{
    return m_task_sequence_status;
}

auto base_station::print_task_info() -> void
{
    // okec::print("Task sequence size: {}\n", m_task_sequence.size());
    // for (const auto& item : m_task_sequence) {
    //     okec::print("Item: {}\n", item.dump());
    // }
}

auto base_station::handle_next() -> void
{
    m_decision_engine->handle_next();
    // // 分发任务列表，每次拿出第一个未分发的任务
    // for (std::size_t i = 0; i < m_task_sequence.size(); ++i)
    // {
    //     // 任务尚未分发
    //     if (!m_task_sequence_status[i])
    //     {
    //         // auto [target_ip, target_port, distance] = dmaker.make_decision(m_task_sequence[i]);
    //         auto [target_ip, target_port, distance] = m_decision_engine->make_decision(m_task_sequence[i]);
    //         print_info(okec::format("The base station([{:ip}]) has made the decision. (target ip: {}, target port: {}, distance:{}m)", 
    //             this->get_address(), target_ip, target_port, distance));
            
    //         // 计算并记录传输时间
    //         Ptr<NetDevice> device = m_node->GetDevice(0);
    //         Ptr<Channel> channel = device->GetChannel();
    //         if (channel) {
    //             StringValue bandwidth;
    //             channel->GetAttribute("DataRate", bandwidth);
    //             long long size = std::stoll(m_task_sequence[i].get_header("input_size")) * 8; // 转换为 bits
    //             double trans_time = (double)size / std::stold(bandwidth.Get());
    //             double stime = distance * 1000 / 200000000; // 2 * 10^8
    //             print_info(okec::format("The base station([{:ip}]) has calculated the transmission time. (distance: {}, bandwidth: {}, trans_time: {}, send_time: {})", 
    //                 this->get_address(), distance, bandwidth.Get(), trans_time, stime));
    //             m_task_sequence[i].set_header("send_time", std::to_string(trans_time + stime));
    //         }

    //         message msg;
    //         msg.type(message_handling);
    //         msg.content(m_task_sequence[i]);
    //         print_info(okec::format("The base station([{:ip}]) dispatchs the task(task_id = {}) to {}",
    //             this->get_address(), m_task_sequence[i].get_header("task_id"), target_ip));
    //         m_udp_application->write(msg.to_packet(), ns3::Ipv4Address(target_ip.c_str()), target_port);

    //         // 更改任务分发状态
    //         m_task_sequence_status[i] = true;
    //         break;
    //     }
    // }
}

base_station_container::base_station_container(simulator& sim, std::size_t n)
{
    m_base_stations.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        auto bs = std::make_shared<base_station>(sim);
        m_base_stations.emplace_back(bs);
    }
}

auto base_station_container::operator[](std::size_t index) -> pointer_t
{
    return this->get(index);
}

auto base_station_container::operator()(std::size_t index) -> pointer_t
{
    return this->get(index);
}

auto base_station_container::get(std::size_t index) -> pointer_t
{
    CHECK_INDEX(index);
    return m_base_stations[index];
}

auto base_station_container::size() -> std::size_t
{
    return m_base_stations.size();
}

auto base_station_container::set_request_handler(
    std::string_view msg_type, callback_type callback) -> void
{
    std::ranges::for_each(m_base_stations,
        [&msg_type, callback](pointer_t bs) {
        bs->set_request_handler(msg_type, callback);
    });
}

auto base_station_container::set_es_request_handler(
    std::string_view msg_type, es_callback_type callback) -> void
{
    std::ranges::for_each(m_base_stations,
        [&msg_type, callback](pointer_t bs) {
        bs->set_es_request_handler(msg_type, callback);
    });
}

auto base_station_container::set_decision_engine(std::shared_ptr<decision_engine> engine) -> void
{
    for (pointer_t bs : m_base_stations) {
        bs->set_decision_engine(engine);
    }
}

} // namespace okec