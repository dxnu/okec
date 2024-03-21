#ifndef MULTIPLE_LAN_WLAN_NETWORK_MODEL_HPP_
#define MULTIPLE_LAN_WLAN_NETWORK_MODEL_HPP_


#include "network_model.hpp"

namespace okec
{

/**
 * @brief A network model involving multiple base stations, clients, and edge servers.
 * 
*/
struct multiple_LAN_WLAN_network_model {

    /**
     * @brief A custom function to initialize a scenario with multiple LANs and multiple WLANs.
     * 
     * @param clients Mutiple client devices.
     * @param base_stations Mutiple base stations.
     * 
     * @note This function assumes that clients and base_stations have the same size.
    */
    auto network_initializer(
        std::vector<client_device_container>& clients,
        base_station_container& base_stations) -> void {
        int APs = base_stations.size();
        if (APs != static_cast<int>(clients.size())) {
            fmt::print(fg(fmt::color::red), "Fatal error! (network_initializer) Client size does not match the BS size!\n");
            return;
        }

        // Create the P2P connection between AP and LAN
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
        
        // Create multiple LAN
        NetDeviceContainer p2pDevices;
        NetDeviceContainer csmaDevices[APs]; // Each CSMA LAN must have a unique net device.
        for (auto i : std::views::iota(0, APs)) {
            p2pDevices.Add(p2pHelper.Install(p2pNodes[i]));
            csmaDevices[i] = csmaHelper.Install(edgeNodes[i]);
        }


        // Create multiple AP and STA
        NodeContainer wifiApNodes;
        NodeContainer wifiStaNodes[APs];
        for (auto i : std::views::iota(0, APs)) {
            wifiApNodes.Add(base_stations[i]->get_node());
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


        // Initialize the internet stack and assign addresses to every node.
        InternetStackHelper stack;
        stack.Install(wifiApNodes);
        for (auto i : std::views::iota(0, APs)) {
            stack.Install(wifiStaNodes[i]);
            stack.Install(edgeNodes[i]);
        }

        Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");
        for (auto i : std::views::iota(0, APs)) {
            address.Assign(p2pDevices.Get(i));
            address.NewNetwork();
        }

        for (auto i : std::views::iota(0, APs)) {
            address.Assign(csmaDevices[i]);
            address.NewNetwork();
        }

        for (auto i : std::views::iota(0, APs)) {
            address.Assign(apDevices.Get(i));
            address.Assign(staDevices[i]);
            address.NewNetwork();
        }

        // Connect base stations
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

        // For routers to be able to forward packets, they need to have routing rules.
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

};


template<> inline constexpr bool enable_network_model<multiple_LAN_WLAN_network_model> = true;

} // namespace okec


#endif // MULTIPLE_LAN_WLAN_NETWORK_MODEL_HPP_