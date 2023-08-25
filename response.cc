#include "response.h"

namespace okec
{

response::response()
{
}

auto response::handling_device(const std::string& type, const std::string& address) -> void
{
    m_response["device_type"] = type;
    m_response["device_address"] = address;
}

auto response::handling_device() -> std::pair<const std::string, const std::string>
{
    return std::make_pair(m_response["device_type"], m_response["device_address"]);
}

auto response::task_id() const -> std::string
{
    return m_response["task_id"];
}

auto response::task_id(std::string id) -> void
{
    m_response["task_id"] = std::move(id);
}

auto response::group() const -> std::string
{
    return m_response["group"];
}

auto response::group(std::string g) -> void
{
    m_response["group"] = std::move(g);
}

auto response::empty() -> bool
{
    return m_response.is_null();
}

auto response::to_packet() const -> Ptr<Packet>
{
    return packet_helper::make_packet(m_response.dump());
}

auto response::to_string() const -> std::string
{
    return m_response.dump();
}

auto response::size() -> std::size_t
{
    return m_response.size();
}

auto make_response() -> Ptr<response>
{
    return ns3::Create<response>();
}

} // namespace okec