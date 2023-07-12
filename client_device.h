#ifndef SIMEG_CLIENT_DEVICE_H
#define SIMEG_CLIENT_DEVICE_H

#include "base_station.h"
#include "format_helper.hpp"
#include "resource.h"
#include "task.h"
#include "udp_application.h"
#include "ns3/node.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include <functional>
#include <vector>



namespace okec
{

using okec::udp_application;
using okec::base_station;


class client_device
{
    using model_type = std::function<std::pair<Ipv4Address, uint16_t>
        (const task&, const client_device&, const okec::cloud_server&)>;

public:
    client_device();

    // 剩余cpu cycles
    auto free_cpu_cycles() const -> int;

    // 剩余内存
    auto free_memory() const -> int;

    // 返回当前设备的IP地址
    auto get_address() const -> ns3::Ipv4Address;

    // 返回当前设备的端口号
    auto get_port() const -> uint16_t;

    auto get_node() -> Ptr<Node>;

    // 处理任务
    auto handle_task(const task& t, const cloud_server& cs) -> void;

    // 为当前设备安装资源
    auto install_resource(Ptr<resource> res) -> void;

    auto set_offload_model(model_type model) -> void;

    // 发送任务
    // 发送时间如果是0s，因为UdpApplication的StartTime也是0s，所以m_socket可能尚未初始化，此时Write将无法发送
    auto send_task(const base_station& bs, task &t, const ns3::Time &delay = ns3::Seconds(1.0)) -> void;

private:
    // 处理请求回调
    auto handle_response(Ptr<Packet> packet, const Address& remote_address) -> void;

private:
    Ptr<Node> m_node;
    Ptr<udp_application> m_udp_application;
    model_type m_model;
};



class client_device_container
{
    using value_type   = client_device;
    using pointer_type = std::shared_ptr<value_type>;

public:
    // 创建含有n个ClientDevice的容器
    client_device_container(std::size_t n)
    {
        m_devices.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            m_devices.emplace_back(std::make_shared<value_type>());
    }

    // 与基站建立WIFI通信
    // Note: 基站的地址也是在这里分配的
    auto ConnectWithBaseStationWIFI(base_station& bs, const Ipv4Address network, const Ipv4Mask mask) -> void
    {
        // ns3::NodeContainer wifiStaNodes;
        // this->GetNodes(wifiStaNodes);
        // ns3::NodeContainer wifiApNode;
        // wifiApNode.Add(bs.GetNode());

        // ns3::YansWifiChannelHelper channel = ns3::YansWifiChannelHelper::Default();
        // ns3::YansWifiPhyHelper phy;
        // phy.SetChannel(channel.Create());

        // ns3::WifiMacHelper mac;
        // ns3::WifiHelper wifi;
        // std::ostringstream ossControlMode;
        // ossControlMode << "OfdmRate";
        // wifi.SetStandard(ns3::WIFI_STANDARD_80211n);

        // int mcs = 2;
        // auto nonHtRefRateMbps = ns3::HtPhy::GetNonHtReferenceRate(mcs) / 1e6;
        // ossControlMode << nonHtRefRateMbps << "Mbps";

        // std::ostringstream ossDataMode;
        // ossDataMode << "HtMcs" << mcs;
        // wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
        //                              "DataMode",
        //                              ns3::StringValue(ossDataMode.str()),
        //                              "ControlMode",
        //                              ns3::StringValue(ossControlMode.str()));
        
        // int sgi = 1; // 0 1 2
        // wifi.ConfigHtOptions("ShortGuardIntervalSupported", BooleanValue(sgi));
        
        // int channelWidth = 20;
        // ns3::Ssid ssid = ns3::Ssid("ns3-80211n");
        // ns3::TupleValue<ns3::UintegerValue, ns3::UintegerValue, ns3::EnumValue, ns3::UintegerValue> channelValue;
        // ns3::WifiPhyBand band = ns3::WIFI_PHY_BAND_5GHZ;
        // channelValue.Set(ns3::WifiPhy::ChannelTuple{0, channelWidth, band, 0});

        // mac.SetType("ns3::StaWifiMac", "Ssid", ns3::SsidValue(ssid));
        // phy.Set("ChannelSettings", channelValue);

        // // 为STA安装WIFI网络
        // ns3::NetDeviceContainer staDevices;
        // staDevices = wifi.Install(phy, mac, wifiStaNodes);

        // mac.SetType("ns3::ApWifiMac",
        //             "EnableBeaconJitter",
        //             ns3::BooleanValue(false),
        //             "Ssid",
        //             ns3::SsidValue(ssid));
        // // 为AP安装WIFI网络
        // ns3::NetDeviceContainer apDevice;
        // apDevice = wifi.Install(phy, mac, wifiApNode);

        // // 加入移动模型，让STA可以移动，AP固定
        // ns3::MobilityHelper mobility;
        // // Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

        // // double distance = 1.0;
        // // positionAlloc->Add(ns3::Vector(0.0, 0.0, 0.0));
        // // positionAlloc->Add(ns3::Vector(distance, 0.0, 0.0));
        // // mobility.SetPositionAllocator(positionAlloc);

        // // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

        // // mobility.Install(wifiApNode);
        // // mobility.Install(wifiStaNodes);

        // mobility.SetPositionAllocator("ns3::GridPositionAllocator", 
        //     "MinX", ns3::DoubleValue(0.0), "MinY", ns3::DoubleValue(0.0),
        //     "DeltaX", ns3::DoubleValue(5.0), "DeltaY", ns3::DoubleValue(10.0),
        //     "GridWidth", ns3::UintegerValue(3), "LayoutType", ns3::StringValue("RowFirst"));
        
        // // 设置初始时节点的摆放位置
        // mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", ns3::RectangleValue(ns3::Rectangle(-50, 50, -50, 50)));
        // mobility.Install(wifiStaNodes);
        // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        // mobility.Install(wifiApNode);

        // /* Internet stack*/
        // ns3::InternetStackHelper stack;
        // stack.Install(wifiApNode);
        // stack.Install(wifiStaNodes);

        // ns3::Ipv4AddressHelper address;
        // address.SetBase(network, mask);
        // ns3::Ipv4InterfaceContainer staNodesInterface;
        // ns3::Ipv4InterfaceContainer apNodeInterface;

        // staNodesInterface = address.Assign(staDevices);
        // apNodeInterface = address.Assign(apDevice);
    }

    // 获取所有Nodes
    auto get_nodes(ns3::NodeContainer &nodes) -> void
    {
        for (auto& device : m_devices)
            nodes.Add(device->get_node());
    }

    auto get_device(std::size_t index) -> pointer_type
    {
        return m_devices[index];
    }


private:
    std::vector<pointer_type> m_devices;
};


} // namespace simeg

#endif // SIMEG_CLIENT_DEVICE_H