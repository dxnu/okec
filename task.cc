#include "task.h"
#include "ns3/ptr.h"


namespace okec
{

task::task()
{
}

auto task::budget() const -> int
{
    return m_task["budget"];
}

auto task::budget(int money) -> void
{
    m_task["budget"] = money;
}

auto task::deadline() const -> int
{
    return m_task["deadline"];
}

auto task::deadline(int duration) -> void
{
    m_task["deadline"] = duration;
}

auto task::from(const std::string& ip, uint16_t port) -> void
{
    m_task["from_ip"] = ip;
    m_task["from_port"] = port;
}

auto task::from() -> std::pair<const std::string, uint16_t>
{
    return std::make_pair(m_task["from_ip"], m_task["from_port"]);
}

auto task::needed_cpu_cycles() const -> int
{
    return m_task["needed_cpu_cycles"];
}

auto task::needed_cpu_cycles(int cycles) -> void
{
    m_task["needed_cpu_cycles"] = cycles;
}

auto task::needed_memory() const -> int
{
    return m_task["needed_memory"];
}

auto task::needed_memory(int memory) -> void
{
    m_task["needed_memory"] = memory;
}

auto task::priority() const -> int
{
    return m_task["priority"];
}

auto task::priority(int prior) -> void
{
    m_task["priority"] = prior;
}

auto task::id() const -> std::string
{
    return m_task["id"];
}

auto task::id(std::string id) -> void
{
    m_task["id"] = id;
}

auto task::empty() -> bool
{
    return m_task.is_null();
}

auto task::to_packet() const -> Ptr<Packet>
{
    return make_packet(m_task.dump());
}

auto task::to_string() const -> std::string
{
    return m_task.dump();
}

auto task::size() -> std::size_t
{
    return m_task.size();
}

auto task::from_packet(Ptr<Packet> packet) -> Ptr<task>
{
    Ptr<task> t = ns3::Create<task>();
    json j = to_json(packet);

    // 可以解析任务，则构建任务
    if (!j.is_null()) {
        t->budget(j["budget"].get<int>());
        t->deadline(j["deadline"].get<int>());
        t->from(j["from_ip"].get<std::string>(), j["from_port"].get<uint16_t>());
        t->needed_cpu_cycles(j["needed_cpu_cycles"].get<int>());
        t->needed_memory(j["needed_memory"].get<int>());
        t->priority(j["priority"].get<int>());
        t->id(j["id"].get<std::string>());
    }

    return t;
}

} // namespace okec