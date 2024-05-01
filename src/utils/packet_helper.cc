///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/response.h>
#include <okec/common/task.h>
#include <okec/utils/packet_helper.h>


namespace okec {
namespace packet_helper {


auto make_packet(std::string_view sv) -> ns3::Ptr<ns3::Packet>
{
    return ns3::Create<ns3::Packet>((uint8_t*)sv.data(), sv.length() + 1);
}

auto to_string(ns3::Ptr<ns3::Packet> packet) -> std::string
{
    auto size = packet->GetSize();
    auto buffer = new uint8_t[size];
    packet->CopyData(buffer, size);
    auto data = std::string(buffer, buffer + size);
    delete[] buffer;
    
    return data;
}

auto to_json(ns3::Ptr<ns3::Packet> packet) -> json
{
    json j;
    auto data = to_string(packet);
    return j.accept(data) ? j.parse(data) : j;
}


} // namespace packet_helper
} // namespace okec