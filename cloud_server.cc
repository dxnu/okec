#include "cloud_server.h"
#include "base_station.h"
#include "message.h"


namespace okec
{

cloud_server::cloud_server()
    : m_node{ ns3::CreateObject<Node>() },
      m_udp_application{ ns3::CreateObject<udp_application>() }
{
    m_udp_application->SetStartTime(Seconds(0));
    m_udp_application->SetStopTime(Seconds(10));

    // 为当前设备安装通信功能
    m_node->AddApplication(m_udp_application);

    // 设置响应回调函数
    m_udp_application->set_request_handler(message_offloading_task, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->handle_request(packet, remote_address);
    });

    // 卸载模型
    set_offload_model([this](const task& t, const std::vector<bs_ref_type>& base_stations) {
        // BS是否有足够资源
        for (const auto& bs : base_stations) {
            if (bs.get().has_free_resource(t))
                return std::make_pair(bs.get().get_address(), bs.get().get_port());
        }

        // BS没有足够资源，云端处理
        return std::make_pair(this->get_address(), this->get_port());
    });
}

auto cloud_server::get_node() -> Ptr<Node>
{
    return m_node;
}

auto cloud_server::get_nodes(NodeContainer& nodes) -> void
{
    nodes.Add(m_node);
}

auto cloud_server::get_address() const -> Ipv4Address
{
    auto ipv4 = m_node->GetObject<Ipv4>();
    return ipv4 ? ipv4->GetAddress(1, 0).GetLocal() : Ipv4Address{};
}

auto cloud_server::get_port() const -> uint16_t
{
    return m_udp_application->get_port();
}

auto cloud_server::push_base_station(bs_ref_type bs) -> void
{
    m_base_stations.push_back(bs);
}

auto cloud_server::set_offload_model(model_type model) -> void
{
    m_model = model;
}

auto cloud_server::offload_task(const task& t) const -> std::pair<Ipv4Address, uint16_t>
{
    return m_model(t, m_base_stations);
}

auto cloud_server::handle_request(Ptr<Packet> packet, const Address& remote_address) -> void
{
    fmt::print("cloud handle request\n");

    // 返回响应结果
    auto t = message::to_task(packet);
    auto [ip, port] = t->from();
    fmt::print("cloud server response to {}:{}\n", ip, port);
    
    message msg {
        { "msgtype", message_response },
        { "content", "response value: 10000000" }
    };
    m_udp_application->write(msg.to_packet(), Ipv4Address{ip.c_str()}, port);
}

} // namespace okec
