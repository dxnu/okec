///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_MESSAGE_HELPER_HPP_
#define OKEC_MESSAGE_HELPER_HPP_

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

#endif // OKEC_MESSAGE_HELPER_HPP_