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

auto client_device::send_to(task& t) -> void
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
            { "send_time", "" },
            { "power_consumption", "" }
        });

        // 能够本地处理
        if (m_decision_engine->local_test(item, this)) {
            namespace mp = boost::multiprecision;
            using big_float = mp::number<mp::cpp_dec_float<9>>;
            big_float cpu_demand(item.get_header("cpu_cycle"));
            big_float cpu_supply(this->get_resource()->get_value("cpu_cycle"));
            cpu_supply *= 1000000; // megahertz * 10^6 = cpu cycles
            big_float processing_time = cpu_demand / cpu_supply;

            // 标记资源已用完
            auto old_cpu_cycles = this->get_resource()->reset_value("cpu_cycle", "0");

            double time = processing_time.convert_to<double>();

            // 执行任务
            Simulator::Schedule(ns3::Seconds(time), +[](const ns3::Time& time, double processing_time, 
            client_device* self, const task_element& t, const std::string& old_cpu_cycles) {

                // 恢复资源
                // 分发太快了，还没等到恢复，任务已经被分发到别处了
                self->get_resource()->reset_value("cpu_cycle", old_cpu_cycles);

                // 返回消息
                message response {
                    { "msgtype", "response" },
                    { "task_id", t.get_header("task_id") },
                    { "device_type", "local" },
                    { "device_address", fmt::format("{:ip}", self->get_address()) },
                    { "processing_time", fmt::format("{:.9f}", processing_time) },
                    { "power_consumption", fmt::format("{:.3f}", 
                        processing_time * std::stod(self->get_resource()->get_value("TDP"))) },
                    { "send_time", "0" },
                    { "group", t.get_header("group") }
                };
                fmt::print("response: {}\n", response.dump());
                // 自调用消息，通知
                self->dispatch(message_response, response.to_packet(), self->get_address());
            }, Simulator::Now(), time, this, item, old_cpu_cycles);
        } else { // 发送到远端处理
            // 追加任务发送地址信息
            item.set_header("from_ip", fmt::format("{:ip}", this->get_address()));
            item.set_header("from_port", std::to_string(this->get_port()));

            msg.content(item);
            const auto bs = m_decision_engine->get_decision_device();
            ns3::Simulator::Schedule(ns3::Seconds(launch_delay), &udp_application::write, m_udp_application, msg.to_packet(), bs->get_address(), bs->get_port());
            launch_delay += 0.1;
        }
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