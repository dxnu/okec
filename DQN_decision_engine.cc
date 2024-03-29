#include "DQN_decision_engine.h"
#include "message.h"
#include "base_station.h"
#include "client_device.h"
#include "cloud_server.h"
#include "edge_device.h"
#include <functional> // std::bind_front

namespace okec
{
DQN_decision_engine::DQN_decision_engine(
    client_device_container* clients,
    base_station_container* base_stations)
    : clients_{clients}
    , base_stations_{base_stations}
{
    // Set the decision device
    if (base_stations->size() > 0uz)
        m_decision_device = base_stations->get(0);

    // Initialize the device cache
    this->initialize_device(base_stations);

    // Capture the decision and response message on base stations.
    base_stations->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    base_stations->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture the handling message on edge servers
    base_stations->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture the response message on client devices.
    clients->set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
}

DQN_decision_engine::DQN_decision_engine(
    std::vector<client_device_container>* clients_container,
    base_station_container* base_stations)
    : clients_container_{clients_container}
    , base_stations_{base_stations}
{
    // Set the decision device
    if (base_stations->size() > 0uz)
        m_decision_device = base_stations->get(0);

    // Initialize the device cache
    this->initialize_device(base_stations);

    // Capture the decision and response message on base stations.
    base_stations->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    base_stations->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture the handling message on edge servers
    base_stations->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture the response message on client devices.
    for (auto& clients : *clients_container) {
        clients.set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
    }

    fmt::print("{}\n", this->cache().dump(4));
}

auto DQN_decision_engine::make_decision(const task_element& header) -> result_t
{
    return result_t();
}

auto DQN_decision_engine::local_test(const task_element& header, client_device* client) -> bool
{
    return false;
}

auto DQN_decision_engine::send(task_element& t, client_device* client) -> bool
{
    static double launch_delay = 1.0;

    client->response_cache().emplace_back({
        { "task_id", t.get_header("task_id") },
        { "group", t.get_header("group") },
        { "finished", "0" }, // 1 indicates finished, while 0 signifies the opposite.
        { "device_type", "" },
        { "device_address", "" },
        { "time_consuming", "" },
        { "send_time", "" },
        { "power_consumption", "" }
    });

    // 将所有任务都发送到决策设备，从而得到所有任务的信息
    // 追加任务发送地址信息
    t.set_header("from_ip", fmt::format("{:ip}", client->get_address()));
    t.set_header("from_port", std::to_string(client->get_port()));
    message msg;
    msg.type(message_decision);
    msg.content(t);
    const auto bs = this->get_decision_device();
    ns3::Simulator::Schedule(ns3::Seconds(launch_delay), &client_device::write, client, msg.to_packet(), bs->get_address(), bs->get_port());
    launch_delay += 0.1;

    return true;
}

auto DQN_decision_engine::train(const task& t) -> void
{
    train_task = t;
    auto n_actions = this->cache().size();
    auto n_features = this->cache().size();
    RL = std::make_shared<DeepQNetwork>(n_actions, n_features, 0.01, 0.9, 0.9, 200, 2000);

    // get observation
    std::vector<double> state;
    state.reserve(this->cache().size());
    for (auto it = this->cache().cbegin(); it != this->cache().cend(); ++it)
        state.push_back(TO_DOUBLE((*it)["cpu"])); // edge resources

    auto observation = torch::tensor(state, torch::dtype(torch::kFloat64)).unsqueeze(0);

    for (auto& t : train_task.elements()) {
        t.set_header("status", "0"); // 0: 未处理 1: 已处理
    }

    train_next(std::move(observation));
    // auto env = std::make_shared<Env>(this->cache(), t);

    // fmt::print(fg(fmt::color::yellow), "actions: {}, features: {}\n", env->n_actions(), env->n_features());

    // auto RL = DeepQNetwork(env->n_actions(), env->n_features(), 0.01, 0.9, 0.9, 200, 2000);
    
    // // run edge
    // int step = 0;

    // for ([[maybe_unused]] auto episode : std::views::iota(0, 1)) {
    //     // observation
    //     auto observation = env->reset();
    //     std::cout << observation << std::endl;

    //     for (;;) {
    //         // RL choose action based on observation
    //         auto action = RL.choose_action(observation);

    //         fmt::print("choose action: {}\n", action);

    //         auto [observation_, reward, done] = env->step2(action);
            
    //         // std::cout << "\n";
    //         // std::cout << observation_ << "\n";
    //         // std::cout << "reward: " << reward << " done: " << done << "\n";

    //         RL.store_transition(observation, action, reward, observation_);

    //         // 超过200条transition之后每隔5步学习一次
    //         if (step > 200 and step % 5 == 0) {
    //             RL.learn();
    //         }

    //         observation = observation_;

    //         if (done)
    //             break;

    //         step += 1;
    //     }
    // }

    // fmt::print("end of train\n");
    // // RL.print_memory();

    // env->print_cache();
    
    // // Simulator::Schedule(Seconds(1), [env]() {
    // //     env->print_cache();
    // // });
}

auto DQN_decision_engine::initialize() -> void
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

auto DQN_decision_engine::handle_next() -> void
{
    return void();
}

auto DQN_decision_engine::on_bs_decision_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}.", bs->get_address(), inetRemoteAddress.GetIpv4()));

    auto item = okec::task_element::from_msg_packet(packet);
    bs->task_sequence(std::move(item));

    // bs->print_task_info();
    // bs->handle_next_task();
}

auto DQN_decision_engine::on_bs_response_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto DQN_decision_engine::on_cs_handling_message(
    cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto DQN_decision_engine::on_es_handling_message(
    edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto DQN_decision_engine::on_clients_reponse_message(
    client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto DQN_decision_engine::train_next(torch::Tensor observation) -> void
{
    // std::cout << "observation:\n" << observation << "\n";
    // fmt::print("train task:\n {}\n", train_task.dump(4));

    float reward;
    auto task_elements = train_task.elements();
    if (auto it = std::ranges::find_if(task_elements, [](auto const& item) {
        return item.get_header("status") == "0";
    }); it != std::end(task_elements)) {
        auto action = RL->choose_action(observation);
        fmt::print("choose action: {}\n", action);

        auto& edge_cache = this->cache().view();
        auto& server = edge_cache.at(action);
        // fmt::print("server:\n{}\n", server.dump(4));

        auto cpu_supply = TO_DOUBLE(server["cpu"]);
        auto cpu_demand = std::stod(it->get_header("cpu"));
        if (cpu_supply < cpu_demand) { // 无法处理
            reward = -1;
            RL->store_transition(observation, action, reward, observation); // 状态不曾改变
            fmt::print("reward: {}, done: {}\n", reward, false);
        } else { // 可以处理
            reward = 1;
            it->set_header("status", "1");

            // 消耗资源
            server["cpu"] = std::to_string(cpu_supply - cpu_demand);
            fmt::print(fg(fmt::color::red), "[{}] 消耗资源：{} --> {}\n", TO_STR(server["ip"]), cpu_supply, TO_DOUBLE(server["cpu"]));

            // 处理时间
            double processing_time = cpu_demand / cpu_supply;
            it->set_header("processing_time", std::to_string(processing_time));

            // 资源恢复
            auto self = shared_from_base<this_type>();
            Simulator::Schedule(Seconds(processing_time), [self, action, cpu_demand]() {
                auto& edge_cache = self->cache().view();
                auto& server = edge_cache.at(action);
                double cur_cpu = TO_DOUBLE(server["cpu"]);
                double new_cpu = cur_cpu + cpu_demand;
                print_info(fmt::format("[{}] 恢复资源：{} --> {:.2f}(demand: {})", TO_STR(server["ip"]), cur_cpu, new_cpu, cpu_demand));
                
                // 恢复资源
                server["cpu"] = std::to_string(cur_cpu + cpu_demand);

                std::vector<double> flattened_state;
                for (const auto& edge : edge_cache) {
                    flattened_state.push_back(TO_DOUBLE(edge["cpu"]));
                }
                auto observation_new = torch::tensor(flattened_state, torch::dtype(torch::kFloat64)).unsqueeze(0);
                self->train_next(observation_new); // 继续训练下一个
            });

            // 更新状态
            std::vector<double> flattened_state;
            for (const auto& edge : edge_cache) {
                flattened_state.push_back(TO_DOUBLE(edge["cpu"]));
            }
            auto observation_new = torch::tensor(flattened_state, torch::dtype(torch::kFloat64)).unsqueeze(0);
            RL->store_transition(observation, action, reward, observation_new);
            // std::cout << "new state\n" << observation_new << "\n";

            // 结束或继续处理
            if (!train_task.contains({"status", "0"})) {
                fmt::print("reward: {}, done: {}\n", reward, true);
                fmt::print("end of train\n"); // done
                train_task.print();
            } else {
                fmt::print("reward: {}, done: {}\n", reward, false);
                this->train_next(observation_new); // 继续训练下一个
            }
        }
    }
}

} // namespace okec