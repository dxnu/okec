#ifndef OKEC_MESSAGE_HELPER_
#define OKEC_MESSAGE_HELPER_

#include <okec/common/message.h>
#include <okec/common/message_handler.hpp>


namespace okec
{


static inline auto get_message_type(ns3::Ptr<ns3::Packet> packet) {
    std::string result{};
    json j = packet_helper::to_json(packet);
    if (!j.is_null() && j.contains("msgtype")) {
        result = j["msgtype"].get<std::string>();
    }

    return result;
}


} // namespace okec

#endif // OKEC_MESSAGE_HELPER_