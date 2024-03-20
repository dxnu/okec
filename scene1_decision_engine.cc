#include "scene1_decision_engine.h"
#include "message.h"
#include "base_station.h"
#include "client_device.h"
#include "edge_device.h"
#include <functional> // std::bind_front

namespace okec {

scene1_decision_engine::scene1_decision_engine(
    client_device_container* client_devices,
    base_station_container* bs_container)
{
    // 设置决策设备
    m_decision_device = bs_container->get(0);

    // 初始化资源缓存信息
    this->initialize_device(bs_container);

    // Capture decision message
    bs_container->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    bs_container->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture es handling message
    bs_container->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture clients response message
    client_devices->set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));

    fmt::print("{}\n", this->cache().dump(4));
}

scene1_decision_engine::scene1_decision_engine(
    std::vector<client_device_container>& client_devices,
    base_station_container* bs_container)
{
        // 设置决策设备
    m_decision_device = bs_container->get(1);

    // 初始化资源缓存信息
    this->initialize_device(bs_container);

    // Capture decision message
    bs_container->set_request_handler(message_decision, std::bind_front(&this_type::on_bs_decision_message, this));
    bs_container->set_request_handler(message_response, std::bind_front(&this_type::on_bs_response_message, this));

    // Capture es handling message
    bs_container->set_es_request_handler(message_handling, std::bind_front(&this_type::on_es_handling_message, this));

    // Capture clients response message
    for (auto& clients : client_devices) {
        clients.set_request_handler(message_response, std::bind_front(&this_type::on_clients_reponse_message, this));
    }

    fmt::print("{}\n", this->cache().dump(4));
}

auto scene1_decision_engine::make_decision(const task_element& header) -> result_t
{
    return result_t();
}

auto scene1_decision_engine::local_test(const task_element& header, client_device* client) -> bool
{
    return false;
}

auto scene1_decision_engine::send(task_element& t, client_device* client) -> bool
{
    static double launch_delay = 1.0;

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

auto scene1_decision_engine::on_bs_decision_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    print_info(fmt::format("The base station[{:ip}] has received the decision request from {:ip}\n", bs->get_address(), inetRemoteAddress.GetIpv4()));
}

auto scene1_decision_engine::on_bs_response_message(
    base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto scene1_decision_engine::on_es_handling_message(
    edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void
{
}

auto scene1_decision_engine::on_clients_reponse_message(
    client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void
{

}




} // namespace okec