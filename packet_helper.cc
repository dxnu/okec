#include "packet_helper.h"
#include "task.h"


namespace okec {
namespace packet_helper {


auto make_packet(std::string_view sv) -> Ptr<Packet>
{
    return ns3::Create<ns3::Packet>((uint8_t*)sv.data(), sv.length() + 1);
}

auto to_string(Ptr<Packet> packet) -> std::string
{
    auto size = packet->GetSize();
    auto buffer = new uint8_t[size];
    packet->CopyData(buffer, size);
    auto data = std::string(buffer, buffer + size);
    delete[] buffer;
    
    return data;
}

auto to_json(Ptr<Packet> packet) -> json
{
    json j;
    auto data = to_string(packet);
    return j.accept(data) ? j.parse(data) : j;
}

auto to_task(Ptr<Packet> packet) -> Ptr<task>
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

} // namespace packet_helper
} // namespace okec