///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_NETWORK_MODEL_HPP_
#define OKEC_NETWORK_MODEL_HPP_

#include <okec/devices/base_station.h>
#include <okec/devices/client_device.h>
#include <okec/common/resource.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/rectangle.h"


namespace okec
{

template <class T>
inline constexpr bool enable_network_model = false;

template <class T, class... Args>
concept network_model = enable_network_model<T>
                        && requires (T t, Args... args) { t.network_initializer(args...); };

template <class Model, class... Args>
    requires network_model<Model, Args...>
auto network_initializer(Model& model, Args&&... args) {
    model.network_initializer(std::forward<decltype(args)>(args)...);
}


void initialize_communication(client_device_container& client, 
    base_station& bs, cloud_server& cs)
{
    ns3::NodeContainer lan1_nodes, lan2_nodes, lan3_nodes;
    client.get_nodes(lan1_nodes); // client devices
    bs.get_nodes(lan2_nodes);     // base station and edge devices
    cs.get_nodes(lan3_nodes);     // cloud server

    ns3::NodeContainer router_nodes1(2), router_nodes2(2);

    // LAN1-->R1
    ns3::CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    lan1_nodes.Add(router_nodes1.Get(0));

    ns3::NetDeviceContainer lan1_devices;
    lan1_devices = csma1.Install(lan1_nodes);

    // LAN2-->Router
    ns3::CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    lan2_nodes.Add(router_nodes1.Get(1)); // LAN2-->R2
    lan2_nodes.Add(router_nodes2.Get(0)); // LAN2-->R3

    ns3::NetDeviceContainer lan2_devices;
    lan2_devices = csma2.Install(lan2_nodes);

    // R1<-->R2
    ns3::PointToPointHelper p2p_one;
    p2p_one.SetDeviceAttribute("DataRate", ns3::StringValue("10Mbps"));
    p2p_one.SetChannelAttribute("Delay", ns3::StringValue("2ms"));
    
    ns3::NetDeviceContainer router12_devices;
    router12_devices = p2p_one.Install(router_nodes1);

    // LAN3-->R4
    ns3::CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    lan3_nodes.Add(router_nodes2.Get(1));

    ns3::NetDeviceContainer lan3_devices;
    lan3_devices = csma3.Install(lan3_nodes);

    // R3<-->R4
    ns3::PointToPointHelper p2p_two;
    p2p_two.SetDeviceAttribute("DataRate", ns3::StringValue("10Mbps"));
    p2p_two.SetChannelAttribute("Delay", ns3::StringValue("2ms"));

    ns3::NetDeviceContainer router34_devices;
    router34_devices = p2p_two.Install(router_nodes2);

    // Setting IP addresses
    ns3::InternetStackHelper stack;
    stack.Install(lan1_nodes);
    stack.Install(lan2_nodes);
    stack.Install(lan3_nodes);

    ns3::Ipv4AddressHelper address;
    // 为LAN1分配地址
    address.SetBase("10.1.1.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan1_interfaces;
    lan1_interfaces = address.Assign(lan1_devices);

    // 为LAN2分配地址
    address.SetBase("10.1.2.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan2_interfaces;
    lan2_interfaces = address.Assign(lan2_devices);

    // 为LAN3分配地址
    address.SetBase("10.1.50.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan3_interfaces;
    lan3_interfaces = address.Assign(lan3_devices);

    // 为R1, R2分配地址
    address.SetBase("10.1.100.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer router12_interface;
    router12_interface = address.Assign(router12_devices);

    // 为R3, R4分配地址
    address.SetBase("10.1.150.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer router34_interface;
    router34_interface = address.Assign(router34_devices);

    // For routers to be able to forward packets, they need to have routing rules.
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void initialize_communication(client_device_container& client, 
    base_station_container& base_stations, cloud_server& cloud)
{
    // 读取用户端、基站、云服务器三层的设备
    ns3::NodeContainer lan_client, lan_cloud;
    client.get_nodes(lan_client);
    cloud.get_nodes(lan_cloud);

    auto bs_size = base_stations.size();
    ns3::NodeContainer lan_bs[bs_size];
    std::size_t i;
    for (i = 0; i < bs_size; ++i) {
        base_stations[i]->get_nodes(lan_bs[i]);
    }

    // 准备路由器
    // 为每个基站都配置左右两个路由器
    ns3::NodeContainer router_level_one[bs_size], router_level_two[bs_size];
    for (i = 0; i < bs_size; ++i) {
        router_level_one[i].Create(2);
        router_level_two[i].Create(2);
    }

    // 连接用户设备和基站间的路由器
    ns3::CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", ns3::StringValue("50Mbps"));
    csma1.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    for (i = 0; i < bs_size; ++i) {
        lan_client.Add(router_level_one[i].Get(0));
    }
    ns3::NetDeviceContainer lan_client_devices;
    lan_client_devices = csma1.Install(lan_client);

    // 基站配置路由器
    ns3::CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    ns3::NetDeviceContainer lan_bs_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        lan_bs[i].Add(router_level_one[i].Get(1)); // 连接基站与用户设备间的路由器
        lan_bs[i].Add(router_level_two[i].Get(0)); // 连接基站与云服务器间的路由器

        lan_bs_devices[i] = csma2.Install(lan_bs[i]);
    }

    // 配置一层路由器
    ns3::PointToPointHelper p2p_one;
    p2p_one.SetDeviceAttribute("DataRate", ns3::StringValue("10Mbps"));
    p2p_one.SetChannelAttribute("Delay", ns3::StringValue("2ms"));
    ns3::NetDeviceContainer router_level_one_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        router_level_one_devices[i] = p2p_one.Install(router_level_one[i]);
    }

    // 配置云服务器路由
    ns3::CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    ns3::NetDeviceContainer lan_cloud_devices;
    for (i = 0; i < bs_size; ++i) {
        lan_cloud.Add(router_level_two[i].Get(1));

        lan_cloud_devices = csma3.Install(lan_cloud);
    }

    // 配置二层路由器
    ns3::PointToPointHelper p2p_two;
    p2p_two.SetDeviceAttribute("DataRate", ns3::StringValue("10Mbps"));
    p2p_two.SetChannelAttribute("Delay", ns3::StringValue("2ms"));
    ns3::NetDeviceContainer router_level_two_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        router_level_two_devices[i] = p2p_two.Install(router_level_two[i]);
    }

    // 安装协议栈
    ns3::InternetStackHelper stack;
    stack.Install(lan_client);
    stack.Install(lan_cloud);
    for (i = 0; i < bs_size; ++i) {
        stack.Install(lan_bs[i]);
    }

    // 设置 IP 地址
    ns3::Ipv4AddressHelper address;

    // 为用户设备设置 IP 地址
    address.SetBase("10.1.1.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan_client_interfaces;
    lan_client_interfaces = address.Assign(lan_client_devices);

    // 为基站设置 IP 地址
    int base = 2;
    ns3::Ipv4InterfaceContainer lan_bs_interfaces[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip{ "10.1." };
        ip.append(std::to_string(base++));
        ip.append(".0");
        address.SetBase(ip.c_str(), "255.255.255.0");
        lan_bs_interfaces[i] = address.Assign(lan_bs_devices[i]);
    }
    
    // 为云服务器设置 IP 地址
    address.SetBase("10.1.50.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan_cloud_interfaces;
    lan_cloud_interfaces = address.Assign(lan_cloud_devices);

    // 为一层路由设置 IP 地址
    base = 100;
    ns3::Ipv4InterfaceContainer router_level_one_interface[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip{ "10.1." };
        ip.append(std::to_string(base++));
        ip.append(".0");
        address.SetBase(ip.c_str(), "255.255.255.0");
        router_level_one_interface[i] = address.Assign(router_level_one_devices[i]);
    }

    // 为二层路由设置 IP 地址
    base = 150;
    ns3::Ipv4InterfaceContainer router_level_two_interface[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip{ "10.1." };
        ip.append(std::to_string(base++));
        ip.append(".0");
        address.SetBase(ip.c_str(), "255.255.255.0");
        router_level_two_interface[i] = address.Assign(router_level_two_devices[i]);
    }

    // For routers to be able to forward packets, they need to have routing rules.
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

} // namespace okec


#endif // OKEC_NETWORK_MODEL_HPP_