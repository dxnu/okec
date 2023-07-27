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

    okec::base_station bs;
    okec::cloud_server cs;
    okec::edge_device_container edge_devices(10);
    okec::client_device_container client_devices(4);
    bs.connect_device(edge_devices);
    // okec::initialize_communication(bs, clientDeviceContainer);
    okec::initialize_communication(client_devices, bs, cs);
    bs.link_cloud(cs);
    cs.push_base_station(bs);

    okec::task_container t_container(50);
    t_container.random_initialization();
    t_container.print();

    // 配置资源
    okec::resource_container client_rcontainer(client_devices.size());
    okec::resource_container edge_rcontainer(edge_devices.size());
    client_rcontainer.random_initialization();
    client_rcontainer.print("Client Device Resources:");
    edge_rcontainer.random_initialization();
    edge_rcontainer.print("Edge Device Resources:");

    client_devices.install_resources(client_rcontainer); // 一键为所有用户设备配置资源
    edge_devices.install_resources(edge_rcontainer);     // 一键为所有边缘设备安装资源

    // okec::resource_container r_container(edge_devices.size() + 1);
    // r_container.random_initialization();
    // r_container.print();

    // auto device_0 = client_devices.get_device(0);
    // device_0->install_resource(r_container[0]); // 对当前设备配置资源

    // edge_devices.install_resources(r_container, 1); // 一键为所有边缘设备安装资源
    // auto server_2 = edge_devices.get_device(2);
    // server_2->install_resource(r_container[1]);

    // 发送任务
    auto device_0 = client_devices.get_device(0);
    device_0->send_tasks(bs, cs, t_container);
    

    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
}