#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>

#include "format_helper.hpp"
#include "okec.hpp"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("MyEIP");

void ResourceMemoryTrace(double oldval, double newval)
{
    std::cout << "-->ResourceMemory changed: from " << oldval << " to " << newval << std::endl;
}

void ResourceCpuTrace(double oldval, double newval)
{
    std::cout << "-->ResourceCpu changed: from " << oldval << " to " << newval << std::endl;
}

void ResourceMemoryContextTrace(std::string context, int oldval, int newval)
{
    std::cout << context << "-->ResourceMemory changed: from " << oldval << " to " << newval << std::endl;
}

void ResourceCpuContextTrace(std::string context, int oldval, int newval)
{
    std::cout << context << "-->ResourceCpu changed: from " << oldval << " to " << newval << std::endl;
}


auto main(int argc, char **argv) -> int
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("MyEIP", LOG_LEVEL_INFO);

    fmt::print("C++ version: {}\n", __cplusplus);
    ////////////////////////////////////////////////////////////////

    LogComponentEnable("udp_application", LOG_LEVEL_INFO);

    okec::base_station_container base_stations(2);
    okec::cloud_server cs;
    okec::client_device_container client_devices(4);
    okec::edge_device_container edge_devices1(1);
    okec::edge_device_container edge_devices2(3);
    // bs->connect_device(edge_devices1);
    // base_stations[0]->connect_device(edge_devices1);
    // base_stations[1]->connect_device(edge_devices2);
    if (!base_stations.connect_device(edge_devices1, edge_devices2))
        return EXIT_FAILURE;
    // okec::initialize_communication(client_devices, *bs, cs);
    okec::initialize_communication(client_devices, base_stations, cs);

    // bs->link_cloud(cs);
    base_stations.link_cloud(cs);
    // cs.push_base_station(bs);
    cs.push_base_stations(&base_stations);
    // cs.push_base_station(base_stations[0]);
    // cs.push_base_station(base_stations[1]);

    okec::task_container t_container(2);
    t_container.random_initialization();
    t_container.print();

    // 配置资源
    okec::resource_container client_rcontainer(client_devices.size());
    okec::resource_container edge1_rcontainer(edge_devices1.size());
    okec::resource_container edge2_rcontainer(edge_devices2.size());
    client_rcontainer.random_initialization();
    client_rcontainer.print("Client Device Resources:");
    edge1_rcontainer.random_initialization();
    edge1_rcontainer.print("Edge Device 1 Resources:");
    edge2_rcontainer.random_initialization();
    edge2_rcontainer.print("Edge Device 2 Resources:");

    client_devices.install_resources(client_rcontainer); // 一键为所有用户设备配置资源
    edge_devices1.install_resources(edge1_rcontainer);   // 一键为所有边缘设备安装资源
    edge_devices2.install_resources(edge2_rcontainer);   // 一键为所有边缘设备安装资源

    // 发送任务
    auto device_0 = client_devices.get_device(0);
    device_0->send_tasks(base_stations[1], cs, t_container);
    

    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
}