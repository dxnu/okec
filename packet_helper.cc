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


} // namespace packet_helper
} // namespace okec