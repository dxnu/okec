///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/algorithms/machine_learning/DQN_decision_engine.h>
#include <okec/common/message.h>
#include <okec/devices/base_station.h>
#include <okec/devices/client_device.h>
#include <okec/devices/cloud_server.h>
#include <okec/devices/edge_device.h>
#include <okec/utils/log.h>
#include <functional> // std::bind_front


namespace okec
{

Env::Env(const device_cache& cache, const task& t, std::shared_ptr<DeepQNetwork> RL)
    : t_(t)
    , cache_(cache)
    , RL_(RL)
    , step_(0)
{
    // 先为所有任务设置处理标识
    for (auto& t : t_.elements_view()) {
        t.set_header("status", "0"); // 0: 未处理 1: 已处理
    }

    // observation_ = next_observation();
}

auto Env::reset() -> torch::Tensor
{
    // torch::tensor makes a copy, from_blob does not (but torch::from_blob(vector).clone() does)
    // auto observation = torch::from_blob(state_.data(), {static_cast<long>(state_.size())}, torch::kFloat64);
    // return observation.unsqueeze(0);
    return observation_;
}

auto Env::next_observation() -> torch::Tensor
{
    auto task_elements = this->t_.elements_view();
    if (auto it = std::ranges::find_if(task_elements, [](auto const& item) {
        return item.get_header("status") == "0";
    }); it != std::end(task_elements)) {
        auto& edge_cache = this->cache_.view();
        std::vector<double> flattened_state;
        for (const auto& edge : edge_cache) {
            flattened_state.push_back(TO_DOUBLE(edge["cpu"]));
        }

        flattened_state.push_back(std::stod((it)->get_header("cpu")));
        return torch::tensor(flattened_state, torch::dtype(torch::kFloat64)).unsqueeze(0);
    }

    return torch::Tensor();
}

auto Env::train() -> void
{
    auto observation = next_observation();
    if (observation.defined())
        train_next(std::move(observation));
}

// 任务量特别大时，是否会存在不会调用 train_next 的情况？？？ 需要思考一下
auto Env::train_next(torch::Tensor observation) -> void
{
    // static std::size_t step = 0;
    // std::cout << "observation:\n" << observation << "\n";
    // okec::print("train task:\n {}\n", t_.dump(4));

    float reward;
    auto task_elements = t_.elements_view();
    if (auto it = std::ranges::find_if(task_elements, [](auto const& item) {
        return item.get_header("status") == "0";
    }); it != std::end(task_elements)) {
        auto& edge_cache = cache_.view();
        auto action = RL_->choose_action(observation);
        auto& server = edge_cache.at(action);
        // okec::print("choose action: {}\n", action);
        // okec::print("server:\n{}\n", server.dump(4));


        float alpha = 0.8; // 6/4
        float beta = 0.2; // 9/1 出现过23 8/2 也是

        auto cpu_supply = TO_DOUBLE(server["cpu"]);
        auto cpu_demand = std::stod(it->get_header("cpu"));

        // 计算平均处理时间
        std::vector<double> time;
        for (const auto& edge : edge_cache) {
            double e_supply = TO_DOUBLE(edge["cpu"]);
            if (e_supply != 0)
                time.push_back(cpu_demand / e_supply);
        }
        double average_processing_time = std::accumulate(time.begin(), time.end(), .0) / time.size();
        double processing_time;

        // okec::print("正在处理 {}, supply: {}, demand: {}\n", it->get_header("task_id"), cpu_supply, cpu_demand);

        if (cpu_supply < cpu_demand) { // 无法处理
            processing_time = cpu_demand / cpu_supply;
            // reward = -alpha * processing_time + beta * (cpu_supply - cpu_demand);
            reward = -alpha * processing_time + beta * cpu_supply;
            // reward = alpha * (average_processing_time - processing_time) + beta * (cpu_supply - cpu_demand);
            RL_->store_transition(observation, action, reward, observation); // 状态不曾改变
            // okec::print("reward: {}, done: {}\n", reward, false);

            this->learn(step_++);
            // this->train_next(std::move(observation));
        } else { // 可以处理
            processing_time = cpu_demand / cpu_supply;
            double new_cpu = cpu_supply - cpu_demand;
            it->set_header("status", "1");
            it->set_header("processing_time", std::to_string(processing_time));

            // 消耗资源
            server["cpu"] = std::to_string(new_cpu);
            // okec::print("[{}] 消耗资源：{} --> {}\n", TO_STR(server["ip"]), cpu_supply, TO_DOUBLE(server["cpu"]));

            this->trace_resource();

            reward = -alpha * processing_time + beta * new_cpu;
            // reward = alpha * (average_processing_time - processing_time) + beta * new_cpu;
            

            // 资源恢复
            auto self = shared_from_this();
            ns3::Simulator::Schedule(ns3::Seconds(processing_time), [self, action, cpu_demand, alpha, beta, average_processing_time]() {
                auto& edge_cache = self->cache_.view();
                auto& server = edge_cache.at(action);
                double cur_cpu = TO_DOUBLE(server["cpu"]);
                double new_cpu = cur_cpu + cpu_demand;
                // okec::format("[{}] 恢复资源：{} --> {:.2f}(demand: {})", TO_STR(server["ip"]), cur_cpu, new_cpu, cpu_demand);
                
                // 恢复资源
                float reward;

                // self->t_.print();

                auto observation = self->next_observation();
                server["cpu"] = std::to_string(new_cpu);

                self->trace_resource();

                if (observation.defined()) {
                    // std::cout << "observation _:\n" << observation << "\n";

                    auto sizes = observation.sizes();
                    auto accessor = observation.accessor<double, 2>();
                    double cpu_demand = accessor[sizes[0]-1][sizes[1]-1]; // 状态中最后一位是任务需求
                    // std::cout << "任务需求: " << cpu_demand << "\n";

                    // double processing_time = cpu_demand / new_cpu;
                    // reward = -alpha * (cpu_demand / new_cpu) + beta * (new_cpu - cpu_demand);
                    reward = -alpha * (cpu_demand / new_cpu) + beta * new_cpu;
                    // reward = alpha * (average_processing_time - processing_time) + beta * (new_cpu - cpu_demand);
                    // reward = alpha * (average_processing_time - processing_time) + beta * new_cpu;
                    self->RL_->store_transition(observation, action, reward, observation);

                    self->learn(self->step_++);

                    self->train_next(std::move(observation));
                }
            });


            // 结束或继续处理
            if (!t_.contains({"status", "0"})) {
                // okec::print("reward: {}, done: {}\n", reward, true);

                if (done_fn_) {
                    done_fn_(t_, cache_);
                }
            } else {
                // okec::print("reward: {}, done: {}\n", reward, false);
                // 更新状态
                auto observation_new = next_observation();
                RL_->store_transition(observation, action, reward, observation_new);
                // std::cout << "new state\n" << observation_new << "\n";

                this->learn(step_++);
                this->train_next(std::move(observation_new)); // 继续训练下一个
            }
        }
    }
}

auto Env::when_done(done_callback_t callback) -> void
{
    done_fn_ = callback;
}

auto Env::print_cache() -> void
{
    okec::print("print_cache:\n{}\n", cache_.dump(4));
    okec::print("tasks:\n{}\n", t_.dump(4));
}

auto Env::learn(std::size_t step) -> void
{
    // 超过200条transition之后每隔5步学习一次

    if (step > 200 and step % 5 == 0) {
        RL_->learn();
    }
}

auto Env::trace_resource(int flag) -> void
{
    static std::ofstream file;
    if (!file.is_open()) {
        file.open("./data/rf-discrete-resource_tracer.csv", std::ios::out/* | std::ios::app*/);
        if (!file.is_open()) {
            return;
        }
    }

    if (flag) {
        file << "\n\n\n\n===================Episode-" + std::to_string(episode) + "===========================\n\n\n\n";
    }

    // for (const auto& item : m_resources) {
    //     file << okec::format("At time {:.2f}s,{:ip}", Simulator::Now().GetSeconds(), item->get_address());
    //     for (auto it = item->begin(); it != item->end(); ++it) {
    //         file << okec::format(",{}: {}", it.key(), it.value());
    //     }
    //     file << "\n";
    // }
    // file << "\n";
    file << okec::format("{:.2f} [episode={}]", ns3::Simulator::Now().GetSeconds(), episode);
    auto& edge_cache = this->cache_.view();
    for (const auto& edge : edge_cache) {
        file << "," << TO_DOUBLE(edge["cpu"]);
    }
    file << "\n";
}

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

    okec::print("{}\n", this->cache().dump(4));
}

auto DQN_decision_engine::make_decision(const task_element& header) -> result_t
{
    return result_t();
}

auto DQN_decision_engine::local_test(const task_element& header, client_device* client) -> bool
{
    return false;
}

auto DQN_decision_engine::send(task_element t, std::shared_ptr<client_device> client) -> bool
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
    t.set_header("from_ip", okec::format("{:ip}", client->get_address()));
    t.set_header("from_port", std::to_string(client->get_port()));
    message msg;
    msg.type(message_decision);
    msg.content(t);
    const auto bs = this->get_decision_device();
    auto write = [client, bs, content = msg.to_packet()]() {
        client->write(content, bs->get_address(), bs->get_port());
    };
    ns3::Simulator::Schedule(ns3::Seconds(launch_delay), write);
    // ns3::Simulator::Schedule(ns3::Seconds(launch_delay), &client_device::write, client, msg.to_packet(), bs->get_address(), bs->get_port());
    launch_delay += 0.1;

    return true;
}

auto DQN_decision_engine::train(const task& train_task, int episode) -> void
{
    auto n_actions = this->cache().size();
    auto n_features = this->cache().size() + 1; // +1 是 task cpu demand
    RL = std::make_shared<DeepQNetwork>(n_actions, n_features, 0.01, 0.9, 0.9, 200, 2000, 128, 0.0001);


    train_start(train_task, episode, episode);


    // train_task = t;
    // auto n_actions = this->cache().size();
    // auto n_features = this->cache().size();
    // RL = std::make_shared<DeepQNetwork>(n_actions, n_features, 0.01, 0.9, 0.9, 200, 2000);

    // // get observation
    // std::vector<double> state;
    // state.reserve(this->cache().size());
    // for (auto it = this->cache().cbegin(); it != this->cache().cend(); ++it)
    //     state.push_back(TO_DOUBLE((*it)["cpu"])); // edge resources

    // auto observation = torch::tensor(state, torch::dtype(torch::kFloat64)).unsqueeze(0);

    // for (auto& t : train_task.elements()) {
    //     t.set_header("status", "0"); // 0: 未处理 1: 已处理
    // }

    // train_next(std::move(observation));




    // auto env = std::make_shared<Env>(this->cache(), t);

    // okec::print("actions: {}, features: {}\n", env->n_actions(), env->n_features());

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

    //         okec::print("choose action: {}\n", action);

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

    // okec::print("end of train\n");
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
    base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    log::debug("The base station[{:ip}] has received the decision request from {:ip}.", bs->get_address(), inetRemoteAddress.GetIpv4());

    auto item = okec::task_element::from_msg_packet(packet);
    bs->task_sequence(std::move(item));

    // bs->print_task_info();
    // bs->handle_next_task();
}

auto DQN_decision_engine::on_bs_response_message(
    base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
}

auto DQN_decision_engine::on_cs_handling_message(
    cloud_server* cs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
}

auto DQN_decision_engine::on_es_handling_message(
    edge_device* es, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
}

auto DQN_decision_engine::on_clients_reponse_message(
    client_device* client, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void
{
}

auto DQN_decision_engine::train_start(const task& train_task, int episode, int episode_all) -> void
{
    // static std::vector<double> total_times;
    if (episode <= 0) {
        // RL->print_memory();
        // okec::print("total times\n{}\n", total_times);
        auto total_time = std::accumulate(total_times_.begin(), total_times_.end(), .0);
        auto [min, max] = std::ranges::minmax(total_times_);
        log::info("Average total times: {}, min: {}, max: {}", total_time / total_times_.size(), min, max);
        // RL->plot_cost();
        return;
    }

    log::info("Training iteration {} is in progress.", episode_all - episode + 1);


    // 离散训练，必须每轮都创建一份对象，以隔离状态
    auto env = std::make_shared<Env>(this->cache(), train_task, RL);

    // 记录初始资源情况
    env->episode = episode_all - episode + 1;
    env->trace_resource(env->episode);

    auto self = shared_from_base<this_type>();
    env->when_done([self, &train_task, episode, episode_all](const task& t, const device_cache& cache) {
        log::debug("train end (episode={})", episode_all - episode + 1);
        double total_time = .0f;
        for (const auto& elem : t.elements()) {
            total_time += std::stod(elem.get_header("processing_time"));
        }
        // t.print();
        self->total_times_.push_back(total_time);
        log::success("Total processing time: {}", total_time);
        // t.print();
        // okec::print("cache: \n {}\n", cache.dump(4));
        
        // 继续训练下一轮
        self->train_start(train_task, episode - 1, episode_all);
    });

    log::debug("train begin (episode={})", episode_all - episode + 1);
    env->train();
}



} // namespace okec