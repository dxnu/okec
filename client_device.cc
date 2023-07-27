#include "client_device.h"
#include "message.h"
#include "resource.h"


namespace okec
{
client_device::client_device()
    : m_node{ ns3::CreateObject<Node>() },
      m_udp_application{ ns3::CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(10));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置响应回调函数
    m_udp_application->set_request_handler(message_response, [this](ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) {
        this->handle_response(packet, remote_address);
    });
}

auto client_device::free_cpu_cycles() const -> int
{
    auto res = m_node->GetObject<resource>();
    if (res == nullptr)
        return -1;

    return res->cpu_cycles();
}

auto client_device::free_memory() const -> int
{
    auto res = m_node->GetObject<resource>();
    if (res == nullptr)
        return -1;

    return res->memory();
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

auto client_device::send_task(const base_station& bs, const cloud_server& cs, Ptr<task> t, const ns3::Time& delay) -> void
{
    t->from(fmt::format("{:ip}", this->get_address()), this->get_port());

    // 资源充足，本地处理
    if (this->free_cpu_cycles() > t->needed_cpu_cycles() &&
        this->free_memory() > t->needed_memory()) {
        fmt::print("handling task at current client device.\n");
        return;
    }

    // 资源不足，远端处理
    message msg;
    msg.type(message_offloading_task);
    msg.content(t);
    auto packet = packet_helper::make_packet(msg.dump());

    // ns3::Simulator::Schedule(delay, &udp_application::write, m_udp_application, packet, bs.get_address(), bs.get_port());
    // 
    ns3::Simulator::Schedule(delay, &udp_application::write, m_udp_application, packet, cs.get_address(), cs.get_port());
}

auto client_device::send_tasks(const base_station& bs, const cloud_server& cs,
    task_container& container, const ns3::Time& delay) -> void
{
    double launch_time = delay.ToDouble(ns3::Time::S);
    std::for_each(container.begin(), container.end(), [&](Ptr<task> t) {
        this->send_task(bs, cs, t, Seconds(launch_time));
        launch_time += 0.1;
    });
}

auto client_device::handle_response(Ptr<Packet> packet, const Address& remote_address) -> void
{
    fmt::print("client device[{:ip}]", m_udp_application->get_address(), m_udp_application->get_port());

    message msg { packet };
    auto response = msg.content<std::string>();
    ns3::InetSocketAddress address = ns3::InetSocketAddress::ConvertFrom(remote_address);
    fmt::print(" gets response [{}] from {:ip}.\n", response, address.GetIpv4());
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

} // namespace okec