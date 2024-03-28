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
    auto env = std::make_shared<Env>(this->cache(), t);

    fmt::print(fg(fmt::color::yellow), "actions: {}, features: {}\n", env->n_actions(), env->n_features());

    auto RL = DeepQNetwork(env->n_actions(), env->n_features(), 0.01, 0.9, 0.9, 200, 2000);
    
    // run edge
    int step = 0;

    for ([[maybe_unused]] auto episode : std::views::iota(0, 1)) {
        // observation
        auto observation = env->reset();
        std::cout << observation << std::endl;

        for (;;) {
            // RL choose action based on observation
            auto action = RL.choose_action(observation);

            fmt::print("choose action: {}\n", action);

            auto [observation_, reward, done] = env->step(action);
            
            // std::cout << "\n";
            // std::cout << observation_ << "\n";
            // std::cout << "reward: " << reward << " done: " << done << "\n";

            exit(1);

            RL.store_transition(observation, action, reward, observation_);

            // 超过200条transition之后每隔5步学习一次
            if (step > 200 and step % 5 == 0) {
                RL.learn();
            }

            observation = observation_;

            if (done)
                break;

            step += 1;
        }
    }

    fmt::print("end of train\n");
    // RL.print_memory();
    
    // Simulator::Schedule(Seconds(1), [env]() {
    //     env->print_cache();
    // });
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

} // namespace okec