///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/algorithms/classic/worst_fit_decision_engine.h>
#include <okec/common/message.h>
#include <okec/devices/base_station.h>
#include <okec/devices/client_device.h>
#include <okec/devices/edge_device.h>
#include <okec/utils/log.h>
#include <functional> // bind_front

namespace okec {

worst_fit_decision_engine::worst_fit_decision_engine(
    client_device_container* clients,
    base_station_container* base_stations)
    : clients_{clients}
    , base_stations_{base_stations}
{
    // 设置决策设备
    m_decision_device = base_stations->get(0);

    // 初始化资源缓存信息
    this->initialize_device(base_stations);

    // Capture decision message
    base_stations->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    base_stations->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture es handling message
    base_stations->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture clients response message
    clients->set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
}

worst_fit_decision_engine::worst_fit_decision_engine(
    std::vector<client_device_container>* clients_container,
    base_station_container* base_stations)
    : clients_container_{clients_container}
    , base_stations_{base_stations}
{
    // 设置决策设备
    m_decision_device = base_stations->get(0);

    // 初始化资源缓存信息
    this->initialize_device(base_stations);

    // Capture decision message
    base_stations->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    base_stations->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture es handling message
    base_stations->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture clients response message
    for (auto& clients : *clients_container) {
        clients.set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
    }

    fmt::print("{}\n", this->cache().dump(4));
}

auto worst_fit_decision_engine::make_decision(const task_element& header) -> result_t
{
    auto edge_cache = this->cache().data();
    auto edge_max = *std::max_element(edge_cache.begin(), edge_cache.end(), 
        [](const device_cache::value_type& lhs, const device_cache::value_type& rhs) {
            return TO_DOUBLE(lhs["cpu"]) < TO_DOUBLE(rhs["cpu"]);
        });

    // fmt::print("edge max: {}\n", TO_STR(edge_max["ip"]));
    
    double cpu_demand = std::stod(header.get_header("cpu"));
    double cpu_supply = TO_DOUBLE(edge_max["cpu"]);
    // double tolorable_time = std::stod(header.get_header("deadline"));
    // If found a avaliable edge server
    if (cpu_supply >= cpu_demand) {
        // double processing_time = cpu_demand / cpu_supply;

        // 暂时不考虑 tolerable time
        // if (processing_time < tolorable_time) {
        //     return {
        //         { "ip", edge_max["ip"] },
        //         { "port", edge_max["port"] },
        //         { "cpu_supply", std::to_string(cpu_supply) }
        //     };
        // }
        return {
            { "ip", edge_max["ip"] },
            { "port", edge_max["port"] },
            { "cpu_supply", std::to_string(cpu_supply) }
        };
    }

    return result_t();
}

auto worst_fit_decision_engine::local_test(const task_element& header, client_device* client) -> bool
{
    return false;
}

auto worst_fit_decision_engine::send(task_element t, std::shared_ptr<client_device> client) -> bool
{
    static double launch_delay = 0.3;

    client->response_cache().emplace_back({
        { "task_id", t.get_header("task_id") },
        { "group", t.get_header("group") },
        { "finished", "0" }, // 0: unfinished, Y: finished, N: offloading failure
        { "device_type", "" },
        { "device_address", "" },
        { "time_consuming", "" }
    });

    // fmt::print("Received tasks:\n{}\n", t.j_data().dump(4));

    // 不管本地，全部往边缘服务器卸载
    t.set_header("from_ip", fmt::format("{:ip}", client->get_address()));
    t.set_header("from_port", std::to_string(client->get_port()));
    message msg;
    msg.type(message_decision);
    msg.content(t);
    const auto bs = this->get_decision_device();
    auto write = [client, bs, content = msg.to_packet()]() {
        client->write(content, bs->get_address(), bs->get_port());
    };
    ns3::Simulator::Schedule(ns3::Seconds(launch_delay), write);
    launch_delay += 0.01;

    return true;
}

auto worst_fit_decision_engine::initialize() -> void
{
    if (clients_) {
        clients_->set_decision_engine(shared_from_base<this_type>());
    }

    if (clients_container_) {
        for (auto& clients : *clients_container_) {
            clients.set_decision_engine(shared_from_base<this_type>());
        }
    }

    if (base_stations_) {
        base_stations_->set_decision_engine(shared_from_base<this_type>());
    }
}

auto worst_fit_decision_engine::handle_next() -> void
{

    auto& task_sequence = m_decision_device->task_sequence();
    // auto& task_sequence_status = m_decision_device->task_sequence_status();
    log::info("handle_next.... current task sequence size: {}", task_sequence.size());

    if (auto it = std::ranges::find_if(task_sequence, [](auto const& item) {
        return item.get_header("status") == "0";
    }); it != std::end(task_sequence)) {
        auto target = make_decision(*it);
        // 决策失败，无法处理任务
        if (target.is_null()) {
            log::info("No device can handle the task({})!", it->get_header("task_id"));

            // message response {
            //     { "msgtype", "response" },
            //     { "task_id", (*it).get_header("task_id") },
            //     { "group", (*it).get_header("group") },
            //     { "device_type", "null" },
            //     { "device_address", "null" },
            //     { "processing_time", "null" }
            // };
            // auto from_ip = (*it).get_header("from_ip");
            // auto from_port = (*it).get_header("from_port");
            // m_decision_device->write(response.to_packet(), Ipv4Address(from_ip.c_str()), std::stoi(from_port));
            // task_sequence.erase(it); // 处理过的任务从队列中删除，分发失败也是处理过的，只是当前没有设备能够处理该任务
            // this->handle_next(); // 继续尝试处理下一个
            
            // 等个两秒再尝试
            // Simulator::Schedule(Seconds(2), +[](this_type* self) {
            //     self->handle_next();
            // }, this);
            // 等待资源释放后自动重新尝试
            return;
        }

        // 决策成功，可以处理任务
        message msg;
        msg.type(message_handling);
        msg.content(*it);
        msg.attribute("cpu_supply", TO_STR(target["cpu_supply"]));
        it->set_header("status", "1"); // 更改任务分发状态
        m_decision_device->write(msg.to_packet(), ns3::Ipv4Address(TO_STR(target["ip"]).c_str()), TO_INT(target["port"]));
    }
    
    // 分发任务列表，每次拿出第一个未分发的任务
    // for (std::size_t i = 0; i < task_sequence.size(); ++i)
    // {
    //     // 任务尚未分发
    //     if (!static_cast<int>(task_sequence_status[i]))
    //     {
    //         auto target = make_decision(task_sequence[i]);
    //         if (target.is_null()) {
    //             print_info(fmt::format("No device can handle the task({})!", task_sequence[i].get_header("task_id")));
    //             message response {
    //                 { "msgtype", "response" },
    //                 { "task_id", task_sequence[i].get_header("task_id") },
    //                 { "group", task_sequence[i].get_header("group") },
    //                 { "device_type", "null" },
    //                 { "device_address", "null" },
    //                 { "processing_time", "null" }
    //             };
    //             auto from_ip = task_sequence[i].get_header("from_ip");
    //             auto from_port = task_sequence[i].get_header("from_port");
    //             m_decision_device->write(response.to_packet(), Ipv4Address(from_ip.c_str()), std::stoi(from_port));
    //             // 分发失败也要进行清除操作
    //             task_sequence.erase(std::next(task_sequence.begin(), i), std::next(task_sequence.begin(), i + 1));
    //             task_sequence_status.erase(std::next(task_sequence_status.begin(), i), std::next(task_sequence_status.begin(), i + 1));
    //             continue; // 继续尝试处理下一个
    //         }

    //         // print_info(fmt::format("Decision(task id:{}) is done. (target ip: {}, target port: {}.)", 
    //         //     task_sequence[i].get_header("task_id"), TO_STR(target["ip"]), TO_INT(target["port"])));
    //         // 计算并记录传输时间
    //         Ptr<NetDevice> device = m_decision_device->get_node()->GetDevice(0);
    //         Ptr<Channel> channel = device->GetChannel();
    //         if (channel) {
    //             StringValue band_width;
    //             channel->GetAttribute("Delay", band_width);

    //             DataRateValue dataRateValue;
    //             device->GetAttribute("DataRate", dataRateValue);
    //             DataRate dataRate = dataRateValue.Get();

    //             // fmt::print("DataRate: {}Mbps, delay: {}ms\n", dataRate.GetBitRate() / 1000000, Time(band_width.Get()).GetMilliSeconds());
    //         }

    //         message msg;
    //         msg.type(message_handling);
    //         msg.content(task_sequence[i]);
    //         msg.attribute("cpu_supply", TO_STR(target["cpu_supply"]));
    //         // print_info(fmt::format("bs({:ip}) dispatchs the task(task_id = {}) to {}",
    //         //     m_decision_device->get_address(), task_sequence[i].get_header("task_id"), TO_STR(target["ip"])));
    //         m_decision_device->write(msg.to_packet(), ns3::Ipv4Address(TO_STR(target["ip"]).c_str()), TO_INT(target["port"]));
            
    //         task_sequence_status[i] = 1; // // 更改任务分发状态
    //         task_sequence[i].set_header("status", "1");
    //         break; // 若是继续处理下一个，处理任务的边缘设备资源变化信息可能还未收到，也许会导致错误决策
    //     }
    // }
}

auto worst_fit_decision_engine::on_bs_decision_message(
    base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    // static bool first_time = true;
    // auto ipv4_remote = InetSocketAddress::ConvertFrom(remote_address).GetIpv4();
    // print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}", bs->get_address(), ipv4_remote));

    // fmt::print("Resource cache:\n{}\n", this->cache().dump(4));

    // task_element 为单位
    auto item = okec::task_element::from_msg_packet(packet);
    item.set_header("status", "0"); // 增加处理状态信息 0: 未处理 1: 已处理
    bs->task_sequence(std::move(item));
    

    // !!!
    // 接收到所有任务再统一处理，可避免 handle_next 时任务列表为空的问题（因为网络还没收到下一个任务，下一个任务到达时刻在资源变化之后）
    // 如果 handle_next 时任务列表为空，执行流程将被打断
    // 但是也有一个问题，如果资源恢复的数量还是不足以处理当前任务，那么执行流程也会被打断，不过资源迟早会全部释放，想来不是问题
    // if (first_time/* && bs->task_sequence().size() >= 50*/) {
    //     this->handle_next();
    //     first_time = false;
    // }
    this->handle_next();
}

auto worst_fit_decision_engine::on_bs_response_message(
    base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    // auto ipv4_remote = ns3::InetSocketAddress::ConvertFrom(remote_address).GetIpv4();
    // log::success("bs({:ip}) has received a response from {:ip}", bs->get_address(), ipv4_remote);

    message msg(packet);
    // fmt::print("{}\n", msg.dump());
    auto& task_sequence = bs->task_sequence();
    // auto& task_sequence_status = bs->task_sequence_status();

    if (auto it = std::ranges::find_if(task_sequence, [&msg](auto const& item) {
        return item.get_header("task_id") == msg.get_value("task_id");
    }); it != std::end(task_sequence)) {
        msg.attribute("group", (*it).get_header("group"));
        auto from_ip = (*it).get_header("from_ip");
        auto from_port = (*it).get_header("from_port");
        bs->write(msg.to_packet(), ns3::Ipv4Address(from_ip.c_str()), std::stoi(from_port));

        // 处理过的任务从队列中清除
        task_sequence.erase(it);
    }

    // this->handle_next();

    // for (std::size_t i = 0; i < task_sequence.size(); ++i)
    // {
    //     // 将处理结果转发回客户端
    //     if (task_sequence[i].get_header("task_id") == msg.get_value("task_id"))
    //     {
    //         msg.attribute("group", task_sequence[i].get_header("group"));
    //         auto from_ip = task_sequence[i].get_header("from_ip");
    //         auto from_port = task_sequence[i].get_header("from_port");
    //         bs->write(msg.to_packet(), ns3::Ipv4Address(from_ip.c_str()), std::stoi(from_port));

    //         // 清除任务队列和分发状态
    //         task_sequence.erase(std::next(task_sequence.begin(), i), std::next(task_sequence.begin(), i + 1));
    //         task_sequence_status.erase(std::next(task_sequence_status.begin(), i), std::next(task_sequence_status.begin(), i + 1));
    //         break;
    //     }
    // }
}

auto worst_fit_decision_engine::on_es_handling_message(
    edge_device* es, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    // this->handle_next(); // 这里开始下一个，由于资源尚未更改，容易导致 Conflict.

    auto ipv4_remote = ns3::InetSocketAddress::ConvertFrom(remote_address).GetIpv4();
    message msg(packet);
    auto task_item = msg.get_task_element(); // task_element::from_msg_packet(packet);
    auto task_id = task_item.get_header("task_id");

    log::info("edge server({:ip}) has received a task({}).", es->get_address(), task_id);

    auto es_resource = es->get_resource();
    auto cpu_supply = std::stod(es_resource->get_value("cpu"));
    auto cpu_demand = std::stod(task_item.get_header("cpu"));
    auto uncertain_cpu_supply = std::stod(msg.get_value("cpu_supply"));

    // 存在冲突，需要重新决策
    if (uncertain_cpu_supply != cpu_supply || cpu_supply < cpu_demand) {
        // 需要重新分配
        log::error("Conflict! cpu_demand: {}, cpu_supply: {}, real_supply: {}.", cpu_demand, uncertain_cpu_supply, cpu_supply);
        this->conflict(es, task_item, ipv4_remote, es->get_port());
        return;
    }

    // 更改CPU资源
    es_resource->reset_value("cpu", std::to_string(cpu_supply - cpu_demand));
    this->resource_changed(es, ipv4_remote, es->get_port());

    // 处理任务
    double processing_time = cpu_demand / cpu_supply; // 任务能分发过来，cpu_supply 就不可能为0

    log::info("edge server({:ip}) consumes resources: {} --> {}", es->get_address(), cpu_supply, cpu_supply - cpu_demand);
    log::info("task(id={}) demand: {}, supply: {}, processing_time: {}", task_id, cpu_demand, cpu_supply, processing_time);

    auto self = shared_from_base<this_type>();
    ns3::Simulator::Schedule(ns3::Seconds(processing_time), [self, es, ipv4_remote, task_id, processing_time, cpu_demand]() {
        // 处理完成，释放内存
        auto device_resource = es->get_resource();
        auto cur_cpu = std::stod(device_resource->get_value("cpu"));
        device_resource->reset_value("cpu", std::to_string(cur_cpu + cpu_demand));
        auto device_address = fmt::format("{:ip}", es->get_address());

        log::info("edge server({}) restores resources: {} --> {:.2f}(demand: {})", device_address, cur_cpu, cur_cpu + cpu_demand, cpu_demand);

        self->resource_changed(es, ipv4_remote, es->get_port());

        message response {
            { "msgtype", "response" },
            { "task_id", task_id },
            { "device_type", "es" },
            { "device_address", device_address },
            { "processing_time", fmt::format("{:.9f}", processing_time) }
        };
        es->write(response.to_packet(), ipv4_remote, es->get_port());
    });
}

auto worst_fit_decision_engine::on_clients_reponse_message(
    client_device* client, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    message msg(packet);

    auto it = client->response_cache().find_if([&msg](const response::value_type& item) {
        return item["group"] == msg.get_value("group") && item["task_id"] == msg.get_value("task_id");
    });
    if (it != client->response_cache().end()) {
        (*it)["device_type"] = msg.get_value("device_type");
        (*it)["device_address"] = msg.get_value("device_address");
        (*it)["time_consuming"] = msg.get_value("processing_time");
        (*it)["finished"] = msg.get_value("device_type") != "null" ? "Y" : "N";

        log::success("client({:ip}) has received a response for task(id={}).", client->get_address(), msg.get_value("task_id"));
    }

    // 检查是否存在当前任务的信息
    auto exist = client->response_cache().find_if([&msg](const auto& item) {
        return item["group"] == msg.get_value("group");
    });
    if (exist == client->response_cache().end()) {
        log::error("Fatal error! Invalid response."); // 说明发出去的数据被修改，或是 m_response 被无意间删除了信息
        return;
    }

    // 全部完成
    auto unfinished = client->response_cache().find_if([&msg](const auto& item) {
        return item["group"] == msg.get_value("group") && item["finished"] == "0";
    });
    if (unfinished == client->response_cache().end()) {
        client->when_done(client->response_cache().dump_with({ "group", msg.get_value("group") }));
    }

}


} // namespace okec