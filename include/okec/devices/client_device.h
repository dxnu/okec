///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_CLIENT_DEVICE_H_
#define OKEC_CLIENT_DEVICE_H_

#include <okec/algorithms/decision_engine.h>
#include <okec/common/message.h>
#include <okec/common/resource.h>
#include <okec/common/task.h>
#include <okec/utils/format_helper.hpp>
#include <coroutine>
#include <functional>
#include <memory>
#include <vector>


namespace okec
{


class udp_application;
class base_station;
class response_awaiter;
class simulator;


class client_device : public std::enable_shared_from_this<client_device>
{
    using response_type   = response;
    using done_callback_t = std::function<void(const response_type&)>;
public:
    using callback_type  = std::function<void(client_device*, ns3::Ptr<ns3::Packet>, const ns3::Address&)>;

public:
    client_device(simulator& sim);

    auto get_resource() -> ns3::Ptr<resource>;

    // 返回当前设备的IP地址
    auto get_address() const -> ns3::Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    auto get_node() -> ns3::Ptr<ns3::Node>;

    // 为当前设备安装资源
    auto install_resource(ns3::Ptr<resource> res) -> void;

    // 发送任务
    // 发送时间如果是0s，因为UdpApplication的StartTime也是0s，所以m_socket可能尚未初始化，此时Write将无法发送
    auto send(task t) -> void;

    auto async_send(task t) -> std::suspend_never;

    auto async_read() -> response_awaiter;

    auto async_read(done_callback_t fn) -> void;

    auto when_done(response_type res) -> void;

    auto set_position(double x, double y, double z) -> void;
    auto get_position() -> ns3::Vector;

    auto set_decision_engine(std::shared_ptr<decision_engine> engine) -> void;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

    auto dispatch(std::string_view msg_type, ns3::Ptr<ns3::Packet> packet, const ns3::Address& address) -> void;

    auto response_cache() -> response_type&;

    auto has_done_callback() -> bool;
    auto done_callback(response_type res) -> void;

    auto write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) const -> void;


private:
    simulator& sim_;
    ns3::Ptr<ns3::Node> m_node;
    ns3::Ptr<udp_application> m_udp_application;
    response_type m_response;
    done_callback_t m_done_fn;
    std::shared_ptr<decision_engine> m_decision_engine;
};



class client_device_container
{
    using value_type    = client_device;
    using pointer_type  = std::shared_ptr<value_type>;
    using callback_type = client_device::callback_type;

public:
    // 创建含有n个ClientDevice的容器
    client_device_container(simulator& sim, std::size_t n) {
        m_devices.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            m_devices.emplace_back(std::make_shared<value_type>(sim));
    }

    // 获取所有Nodes
    auto get_nodes(ns3::NodeContainer &nodes) -> void {
        for (auto& device : m_devices)
            nodes.Add(device->get_node());
    }

    auto begin() -> std::vector<pointer_type>::iterator {
        return m_devices.begin();
    }

    auto end() -> std::vector<pointer_type>::iterator {
        return m_devices.end();
    }

    auto operator[](std::size_t index) -> pointer_type;
    auto operator()(std::size_t index) -> pointer_type;

    auto get_device(std::size_t index) -> pointer_type;

    auto size() -> std::size_t;

    auto install_resources(resource_container& res, int offset = 0) -> void;

    auto set_decision_engine(std::shared_ptr<decision_engine> engine) -> void;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

private:
    std::vector<pointer_type> m_devices;
};


} // namespace okec

#endif // OKEC_CLIENT_DEVICE_H_