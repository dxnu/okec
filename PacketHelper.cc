#include "PacketHelper.h"
#include "ns3/core-module.h"


namespace ns3
{


Ptr<Packet> MakePacket(std::string_view sv)
{
    return Create<Packet>((uint8_t*)sv.data(), sv.length() + 1);
}

std::string GetStringPacket(Ptr<Packet> packet)
{
    auto size = packet->GetSize();
    auto buffer = new uint8_t[size];
    packet->CopyData(buffer, size);
    auto data = std::string(buffer, buffer + size);
    delete[] buffer;
    
    return data;
}

Ptr<Packet> MakeTaskPacket(std::string_view taskId, double taskMemory, double taskCPU)
{
    json j;
    j["task_id"] = taskId;
    j["task_memory"] = taskMemory;
    j["task_cpu"] = taskCPU;
    
    return MakePacket(j.dump());
}

json GetJsonFromPacket(Ptr<Packet> packet)
{
    json j;
    auto data = GetStringPacket(packet);
    return j.accept(data) ? j.parse(data) : j;
}

} // namespace ns3