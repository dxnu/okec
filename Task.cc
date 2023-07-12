#include "Task.h"
#include <stdexcept>
#define FMT_HEADER_ONLY
#include <fmt/core.h>



#define CHECK_INDEX(index) \
if (index > GetSize()) throw std::out_of_range{"index out of range"}


namespace ns3
{


Task::Task()
    : SimpleRefCount<Task>()
{
    Build("", 0, 0, 0);
}

Task::Task(std::string_view taskId, double memory, double cpu, double budget)
    : SimpleRefCount<Task>()
{
    Build(taskId, memory, cpu, budget);
}

void Task::SetId(const std::string& taskId)
{
    m_task["task_id"] = taskId;
}

std::string Task::GetId() const
{
    return m_task["task_id"];
}

void Task::SetMemory(double memory)
{
    m_task["task_memory"] = memory;
}

double Task::GetMemory() const
{
    return m_task["task_memory"];
}

void Task::SetCpu(double cpu)
{
    m_task["task_cpu"] = cpu;
}

double Task::GetCpu() const
{
    return m_task["task_cpu"];
}

void Task::SetBudget(double budget)
{
    m_task["task_budget"] = budget;
}

double Task::GetBudget() const
{
    return m_task["task_budget"];
}

void Task::From(const std::string& ip, unsigned int port)
{
    m_task["from_ip"] = ip;
    m_task["from_port"] = port;
}

auto Task::GetAddress() -> std::pair<const std::string, unsigned int>
{
    return std::make_pair(m_task["from_ip"], m_task["from_port"]);
}

Ptr<Packet> Task::GetPacket() const
{
    return MakePacket(m_task.dump());
}

Ptr<Task> Task::FromPacket(Ptr<Packet> packet)
{
    Ptr<Task> task = Create<Task>();
    json j = GetJsonFromPacket(packet);

    // 可以解析任务，则构建任务
    if (!j.is_null()) {
        task->Build(j["task_id"].get<std::string>(), j["task_memory"].get<double>(), 
                   j["task_cpu"].get<double>(), j["task_budget"].get<double>());
        task->From(j["from_ip"].get<std::string>(), j["from_port"].get<unsigned int>());
    }

    return task;
}

void Task::Build(std::string_view taskId, double memory, double cpu, double budget)
{
    m_task["task_id"]     = taskId;
    m_task["task_memory"] = memory;
    m_task["task_cpu"]    = cpu;
    m_task["task_budget"] = budget;
}

bool Task::Empty() const
{
    return m_task["task_id"]  == "" && m_task["task_memory"] == 0 &&
           m_task["task_cpu"] == 0  && m_task["task_budget"] == 0;
}

TaskContainer::TaskContainer(int num)
    : m_tasks(num, Task{})
{   
}

Task& TaskContainer::operator[](int index) {
    CHECK_INDEX(index);

    return m_tasks[index];
}

Task& TaskContainer::operator()(int index) {
    CHECK_INDEX(index);
    
    return m_tasks[index];
}

Task& TaskContainer::Get(int index)
{
    CHECK_INDEX(index);

    return m_tasks[index];
}

int TaskContainer::GetSize() const
{
    return m_tasks.size();
}

auto TaskContainer::begin()
{
    return m_tasks.begin();
}

auto TaskContainer::end()
{
    return m_tasks.end();
}

} // namespace ns3