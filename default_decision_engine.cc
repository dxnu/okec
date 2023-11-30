#include "default_decision_engine.h"
#include "format_helper.hpp"
#include "message.h"
#include "base_station.h"
#include "client_device.h"
#include "cloud_server.h"
#include "edge_device.h"
#include <functional> // std::bind_front




namespace okec
{

default_decision_engine::default_decision_engine(
    base_station_container* bs_container, cloud_server* cs)
{
    // 设置决策设备
    m_decision_device = bs_container->get(0);

    initialize_device(bs_container, cs);

    // Capture decision message
    bs_container->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));

    // Capture cs handling message
    cs->set_request_handler(message_handling, std::bind_front(&this_type::on_cs_handling_message, this));

    // Capture es handling message
    bs_container->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));
}


auto default_decision_engine::make_decision(const task_element& header) -> result_t
{
    auto comp = [](const device_cache::value_type& lhs, const device_cache::value_type& rhs) -> bool {
        return TO_INT(lhs["cpu_cycle"]) < TO_INT(rhs["cpu_cycle"]);
    };

    // 获取边缘设备数据
    auto edge_cache = this->cache().data();
    edge_cache.erase(std::remove_if(edge_cache.begin(), edge_cache.end(), [](const device_cache::value_type& item) {
        return item["device_type"] == "cs";
    }));

    // 寻找资源最多的边缘设备
    auto device_max = *std::max_element(edge_cache.begin(), edge_cache.end(), comp);

    big_float cpu_demand(header.get_header("cpu_cycle"));
    big_float cpu_supply(device_max["cpu_cycle"].template get<std::string>());

    // If the device is avaliable.
    if (cpu_supply > 0) {
        cpu_supply *= 1000000; // megahertz * 10^6 = cpu cycles
        big_float processing_time = cpu_demand / cpu_supply; // calculate processing time of device
        print_info(fmt::format("Decision engine assesses whether edge device can process the task: {} ---- {} (tolorable time)", 
            processing_time.str(9, std::ios::fixed), header.get_header("deadline")));
        // 能够满足时延要求
        if (processing_time < big_float(header.get_header("deadline"))) {
            Vector position(TO_DOUBLE(device_max["pos_x"]), TO_DOUBLE(device_max["pos_y"]), TO_DOUBLE(device_max["pos_z"]));
            return std::make_tuple(device_max["ip"], TO_INT(device_max["port"]), calculate_distance(position));
        }
    }

    // Otherwise, dispatch the task to cloud.
    auto it = this->cache().find_if([](const device_cache::value_type& item) {
        return item["device_type"] == "cs";
    });
    if (it != this->cache().end()) {
        const auto& device = *it;
        Vector position(TO_DOUBLE(device["pos_x"]), TO_DOUBLE(device["pos_y"]), TO_DOUBLE(device["pos_z"]));
        auto distance = calculate_distance(position);
        return std::make_tuple(device["ip"], TO_INT(device["port"]), distance);
    }

    return result_t(); // 决策失败
}

auto default_decision_engine::local_test(const task_element& header,
    const client_device* client) -> bool
{
    return false;
}

auto default_decision_engine::on_bs_decision_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}\n", bs->get_address(), inetRemoteAddress.GetIpv4()));

    // 将任务(task 为单位)装入任务列表
    // auto t = okec::task::from_msg_packet(packet);
    // for (const auto& item : t.elements()) {
    //     bs->task_sequence(item);
    // }

    // task_element 为单位
    auto item = okec::task_element::from_msg_packet(packet);
    bs->task_sequence(std::move(item));

    // bs->print_task_info();
    bs->handle_next_task();
}

auto default_decision_engine::on_cs_handling_message(
    cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);

    auto task_item = task_element::from_msg_packet(packet);
    auto instructions = task_item.get_body("instructions");
    auto task_id = task_item.get_header("task_id");
    big_float cpu_demand(task_item.get_header("cpu_cycle"));

    fmt::print(fg(fmt::color::yellow), "At time {:.2f}s The Cloud server[{:ip}] is processing the task(task_id = {}).\n", 
        Simulator::Now().GetSeconds(), cs->get_address(), task_id);

    auto device_resource = cs->get_resource();
    big_float cpu_supply(device_resource->get_value("cpu_cycle"));
    cpu_supply *= 1000000; // megahertz * 10^6 = cpu cycles
    big_float processing_time = cpu_demand / cpu_supply;
    
    double time = processing_time.convert_to<double>();

    // 执行任务
    Simulator::Schedule(ns3::Seconds(time), +[](const ns3::Time& time, double processing_time, cloud_server* self, const std::string& task_id, const ns3::Ipv4Address& desination) {

        // 服务器资源不改变，发送过去是为了通知执行下一个任务
        auto device_address = fmt::format("{:ip}", self->get_address());
        message notify_msg;
        notify_msg.type(message_resource_changed);
        notify_msg.attribute("ip", device_address);
        notify_msg.attribute("port", std::to_string(self->get_port()));
        notify_msg.content(*self->get_resource());
        self->write(notify_msg.to_packet(), desination, self->get_port());

        // 返回消息
        message response {
            { "msgtype", "response" },
            { "task_id", task_id },
            { "device_type", "cs" },
            { "device_address", device_address },
            { "processing_time", fmt::format("{:.{}f}", processing_time, 9) }
        };
        fmt::print("response: {}\n", response.dump());
        self->write(response.to_packet(), desination, self->get_port());
    }, Simulator::Now(), time, cs, task_id, inetRemoteAddress.GetIpv4());
}

auto default_decision_engine::on_es_handling_message(
    edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);
    auto task_item = task_element::from_msg_packet(packet);
    auto instructions = task_item.get_body("instructions");
    auto task_id = task_item.get_header("task_id");
    big_float cpu_demand(task_item.get_header("cpu_cycle"));

    auto device_resource = es->get_resource();
    big_float cpu_supply(device_resource->get_value("cpu_cycle"));
    cpu_supply *= 1000000; // megahertz * 10^6 = cpu cycles
    big_float processing_time = cpu_demand / cpu_supply;

    fmt::print(fg(fmt::color::yellow), "At time {:.2f}s The edge server[{:ip}] is processing the task(task_id = {}).\n", 
        Simulator::Now().GetSeconds(), es->get_address(), task_id);


    // 通知资源更新情况
    auto old_cpu_cycles = device_resource->reset_value("cpu_cycle", "0"); // 标记资源已用完
    message notify_msg;
    notify_msg.type(message_resource_changed);
    notify_msg.attribute("ip", fmt::format("{:ip}", es->get_address()));
    notify_msg.attribute("port", std::to_string(es->get_port()));
    notify_msg.content(*device_resource);
    print_info(fmt::format("The edge server[{:ip}] sends resource changed notifications to decision engine.", es->get_address()));
    es->write(notify_msg.to_packet(), inetRemoteAddress.GetIpv4(), es->get_port());
    
    double time = processing_time.convert_to<double>();

    // 执行任务
    Simulator::Schedule(ns3::Seconds(time), +[](double processing_time, edge_device* es, const std::string& old_cpu_cycles, const std::string& task_id, const ns3::Ipv4Address& desination) {
        // 释放资源
        auto device_resource = es->get_resource();
        device_resource->reset_value("cpu_cycle", old_cpu_cycles);
        auto device_address = fmt::format("{:ip}", es->get_address());
        
        message notify_msg;
        notify_msg.type(message_resource_changed);
        notify_msg.attribute("ip", fmt::format("{:ip}", es->get_address()));
        notify_msg.attribute("port", std::to_string(es->get_port()));
        notify_msg.content(*device_resource);
        print_info(fmt::format("The edge server[{}] sends resource changed notifications to decision engine.", device_address));
        es->write(notify_msg.to_packet(), desination, es->get_port());

        // 返回消息
        message response {
            { "msgtype", "response" },
            { "task_id", task_id },
            { "device_type", "es" },
            { "device_address", device_address },
            { "processing_time", fmt::format("{:.{}f}", processing_time, 9) }
        };
        es->write(response.to_packet(), desination, es->get_port());
    }, time, es, old_cpu_cycles, task_id, inetRemoteAddress.GetIpv4());
}

} // namespace okec