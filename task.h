#ifndef OKEC_TASK_H
#define OKEC_TASK_H

#include "packet_helper.h"
#include <string>


namespace okec
{

struct task_header
{
    int budget;
    int deadline;
    std::string from_ip;
    uint16_t from_port;
    std::string group;
    std::string id;
    int needed_cpu_cycles;
    int needed_memory;
    int priority;
};

struct task_body
{
    std::string instructions;
};


class task : public ns3::SimpleRefCount<task>
{
public:
    task();

    // 预算
    auto budget() const -> int;
    auto budget(int money) -> void;

    // 时限
    auto deadline() const -> int;
    auto deadline(int duration) -> void;

    // 任务发送的原始IP地址
    auto from(const std::string &ip, uint16_t port) -> void;
    auto from() -> std::pair<const std::string, uint16_t>;

    auto group() const -> std::string;
    auto group(std::string g) -> void;

    // 处理任务所需cpu cycles
    auto needed_cpu_cycles() const -> int;
    auto needed_cpu_cycles(int cycles) -> void;

    // 处理任务所需memory
    auto needed_memory() const -> int;
    auto needed_memory(int memory) -> void;

    // 任务优先级
    auto priority() const -> int;
    auto priority(int prior) -> void;

    // 任务ID
    auto id() const -> std::string;
    auto id(std::string id) -> void;

    // 任务是否为空
    auto empty() -> bool;

    // 转换为Packet
    auto to_packet() const -> Ptr<Packet>;

    auto to_string() const -> std::string;

    // 任务大小
    auto size() -> std::size_t;

    auto get_header() -> task_header;

    auto get_body() -> task_body;

    // construct task from packet
    static auto from_packet(Ptr<Packet> packet) -> Ptr<task>;

private:
    json m_task;
};


class task_container
{
public:
    task_container(std::size_t n);

    auto operator[](std::size_t index) -> Ptr<task>;
    auto operator()(std::size_t index) -> Ptr<task>;

    auto get(std::size_t index) -> Ptr<task>;

    auto random_initialization(const std::string& group = "ungrouped") -> void;

    auto size() const -> std::size_t;

    auto begin() {
        return m_tasks.begin();
    }

    auto end() {
        return m_tasks.end();
    }

    auto print(std::string title = "") -> void;

private:
    std::vector<Ptr<task>> m_tasks;
};


} // namespace okec

#endif // OKEC_TASK_H