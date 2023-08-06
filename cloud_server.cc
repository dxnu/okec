#include "cloud_server.h"
#include "base_station.h"
#include "message.h"
#include "format_helper.hpp"


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
        this->on_offloading_message(packet, remote_address);
    });

    m_udp_application->set_request_handler(message_dispatching_failure, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_dispatching_failure_message(packet, remote_address);
    });

    m_udp_application->set_request_handler(message_dispatching_success, [this](Ptr<Packet> packet, const Address& remote_address) {
        this->on_dispatching_success_message(packet, remote_address);
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

auto cloud_server::push_base_station(bs_pointer_t bs) -> void
{
    m_base_stations.push_back(bs);
}

auto cloud_server::push_base_stations(base_station_container* base_stations) -> void
{
    for (auto bs : *base_stations) {
        this->push_base_station(bs);
    }
}

auto cloud_server::on_offloading_message(Ptr<Packet> packet, const Address& remote_address) -> void
{
    fmt::print("cloud[{:ip}] handles the request\n", this->get_address());
    
    auto msg = message::from_packet(packet);
    msg.type(message_dispatching);
    
    m_udp_application->write(msg.to_packet(), m_base_stations[0]->get_address(), m_base_stations[0]->get_port());
    
    // 记录当前任务的转发信息
    auto task = msg.to_task();
    auto bs_ip = fmt::format("{:ip}", m_base_stations[0]->get_address());
    m_task_dispatching_record.emplace(task->id(), bs_ip);
}

auto cloud_server::on_dispatching_failure_message(Ptr<Packet> packet, 
    const Address& remote_address) -> void
{
    fmt::print("cloud[{:ip}] handles the request\n", this->get_address());

    // 失败的 IP 地址
    ns3::InetSocketAddress inetRemoteAddress = ns3::InetSocketAddress::ConvertFrom(remote_address);
    auto failed_addr = fmt::format("{:ip}", inetRemoteAddress.GetIpv4());

    auto msg = message::from_packet(packet);
    auto t = msg.to_task();

    auto non_dispatched_base_station = [&t, this](bs_pointer_t bs) {
        auto task_id = t->id();
        auto current_addr = fmt::format("{:ip}", bs->get_address());
        auto records = m_task_dispatching_record.equal_range(task_id);
        for (auto i = records.first; i != records.second; ++i) {
            if (i->second == current_addr) { // record_ip == current_ip
                // fmt::print("record ip:{}, current ip:{}\n", i->second, current_addr);
                return false;
            }
        }
        return true;
    };

    if (auto it = std::find_if(std::begin(m_base_stations), std::end(m_base_stations), 
        non_dispatched_base_station); it != std::end(m_base_stations)) {
        // 交给其他 BS 处理
        msg.type(message_dispatching);
        auto bs_ip = fmt::format("{:ip}", (*it)->get_address());
        m_task_dispatching_record.emplace(t->id(), bs_ip);
        m_udp_application->write(msg.to_packet(), (*it)->get_address(), (*it)->get_port());
    } else {
        // 没有可用 BS，直接云端处理
        auto [ip, port] = t->from();
        fmt::print("cloud returns response to {}:{}\n", ip, port);
        
        message msg {
            { "msgtype", message_response },
            { "content", "response value: 10000000" }
        };
        m_udp_application->write(msg.to_packet(), Ipv4Address{ip.c_str()}, port);

        // 清除当前任务的分发记录
        m_task_dispatching_record.erase(t->id());
    }

}

auto cloud_server::on_dispatching_success_message(Ptr<Packet> packet, 
    const Address& remote_address) -> void
{
    // 清除当前任务的分发记录
    auto t = packet_helper::to_task(packet);
    m_task_dispatching_record.erase(t->id());
}

} // namespace okec
