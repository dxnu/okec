#ifndef OKEC_HPP
#define OKEC_HPP

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/rectangle.h"
#include "base_station.h"
#include "client_device.h"
#include "resource.h"


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



struct default_network_model {
    auto initialize_communication(client_device_container& ues, base_station_container& bss) -> void {
        // 基站
        NodeContainer wifi_stations;
        int bs_size = bss.size();
        for (auto i : std::views::iota(0, bs_size)) {
            wifi_stations.Add(bss[i]->get_node());
        }

        // 用户设备
        NodeContainer wifi_devices;
        ues.get_nodes(wifi_devices);

        fmt::print("wifi_stations:{}, wifi_devices:{}\n", wifi_stations.GetN(), wifi_devices.GetN());

        // 边缘服务器
        NodeContainer p2p_nodes[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            bss[i]->get_nodes(p2p_nodes[i]); // 第一个是基站，后面全是边缘服务器
        }

        // 第一步，WIFI连接用户设备与基站
        ns3::WifiHelper wifi;
        int mcs = 2;
        int sgi = 1; // 0 1 2
        auto nonHtRefRateMbps = ns3::HtPhy::GetNonHtReferenceRate(mcs) / 1e6;
        wifi.SetStandard(WIFI_STANDARD_80211n);
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", 
                                     "DataMode",
                                     StringValue(fmt::format("HtMcs{}", mcs)),
                                     "ControlMode",
                                     StringValue(fmt::format("OfdmRate{}Mbps", nonHtRefRateMbps)));
        wifi.ConfigHtOptions("ShortGuardIntervalSupported", BooleanValue(sgi));

        int channelWidth = 20;
        auto ssid = ns3::Ssid("ns3-80211n");
        auto channelValue = ns3::TupleValue<UintegerValue, UintegerValue, EnumValue, UintegerValue>{};
        WifiPhyBand band = ns3::WIFI_PHY_BAND_5GHZ;
        channelValue.Set(ns3::WifiPhy::ChannelTuple{0, channelWidth, band, 0});

        // Create a channel helper and phy helper, and then create the channel
        auto channel = ns3::YansWifiChannelHelper::Default();
        auto phy = ns3::YansWifiPhyHelper();
        WifiMacHelper mac;
        phy.SetChannel(channel.Create());
        phy.Set("ChannelSettings", channelValue);
        mac.SetType("ns3::StaWifiMac", "Ssid", ns3::SsidValue(ssid));

        // 为STA安装WIFI网络
        NetDeviceContainer wifi_device_devices = wifi.Install(phy, mac, wifi_devices);

        // 为AP安装WIFI网络
        mac.SetType("ns3::ApWifiMac",
                    "EnableBeaconJitter",
                    ns3::BooleanValue(false),
                    "Ssid",
                    ns3::SsidValue(ssid));
        NetDeviceContainer wifi_station_devices = wifi.Install(phy, mac, wifi_stations);

        // 加入移动模型，让STA可以移动，AP固定
        ns3::MobilityHelper mobility;
        mobility.SetPositionAllocator("ns3::GridPositionAllocator", 
            "MinX", DoubleValue(0.0), "MinY", DoubleValue(0.0),
            "DeltaX", DoubleValue(5.0), "DeltaY", DoubleValue(10.0),
            "GridWidth", UintegerValue(3), "LayoutType", StringValue("RowFirst"));
        // 设置初始时节点的摆放位置
        mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
        mobility.Install(wifi_devices);  // STA 随机移动
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(wifi_stations); // AP 固定

        // 第二步，P2P连接基站与边缘服务器
        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps")); // 设置带宽为100Mbps
        p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds (2))); // 设置延迟为2ms

        // 创建每个基站和边缘服务器的点对点链接
        NetDeviceContainer p2p_devices[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            for (uint32_t j = 0; j < p2p_nodes[i].GetN() - 1; ++j) {
                p2p_devices[i].Add(p2p.Install(p2p_nodes[i].Get(0), p2p_nodes[i].Get(j + 1)));
            }
        }

        // 第三步，安装网络协议栈
        InternetStackHelper stack;
        stack.Install(wifi_stations); // 为基站节点安装网络协议栈
        stack.Install(wifi_devices);  // 为用户设备节点安装网络协议栈
        for (auto i : std::views::iota(0, bs_size)) {
            for (auto iter = p2p_nodes[i].Begin() + 1; iter != p2p_nodes[i].End (); ++iter) {
                stack.Install(*iter);
            }
        }

        // 第四步，分配地址
        Ipv4AddressHelper wifi_address;
        wifi_address.SetBase ("10.1.1.0", "255.255.255.0"); // 设置基站节点的IP地址
        Ipv4InterfaceContainer wifi_station_interfaces = wifi_address.Assign(wifi_station_devices);
        

        wifi_address.SetBase ("10.1.2.0", "255.255.255.0"); // 设置用户设备节点的IP地址
        Ipv4InterfaceContainer wifi_device_interfaces = wifi_address.Assign(wifi_device_devices);

        // Ipv4AddressHelper p2p_address;
        // Ipv4InterfaceContainer p2p_interfaces;
        // int base = 3;
        // for (auto i : std::views::iota(0, bs_size)) { // 设置用户设备节点的IP地址
        //     p2p_address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        //     p2p_interfaces.Add(p2p_address.Assign(p2p_devices[i]));
        // }

        // For routers to be able to forward packets, they need to have routing rules.
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
};


void initialize_communication(base_station& bs, client_device_container& clientDevices)
{
    NodeContainer lan1Nodes, lan2Nodes;
    bs.get_nodes(lan1Nodes);
    clientDevices.get_nodes(lan2Nodes);

    NodeContainer routerNodes(2);

    // LAN1-->Router1
    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue ("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue (NanoSeconds (6560)));
    lan1Nodes.Add(routerNodes.Get(0));

    NetDeviceContainer lan1Devices;
    lan1Devices = csma1.Install(lan1Nodes);

    // LAN2-->Router2
    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue ("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue (NanoSeconds (6560)));
    lan2Nodes.Add(routerNodes.Get(1));

    NetDeviceContainer lan2Devices;
    lan2Devices = csma2.Install(lan2Nodes);

    // Router1<-->Router2
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    
    NetDeviceContainer routerDevices;
    routerDevices = pointToPoint.Install(routerNodes);

    // Setting IP addresses
    InternetStackHelper stack;
    stack.Install(lan1Nodes);
    stack.Install(lan2Nodes);

    Ipv4AddressHelper address;
    
    // 为LAN1分配地址
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer lan1Interfaces;
    lan1Interfaces = address.Assign(lan1Devices);

    // 为LAN2分配地址
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer lan2Interfaces;
    lan2Interfaces = address.Assign(lan2Devices);

    // 为路由分配地址
    address.SetBase("10.1.100.0", "255.255.255.0");
    Ipv4InterfaceContainer routerInterfaces;
    routerInterfaces = address.Assign(routerDevices);

    // For routers to be able to forward packets, they need to have routing rules.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
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
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds (6560)));
    lan1_nodes.Add(router_nodes1.Get(0));

    ns3::NetDeviceContainer lan1_devices;
    lan1_devices = csma1.Install(lan1_nodes);

    // LAN2-->Router
    ns3::CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    lan2_nodes.Add(router_nodes1.Get(1)); // LAN2-->R2
    lan2_nodes.Add(router_nodes2.Get(0)); // LAN2-->R3

    ns3::NetDeviceContainer lan2_devices;
    lan2_devices = csma2.Install(lan2_nodes);

    // R1<-->R2
    ns3::PointToPointHelper p2p_one;
    p2p_one.SetDeviceAttribute("DataRate", StringValue ("10Mbps"));
    p2p_one.SetChannelAttribute("Delay", StringValue ("2ms"));
    
    ns3::NetDeviceContainer router12_devices;
    router12_devices = p2p_one.Install(router_nodes1);

    // LAN3-->R4
    ns3::CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    lan3_nodes.Add(router_nodes2.Get(1));

    ns3::NetDeviceContainer lan3_devices;
    lan3_devices = csma3.Install(lan3_nodes);

    // R3<-->R4
    ns3::PointToPointHelper p2p_two;
    p2p_two.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p_two.SetChannelAttribute("Delay", StringValue("2ms"));

    ns3::NetDeviceContainer router34_devices;
    router34_devices = p2p_two.Install(router_nodes2);

    // Setting IP addresses
    ns3::InternetStackHelper stack;
    stack.Install(lan1_nodes);
    stack.Install(lan2_nodes);
    stack.Install(lan3_nodes);

    Ipv4AddressHelper address;
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
    Ipv4InterfaceContainer router12_interface;
    router12_interface = address.Assign(router12_devices);

    // 为R3, R4分配地址
    address.SetBase("10.1.150.0", "255.255.255.0");
    Ipv4InterfaceContainer router34_interface;
    router34_interface = address.Assign(router34_devices);

    // For routers to be able to forward packets, they need to have routing rules.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
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
    csma1.SetChannelAttribute("DataRate", StringValue("50Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds (6560)));
    for (i = 0; i < bs_size; ++i) {
        lan_client.Add(router_level_one[i].Get(0));
    }
    ns3::NetDeviceContainer lan_client_devices;
    lan_client_devices = csma1.Install(lan_client);

    // 基站配置路由器
    ns3::CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    ns3::NetDeviceContainer lan_bs_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        lan_bs[i].Add(router_level_one[i].Get(1)); // 连接基站与用户设备间的路由器
        lan_bs[i].Add(router_level_two[i].Get(0)); // 连接基站与云服务器间的路由器

        lan_bs_devices[i] = csma2.Install(lan_bs[i]);
    }

    // 配置一层路由器
    ns3::PointToPointHelper p2p_one;
    p2p_one.SetDeviceAttribute("DataRate", StringValue ("10Mbps"));
    p2p_one.SetChannelAttribute("Delay", StringValue ("2ms"));
    ns3::NetDeviceContainer router_level_one_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        router_level_one_devices[i] = p2p_one.Install(router_level_one[i]);
    }

    // 配置云服务器路由
    ns3::CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    ns3::NetDeviceContainer lan_cloud_devices;
    for (i = 0; i < bs_size; ++i) {
        lan_cloud.Add(router_level_two[i].Get(1));

        lan_cloud_devices = csma3.Install(lan_cloud);
    }

    // 配置二层路由器
    ns3::PointToPointHelper p2p_two;
    p2p_two.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p_two.SetChannelAttribute("Delay", StringValue("2ms"));
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
    Ipv4AddressHelper address;

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
    Ipv4InterfaceContainer router_level_one_interface[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip{ "10.1." };
        ip.append(std::to_string(base++));
        ip.append(".0");
        address.SetBase(ip.c_str(), "255.255.255.0");
        router_level_one_interface[i] = address.Assign(router_level_one_devices[i]);
    }

    // 为二层路由设置 IP 地址
    base = 150;
    Ipv4InterfaceContainer router_level_two_interface[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip{ "10.1." };
        ip.append(std::to_string(base++));
        ip.append(".0");
        address.SetBase(ip.c_str(), "255.255.255.0");
        router_level_two_interface[i] = address.Assign(router_level_two_devices[i]);
    }

    // For routers to be able to forward packets, they need to have routing rules.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    
}

void initialize_communication222(client_device_container& client, 
    base_station_container& base_stations, cloud_server& cloud)
{
    // LAN_Client and LAN_Cloud
    ns3::NodeContainer lan_client, lan_cloud;
    client.get_nodes(lan_client);
    cloud.get_nodes(lan_cloud);

    // LAN_BS
    auto bs_size = base_stations.size();
    fmt::print("bs size: {}\n", bs_size);
    ns3::NodeContainer lan_bs[bs_size];
    std::size_t i;
    for (i = 0; i < bs_size; ++i) {
        base_stations[i]->get_nodes(lan_bs[i]);
    }

    // Router
    ns3::NodeContainer router1[bs_size], router2[bs_size];
    for (i = 0; i < bs_size; ++i) {
        router1[i].Create(2);
        router2[i].Create(2);
    }

    // LAN_Client --> R1
    ns3::CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds (6560)));
    for (i = 0; i < bs_size; ++i) {
        lan_client.Add(router1[i].Get(0));
    }

    ns3::NetDeviceContainer lan_client_devices;
    lan_client_devices = csma1.Install(lan_client);

    // LAN_BS --> R1,R2
    ns3::CsmaHelper csma2[bs_size];
    for (i = 0; i < bs_size; ++i) {
        csma2[i].SetChannelAttribute("DataRate", StringValue("100Mbps"));
        csma2[i].SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

        lan_bs[i].Add(router1[i].Get(1));
        lan_bs[i].Add(router2[i].Get(0));
    }

    ns3::NetDeviceContainer lan_bs_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        lan_bs_devices[i] = csma2[i].Install(lan_bs[i]);
    }

    // R1 --> R1
    ns3::PointToPointHelper p2p_one[bs_size];

    ns3::NetDeviceContainer router1_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        p2p_one[i].SetDeviceAttribute("DataRate", StringValue ("10Mbps"));
        p2p_one[i].SetChannelAttribute("Delay", StringValue ("2ms"));

        router1_devices[i] = p2p_one[i].Install(router1[i]);
    }

    // LAN_Cloud --> R2
    ns3::CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    for (i = 0; i < bs_size; ++i) {
        lan_cloud.Add(router2[i].Get(1));
    }

    ns3::NetDeviceContainer lan_cloud_devices;
    lan_cloud_devices = csma3.Install(lan_cloud);

    // R2 --> R2
    ns3::PointToPointHelper p2p_two[bs_size];
    

    ns3::NetDeviceContainer router2_devices[bs_size];
    for (i = 0; i < bs_size; ++i) {
        p2p_two[i].SetDeviceAttribute("DataRate", StringValue("10Mbps"));
        p2p_two[i].SetChannelAttribute("Delay", StringValue("2ms"));

        router2_devices[i] = p2p_two[i].Install(router2[i]);
    }

    fmt::print("begin set ip\n");

    // Setting IP addresses
    ns3::InternetStackHelper stack;
    stack.Install(lan_client);
    stack.Install(lan_cloud);
    ns3::InternetStackHelper bs_stack[bs_size];
    for (i = 0; i < bs_size; ++i) {
        bs_stack[i].Install(lan_bs[i]);
    }

    fmt::print("flag1 set ip\n");

    Ipv4AddressHelper address;
    // 为 LAN_Client 分配地址
    address.SetBase("10.1.1.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan_client_interfaces;
    lan_client_interfaces = address.Assign(lan_client_devices);

    fmt::print("flag2 set ip\n");

    // 为 LAN_BS 分配地址
    int base = 2;
    ns3::Ipv4InterfaceContainer lan_bs_interfaces[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip_base { "10.1." };
        ip_base.append(std::to_string(base++));
        ip_base.append(".0"); // 10.1.(2, 3....).0
        
        address.SetBase(ip_base.c_str(), "255.255.255.0");
        lan_bs_interfaces[i] = address.Assign(lan_bs_devices[i]);
    }

    fmt::print("flag3 set ip\n");

    // 为 LAN_Cloud 分配地址
    address.SetBase("10.1.50.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer lan_cloud_interfaces;
    lan_cloud_interfaces = address.Assign(lan_cloud_devices);

    fmt::print("flag4 set ip\n");

    // 为 router1 分配地址
    base = 100;
    Ipv4InterfaceContainer router1_interface[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip_base { "10.1." };
        ip_base.append(std::to_string(base++));
        ip_base.append(".0"); // 10.1.(100, 101....).0

        address.SetBase(ip_base.c_str(), "255.255.255.0");
        router1_interface[i] = address.Assign(router1_devices[i]);
    }

    fmt::print("flag5 set ip\n");

    // 为 router2 分配地址
    base = 150;
    Ipv4InterfaceContainer router2_interface[bs_size];
    for (i = 0; i < bs_size; ++i) {
        std::string ip_base { "10.1." };
        ip_base.append(std::to_string(base++));
        ip_base.append(".0"); // 10.1.(100, 101....).0

        address.SetBase(ip_base.c_str(), "255.255.255.0");
        router2_interface[i] = address.Assign(router2_devices[i]);
    }

    fmt::print("flag6 set ip\n");

    // For routers to be able to forward packets, they need to have routing rules.
    // Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    fmt::print("flag7\n");
}


} // namespace okec


#endif // OKEC_HPP