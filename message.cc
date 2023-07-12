#include "message.h"


namespace okec
{

message::message(Ptr<Packet> packet)
{
    auto j = to_json(packet);
    if (!j.is_null())
        j_ = std::move(j);
}

message::message(std::initializer_list<std::pair<std::string_view, std::string_view>> values)
{
    for (auto [key, value] : values) {
        j_[key] = value;
    }
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

auto message::to_task(Ptr<Packet> packet) -> Ptr<task>
{
    json j = to_json(packet);
    Ptr<task> t = ns3::Create<task>();
    // 可以解析任务，则构建任务
    if (!j.is_null()) {
        t->budget(j["content"]["task"]["budget"].get<int>());
        t->deadline(j["content"]["task"]["deadline"].get<int>());
        t->from(j["content"]["task"]["from_ip"].get<std::string>(),
                j["content"]["task"]["from_port"].get<uint16_t>());
        t->needed_cpu_cycles(j["content"]["task"]["needed_cpu_cycles"].get<int>());
        t->needed_memory(j["content"]["task"]["needed_memory"].get<int>());
        t->priority(j["content"]["task"]["priority"].get<int>());
        t->id(j["content"]["task"]["id"].get<std::string>());
    }

    return t;
}

auto message::to_packet() -> Ptr<Packet>
{
    return make_packet(this->dump());
}

auto message::content(task& t) -> void {
    j_["content"]["task"]["budget"] = t.budget();
    j_["content"]["task"]["deadline"] = t.deadline();
    auto [ip, port] = t.from();
    j_["content"]["task"]["from_ip"] = ip;
    j_["content"]["task"]["from_port"] = port;
    j_["content"]["task"]["needed_cpu_cycles"] = t.needed_cpu_cycles();
    j_["content"]["task"]["needed_memory"] = t.needed_memory();
    j_["content"]["task"]["priority"] = t.priority();
    j_["content"]["task"]["id"] = t.id();
}

} // namespace okec