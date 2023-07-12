#ifndef NS3_PACKET_HELPER_HPP_
#define NS3_PACKET_HELPER_HPP_

#include "ns3/packet.h"
#include <string_view>
#include "nlohmann/json.hpp"

using json = nlohmann::json;


namespace ns3
{


/**
 *  \brief Make a packet from string.
*/
Ptr<Packet> MakePacket(std::string_view sv);

/**
 * \brief Get string data in a packet.
*/
std::string GetStringPacket(Ptr<Packet> packet);

/**
 * \brief Make a task packet.
*/
Ptr<Packet> MakeTaskPacket(std::string_view taskId, double taskMemory, double taskCPU);

/**
 * 
*/
json GetJsonFromPacket(Ptr<Packet> packet);


} // namespace ns3

#endif // NS3_PACKET_HELPER_HPP_