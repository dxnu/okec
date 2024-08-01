///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/message.h>

namespace okec
{

message::message(ns3::Ptr<ns3::Packet> packet)
{
    auto j = packet_helper::to_json(packet);
    if (!j.is_null())
        j_ = std::move(j);
}

message::message(std::initializer_list<std::pair<std::string_view, std::string_view>> values)
{
    for (auto [key, value] : values) {
        j_[key] = value;
    }
}

message::message(const message& other)
    : j_ { other.j_ }
{
}

message& message::operator=(message other) noexcept
{
    swap(*this, other);
    return *this;
}

auto message::attribute(std::string_view key, std::string_view value) -> void
{
    j_[key] = value;
}

auto message::get_value(std::string_view key) -> std::string
{
    std::string result{};
    if (j_.contains(key))
        result = j_[key].get<std::string>();
    
    return result;
}

auto message::dump() -> std::string
{
    return j_.dump();
}

auto message::type(std::string_view sv) -> void {
    j_["msgtype"] = sv;
}

auto message::type() -> std::string
{
    return j_["msgtype"];
}

// auto message::to_response() -> Ptr<response>
// {
//     Ptr<response> r = ns3::Create<response>();
//     if (!j_.is_null()) {
//         r->task_id(j_["content"]["response"]["task_id"].get<std::string>());
//         r->handling_device(j_["content"]["response"]["device_type"].get<std::string>(),
//                            j_["content"]["response"]["device_address"].get<std::string>());
//         r->group(j_["content"]["response"]["group"].get<std::string>());
//     }

//     return r;
// }

auto message::to_packet() -> ns3::Ptr<ns3::Packet>
{
    return packet_helper::make_packet(this->dump());
}

auto message::from_packet(ns3::Ptr<ns3::Packet> packet) -> message
{
    return message(packet);
}

auto message::content(const task& t) -> void {
    j_["content"] = t.j_data();
}

auto message::content(const task_element& item) -> void
{
    j_["content"] = item.j_data();
}

auto message::content(const resource& r) -> void
{
    j_["content"] = r.j_data();
}

auto message::get_task_element() -> task_element
{
    if (!j_.is_null() && j_.contains("/content/header"_json_pointer))
        return task_element(j_["content"]);
    
    return task_element{nullptr};
}

auto message::valid() -> bool
{
    if (j_.contains("msgtype") && j_.contains("content"))
        return true;
    else
        return false;
}

void swap(message& lhs, message& rhs) noexcept
{
    using std::swap;
    swap(lhs.j_, rhs.j_);
}

} // namespace okec