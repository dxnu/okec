#ifndef MULTIPLE_AND_SINGLE_LAN_WLAN_NETWORK_MODEL_HPP_
#define MULTIPLE_AND_SINGLE_LAN_WLAN_NETWORK_MODEL_HPP_

#include "network_model.hpp"


namespace okec
{

struct multiple_and_single_LAN_WLAN_network_model {
    auto network_initializer(
        client_device_container& clients,
        base_station_container::pointer_t base_station,
        Ipv4AddressHelper& address) -> void {
        
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

        // Initialize addresses.
        address.Assign(p2pDevices);
        address.NewNetwork();

        address.Assign(csmaDevices);
        address.NewNetwork();

        address.Assign(staDevices);
        address.Assign(apDevices);
        address.NewNetwork();
    }

    auto network_initializer(
        std::vector<client_device_container>& clients,
        base_station_container& base_stations) -> void {

        int APs = base_stations.size();
        if (APs != static_cast<int>(clients.size())) {
            fmt::print(fg(fmt::color::red), "Fatal error! (network_initializer) Client size does not match the BS size!\n");
            return;
        }


        Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");

        for (auto i : std::views::iota(0, APs)) {
            network_initializer(clients[i], base_stations[i], address);
        }

        NodeContainer p2pAPNodes[APs-1];
        for (auto const& indices : std::views::iota(0, APs) | std::views::slide(2)) {
            auto it = std::begin(indices);
            p2pAPNodes[*it].Add(base_stations[*it]->get_node());
            p2pAPNodes[*it].Add(base_stations[*std::next(it)]->get_node());
        }


        PointToPointHelper p2pAPHelper;
        p2pAPHelper.SetDeviceAttribute("DataRate", StringValue("50Mbps"));
        p2pAPHelper.SetChannelAttribute("Delay", StringValue("5ms"));

        NetDeviceContainer p2pAPDevices[APs-1];
        for (auto i : std::views::iota(0, APs-1)) {
            p2pAPDevices[i] = p2pAPHelper.Install(p2pAPNodes[i]);
        }

        for (auto i : std::views::iota(0, APs-1)) {
            address.Assign(p2pAPDevices[i]);
            address.NewNetwork();
        }

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
};

template<> inline constexpr bool enable_network_model<multiple_and_single_LAN_WLAN_network_model> = true;

} // namespace okec


#endif // MULTIPLE_AND_SINGLE_LAN_WLAN_NETWORK_MODEL_HPP_