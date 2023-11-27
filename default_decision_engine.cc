#include "default_decision_engine.h"
#include "format_helper.hpp"
#include "message.h"
#include "base_station.h"
#include "cloud_server.h"
#include "edge_device.h"
#include <functional> // std::bind_front



namespace okec
{

default_decision_engine::default_decision_engine(
    base_station_container* bs_container, cloud_server* cs)
{
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
    auto device_cache = this->device_cache();
    for (const auto& device : device_cache) {
        // 检查资源充足性
        if (TO_INT(device["cpu_cycle"]) < 1 || device["device_type"] == "cs")
            continue;

        double time = std::stod(header.get_header("cpu_cycle")) / TO_DOUBLE(device["cpu_cycle"]);
        print_info(fmt::format("Decision engine assesses whether edge device can process the task: {} ---- {} (tolorable time)", time, header.get_header("deadline")));
        if (time < std::stod(header.get_header("deadline"))) {
            Vector position(TO_DOUBLE(device["pos_x"]), TO_DOUBLE(device["pos_y"]), TO_DOUBLE(device["pos_z"]));
            auto distance = calculate_distance(position);
            return std::make_tuple(device["ip"], TO_INT(device["port"]), distance);
        }
    }

    // 发送到云服务器
    auto device = this->find_device_cache({ "device_type", "cs" });
    if (!device.is_null()) {
        Vector position(TO_DOUBLE(device["pos_x"]), TO_DOUBLE(device["pos_y"]), TO_DOUBLE(device["pos_z"]));
        auto distance = calculate_distance(position);
        return std::make_tuple(device["ip"], TO_INT(device["port"]), distance);
    }
    
    return result_t(); // 决策失败
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
    auto cpu_demand = task_item.get_header("cpu_cycle");
    auto task_id = task_item.get_header("task_id");

    fmt::print(fg(fmt::color::yellow), "At time {:.2f}s The Cloud server[{:ip}] is processing the task(task_id = {}).\n", 
        Simulator::Now().GetSeconds(), cs->get_address(), task_id);

    auto device_resource = cs->get_resource();
    double processing_time = std::stod(cpu_demand) / std::stod(device_resource->get_value("cpu_cycle"));
    

    // 执行任务
    Simulator::Schedule(ns3::Seconds(processing_time), +[](const ns3::Time& time, double processing_time, cloud_server* self, const std::string& task_id, const ns3::Ipv4Address& desination) {

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
            { "processing_time", fmt::format("{:.{}f}", processing_time, 4) }
        };
        fmt::print("response: {}\n", response.dump());
        self->write(response.to_packet(), desination, self->get_port());
    }, Simulator::Now(), processing_time, cs, task_id, inetRemoteAddress.GetIpv4());
}

auto default_decision_engine::on_es_handling_message(
    edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);
    auto task_item = task_element::from_msg_packet(packet);
    auto instructions = task_item.get_body("instructions");
    auto cpu_demand = task_item.get_header("cpu_cycle");
    auto task_id = task_item.get_header("task_id");

    fmt::print(fg(fmt::color::yellow), "At time {:.2f}s The edge server[{:ip}] is processing the task(task_id = {}).\n", 
        Simulator::Now().GetSeconds(), es->get_address(), task_id);

    auto device_resource = es->get_resource();
    double processing_time = std::stod(cpu_demand) / std::stod(device_resource->get_value("cpu_cycle"));


    // 通知资源更新情况
    auto old_cpu_cycles = device_resource->reset_value("cpu_cycle", "0"); // 标记资源已用完
    message notify_msg;
    notify_msg.type(message_resource_changed);
    notify_msg.attribute("ip", fmt::format("{:ip}", es->get_address()));
    notify_msg.attribute("port", std::to_string(es->get_port()));
    notify_msg.content(*device_resource);
    print_info(fmt::format("The edge server[{:ip}] sends resource changed notifications to decision-maker.", es->get_address()));
    es->write(notify_msg.to_packet(), inetRemoteAddress.GetIpv4(), es->get_port());
    

    // 执行任务
    Simulator::Schedule(ns3::Seconds(processing_time), +[](double processing_time, edge_device* es, const std::string& old_cpu_cycles, const std::string& task_id, const ns3::Ipv4Address& desination) {
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
            { "processing_time", fmt::format("{:.{}f}", processing_time, 4) }
        };
        es->write(response.to_packet(), desination, es->get_port());
    }, processing_time, es, old_cpu_cycles, task_id, inetRemoteAddress.GetIpv4());
}

} // namespace okec