#ifndef OKEC_SCENE2_NETWORK_MODEL_HPP_
#define OKEC_SCENE2_NETWORK_MODEL_HPP_

#include "okec.hpp"

namespace okec
{

struct scene2_network_model {
    auto network_initializer(client_device_container& ues, base_station_container& bss) -> void {
        // 第一步，获取用户设备、基站

        fmt::print("1\n");

        int bs_size = bss.size();
        NodeContainer edgeServers[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            bss[i]->get_edge_nodes(edgeServers[i]);
        }

        NodeContainer p2pNodes[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            p2pNodes[i].Add(bss[i]->get_node()); // 基站
            p2pNodes[i].Add(edgeServers[i].Get(0)); // 第一个边缘服务器
        }

        fmt::print("9\n");

        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
        p2p.SetChannelAttribute("Delay", StringValue("2ms"));
        
        NetDeviceContainer p2pDevices[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            p2pDevices[i] = p2p.Install(p2pNodes[i]);
        }


        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
        NetDeviceContainer csmaDevices[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            csmaDevices[i] = csma.Install(edgeServers[i]);
        }

        fmt::print("10\n");

        // 用户设备
        NodeContainer wifiStaNodes;
        ues.get_nodes(wifiStaNodes);
        NodeContainer wifiApNodes[bs_size];
        for (auto i : std::views::iota(0, bs_size)) {
            // wifiApNodes[i].Add(p2pNodes[i].Get(0));
            wifiApNodes[i].Add(bss[i]->get_node());
        }

        YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
        YansWifiPhyHelper phy;
        phy.SetChannel(channel.Create());

        WifiMacHelper mac;
        Ssid ssid = Ssid("ns-3-ssid");

        WifiHelper wifi;

        NetDeviceContainer staDevices;
        mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
        staDevices = wifi.Install(phy, mac, wifiStaNodes);

        fmt::print("11\n");

        NetDeviceContainer apDevices[bs_size];
        mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
        for (auto i : std::views::iota(0, bs_size)) { // 也许有问题
            apDevices[bs_size] = wifi.Install(phy, mac, wifiApNodes[i]);
        }

        fmt::print("12\n");

        MobilityHelper mobility;
        mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

        mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds",
                                RectangleValue(Rectangle(-50, 50, -50, 50)));
        mobility.Install(wifiStaNodes);

        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        for (auto i : std::views::iota(0, bs_size)) { // 也许有问题
            mobility.Install(wifiApNodes[i]);
        }

        fmt::print("2--wifiApNodes size: {}\n", wifiApNodes[0].GetN());
        fmt::print("2--wifiApNodes size: {}\n", wifiApNodes[1].GetN());


        InternetStackHelper stack;
        stack.Install(wifiStaNodes);
        for (auto i : std::views::iota(0, bs_size)) { // 也许有问题
            fmt::print("6\n");
            stack.Install(edgeServers[i]);
            fmt::print("7--wifiApNodes size: {}\n", wifiApNodes[0].GetN());
            fmt::print("7--wifiApNodes size: {}\n", wifiApNodes[1].GetN());
            // for (NodeContainer::Iterator iter = wifiApNodes[i].Begin(); iter != wifiApNodes[i].End(); ++iter)
            // {
            //     Ptr<Node> node = *iter;

            // }
            fmt::print("8\n");
            stack.Install(wifiApNodes[i]);
        }

        fmt::print("5\n");


        Ipv4AddressHelper address;

        address.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer p2pInterfaces[bs_size];
        for (auto i : std::views::iota(0, bs_size)) { // 也许有问题
            p2pInterfaces[i] = address.Assign(p2pDevices[i]);
        }

        fmt::print("4\n");

        address.SetBase("10.1.2.0", "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfaces[bs_size];
        for (auto i : std::views::iota(0, bs_size)) { // 也许有问题
            csmaInterfaces[bs_size] = address.Assign(csmaDevices[i]);
        }

        address.SetBase("10.1.3.0", "255.255.255.0");
        address.Assign(staDevices);
        for (auto i : std::views::iota(0, bs_size)) { // 也许有问题
            address.Assign(apDevices[i]);
        }

        fmt::print("3\n");


    }
};

template<> inline constexpr bool enable_network_model<scene2_network_model> = true;

} // namespace okec


#endif // OKEC_SCENE2_NETWORK_MODEL_HPP_