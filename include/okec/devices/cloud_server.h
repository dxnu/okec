///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_CLOUD_DEVICE_H
#define OKEC_CLOUD_DEVICE_H

#include <okec/common/task.h>
#include <okec/common/resource.h>
#include <okec/network/udp_application.h>
#include "ns3/internet-module.h"
#include "ns3/node-container.h"


namespace okec
{

class simulator;

class cloud_server {
    using callback_type  = std::function<void(cloud_server*, ns3::Ptr<ns3::Packet>, const ns3::Address&)>;

public:
    cloud_server(simulator& sim);

    auto get_node() -> ns3::Ptr<ns3::Node>;

    auto get_nodes(ns3::NodeContainer &nodes) -> void;

    // 返回当前设备的IP地址
    auto get_address() const -> ns3::Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    // 获取当前设备绑定的资源
    auto get_resource() -> ns3::Ptr<resource>;

    // 为当前设备安装资源
    auto install_resource(ns3::Ptr<resource> res) -> void;

    auto set_position(double x, double y, double z) -> void;
    auto get_position() -> ns3::Vector;

    auto set_request_handler(std::string_view msg_type, callback_type callback) -> void;

    auto write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) const -> void;

private:
    // 处理请求回调函数
    auto on_get_resource_information(ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

private:
    simulator& sim_;
    ns3::Ptr<ns3::Node> m_node;
    ns3::Ptr<udp_application> m_udp_application;
};


} // namespace okec


#endif // OKEC_CLOUD_DEVICE_H