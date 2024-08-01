///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_MULTIPLE_LAN_WLAN_NETWORK_MODEL_HPP_
#define OKEC_MULTIPLE_LAN_WLAN_NETWORK_MODEL_HPP_

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
            log::error("Fatal error! (network_initializer) Client size does not match the BS size!");
            return;
        }

        // Create the P2P connection between AP and LAN
        ns3::NodeContainer p2pNodes[APs];
        ns3::NodeContainer edgeNodes[APs];
        for (auto i : std::views::iota(0, APs)) {
            base_stations[i]->get_edge_nodes(edgeNodes[i]);
            p2pNodes[i].Add(base_stations[i]->get_node());
            p2pNodes[i].Add(edgeNodes[i].Get(0));
        }

        ns3::PointToPointHelper p2pHelper;
        p2pHelper.SetDeviceAttribute("DataRate", ns3::StringValue("5Mbps"));
        p2pHelper.SetChannelAttribute("Delay", ns3::StringValue("2ms"));
        ns3::CsmaHelper csmaHelper;
        csmaHelper.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
        csmaHelper.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
        
        // Create multiple LAN
        ns3::NetDeviceContainer p2pDevices;
        ns3::NetDeviceContainer csmaDevices[APs]; // Each CSMA LAN must have a unique net device.
        for (auto i : std::views::iota(0, APs)) {
            p2pDevices.Add(p2pHelper.Install(p2pNodes[i]));
            csmaDevices[i] = csmaHelper.Install(edgeNodes[i]);
        }


        // Create multiple AP and STA
        ns3::NodeContainer wifiApNodes;
        ns3::NodeContainer wifiStaNodes[APs];
        for (auto i : std::views::iota(0, APs)) {
            wifiApNodes.Add(base_stations[i]->get_node());
            clients[i].get_nodes(wifiStaNodes[i]);
        }

        ns3::YansWifiChannelHelper wifiChannel = ns3::YansWifiChannelHelper::Default();
        ns3::YansWifiPhyHelper wifiPhy;
        ns3::WifiMacHelper wifiMac;
        ns3::WifiHelper wifiHelper;
        ns3::Ssid ssid;

        ns3::NetDeviceContainer apDevices;
        ns3::NetDeviceContainer staDevices[APs];
        for (auto i : std::views::iota(0, APs)) {
            ssid = ns3::Ssid("scene2-network-" + std::to_string(i));
            wifiPhy.SetChannel(wifiChannel.Create());
            
            // Assign SSID for each AP
            wifiMac.SetType("ns3::ApWifiMac", "Ssid", ns3::SsidValue(ssid));
            ns3::NetDeviceContainer apDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiApNodes.Get(i));
            apDevices.Add(apDevice);

            // Assign STA to connect to specific SSID
            wifiMac.SetType("ns3::StaWifiMac", "Ssid", ns3::SsidValue(ssid), "ActiveProbing", ns3::BooleanValue(false));
            ns3::NetDeviceContainer staDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiStaNodes[i]);
            staDevices[i].Add(staDevice);
        }

        ns3::MobilityHelper mobility;
        mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX",
                                    ns3::DoubleValue(0.0),
                                    "MinY",
                                    ns3::DoubleValue(0.0),
                                    "DeltaX",
                                    ns3::DoubleValue(5.0),
                                    "DeltaY",
                                    ns3::DoubleValue(10.0),
                                    "GridWidth",
                                    ns3::UintegerValue(3),
                                    "LayoutType",
                                    ns3::StringValue("RowFirst"));

        mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds",
                                ns3::RectangleValue(ns3::Rectangle(-50, 50, -50, 50)));
        for (auto i : std::views::iota(0, APs)) {
            mobility.Install(wifiStaNodes[i]);
        }
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(wifiApNodes);


        // Initialize the internet stack and assign addresses to every node.
        ns3::InternetStackHelper stack;
        stack.Install(wifiApNodes);
        for (auto i : std::views::iota(0, APs)) {
            stack.Install(wifiStaNodes[i]);
            stack.Install(edgeNodes[i]);
        }

        ns3::Ipv4AddressHelper address;
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
        ns3::NodeContainer p2pAPNodes[APs-1];
        for (auto const& indices : std::views::iota(0, APs) | std::views::slide(2)) {
            auto it = std::begin(indices);
            p2pAPNodes[*it].Add(base_stations[*it]->get_node());
            p2pAPNodes[*it].Add(base_stations[*std::next(it)]->get_node());
        }

        ns3::PointToPointHelper p2pAPHelper;
        p2pAPHelper.SetDeviceAttribute("DataRate", ns3::StringValue("50Mbps"));
        p2pAPHelper.SetChannelAttribute("Delay", ns3::StringValue("5ms"));

        ns3::NetDeviceContainer p2pAPDevices[APs-1];
        for (auto i : std::views::iota(0, APs-1)) {
            p2pAPDevices[i] = p2pAPHelper.Install(p2pAPNodes[i]);
        }

        for (auto i : std::views::iota(0, APs-1)) {
            address.Assign(p2pAPDevices[i]);
            address.NewNetwork();
        }

        // For routers to be able to forward packets, they need to have routing rules.
        ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

};


template<> inline constexpr bool enable_network_model<multiple_LAN_WLAN_network_model> = true;

} // namespace okec


#endif // OKEC_MULTIPLE_LAN_WLAN_NETWORK_MODEL_HPP_