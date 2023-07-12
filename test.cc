// #include "ns3/core-module.h"
// #include "ns3/network-module.h"
// #include "ns3/applications-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/csma-module.h"
// #include "ns3/point-to-point-module.h"
// #include "EdgeCommunicatorApplication.h"
// #include "Task.h"
// #include "Resource.h"
// #include <iostream>

// #include "FormattingOutput.h"
// #include "ClientDevice.h"



// using namespace ns3;


// NS_LOG_COMPONENT_DEFINE("MyEIP");

// void ResourceMemoryTrace(double oldval, double newval)
// {
//     std::cout << "-->ResourceMemory changed: from " << oldval << " to " << newval << std::endl;
// }

// void ResourceCpuTrace(double oldval, double newval)
// {
//     std::cout << "-->ResourceCpu changed: from " << oldval << " to " << newval << std::endl;
// }

// void ResourceMemoryContextTrace(std::string context, double oldval, double newval)
// {
//     std::cout << context << "-->ResourceMemory changed: from " << oldval << " to " << newval << std::endl;
// }

// void ResourceCpuContextTrace(std::string context, double oldval, double newval)
// {
//     std::cout << context << "-->ResourceCpu changed: from " << oldval << " to " << newval << std::endl;
// }


// int main(int argc, char **argv)
// {
//     CommandLine cmd;
//     cmd.Parse(argc, argv);

//     Time::SetResolution(Time::NS);
//     LogComponentEnable("MyEIP", LOG_LEVEL_INFO);

//     ////////////////////////////////////////////////////////////////
//     {
//         // simeg::ClientDevice<int> client1, client2;
//         // PointToPointHelper pointToPoint;
//         // pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
//         // pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
//         // NetDeviceContainer devices = pointToPoint.Install(client1, client2);
//         // InternetStackHelper stack;
//         // stack.Install(client1);
//         // stack.Install(client2);

//         // Ipv4AddressHelper address;
//         // address.SetBase("10.1.2.0", "255.255.255.0");
//         // Ipv4InterfaceContainer interfaces;
//         // interfaces = address.Assign(devices);
        
//         LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
//         LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

//         simeg::BaseStation bs;
//         simeg::ClientDeviceContainer<int> clientDeviceContainer(4); 
//         auto interfaces = clientDeviceContainer.ConnectWithBaseStationCSMA(bs, "10.1.2.0", "255.255.255.0");

//         // Task task;
//         // task.Build("00000001", 3.5, 5.5, 2.3);
//         // clientDeviceContainer[0].SendTask(task, bs);
//         Ipv4GlobalRoutingHelper::PopulateRoutingTables();
//         Packet::EnablePrinting();

//         NodeContainer nodes;
//         nodes.Add(bs);
//         nodes.Add(clientDeviceContainer[0]);
//         nodes.Add(clientDeviceContainer[1]);
//         nodes.Add(clientDeviceContainer[2]);
//         nodes.Add(clientDeviceContainer[3]);
//         UdpEchoServerHelper echoServer(9);
//         ApplicationContainer serverApps = echoServer.Install(bs);
//         serverApps.Start(Seconds(0));
//         serverApps.Stop(Seconds(10));
//         UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
//         echoClient.SetAttribute("MaxPackets", UintegerValue(50));
//         echoClient.SetAttribute("Interval", TimeValue(Seconds(0.5)));
//         echoClient.SetAttribute("PacketSize", UintegerValue(1024));
//         ApplicationContainer clientApps = echoClient.Install(nodes.Get(1));
//         clientApps.Start(Seconds(1.0));
//         clientApps.Stop(Seconds(10.0));

//     }

//     ////////////////////////////////////////////////////////////////
//     // NodeContainer nodes;
//     // nodes.Create(4);

//     // CsmaHelper csma;
//     // csma.SetChannelAttribute("DataRate", StringValue("1Gbps"));
//     // csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

//     // NetDeviceContainer csmaDevs;
//     // csmaDevs = csma.Install(nodes);
//     // csma.EnableAsciiAll("simple-udp");

//     // InternetStackHelper stack;
//     // stack.Install(nodes);

//     // Ipv4AddressHelper address;
//     // address.SetBase("10.1.1.0", "255.255.255.0");
//     // Ipv4InterfaceContainer interfaces;
//     // interfaces = address.Assign(csmaDevs);

//     // Ipv4GlobalRoutingHelper::PopulateRoutingTables();
//     // Packet::EnablePrinting();

//     // ResourceContainer resources(2);
//     // // 给Node1安装资源
//     // resources[0]->Build("HuaWei", 10, 2.7, 80);
//     // resources[0]->TraceConnect("ResourceMemory", to_string(interfaces.GetAddress(1)), MakeCallback(&ResourceMemoryContextTrace));
//     // resources[0]->TraceConnect("ResourceCpu", to_string(interfaces.GetAddress(1)), MakeCallback(&ResourceCpuContextTrace));
//     // resources[0]->Install(nodes.Get(1));
//     // // 给Node2安装资源
//     // resources[1]->Build("Tencent", 15, 5, 100);
//     // resources[1]->TraceConnect("ResourceMemory", to_string(interfaces.GetAddress(2)), MakeCallback(&ResourceMemoryContextTrace));
//     // resources[1]->TraceConnect("ResourceCpu", to_string(interfaces.GetAddress(2)), MakeCallback(&ResourceCpuContextTrace));
//     // resources[1]->Install(nodes.Get(2));

//     // // Bind EdgeCommunicator to Nodes.
//     // EdgeCommunicatorContainer communicators(3, &nodes);
//     // communicators.SetStartTime(Seconds(0));
//     // communicators.SetStopTime(Seconds(10));
//     // nodes.Get(0)->AddApplication(communicators[0]);
//     // nodes.Get(1)->AddApplication(communicators[1]);
//     // nodes.Get(2)->AddApplication(communicators[2]);
    
//     // // Node0给Node1发任务
//     // // !!只有程序运行起来了之后，才能使用communicator获取地址
//     // TaskContainer tasks(2);
//     // tasks[0].Build("00000001", 5, 0.24, 100);
//     // Simulator::Schedule(Seconds(1), &EdgeCommunicatorApplication::Write, 
//     //     communicators[0], tasks[0].GetPacket(), interfaces.GetAddress(1), communicators[1]->GetPort());

//     // tasks[1].Build("00000002", 10, 1.2, 120);
//     // Simulator::Schedule(Seconds(2), &EdgeCommunicatorApplication::Write, 
//     //     communicators[0], tasks[1].GetPacket(), interfaces.GetAddress(1), communicators[1]->GetPort());

//     // LogComponentEnable("EdgeCommunicatorApplication", LOG_LEVEL_INFO);

//     // Resource::PrintAllResources(nodes);

//     // Simulator::Stop(Seconds(10));
//     Simulator::Run();
//     Simulator::Destroy();

//     return 0;
// }