///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_PACKET_HELPER_H_
#define OKEC_PACKET_HELPER_H_

#include <string_view>
#include <nlohmann/json.hpp>
#include <ns3/packet.h>

using json = nlohmann::json;

namespace okec {

class task;
class response;

namespace packet_helper {

auto make_packet(std::string_view sv) -> ns3::Ptr<ns3::Packet>;

// convert packet to string
auto to_string(ns3::Ptr<ns3::Packet> packet) -> std::string;

// 
auto to_json(ns3::Ptr<ns3::Packet> packet) -> json;


} // namespace packet_helper
} // namespace okec

#endif // OKEC_PACKET_HELPER_H_