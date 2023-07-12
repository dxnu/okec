#ifndef OKEC_HPP
#define OKEC_HPP

#include "base_station.h"
#include "client_device.h"
#include "resource.h"


namespace okec
{


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
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
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


} // namespace okec


#endif // OKEC_HPP