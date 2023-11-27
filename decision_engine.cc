#include "decision_engine.h"
#include "base_station.h"
#include "cloud_server.h"
#include "format_helper.hpp"
#include <algorithm>
#include <charconv>
#include <ranges>


namespace okec
{

auto device_cache_t::dump() -> std::string
{
    return this->cache.dump();
}

auto device_cache_t::dump_first() -> std::string
{
    std::string result{};
    if (this->data().size() > 0)
        result = this->data().at(0).dump();
    return result;
}

auto device_cache_t::data() const -> json
{
    return this->cache["device_cache"]["items"];
}

auto device_cache_t::size() const -> std::size_t
{
    return this->data().size();
}

auto device_cache_t::empty() const -> bool
{
    return this->size() < 1;
}

auto device_cache_t::emplace_back(attributes_t values) -> void
{
    json item;
    for (auto [key, value] : values) {
        item[key] = value;
    }

    this->emplace_back(std::move(item));
}

auto device_cache_t::find_if(attributes_t values) -> device_cache_t
{
    device_cache_t result{};
    for (const auto& item : this->cache["device_cache"]["items"]) {
        bool cond{true};
        for (auto [key, value] : values) {
            if (item[key] != value)
                cond = false;
        }

        if (cond) {
            result.emplace_back(item);
        }
    }
    return result;
}

auto device_cache_t::find_if(attribute_t value) -> device_cache_t
{
    return find_if({value});
}

auto device_cache_t::emplace_back(json item) -> void
{
    this->cache["device_cache"]["items"].emplace_back(std::move(item));
}

auto decision_engine::calculate_distance(const Vector& pos) -> double
{
    Vector this_pos = m_bs_socket->get_position();
    double delta_x = this_pos.x - pos.x;
    double delta_y = this_pos.y - pos.y;
    double delta_z = this_pos.z - pos.z;

    return std::sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
}

auto decision_engine::initialize_device(base_station_container* bs_container, cloud_server* cs) -> void
{
    // Save a base station so we can utilize its communication component.
    m_bs_socket = bs_container->get(0);

    // 记录云服务器信息
    if (cs) {
        auto cs_pos = cs->get_position();
        auto cs_res = cs->get_resource();

        if (cs_res && !cs_res->empty()) {
            m_device_cache.emplace_back({
                { "device_type", "cs" },
                { "ip", fmt::format("{:ip}", cs->get_address()) },
                { "port", std::to_string(cs->get_port()) },
                { "pos_x", std::to_string(cs_pos.x) },
                { "pos_y", std::to_string(cs_pos.y) },
                { "pos_z", std::to_string(cs_pos.z) }
            });

            auto cap_resource = *cs_res;
            m_device_cache.set_if({
                { "device_type", "cs" }
            }, [&cap_resource](auto& item) {
                for (auto it = cap_resource.begin(); it != cap_resource.end(); ++it) {
                    item[it.key()] = it.value();
                }
            });

            print_info(fmt::format("The decision engine got the cloud resource information: {}", 
            m_device_cache.dump_first()));
        } else {
            // 说明设备此时还未绑定资源，通过网络询问一下
            Simulator::Schedule(Seconds(1.0), +[](const std::shared_ptr<base_station> socket, const ns3::Ipv4Address& ip, uint16_t port) {
                message msg;
                msg.type("get_resource_information");
                socket->write(msg.to_packet(), ip, port);
            }, this->m_bs_socket, cs->get_address(), cs->get_port());
        }
    }

    // 记录边缘服务器信息
    double delay = 1.0;
    std::for_each(bs_container->begin(), bs_container->end(),
    [&delay, this](const base_station_container::pointer_t bs) {
        for (const auto& device : bs->get_edge_devices()) {
            auto p_resource = device->get_resource();

            // 动态记录资源信息
            if (p_resource && !p_resource->empty()) {
                // 设备已经绑定资源，直接记录
                auto es_pos = device->get_position();
                auto ip = fmt::format("{:ip}", device->get_address());
                auto port = std::to_string(device->get_port());

                m_device_cache.emplace_back({
                    { "device_type", "es" },
                    { "ip", ip },
                    { "port", port },
                    { "pos_x", std::to_string(es_pos.x) },
                    { "pos_y", std::to_string(es_pos.y) },
                    { "pos_z", std::to_string(es_pos.z) }
                });

                auto cap_resource = *p_resource;
                m_device_cache.set_if({
                    { "ip", ip },
                    { "port", port }
                }, [&cap_resource](auto& item) {
                    for (auto it = cap_resource.begin(); it != cap_resource.end(); ++it) {
                        item[it.key()] = it.value();
                    }
                });

                print_info(fmt::format("The decision engine got the device resource information: {}", 
                    m_device_cache.find_if({ "ip", ip}).dump_first()));


            } else {
                // 说明设备此时还未绑定资源，通过网络询问一下
                Simulator::Schedule(Seconds(delay), +[](const std::shared_ptr<base_station> socket, const ns3::Ipv4Address& ip, uint16_t port) {
                    message msg;
                    msg.type(message_get_resource_information);
                    socket->write(msg.to_packet(), ip, port);
                }, this->m_bs_socket, device->get_address(), device->get_port());
                delay += 0.1;
            }
        }
    });

    // fmt::print("Info: {}\n", m_device_cache.dump());

    // 捕获通过网络问询的信息，更新设备信息（能收到就一定存在资源信息）
    m_bs_socket->set_request_handler(message_resource_information, 
        [this](okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) {
            print_info(fmt::format("The decision engine has received device resource information: {}", okec::packet_helper::to_string(packet)));
            auto msg = message::from_packet(packet);
            auto es_resource = resource::from_msg_packet(packet);
            auto ip = msg.get_value("ip");
            auto port = msg.get_value("port");

            m_device_cache.emplace_back({
                { "device_type", msg.get_value("device_type") },
                { "ip", ip },
                { "port", port },
                { "pos_x", msg.get_value("pos_x") },
                { "pos_y", msg.get_value("pos_y") },
                { "pos_z", msg.get_value("pos_z") }
            });

            m_device_cache.set_if({
                { "ip", ip },
                { "port", port }
            }, [&es_resource](auto& item) {
                for (auto it = es_resource.begin(); it != es_resource.end(); ++it) {
                    item[it.key()] = it.value();
                }
            });
        });

    // 捕获资源变化信息
    // 资源更新(外部所指定的BS不一定是第0个，所以要为所有BS设置消息以确保捕获)
    bs_container->set_request_handler(message_resource_changed, 
        [this](okec::base_station* bs, Ptr<Packet> packet, const Address& remote_address) {
            fmt::print(fg(fmt::color::white), "At time {:.2f}s The decision engine got notified about device resource changes: {}\n", Simulator::Now().GetSeconds() , okec::packet_helper::to_string(packet));
            auto msg = message::from_packet(packet);
            auto es_resource = resource::from_msg_packet(packet);
            auto ip = msg.get_value("ip");
            auto port = msg.get_value("port");

            // 更新资源信息
            m_device_cache.set_if({
                { "ip", ip },
                { "port", port }
            }, [&es_resource](auto& item) {
                for (auto it = es_resource.begin(); it != es_resource.end(); ++it) {
                    item[it.key()] = it.value();
                }
            });

            // 继续处理下一个任务的分发
            bs->handle_next_task();
        });
}

auto decision_engine::device_cache() const -> json
{
    return m_device_cache.data();
}

auto decision_engine::find_device_cache(device_cache_t::values_type values) -> json
{
    device_cache_t result = m_device_cache.find_if(values);
    return result.empty() ? json{} : result.data().at(0);
}

auto decision_engine::find_device_cache(device_cache_t::value_type value) -> json
{
    return find_device_cache({value});
}

} // namespace okec