#ifndef US3_TASK_H_
#define US3_TASK_H_

#include "ns3/core-module.h"
#include "PacketHelper.h"


namespace ns3
{

class Task : public SimpleRefCount<Task>
{
public:
    Task();
    Task(std::string_view taskId, double memory, double cpu, double budget);

    void SetId(const std::string& taskId);
    std::string GetId() const;

    void SetMemory(double memory);
    double GetMemory() const;

    void SetCpu(double cpu);
    double GetCpu() const;

    void SetBudget(double budget);
    double GetBudget() const;

    auto From(const std::string& ip, unsigned int port) -> void;
    auto GetAddress() -> std::pair<const std::string, unsigned int>;

    Ptr<Packet> GetPacket() const;

    static Ptr<Task> FromPacket(Ptr<Packet> packet);

    void Build(std::string_view taskId, double memory, double cpu, double budget);

    bool Empty() const;

private:
    json m_task;
};


class TaskContainer
{
public:
    TaskContainer(int num);

    Task& operator[](int index);

    Task& operator()(int index);

    Task& Get(int index);

    int GetSize() const;

    auto begin();

    auto end();

private:
    std::vector<Task> m_tasks;
};

} // namespace ns3

#endif // US3_TASK_H_