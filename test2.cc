#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "EdgeCommunicatorApplication.h"
#include "Task.h"
#include "Resource.h"
#include <iostream>

#include "format_helper.hpp"
#include "okec.hpp"


#include "Resource.h"


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
    okec::edge_device_container edge_devices(4);
    okec::client_device_container client_devices(4);
    bs.connect_device(edge_devices);
    // okec::initialize_communication(bs, clientDeviceContainer);
    okec::initialize_communication(client_devices, bs, cs);
    bs.link_cloud(cs);

    okec::task t;
    t.id("0001");
    t.budget(50);
    t.deadline(5);
    t.needed_cpu_cycles(150);
    t.needed_memory(200);
    t.priority(99);

    auto device_0 = client_devices.get_device(0);

    // 必须使用 CreateObject 创建，否则报错
    auto res = okec::make_resource();
    res->cpu_cycles(200);
    res->memory(250);
    res->price(999);
    device_0->install_resource(res);


    // client_devices.get_device(0)->send_task(bs, t);
    device_0->set_offload_model(
        [](const okec::task& t, const okec::client_device& client, const okec::cloud_server& cs) {
        // 可以本地执行
        if (client.free_cpu_cycles() > t.needed_cpu_cycles() &&
            client.free_memory() > t.needed_memory()) {
            return std::make_pair(client.get_address(), client.get_port());
        }

        // 本地无法执行，需要转发，则 cs 来完成实际转发目的
        return cs.offload_task(t);
    });
    device_0->send_task(bs, t);
    // device_0->handle_task(t, cs);
    


    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
}