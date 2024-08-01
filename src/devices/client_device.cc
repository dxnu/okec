///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/devices/client_device.h>
#include <okec/common/resource.h>
#include <okec/devices/base_station.h>
#include <okec/network/udp_application.h>
#include <okec/common/awaitable.h>
#include <okec/common/response.h>
#include <okec/common/simulator.h>
#include <ns3/mobility-module.h>



namespace okec
{

client_device::client_device(simulator& sim)
    : sim_{ sim },
      m_node{ ns3::CreateObject<ns3::Node>() },
      m_udp_application{ ns3::CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(ns3::Seconds(0));
    m_udp_application->SetStopTime(sim_.stop_time());

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);
}

auto client_device::get_resource() -> ns3::Ptr<resource>
{
    return m_node->GetObject<resource>();
}

auto client_device::get_address() const -> ns3::Ipv4Address
{
    auto ipv4 = m_node->GetObject<ns3::Ipv4>();
    return ipv4->GetAddress(1, 0).GetLocal();
}

auto client_device::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto client_device::get_node() -> ns3::Ptr<ns3::Node>
{
    return m_node;
}

auto client_device::install_resource(ns3::Ptr<resource> res) -> void
{
    res->install(m_node);
}

auto client_device::send(task t) -> void
{
    // 任务不能以 task 为单位发送，因为 task 可能会非常大，导致发送的数据断页，在目的端便无法恢复数据
    // 以 task_element 为单位发送则可以避免 task 大小可能会带来的问题
    // double launch_delay{ 1.0 };
    for (auto&& item : t.elements_view()) {
        m_decision_engine->send(std::move(item), shared_from_this());
    }
}

auto client_device::async_send(task t) -> std::suspend_never
{
    for (auto&& item : t.elements_view()) {
        m_decision_engine->send(std::move(item), shared_from_this());
    }

    return {};
}

auto client_device::async_read() -> response_awaiter
{
    return response_awaiter{sim_, okec::format("{:ip}", this->get_address())};
}

auto client_device::async_read(done_callback_t fn) -> void
{
    m_done_fn = fn;
}

auto client_device::when_done(response_type resp) -> void
{
    auto ip = okec::format("{:ip}", this->get_address());
    if (sim_.is_valid(ip)) {
        sim_.complete(ip, std::move(resp));
    }

    if (this->has_done_callback()) {
        std::invoke(m_done_fn, std::move(resp));
    }
}

auto client_device::set_position(double x, double y, double z) -> void
{
    ns3::Ptr<ns3::MobilityModel> mobility = m_node->GetObject<ns3::MobilityModel>();
    if (!mobility) {
        mobility = ns3::CreateObject<ns3::ConstantPositionMobilityModel>();
        mobility->SetPosition(ns3::Vector(x, y, z));
        m_node->AggregateObject(mobility);
    } else {
        mobility->SetPosition(ns3::Vector(x, y, z));
    }
}

auto client_device::get_position() -> ns3::Vector
{
    ns3::Ptr<ns3::MobilityModel> mobility = m_node->GetObject<ns3::MobilityModel>();
    return mobility ? mobility->GetPosition() : ns3::Vector(-1.0, -1.0, -1.0);
}

auto client_device::set_decision_engine(std::shared_ptr<decision_engine> engine) -> void
{
    m_decision_engine = engine;
}

auto client_device::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_udp_application->set_request_handler(msg_type, 
        [callback, this](ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) {
            callback(this, packet, remote_address);
        });
}

auto client_device::dispatch(std::string_view msg_type, ns3::Ptr<ns3::Packet> packet, const ns3::Address& address) -> void
{
    m_udp_application->dispatch(msg_type, packet, address);
}

auto client_device::response_cache() -> response_type&
{
    return m_response;
}

auto client_device::has_done_callback() -> bool
{
    return m_done_fn ? true : false;
}

auto client_device::done_callback(response_type res) -> void
{
    std::invoke(m_done_fn, std::move(res));
}

auto client_device::write(ns3::Ptr<ns3::Packet> packet, ns3::Ipv4Address destination, uint16_t port) const -> void
{
    m_udp_application->write(packet, destination, port);
}

auto client_device_container::operator[](std::size_t index) -> pointer_type
{
    return this->get_device(index);
}

auto client_device_container::operator()(std::size_t index) -> pointer_type
{
    return this->get_device(index);
}

auto client_device_container::get_device(std::size_t index) -> pointer_type
{
    return m_devices[index];
}

auto client_device_container::size() -> std::size_t
{
    return m_devices.size();
}

auto client_device_container::install_resources(resource_container& res, int offset) -> void
{
    for (std::size_t i = 0; i < this->size(); ++i) {
        if (i + offset < res.size())
            m_devices[i]->install_resource(res[i + offset]);
    }
}

auto client_device_container::set_decision_engine(std::shared_ptr<decision_engine> engine) -> void
{
    for (pointer_type bs : m_devices) {
        bs->set_decision_engine(engine);
    }
}

auto client_device_container::set_request_handler(std::string_view msg_type, callback_type callback)
    -> void
{
    std::ranges::for_each(m_devices,
        [&msg_type, callback](pointer_type client) {
        client->set_request_handler(msg_type, callback);
    });
}

} // namespace okec