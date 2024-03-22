#include "client_device.h"
#include "resource.h"
#include "base_station.h"
#include "udp_application.h"
#include "ns3/mobility-module.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/number.hpp>

namespace okec
{

client_device::client_device()
    : m_node{ ns3::CreateObject<Node>() },
      m_udp_application{ ns3::CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(300));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);
}

auto client_device::get_resource() -> Ptr<resource>
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

auto client_device::get_node() -> Ptr<Node>
{
    return m_node;
}

auto client_device::install_resource(Ptr<resource> res) -> void
{
    res->install(m_node);
}

auto client_device::send(task& t) -> void
{
    // 任务不能以 task 为单位发送，因为 task 可能会非常大，导致发送的数据断页，在目的端便无法恢复数据
    // 以 task_element 为单位发送则可以避免 task 大小可能会带来的问题
    // double launch_delay{ 1.0 };
    for (auto& item : t.elements()) {
        m_decision_engine->send(item, this);
    }
}

auto client_device::when_done(done_callback_t fn) -> void
{
    m_done_fn = fn;
}

auto client_device::set_position(double x, double y, double z) -> void
{
    Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel>();
    if (!mobility) {
        mobility = CreateObject<ConstantPositionMobilityModel>();
        mobility->SetPosition(Vector(x, y, z));
        m_node->AggregateObject(mobility);
    } else {
        mobility->SetPosition(Vector(x, y, z));
    }
}

auto client_device::set_decision_engine(std::shared_ptr<decision_engine> engine) -> void
{
    m_decision_engine = engine;
}

auto client_device::set_request_handler(std::string_view msg_type, callback_type callback) -> void
{
    m_udp_application->set_request_handler(msg_type, 
        [callback, this](Ptr<Packet> packet, const Address& remote_address) {
            callback(this, packet, remote_address);
        });
}

auto client_device::dispatch(std::string_view msg_type, Ptr<Packet> packet, const Address& address) -> void
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

auto client_device::write(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) const -> void
{
    m_udp_application->write(packet, destination, port);
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