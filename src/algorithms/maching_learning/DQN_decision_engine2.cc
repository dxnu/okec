///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

// #include "DQN_decision_engine2.h"
// #include "message.h"
// #include "base_station.h"
// #include "client_device.h"
// #include "cloud_server.h"
// #include "edge_device.h"
// #include <functional> // std::bind_front

// namespace okec
// {

// Env::Env(const device_cache& cache, const task& t, std::shared_ptr<DeepQNetwork> RL)
//     : cache_(cache)
//     , t_(t)
//     , RL_(RL)
// {
//     // 先为所有任务设置处理标识
//     for (auto& t : t_.elements()) {
//         t.set_header("status", "0"); // 0: 未处理 1: 已处理
//     }

//     // observation_ = next_observation();
// }

// auto Env::next_observation() -> torch::Tensor
// {
//     auto task_elements = this->t_.elements();
//     if (auto it = std::ranges::find_if(task_elements, [](auto const& item) {
//         return item.get_header("status") == "0";
//     }); it != std::end(task_elements)) {
//         auto& edge_cache = this->cache_.view();
//         std::vector<double> flattened_state;
//         for (const auto& edge : edge_cache) {
//             flattened_state.push_back(TO_DOUBLE(edge["cpu"]));
//         }

//         flattened_state.push_back(std::stod((it)->get_header("cpu")));
//         return torch::tensor(flattened_state, torch::dtype(torch::kFloat64)).unsqueeze(0);
//     }

//     return torch::Tensor();
// }

// auto Env::train() -> void
// {
//     fmt::print("train begin\n");
//     train_next();
// }

// // 任务量特别大时，是否会存在不会调用 train_next 的情况？？？ 需要思考一下
// auto Env::train_next() -> void
// {
//     static std::size_t step = 0;

//     float reward;
//     auto task_elements = t_.elements();
//     if (auto it = std::ranges::find_if(task_elements, [](auto const& item) {
//         return item.get_header("status") == "0";
//     }); it != std::end(task_elements)) {
//         auto& edge_cache = cache_.view();

//         ////////////////////////////////////////////////
//         std::vector<double> action_values;
//         for (const auto& edge : edge_cache) {
//             action_values.push_back(TO_DOUBLE(edge["cpu"]));
//         }
//         auto action = std::distance(action_values.begin(), std::max_element(action_values.begin(), action_values.end()));

//         ////////////////////////////////////////////////

//         // auto action = RL_->choose_action(observation);
//         auto& server = edge_cache.at(action);
//         // fmt::print("choose action: {}\n", action);
//         // fmt::print("server:\n{}\n", server.dump(4));


//         auto cpu_supply = TO_DOUBLE(server["cpu"]);
//         auto cpu_demand = std::stod(it->get_header("cpu"));

//         double processing_time;

//         // fmt::print("正在处理 {}, supply: {}, demand: {}\n", it->get_header("task_id"), cpu_supply, cpu_demand);

//         if (cpu_supply < cpu_demand) { // 无法处理
//             print_info(fmt::format("No device can handle the task({})!", it->get_header("task_id")));
//             return;
//         } else { // 可以处理
//             processing_time = cpu_demand / cpu_supply;
//             double new_cpu = cpu_supply - cpu_demand;
//             it->set_header("status", "1");
//             it->set_header("processing_time", std::to_string(processing_time));

//             // 消耗资源
//             server["cpu"] = std::to_string(new_cpu);
//             this->trace_resource(); // 监控资源
//             fmt::print(fg(fmt::color::red), "[{}] 消耗资源：{} --> {}\n", TO_STR(server["ip"]), cpu_supply, TO_DOUBLE(server["cpu"]));
//             fmt::print(fg(fmt::color::red), "[{}] demand: {}, supply: {}, processing_time: {}\n", it->get_header("task_id"), cpu_demand, cpu_supply, processing_time);

            

//             // 资源恢复
//             auto self = shared_from_this();
//             Simulator::Schedule(Seconds(processing_time), [self, action, cpu_demand]() {
//                 auto& edge_cache = self->cache_.view();
//                 auto& server = edge_cache.at(action);
//                 double cur_cpu = TO_DOUBLE(server["cpu"]);
//                 double new_cpu = cur_cpu + cpu_demand;
//                 fmt::print(fg(fmt::color::yellow), "[{}] 恢复资源：{} --> {:.2f}(demand: {})\n", TO_STR(server["ip"]), cur_cpu, new_cpu, cpu_demand);

//                 // self->t_.print();


//                 server["cpu"] = std::to_string(new_cpu);
//                 self->trace_resource(); // 监控资源

//                 self->train_next();
//             });


//             // 结束或继续处理
//             if (!t_.contains({"status", "0"})) {
//                 // fmt::print("reward: {}, done: {}\n", reward, true);

//                 if (done_fn_) {
//                     done_fn_(t_, cache_);
//                 }
//             } else {
//                 this->train_next(); // 继续训练下一个
//             }
//         }
//     }
// }

// auto Env::when_done(done_callback_t callback) -> void
// {
//     done_fn_ = callback;
// }

// auto Env::print_cache() -> void
// {
//     fmt::print("print_cache:\n{}\n", cache_.dump(4));
//     fmt::print("tasks:\n{}\n", t_.dump(4));
// }

// auto Env::learn(std::size_t step) -> void
// {
//     // 超过200条transition之后每隔5步学习一次

//     if (step > 200 and step % 5 == 0) {
//         RL_->learn();
//     }
// }

// auto Env::trace_resource() -> void
// {
//     static std::ofstream file;
//     if (!file.is_open()) {
//         file.open("scene2-wf-resource_tracer.csv", std::ios::out/* | std::ios::app*/);
//         if (!file.is_open()) {
//             return;
//         }
//     }

//     // for (const auto& item : m_resources) {
//     //     file << fmt::format("At time {:.2f}s,{:ip}", Simulator::Now().GetSeconds(), item->get_address());
//     //     for (auto it = item->begin(); it != item->end(); ++it) {
//     //         file << fmt::format(",{}: {}", it.key(), it.value());
//     //     }
//     //     file << "\n";
//     // }
//     // file << "\n";
//     file << fmt::format("{:.2f}", Simulator::Now().GetSeconds());
//     auto& edge_cache = this->cache_.view();
//     for (const auto& edge : edge_cache) {
//         file << "," << TO_DOUBLE(edge["cpu"]);
//     }
//     file << "\n";
// }

// DQN_decision_engine::DQN_decision_engine(
//     client_device_container* clients,
//     base_station_container* base_stations)
//     : clients_{clients}
//     , base_stations_{base_stations}
// {
//     // Set the decision device
//     if (base_stations->size() > 0uz)
//         m_decision_device = base_stations->get(0);

//     // Initialize the device cache
//     this->initialize_device(base_stations);

//     // Capture the decision and response message on base stations.
//     base_stations->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
//     base_stations->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

//     // Capture the handling message on edge servers
//     base_stations->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

//     // Capture the response message on client devices.
//     clients->set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
// }

// DQN_decision_engine::DQN_decision_engine(
//     std::vector<client_device_container>* clients_container,
//     base_station_container* base_stations)
//     : clients_container_{clients_container}
//     , base_stations_{base_stations}
// {
//     // Set the decision device
//     if (base_stations->size() > 0uz)
//         m_decision_device = base_stations->get(0);

//     // Initialize the device cache
//     this->initialize_device(base_stations);

//     // Capture the decision and response message on base stations.
//     base_stations->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
//     base_stations->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

//     // Capture the handling message on edge servers
//     base_stations->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

//     // Capture the response message on client devices.
//     for (auto& clients : *clients_container) {
//         clients.set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
//     }

//     fmt::print("{}\n", this->cache().dump(4));
// }

// auto DQN_decision_engine::make_decision(const task_element& header) -> result_t
// {
//     return result_t();
// }

// auto DQN_decision_engine::local_test(const task_element& header, client_device* client) -> bool
// {
//     return false;
// }

// auto DQN_decision_engine::send(task_element& t, client_device* client) -> bool
// {
//     static double launch_delay = 1.0;

//     client->response_cache().emplace_back({
//         { "task_id", t.get_header("task_id") },
//         { "group", t.get_header("group") },
//         { "finished", "0" }, // 1 indicates finished, while 0 signifies the opposite.
//         { "device_type", "" },
//         { "device_address", "" },
//         { "time_consuming", "" },
//         { "send_time", "" },
//         { "power_consumption", "" }
//     });

//     // 将所有任务都发送到决策设备，从而得到所有任务的信息
//     // 追加任务发送地址信息
//     t.set_header("from_ip", fmt::format("{:ip}", client->get_address()));
//     t.set_header("from_port", std::to_string(client->get_port()));
//     message msg;
//     msg.type(message_decision);
//     msg.content(t);
//     const auto bs = this->get_decision_device();
//     ns3::Simulator::Schedule(ns3::Seconds(launch_delay), &client_device::write, client, msg.to_packet(), bs->get_address(), bs->get_port());
//     launch_delay += 0.1;

//     return true;
// }

// auto DQN_decision_engine::train(const task& train_task, int episode) -> void
// {
//     auto n_actions = this->cache().size();
//     auto n_features = this->cache().size() + 1; // +1 是 task cpu demand
//     RL = std::make_shared<DeepQNetwork>(n_actions, n_features, 0.01, 0.9, 0.9, 200, 2000, 128, 0.05);

//     train_start(train_task, episode, episode);
// }

// auto DQN_decision_engine::initialize() -> void
// {
//     if (clients_) {
//         clients_->set_decision_engine(shared_from_base<this_type>());
//     }

//     if (clients_container_) {
//         for (auto& clients : *clients_container_) {
//             clients.set_decision_engine(shared_from_base<this_type>());
//         }
//     }

//     if (base_stations_) {
//         base_stations_->set_decision_engine(shared_from_base<this_type>());
//     }
// }

// auto DQN_decision_engine::handle_next() -> void
// {
//     return void();
// }

// auto DQN_decision_engine::on_bs_decision_message(
//     base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
// {
//     ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
//     print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}.", bs->get_address(), inetRemoteAddress.GetIpv4()));

//     auto item = okec::task_element::from_msg_packet(packet);
//     bs->task_sequence(std::move(item));

//     // bs->print_task_info();
//     // bs->handle_next_task();
// }

// auto DQN_decision_engine::on_bs_response_message(
//     base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
// {
// }

// auto DQN_decision_engine::on_cs_handling_message(
//     cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void
// {
// }

// auto DQN_decision_engine::on_es_handling_message(
//     edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
// {
// }

// auto DQN_decision_engine::on_clients_reponse_message(
//     client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void
// {
// }

// auto DQN_decision_engine::train_start(const task& train_task, int episode, int episode_all) -> void
// {
//     if (episode <= 0) {
//         // RL->print_memory();
//         return;
//     }

//     fmt::print(fg(fmt::color::blue), "At time {} seconds 正在训练第 {} 轮......\n", 
//         Simulator::Now().GetSeconds(), episode_all - episode + 1);


//     // 离散训练，必须每轮都创建一份对象，以隔离状态
//     auto env = std::make_shared<Env>(this->cache(), train_task, RL);

//     // 记录初始资源情况
//     env->trace_resource();

//     auto self = shared_from_base<this_type>();
//     env->when_done([self, &train_task, episode, episode_all](const task& t, const device_cache& cache) {
//         print_info("end of train"); // done
//         double total_time = .0f;
//         for (auto& elem : t.elements()) {
//             total_time += std::stod(elem.get_header("processing_time"));
//         }
//         t.print();
//         fmt::print(fg(fmt::color::orange), "total processing time: {} episode: {}\n", total_time, episode_all - episode + 1);
//         fmt::print(fg(fmt::color::orange), "average processing time: {} episode: {}\n", total_time / t.size(), episode_all - episode + 1);
//         // t.print();
//         // fmt::print("cache: \n {}\n", cache.dump(4));
        
//         // 继续训练下一轮
//         self->train_start(train_task, episode - 1, episode_all);
//     });


//     env->train();
// }



// } // namespace okec