#include "qlearning_decision_engine.h"

#include "message.h"
#include "base_station.h"
#include "client_device.h"
#include "cloud_server.h"
#include "edge_device.h"
#include <functional> // std::bind_front

namespace okec
{

qlearning_decision_engine::qlearning_decision_engine(
    client_device_container* client_devices, 
    base_station_container* bs_container,
    cloud_server* cs)
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

    // fmt::print(fg(fmt::color::yellow), "{}", this->cache().view().dump(4));
}

auto qlearning_decision_engine::make_decision(const task_element& header) -> result_t
{
    return result_t("", 0, 0);
}

auto qlearning_decision_engine::local_test(const task_element& header, client_device* client) -> bool
{
    return false;
}

auto qlearning_decision_engine::send(task_element& t, client_device* client) -> bool
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

auto qlearning_decision_engine::train(
    const task& t, std::shared_ptr<client_device> client,
    base_station_container* bs_container, cloud_server* cs) -> void
{
    // int action_space = 1;
    // std::for_each(bs_container->begin(), bs_container->end(),
    //     [&action_space](const base_station_container::pointer_t bs) {
    //         action_space += bs->get_edge_devices().size();
    //     });
    
    // fmt::print(fg(fmt::color::yellow), "action space: {}\n", this->cache().size());

    // fmt::print("cache: \n{}\n", this->cache().dump());

    auto env = Env(this->cache());
    auto RL = DeepQNetwork(env.n_actions(), env.n_features(), 0.01, 0.9, 0.9, 200, 2000);

    // run edge
    int step = 0;

    for (auto episode : std::views::iota(0, 20)) {
        // observation
        auto observation = env.reset();
        // std::cout << observation << std::endl;

        for (;;) {
            // RL choose action based on observation
            auto action = RL.choose_action(observation);

            fmt::print("choose action: {}\n", action);
            
            auto [observation_, reward, done] = env.step(action);

            std::cout << observation_ << "\n";
            std::cout << "reward: " << reward << " done: " << done << "\n";

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
    RL.print_memory();
}

auto qlearning_decision_engine::on_bs_decision_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}\n", bs->get_address(), inetRemoteAddress.GetIpv4()));

    auto item = okec::task_element::from_msg_packet(packet);
    bs->task_sequence(std::move(item));

    // bs->print_task_info();
    // bs->handle_next_task();
}

auto qlearning_decision_engine::on_bs_response_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto qlearning_decision_engine::on_cs_handling_message(
    cloud_server* cs, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto qlearning_decision_engine::on_es_handling_message(
    edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto qlearning_decision_engine::on_clients_reponse_message(
    client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

} // namespace okec