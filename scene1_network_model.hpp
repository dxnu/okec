#ifndef OKEC_SCENE1_NETWORK_MODEL_HPP_
#define OKEC_SCENE1_NETWORK_MODEL_HPP_

#include "okec.hpp"

namespace okec
{

struct scene1_network_model {
    auto network_initializer(client_device_container& clients, base_station_container& base_stations) -> void {
        NodeContainer p2pNodes;
        NodeContainer edgeNodes;
        base_stations[0]->get_edge_nodes(edgeNodes);
        p2pNodes.Add(base_stations[0]->get_node()); // 基站
        p2pNodes.Add(edgeNodes.Get(0)); // 边缘节点

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
        pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

        NetDeviceContainer p2pDevices;
        p2pDevices = pointToPoint.Install(p2pNodes);

        
        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

        NetDeviceContainer csmaDevices;
        csmaDevices = csma.Install(edgeNodes);

        NodeContainer wifiStaNodes;
        clients.get_nodes(wifiStaNodes);
        NodeContainer wifiApNode = p2pNodes.Get(0);

        YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
        YansWifiPhyHelper phy;
        phy.SetChannel(channel.Create());

        WifiMacHelper mac;
        Ssid ssid = Ssid("ns-3-ssid");

        WifiHelper wifi;

        NetDeviceContainer staDevices;
        mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
        staDevices = wifi.Install(phy, mac, wifiStaNodes);

        NetDeviceContainer apDevices;
        mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
        apDevices = wifi.Install(phy, mac, wifiApNode);

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
        mobility.Install(wifiApNode);

        InternetStackHelper stack;
        stack.Install(edgeNodes);
        stack.Install(wifiApNode);
        stack.Install(wifiStaNodes);

        Ipv4AddressHelper address;

        address.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer p2pInterfaces;
        p2pInterfaces = address.Assign(p2pDevices);

        address.SetBase("10.1.2.0", "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfaces;
        csmaInterfaces = address.Assign(csmaDevices);

        address.SetBase("10.1.3.0", "255.255.255.0");
        address.Assign(staDevices);
        address.Assign(apDevices);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
};

template<> inline constexpr bool enable_network_model<scene1_network_model> = true;

} // namespace okec


#endif // OKEC_SCENE1_NETWORK_MODEL_HPP_