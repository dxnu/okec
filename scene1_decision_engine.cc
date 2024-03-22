#include "scene1_decision_engine.h"
#include "message.h"
#include "base_station.h"
#include "client_device.h"
#include "edge_device.h"
#include <functional> // std::bind_front

namespace okec {

scene1_decision_engine::scene1_decision_engine(
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

scene1_decision_engine::scene1_decision_engine(
    std::vector<client_device_container>* clients_container,
    base_station_container* base_stations)
    : clients_container_{clients_container}
    , base_stations_{base_stations}
{
        // 设置决策设备
    m_decision_device = base_stations->get(1);

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

auto scene1_decision_engine::make_decision(const task_element& header) -> result_t
{
    auto edge_cache = this->cache().data();
    auto edge_max = *std::max_element(edge_cache.begin(), edge_cache.end(), 
        [](const device_cache::value_type& lhs, const device_cache::value_type& rhs) {
            return TO_DOUBLE(lhs["cpu"]) < TO_DOUBLE(rhs["cpu"]);
        });

    fmt::print("edge max: {}\n", TO_STR(edge_max["ip"]));
    
    double cpu_demand = std::stod(header.get_header("cpu"));
    double cpu_supply = TO_DOUBLE(edge_max["cpu"]);
    double tolorable_time = std::stod(header.get_header("deadline"));
    // If found a avaliable edge server
    if (cpu_supply > .0) {
        double processing_time = cpu_demand / cpu_supply;
        if (processing_time < tolorable_time) {
            return {
                { "ip", edge_max["ip"] },
                { "port", edge_max["port"] }
            };
        }
    }

    return result_t();
}

auto scene1_decision_engine::local_test(const task_element& header, client_device* client) -> bool
{
    return false;
}

auto scene1_decision_engine::send(task_element& t, client_device* client) -> bool
{
    static double launch_delay = 1.0;

    client->response_cache().emplace_back({
        { "task_id", t.get_header("task_id") },
        { "group", t.get_header("group") },
        { "finished", "0" }, // 0: unfinished, Y: finished, N: offloading failure
        { "device_type", "" },
        { "device_address", "" },
        { "time_consuming", "" }
    });

    fmt::print("Received tasks:\n{}\n", t.j_data().dump(4));

    // 不管本地，全部往边缘服务器卸载
    t.set_header("from_ip", fmt::format("{:ip}", client->get_address()));
    t.set_header("from_port", std::to_string(client->get_port()));
    message msg;
    msg.type(message_decision);
    msg.content(t);
    const auto bs = this->get_decision_device();
    fmt::print("bs: {:ip}, client: {:ip}\n", bs->get_address(), client->get_address());
    ns3::Simulator::Schedule(ns3::Seconds(launch_delay), &client_device::write, client, msg.to_packet(), bs->get_address(), bs->get_port());
    launch_delay += 0.1;

    return true;
}

auto scene1_decision_engine::initialize() -> void
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

auto scene1_decision_engine::handle_next() -> void
{
    auto& task_sequence = m_decision_device->task_sequence();
    auto& task_sequence_status = m_decision_device->task_sequence_status();
    
    // 分发任务列表，每次拿出第一个未分发的任务
    for (std::size_t i = 0; i < task_sequence.size(); ++i)
    {
        // 任务尚未分发
        if (!static_cast<int>(task_sequence_status[i]))
        {
            auto target = make_decision(task_sequence[i]);
            if (target.is_null()) {
                print_info(fmt::format("No device can handle the task({})!", task_sequence[i].get_header("task_id")));
                message response {
                    { "msgtype", "response" },
                    { "task_id", task_sequence[i].get_header("task_id") },
                    { "group", task_sequence[i].get_header("group") },
                    { "device_type", "null" },
                    { "device_address", "null" },
                    { "processing_time", "null" }
                };
                auto from_ip = task_sequence[i].get_header("from_ip");
                auto from_port = task_sequence[i].get_header("from_port");
                m_decision_device->write(response.to_packet(), Ipv4Address(from_ip.c_str()), std::stoi(from_port));
                // 分发失败也要进行清除操作
                task_sequence.erase(std::next(task_sequence.begin(), i), std::next(task_sequence.begin(), i + 1));
                task_sequence_status.erase(std::next(task_sequence_status.begin(), i), std::next(task_sequence_status.begin(), i + 1));
                continue; // 继续尝试处理下一个
            }

            print_info(fmt::format("Decision(task id:{}) is done. (target ip: {}, target port: {}.)", 
                task_sequence[i].get_header("task_id"), TO_STR(target["ip"]), TO_INT(target["port"])));
            // 计算并记录传输时间
            Ptr<NetDevice> device = m_decision_device->get_node()->GetDevice(0);
            Ptr<Channel> channel = device->GetChannel();
            if (channel) {
                StringValue band_width;
                channel->GetAttribute("Delay", band_width);

                DataRateValue dataRateValue;
                device->GetAttribute("DataRate", dataRateValue);
                DataRate dataRate = dataRateValue.Get();

                fmt::print("DataRate: {}Mbps, delay: {}ms\n", dataRate.GetBitRate() / 1000000, Time(band_width.Get()).GetMilliSeconds());
            }

            message msg;
            msg.type(message_handling);
            msg.content(task_sequence[i]);
            print_info(fmt::format("bs({:ip}) dispatchs the task(task_id = {}) to {}",
                m_decision_device->get_address(), task_sequence[i].get_header("task_id"), TO_STR(target["ip"])));
            m_decision_device->write(msg.to_packet(), ns3::Ipv4Address(TO_STR(target["ip"]).c_str()), TO_INT(target["port"]));
            
            task_sequence_status[i] = 1; // // 更改任务分发状态
            break; // 若是继续处理下一个，处理任务的边缘设备资源变化信息可能还未收到，也许会导致错误决策
        }
    }
}

auto scene1_decision_engine::on_bs_decision_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}\n", bs->get_address(), inetRemoteAddress.GetIpv4()));

    fmt::print("Resource cache:\n{}\n", this->cache().dump(4));

    // task_element 为单位
    auto item = okec::task_element::from_msg_packet(packet);
    bs->task_sequence(std::move(item));
    bs->print_task_info();
    
    this->handle_next();
}

auto scene1_decision_engine::on_bs_response_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    auto ipv4_remote = InetSocketAddress::ConvertFrom(remote_address).GetIpv4();
    print_info(fmt::format("bs({:ip}) has received a response from {:ip}", bs->get_address(), ipv4_remote));

    message msg(packet);
    fmt::print("{}\n", msg.dump());
    auto& task_sequence = bs->task_sequence();
    auto& task_sequence_status = bs->task_sequence_status();
    for (std::size_t i = 0; i < task_sequence.size(); ++i)
    {
        // 将处理结果转发回客户端
        if (task_sequence[i].get_header("task_id") == msg.get_value("task_id"))
        {
            msg.attribute("group", task_sequence[i].get_header("group"));
            auto from_ip = task_sequence[i].get_header("from_ip");
            auto from_port = task_sequence[i].get_header("from_port");
            bs->write(msg.to_packet(), ns3::Ipv4Address(from_ip.c_str()), std::stoi(from_port));

            // 清除任务队列和分发状态
            fmt::print("task_sequence size: {}, task_sequence_status size: {}\n", task_sequence.size(), task_sequence_status.size());
            task_sequence.erase(std::next(task_sequence.begin(), i), std::next(task_sequence.begin(), i + 1));
            task_sequence_status.erase(std::next(task_sequence_status.begin(), i), std::next(task_sequence_status.begin(), i + 1));
            fmt::print("task_sequence size: {}, task_sequence_status size: {}\n", task_sequence.size(), task_sequence_status.size());
            break;
        }
    }
}

auto scene1_decision_engine::on_es_handling_message(
    edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
{
    auto ipv4_remote = InetSocketAddress::ConvertFrom(remote_address).GetIpv4();
    auto task_item = task_element::from_msg_packet(packet);
    auto task_id = task_item.get_header("task_id");

    print_info(fmt::format("es({:ip}) has received a task({}).\n", es->get_address(), task_id));

    auto es_resource = es->get_resource();
    auto new_cpu = std::stod(es_resource->get_value("cpu")) - std::stod(task_item.get_header("cpu"));
    auto old_cpu = es_resource->reset_value("cpu", fmt::format("{:.2f}", new_cpu));
    fmt::print("old_cpu: {}, new_cpu: {}\n", old_cpu, es_resource->get_value("cpu"));
    this->resource_changed(es, ipv4_remote, es->get_port());

    // 处理任务
    double cpu_demand = std::stod(task_item.get_header("cpu"));
    double cpu_supply = std::stod(old_cpu);
    double processing_time = cpu_demand / cpu_supply; // 任务能分发过来，cpu_supply 就不可能为0
    Simulator::Schedule(ns3::Seconds(processing_time), +[](edge_device* es, const std::string& old_cpu, const Ipv4Address& desination, const std::string& task_id, double processing_time, std::shared_ptr<this_type> self) {
        // 处理完成，释放内存
        auto device_resource = es->get_resource();
        device_resource->reset_value("cpu", old_cpu);
        auto device_address = fmt::format("{:ip}", es->get_address());

        // 通知资源变化
        self->resource_changed(es, desination, es->get_port());

        // 返回处理结果
        // 返回消息
        message response {
            { "msgtype", "response" },
            { "task_id", task_id },
            { "device_type", "es" },
            { "device_address", device_address },
            { "processing_time", fmt::format("{:.9f}", processing_time) }
        };
        es->write(response.to_packet(), desination, es->get_port());
    }, es, old_cpu, ipv4_remote, task_id, processing_time, shared_from_base<this_type>());
}

auto scene1_decision_engine::on_clients_reponse_message(
    client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void
{
    message msg(packet);
    fmt::print(fg(fmt::color::red), "At time {:.2f}s client({:ip}) has received a packet: {}\n", 
        Simulator::Now().GetSeconds(), client->get_address(), msg.dump());

    auto it = client->response_cache().find_if([&msg](const response::value_type& item) {
        return item["group"] == msg.get_value("group") && item["task_id"] == msg.get_value("task_id");
    });
    if (it != client->response_cache().end()) {
        (*it)["device_type"] = msg.get_value("device_type");
        (*it)["device_address"] = msg.get_value("device_address");
        (*it)["time_consuming"] = msg.get_value("processing_time");
        (*it)["finished"] = msg.get_value("device_type") != "null" ? "Y" : "N";
    }

    // 检查是否存在当前任务的信息
    auto exist = client->response_cache().find_if([&msg](const auto& item) {
        return item["group"] == msg.get_value("group");
    });
    if (exist == client->response_cache().end()) {
        fmt::print(fg(fmt::color::red), "Fatal error! Invalid response.\n"); // 说明发过去的数据被修改，或是 m_response 被无意间删除了信息
        return;
    }

    // 全部完成
    auto unfinished = client->response_cache().find_if([&msg](const auto& item) {
        return item["group"] == msg.get_value("group") && item["finished"] == "0";
    });
    if (unfinished == client->response_cache().end()) {
        if (client->has_done_callback()) {
            client->done_callback(client->response_cache().dump_with({ "group", msg.get_value("group") }));
        }
    }

}




} // namespace okec