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

default_decision_engine::default_decision_engine(client_device_container* client_devices, 
    base_station_container* bs_container, cloud_server* cs)
{
    // 设置决策设备
    m_decision_device = bs_container->get(0);

    initialize_device(bs_container, cs);

    // Capture decision message
    bs_container->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    bs_container->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture cs handling message
    cs->set_request_handler(message_handling, std::bind_front(&this_type::on_cs_handling_message, this));

    // Capture es handling message
    bs_container->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture clients response message
    client_devices->set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
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
    client_device* client) -> bool
{
    big_float cpu_demand(header.get_header("cpu_cycle"));
    big_float cpu_supply(client->get_resource()->get_value("cpu_cycle"));

    if (cpu_supply < 1)
        return false;

    cpu_supply *= 1000000; // megahertz * 10^6 = cpu cycles
    big_float processing_time = cpu_demand / cpu_supply; // calculate processing time of device
    if (processing_time < big_float(header.get_header("deadline"))) {
        return true;
    }
    
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

auto default_decision_engine::on_bs_response_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    InetSocketAddress inetRemoteAddress = InetSocketAddress::ConvertFrom(remote_address);
    print_info(fmt::format("The base station([{:ip}]) received a response from {:ip}", bs->get_address(), inetRemoteAddress.GetIpv4()));

    message msg(packet);

    auto& task_sequence = bs->task_sequence();
    auto& task_sequence_status = bs->task_sequence_status();

    for (std::size_t i = 0; i < task_sequence.size(); ++i)
    {
        // 将处理结果转发回客户端
        if (task_sequence[i].get_header("task_id") == msg.get_value("task_id"))
        {
            msg.attribute("group", task_sequence[i].get_header("group"));
            msg.attribute("send_time", task_sequence[i].get_header("send_time"));
            auto from_ip = task_sequence[i].get_header("from_ip");
            auto from_port = task_sequence[i].get_header("from_port");
            bs->write(msg.to_packet(), ns3::Ipv4Address(from_ip.c_str()), std::stoi(from_port));

            // 清除任务队列和分发状态
            task_sequence.erase(std::next(task_sequence.begin(), i), std::next(task_sequence.begin(), i + 1));
            task_sequence_status.erase(std::next(task_sequence_status.begin(), i), std::next(task_sequence_status.begin(), i + 1));
            
            // 继续处理下一个任务的分发
            // this->handle_next_task(*this);
            break;
        }
    }
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

auto default_decision_engine::on_clients_reponse_message(
    client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void
{
    fmt::print(fg(fmt::color::red), "At time {:.2f}s Client device[{:ip}] receives", Simulator::Now().GetSeconds(), client->get_address(), client->get_port());

    message msg(packet);
    auto task_id = msg.get_value("task_id");
    auto device_type = msg.get_value("device_type");
    auto device_address = msg.get_value("device_address");
    auto group = msg.get_value("group");
    auto time_consuming = msg.get_value("processing_time");
    auto send_time = msg.get_value("send_time");

    fmt::print(fg(fmt::color::red), " response: task_id={}, device_type={}, device_address={}, group={}, processing_time={}, send_time={}\n",
        task_id, device_type, device_address, group, time_consuming, send_time);

    // client->m_response.set_if({
    //     { "group", group },
    //     { "task_id", task_id }
    // }, [&device_type, &device_address, &time_consuming, &send_time](auto& item) {
    //     item["device_type"] = device_type;
    //     item["device_address"] = device_address;
    //     item["time_consuming"] = time_consuming;
    //     item["send_time"] = send_time;
    //     item["finished"] = "1";
    // });

    auto it = client->response_cache().find_if([&group, &task_id](const response::value_type& item) {
        return item["group"] == group && item["task_id"] == task_id;
    });
    if (it != client->response_cache().end()) {
        (*it)["device_type"] = device_type;
        (*it)["device_address"] = device_address;
        (*it)["time_consuming"] = time_consuming;
        (*it)["send_time"] = send_time;
        (*it)["finished"] = "1";
    }


    // 显示当前任务进度条
    auto total = client->response_cache().count_if({ "group", group });
    auto finished = client->response_cache().count_if({{ "group", group }, { "finished", "1" }});
    fmt::print(fg(fmt::color::red), "Current task progress: {0:█^{1}}{0:▒^{2}} {3:.0f}%\n", "",
        finished, total - finished, (double)finished / total * 100);

    // 检查是否存在当前任务的信息
    if (!client->response_cache().find_if({ "group", group })) {
        fmt::print("未发现响应信息\n"); // 说明发过去的数据被修改，或是 m_response 被无意间删除了信息
        return;
    }


    auto partially_finished = client->response_cache().find_if({
        { "group", group },
        { "finished", "0" }
    });

    if (partially_finished) {
        // 部分完成
        // fmt::print("部分完成\n");
    } else {
        // 全部完成
        if (client->has_done_callback()) {
            client->done_callback(client->response_cache().dump_with({ "group", group }));
        }
    }
}

} // namespace okec