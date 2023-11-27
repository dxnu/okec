#include "client_device.h"
#include "resource.h"



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

    // 设置响应回调函数
    m_udp_application->set_request_handler(message_response, [this](ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) {
        this->handle_response(packet, remote_address);
    });
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

auto client_device::send_to(std::shared_ptr<base_station> bs, task& t) -> void
{
    // 任务不能以 task 为单位发送，因为 task 可能会非常大，导致发送的数据断页，在目的端便无法恢复数据
    // 以 task_element 为单位发送则可以避免 task 大小可能会带来的问题
    double launch_delay{ 1.0 };
    message msg;
    msg.type(message_decision);
    for (auto& item : t.elements())
    {
        m_response.emplace_back({
            { "task_id", item.get_header("task_id") },
            { "group", item.get_header("group") },
            { "finished", "0" }, // 1 indicates finished, while 0 signifies the opposite.
            { "device_type", "" },
            { "device_address", "" },
            { "time_consuming", "" },
            { "send_time", "" }
        });

        // 追加任务发送地址信息
        item.set_header("from_ip", fmt::format("{:ip}", this->get_address()));
        item.set_header("from_port", std::to_string(this->get_port()));

        msg.content(item);
        ns3::Simulator::Schedule(ns3::Seconds(launch_delay), &udp_application::write, m_udp_application, msg.to_packet(), bs->get_address(), bs->get_port());
        launch_delay += 0.1;
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

auto client_device::handle_response(Ptr<Packet> packet, const Address& remote_address) -> void
{
    fmt::print(fg(fmt::color::red), "At time {:.2f}s Client device[{:ip}] receives", Simulator::Now().GetSeconds(), m_udp_application->get_address(), m_udp_application->get_port());

    message msg(packet);
    auto task_id = msg.get_value("task_id");
    auto device_type = msg.get_value("device_type");
    auto device_address = msg.get_value("device_address");
    auto group = msg.get_value("group");
    auto time_consuming = msg.get_value("processing_time");
    auto send_time = msg.get_value("send_time");

    fmt::print(fg(fmt::color::red), " response: task_id={}, device_type={}, device_address={}, group={}, processing_time={}, send_time={}\n",
        task_id, device_type, device_address, group, time_consuming, send_time);

    m_response.set_if({
        { "group", group },
        { "task_id", task_id }
    }, [&device_type, &device_address, &time_consuming, &send_time](auto& item) {
        item["device_type"] = device_type;
        item["device_address"] = device_address;
        item["time_consuming"] = time_consuming;
        item["send_time"] = send_time;
        item["finished"] = "1";
    });

    // 显示当前任务进度条
    auto total = m_response.count_if({ "group", group });
    auto finished = m_response.count_if({{ "group", group }, { "finished", "1" }});
    fmt::print(fg(fmt::color::red), "Current task progress: {0:█^{1}}{0:▒^{2}} {3:.0f}%\n", "",
        finished, total - finished, (double)finished / total * 100);

    // 检查是否存在当前任务的信息
    if (!m_response.find_if({ "group", group })) {
        fmt::print("未发现响应信息\n"); // 说明发过去的数据被修改，或是 m_response 被无意间删除了信息
        return;
    }


    auto partially_finished = m_response.find_if({
        { "group", group },
        { "finished", "0" }
    });

    if (partially_finished) {
        // 部分完成
        // fmt::print("部分完成\n");
    } else {
        // 全部完成
        if (m_done_fn) {
            std::invoke(m_done_fn, m_response.dump_with({ "group", group }));
        }
    }
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