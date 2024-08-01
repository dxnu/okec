///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/okecsim/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_CLOUD_EDGE_END_HPP_
#define OKEC_CLOUD_EDGE_END_HPP_

#include <okec/network/network_model.hpp>
#include <okec/utils/random.hpp>


namespace okec
{

// okec::cloud_edge_end_model model;
// okec::network_initializer(model, user_devices, base_stations, cloud_server);
struct cloud_edge_end_model {
    auto network_initializer(
        client_device_container& clients,
        base_station_container::pointer_t base_station,
        cloud_server& cloud,
        ns3::Ipv4AddressHelper& address,
        bool routing = true) -> void {
        
        ns3::NodeContainer p2pNodes;
        ns3::NodeContainer edgeNodes;
        base_station->get_edge_nodes(edgeNodes);
        p2pNodes.Add(base_station->get_node());
        p2pNodes.Add(edgeNodes.Get(0));

        ns3::PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute("DataRate", ns3::StringValue("5Mbps"));
        pointToPoint.SetChannelAttribute("Delay", ns3::StringValue("2ms"));

        ns3::NetDeviceContainer p2pDevices;
        p2pDevices = pointToPoint.Install(p2pNodes);

        ///////////////////////////////////
        // base station and cloud
        ns3::NodeContainer p2pCloudNodes;
        p2pCloudNodes.Add(cloud.get_node());
        p2pCloudNodes.Add(base_station->get_node());

        ns3::PointToPointHelper p2pCloudHelper;
        p2pCloudHelper.SetDeviceAttribute("DataRate", ns3::StringValue("50Mbps"));
        p2pCloudHelper.SetChannelAttribute("Delay", ns3::StringValue("5ms"));

        ns3::NetDeviceContainer p2pCloudDevice;
        p2pCloudDevice = p2pCloudHelper.Install(p2pCloudNodes);

        //////////////////////////////////        

        
        ns3::CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", ns3::StringValue("100Mbps"));
        csma.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));

        ns3::NetDeviceContainer csmaDevices;
        csmaDevices = csma.Install(edgeNodes);

        ns3::NodeContainer wifiStaNodes;
        clients.get_nodes(wifiStaNodes);
        ns3::NodeContainer wifiApNode = p2pNodes.Get(0);

        ns3::YansWifiChannelHelper channel = ns3::YansWifiChannelHelper::Default();
        ns3::YansWifiPhyHelper phy;
        phy.SetChannel(channel.Create());

        ns3::WifiMacHelper mac;
        ns3::Ssid ssid = ns3::Ssid("ns-3-ssid");

        ns3::WifiHelper wifi;

        ns3::NetDeviceContainer staDevices;
        mac.SetType("ns3::StaWifiMac", "Ssid", ns3::SsidValue(ssid), "ActiveProbing", ns3::BooleanValue(false));
        staDevices = wifi.Install(phy, mac, wifiStaNodes);

        ns3::NetDeviceContainer apDevices;
        mac.SetType("ns3::ApWifiMac", "Ssid", ns3::SsidValue(ssid));
        apDevices = wifi.Install(phy, mac, wifiApNode);
        ////////////////////////////////////////
        ns3::Mac48Address bssid("00:11:22:33:44:55"); // 设置BSSID
        ns3::Ptr<ns3::WifiNetDevice> wifiDevice = ns3::DynamicCast<ns3::WifiNetDevice>(apDevices.Get(0));
        uint8_t linkId = 0;
        wifiDevice->GetMac()->SetBssid(bssid, linkId);
        ////////////////////////////////////////

        // ns3::MobilityHelper mobility;

        // mobility.SetPositionAllocator("ns3::GridPositionAllocator",
        //                             "MinX",
        //                             ns3::DoubleValue(0.0),
        //                             "MinY",
        //                             ns3::DoubleValue(0.0),
        //                             "DeltaX",
        //                             ns3::DoubleValue(5.0),
        //                             "DeltaY",
        //                             ns3::DoubleValue(10.0),
        //                             "GridWidth",
        //                             ns3::UintegerValue(100),
        //                             "LayoutType",
        //                             ns3::StringValue("RowFirst"));

        // // 设置节点移动速度范围
        // ns3::Ptr<ns3::UniformRandomVariable> speedVar = ns3::CreateObject<ns3::UniformRandomVariable>();
        // speedVar->SetAttribute("Min", ns3::DoubleValue(5.0));
        // speedVar->SetAttribute("Max", ns3::DoubleValue(10.0));

        // mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
        //                         "Bounds",
        //                         ns3::RectangleValue(ns3::Rectangle(-100, 100, -100, 100)),
        //                         "Speed",
        //                         ns3::PointerValue(speedVar));
        // mobility.Install(wifiStaNodes);

        // // 随机设置每个用户设备的位置
        // for (auto client = clients.begin(); client != clients.end(); ++client) {
        //     (*client)->set_position(rand_range(5.0, 10.0), rand_range(10.0, 20.0), .0);
        // }

        // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        // mobility.Install(wifiApNode);

        ns3::InternetStackHelper stack;
        stack.Install(edgeNodes);
        stack.Install(wifiApNode);
        stack.Install(wifiStaNodes);
        stack.Install(cloud.get_node());

        // Initialize addresses.
        address.Assign(p2pDevices);
        address.NewNetwork();

        address.Assign(csmaDevices);
        address.NewNetwork();

        address.Assign(staDevices);
        address.Assign(apDevices);
        address.NewNetwork();

        address.Assign(p2pCloudDevice);
        address.NewNetwork();

        if (routing)
            ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    // Set up scenarios with a single base station serving clients from the same network segments
    auto network_initializer(
        client_device_container& clients,
        base_station_container::pointer_t base_station,
        cloud_server& cloud) -> void {

        ns3::Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");
        network_initializer(clients, base_station, cloud, address);
    }

    // Set up scenarios with multiple base stations serving clients from different network segments
    auto network_initializer(
        std::vector<client_device_container>& clients,
        base_station_container& base_stations,
        cloud_server& cloud) -> void {

        int APs = base_stations.size();
        if (APs != static_cast<int>(clients.size())) {
            log::error("Fatal error! (network_initializer) Client size does not match the BS size!");
            return;
        }


        ns3::Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");

        for (auto i : std::views::iota(0, APs)) {
            network_initializer(clients[i], base_stations[i], cloud, address, false);
        }

        // Connect all base stations
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

        ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
};


template<> inline constexpr bool enable_network_model<cloud_edge_end_model> = true;

} // namespace okec

#endif // OKEC_CLOUD_EDGE_END_HPP_