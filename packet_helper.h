#ifndef OKEC_PACKET_HELPER_H
#define OKEC_PACKET_HELPER_H

#include "ns3/packet.h"
#include "nlohmann/json.hpp"
#include <string_view>


using json = nlohmann::json;


namespace okec {

using ns3::Ptr;
using ns3::Packet;

class task;

namespace packet_helper {


auto make_packet(std::string_view sv) -> Ptr<Packet>;

// convert packet to string
auto to_string(Ptr<Packet> packet) -> std::string;

// 
auto to_json(Ptr<Packet> packet) -> json;

auto to_task(Ptr<Packet> packet) -> Ptr<task>;


} // namespace packet_helper
} // namespace okec

#endif // OKEC_PACKET_HELPER_H