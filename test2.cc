#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>

#include "format_helper.hpp"
#include "okec.hpp"
#include "message.h"
#include "decision_maker.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("MyEIP");

// void ResourceMemoryTrace(double oldval, double newval)
// {
//     std::cout << "-->ResourceMemory changed: from " << oldval << " to " << newval << std::endl;
// }

// void ResourceCpuTrace(double oldval, double newval)
// {
//     std::cout << "-->ResourceCpu changed: from " << oldval << " to " << newval << std::endl;
// }

// void ResourceMemoryContextTrace(std::string context, int oldval, int newval)
// {
//     std::cout << context << "-->ResourceMemory changed: from " << oldval << " to " << newval << std::endl;
// }

// void ResourceCpuContextTrace(std::string context, int oldval, int newval)
// {
//     std::cout << context << "-->ResourceCpu changed: from " << oldval << " to " << newval << std::endl;
// }

auto on_offloading_message(const okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    using namespace okec;

    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    fmt::print("bs[{:ip}] receives the offloading request from {:ip},", 
        bs->get_address(), inetRemoteAddress.GetIpv4());

    bool handled{};
    auto msg = message::from_packet(packet);
    auto t = msg.to_task();

    auto header = t->get_header();
    fmt::print("header: {} {} {} {} {} {} {}\n", header.budget, header.deadline, header.from_ip, header.from_port, header.id, header.group, header.priority);

    for (auto device : bs->get_edge_devices()) {
        if (device->free_cpu_cycles() > t->needed_cpu_cycles() &&
            device->free_memory() > t->needed_memory() &&
            device->price() <= t->budget()) {

            fmt::print(" dispatching it to {:ip} to handle the concrete tasks.\n", device->get_address());

            // 能够处理
            msg.type(message_handling);
            bs->write(msg.to_packet(), device->get_address(), device->get_port());
            handled = true;
            
            // 擦除分发记录
            bs->erase_dispatching_record(t->id());

            break;
        }
    }

    // 不能处理，消息需要再次转发
    if (!handled) {

        // 是否还未分发到过此基站
        auto non_dispatched_bs = [&t, &bs](std::shared_ptr<base_station> base) {
            return bs->dispatched(t->id(), fmt::format("{:ip}", base->get_address()));
        };

        // 标记当前基站已经处理过该任务，但没有处理成功
        bs->dispatching_record(t->id());


        // 搜索看其他基站是否还没有处理过该任务
        bs->detach(non_dispatched_bs, 
            [&bs, &packet](const ns3::Ipv4Address& address, uint16_t port) {
                fmt::print(" dispatching it to bs[{:ip}] bacause of lacking resource.\n", address);
            
                // 分发到其他基站处理
                bs->write(packet, address, port);
            },
            [&bs, &msg, &t](const ns3::Ipv4Address& address, uint16_t port) {
                fmt::print(" dispatching it to {:ip} bacause of lacking resource.\n", address);

                // 分发到云服务器处理
                msg.type(message_handling);
                bs->write(msg.to_packet(), address, port);

                // 擦除分发记录
                bs->erase_dispatching_record(t->id());
            });
        
    }
}


okec::decision_maker dmaker;

auto on_decision_message(okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    fmt::print("bs[{:ip}] receives the decision request from {:ip}\n", bs->get_address(), inetRemoteAddress.GetIpv4());

    namespace packet_helper =  okec::packet_helper;
    auto t = packet_helper::to_task(packet);
    bs->task_sequence(t); // save the task to task_sequence
    bs->make_decision(dmaker, t->get_header());
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
    if (!base_stations.connect_device(edge_devices1, edge_devices2))
        return EXIT_FAILURE;
    okec::initialize_communication(client_devices, base_stations, cs);

    base_stations.link_cloud(cs);
    // cs.push_base_stations(&base_stations);

    // 初始化决策设备信息
    dmaker.initialize_device(&base_stations, &cs);

    base_stations.set_request_handler(okec::message_decision, &on_decision_message);
    
    okec::task_container t_container(10);
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

    auto cloud_resource = okec::make_resource();
    cloud_resource->cpu_cycles(20000);
    cloud_resource->memory(50000);
    cs.install_resource(cloud_resource);


    // 发送任务
    auto device_0 = client_devices.get_device(0);
    device_0->send_tasks(base_stations[0], cs, t_container);
    

    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
}