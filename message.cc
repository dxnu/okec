#include "message.h"


namespace okec
{

message::message(Ptr<Packet> packet)
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

auto message::attribute(std::string_view key, std::string_view value) -> void
{
    j_[key] = value;
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

auto message::to_task() -> Ptr<task>
{
    Ptr<task> t = ns3::Create<task>();
    // 可以解析任务，则构建任务
    if (!j_.is_null()) {
        t->budget(j_["content"]["task"]["budget"].get<int>());
        t->deadline(j_["content"]["task"]["deadline"].get<int>());
        t->from(j_["content"]["task"]["from_ip"].get<std::string>(),
                j_["content"]["task"]["from_port"].get<uint16_t>());
        t->needed_cpu_cycles(j_["content"]["task"]["needed_cpu_cycles"].get<int>());
        t->needed_memory(j_["content"]["task"]["needed_memory"].get<int>());
        t->priority(j_["content"]["task"]["priority"].get<int>());
        t->id(j_["content"]["task"]["id"].get<std::string>());
        t->group(j_["content"]["task"]["group"].get<std::string>());
    }

    return t;
}

auto message::to_response() -> Ptr<response>
{
    Ptr<response> r = ns3::Create<response>();
    if (!j_.is_null()) {
        r->task_id(j_["content"]["response"]["task_id"].get<std::string>());
        r->handling_device(j_["content"]["response"]["device_type"].get<std::string>(),
                           j_["content"]["response"]["device_address"].get<std::string>());
        r->group(j_["content"]["response"]["group"].get<std::string>());
    }

    return r;
}

auto message::to_packet() -> Ptr<Packet>
{
    return packet_helper::make_packet(this->dump());
}

auto message::from_packet(Ptr<Packet> packet) -> message
{
    return message(packet);
}

auto message::content(Ptr<task> t) -> void {
    j_["content"]["task"]["budget"] = t->budget();
    j_["content"]["task"]["deadline"] = t->deadline();
    auto [ip, port] = t->from();
    j_["content"]["task"]["from_ip"] = ip;
    j_["content"]["task"]["from_port"] = port;
    j_["content"]["task"]["needed_cpu_cycles"] = t->needed_cpu_cycles();
    j_["content"]["task"]["needed_memory"] = t->needed_memory();
    j_["content"]["task"]["priority"] = t->priority();
    j_["content"]["task"]["id"] = t->id();
    j_["content"]["task"]["group"] = t->group();
}

auto message::content(Ptr<response> r) -> void {
    j_["content"]["response"]["task_id"] = r->task_id();
    auto [device_type, device_address] = r->handling_device();
    j_["content"]["response"]["device_type"] = device_type;
    j_["content"]["response"]["device_address"] = device_address;
    j_["content"]["response"]["group"] = r->group();
}

auto message::valid() -> bool
{
    if (j_.contains("msgtype") && j_.contains("content"))
        return true;
    else
        return false;
}

} // namespace okec