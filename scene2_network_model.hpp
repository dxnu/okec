#ifndef OKEC_SCENE2_NETWORK_MODEL_HPP_
#define OKEC_SCENE2_NETWORK_MODEL_HPP_

#include "okec.hpp"

namespace okec
{

struct scene2_network_model {
    auto network_initializer(std::vector<client_device_container>& clients, base_station_container& base_stations) -> void {
        int APs = base_stations.size();
        assert(APs == clients.size());

        // Create multiple AP nodes 
        NodeContainer wifiApNodes;
        for (auto i : std::views::iota(0, APs)) {
            wifiApNodes.Add(base_stations[i]->get_node());
        }

        // Create multiple STA
        NodeContainer wifiStaNodes[APs];
        for (auto i : std::views::iota(0, APs)) {
            clients[i].get_nodes(wifiStaNodes[i]);
        }

        // Configure propagation model
        WifiMacHelper wifiMac;
        WifiHelper wifiHelper;
        YansWifiPhyHelper wifiPhy;
        wifiHelper.SetStandard(WIFI_STANDARD_80211ac);
        wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("VhtMcs0"), "ControlMode", StringValue("VhtMcs0"), "MaxSlrc", UintegerValue(10));
        int channelWidth = 80;

        YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
        wifiPhy.SetChannel(wifiChannel.Create());
        wifiPhy.Set("TxPowerStart", DoubleValue(20.0));
        wifiPhy.Set("TxPowerEnd", DoubleValue(20.0));
        wifiPhy.Set("TxPowerLevels", UintegerValue(1));
        wifiPhy.Set("TxGain", DoubleValue(0));
        wifiPhy.Set("RxGain", DoubleValue(0));
        wifiPhy.Set("RxNoiseFigure", DoubleValue(7));
        wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");


        // Assign SSID for each AP
        NetDeviceContainer apDevices;
        Ssid ssid;
        for (auto i : std::views::iota(0, APs)) {
            ssid = Ssid("scene2-network-" + std::to_string(i));
            wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
            NetDeviceContainer apDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiApNodes.Get(i));
            apDevices.Add(apDevice);
        }

        // Assign STA to connect to specific SSID
        NetDeviceContainer staDevices[APs];
        for (auto i : std::views::iota(0, APs)) {
            ssid = Ssid("scene2-network-" + std::to_string(i));
            wifiMac.SetType ("ns3::StaWifiMac",	"Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
            NetDeviceContainer staDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiStaNodes[i]);
            staDevices[i].Add(staDevice);
        }

        // Install network stack
        InternetStackHelper stack;
        stack.Install(wifiApNodes);
        for (auto i : std::views::iota(0, APs)) {
            stack.Install(wifiStaNodes[i]);
        }

        Ipv4AddressHelper address;
        for (auto i : std::views::iota(0, APs)) {
            address.SetBase(fmt::format("10.1.{}.0", i).c_str(), "255.255.255.0");
            address.Assign(apDevices.Get(i));
            address.Assign(staDevices[i]);
        }
    }

auto network_initializer2(std::vector<client_device_container>& clients, base_station_container& base_stations) -> void {
    int APs = base_stations.size();
    assert(APs == clients.size());



    // CSMA
    NodeContainer p2pNodes[APs];
    NodeContainer edgeNodes[APs];
    for (auto i : std::views::iota(0, APs)) {
        base_stations[i]->get_edge_nodes(edgeNodes[i]);
        p2pNodes[i].Add(base_stations[i]->get_node());
        p2pNodes[i].Add(edgeNodes[i].Get(0));
    }

    PointToPointHelper p2pHelper;
    p2pHelper.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer p2pDevices;
    NetDeviceContainer csmaDevices;
    for (auto i : std::views::iota(0, APs)) {
        p2pDevices.Add(p2pHelper.Install(p2pNodes[i]));
        csmaDevices.Add(csmaHelper.Install(edgeNodes[i]));
    }




    // Create multiple AP nodes 
    NodeContainer wifiApNodes;
    for (auto i : std::views::iota(0, APs)) {
        wifiApNodes.Add(base_stations[i]->get_node());
    }

    // Create multiple STA
    NodeContainer wifiStaNodes[APs];
    for (auto i : std::views::iota(0, APs)) {
        clients[i].get_nodes(wifiStaNodes[i]);
    }

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhy;
    WifiMacHelper wifiMac;
    WifiHelper wifiHelper;
    Ssid ssid;

    NetDeviceContainer apDevices;
    NetDeviceContainer staDevices[APs];
    for (auto i : std::views::iota(0, APs)) {
        ssid = Ssid("scene2-network-" + std::to_string(i));
        wifiPhy.SetChannel(wifiChannel.Create());
        
        // Assign SSID for each AP
        wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
        NetDeviceContainer apDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiApNodes.Get(i));
        apDevices.Add(apDevice);

        // Assign STA to connect to specific SSID
        wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
        NetDeviceContainer staDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiStaNodes[i]);
        staDevices[i].Add(staDevice);
    }

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

    for (auto i : std::views::iota(0, APs)) {
        mobility.Install(wifiStaNodes[i]);
    }
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodes);







    //////////////

    InternetStackHelper stack;
    stack.Install(wifiApNodes);
    for (auto i : std::views::iota(0, APs)) {
        stack.Install(wifiStaNodes[i]);
        stack.Install(edgeNodes[i]); // new
    }

    Ipv4AddressHelper address;
    int base = 1;
    for (auto i : std::views::iota(0, APs)) {
        address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        address.Assign(p2pDevices.Get(i));
    }

    Ipv4InterfaceContainer csmaInterfaces;
    for (auto i : std::views::iota(0, APs)) {
        fmt::print("base: {}\n", base);
        address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        csmaInterfaces.Add(address.Assign(csmaDevices.Get(i)));
    }

    for (auto i : std::views::iota(0, APs)) {
        address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        address.Assign(apDevices.Get(i));
        address.Assign(staDevices[i]);
    }

    // Ipv4AddressHelper address;
    // for (auto i : std::views::iota(0, APs)) {
    //     address.SetBase(fmt::format("10.1.{}.0", i).c_str(), "255.255.255.0");
    //     address.Assign(apDevices.Get(i));
    //     address.Assign(staDevices[i]);
    // }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}
};

template<> inline constexpr bool enable_network_model<scene2_network_model> = true;

} // namespace okec


#endif // OKEC_SCENE2_NETWORK_MODEL_HPP_