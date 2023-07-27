#include "task.h"
#include "format_helper.hpp"
#include "ns3/ptr.h"
#include <algorithm>
#include <random>


#define CHECK_INDEX(index) \
if (index > size()) throw std::out_of_range{"index out of range"}


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
    m_task["id"] = std::move(id);
}

auto task::empty() -> bool
{
    return m_task.is_null();
}

auto task::to_packet() const -> Ptr<Packet>
{
    return packet_helper::make_packet(m_task.dump());
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
    json j = packet_helper::to_json(packet);

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

task_container::task_container(std::size_t n)
{
    m_tasks.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        m_tasks.emplace_back(ns3::Create<task>());
}

auto task_container::operator[](std::size_t index) -> Ptr<task>
{
    return this->get(index);
}

auto task_container::operator()(std::size_t index) -> Ptr<task>
{
    return this->get(index);
}

auto task_container::get(std::size_t index) -> Ptr<task>
{
    CHECK_INDEX(index);
    return m_tasks[index];
}

auto task_container::random_initialization() -> void
{
    // 生成 ID
    int id_len = this->size();
    int width = 8;
    std::vector<int> id_vec(id_len);
    std::vector<std::string> id_data(id_len);
    std::iota(id_vec.begin(), id_vec.end(), 0);
    std::transform(id_vec.begin(), id_vec.end(), id_data.begin(), [&width](int n) {
        auto sn = std::to_string(n);
        return std::string(width - std::min(width, (int)sn.length()), '0') + sn; 
    });

    // 生成预算
    using rng = std::default_random_engine;
    static rng dre{ (rng::result_type)time(0) };
    std::uniform_int_distribution<uint> budget_uid(0, 10000);

    // 生成时限
    std::uniform_int_distribution<uint> deadline_uid(0, 100);

    // 生成需要的 cpu_cycles
    std::uniform_int_distribution<uint> cpu_cycles_uid(0, 5000);

    // 生成需要的 memory
    std::uniform_int_distribution<uint> memory_uid(0, 4000);

    // 生成优先级
    std::uniform_int_distribution<uint> priority_uid(0, 100);

    for (std::size_t i = 0; i < size(); ++i) {
        m_tasks[i]->id(id_data[i]);
        m_tasks[i]->budget(budget_uid(dre));
        m_tasks[i]->deadline(deadline_uid(dre));
        m_tasks[i]->needed_cpu_cycles(cpu_cycles_uid(dre));
        m_tasks[i]->needed_memory(memory_uid(dre));
        m_tasks[i]->priority(priority_uid(dre));
    }

}

auto task_container::size() const -> std::size_t
{
    return m_tasks.size();
}

auto task_container::print(std::string title) -> void
{
    if (!title.empty())
        fmt::print("{}\n", title);
    
    fmt::print("{:ts}\n", *this);
}

} // namespace okec