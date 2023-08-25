#include "packet_helper.h"
#include "task.h"
#include "response.h"


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
        t->group(j["content"]["task"]["group"].get<std::string>());
    }

    return t;
}

auto to_response(Ptr<Packet> packet) -> Ptr<response>
{
    json j = to_json(packet);
    Ptr<response> r = ns3::Create<response>();
    if (!j.is_null()) {
        r->task_id(j["content"]["response"]["task_id"].get<std::string>());
        r->handling_device(j["content"]["response"]["device_type"].get<std::string>(),
                           j["content"]["response"]["device_address"].get<std::string>());
        r->group(j["content"]["response"]["group"].get<std::string>());
    }

    return r;
}

} // namespace packet_helper
} // namespace okec