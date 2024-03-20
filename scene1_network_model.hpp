#ifndef OKEC_SCENE1_NETWORK_MODEL_HPP_
#define OKEC_SCENE1_NETWORK_MODEL_HPP_

#include "okec.hpp"

namespace okec
{

struct scene1_network_model {
    auto network_initializer(client_device_container& clients, base_station_container::pointer_t base_station, int& base) -> void {
        NodeContainer p2pNodes;
        NodeContainer edgeNodes;
        base_station->get_edge_nodes(edgeNodes);
        p2pNodes.Add(base_station->get_node()); // 基站
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

        address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        Ipv4InterfaceContainer p2pInterfaces;
        p2pInterfaces = address.Assign(p2pDevices);

        address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfaces;
        csmaInterfaces = address.Assign(csmaDevices);

        address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
        address.Assign(staDevices);
        address.Assign(apDevices);

        // Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    auto network_initializer2(std::vector<client_device_container>& clients, base_station_container& base_stations) -> void {
        int APs = base_stations.size();
        int base = 1;
        for (auto i : std::views::iota(0, APs)) {
            network_initializer(clients[i], base_stations[i], base);
        }

        NodeContainer p2pNodes[APs-1];
        for (auto const& indices : std::views::iota(0, APs) | std::views::slide(2)) {
            auto it = std::begin(indices);
            // fmt::print("[{} {}] ", *it, *std::next(it));
            p2pNodes[*it].Add(base_stations[*it]->get_node());
            p2pNodes[*it].Add(base_stations[*std::next(it)]->get_node());
        }


        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
        p2p.SetChannelAttribute("Delay", StringValue("2ms"));

        NetDeviceContainer p2pDevices[APs-1];
        for (auto i : std::views::iota(0, APs-1)) {
            p2pDevices[i] = p2p.Install(p2pNodes[i]);
        }

        Ipv4AddressHelper address;
        for (auto i : std::views::iota(0, APs-1)) {
            address.SetBase(fmt::format("10.1.{}.0", base++).c_str(), "255.255.255.0");
            address.Assign(p2pDevices[i]);
        }

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
};

template<> inline constexpr bool enable_network_model<scene1_network_model> = true;

} // namespace okec


#endif // OKEC_SCENE1_NETWORK_MODEL_HPP_