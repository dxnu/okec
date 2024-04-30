///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_HPP_
#define OKEC_HPP_

#include <okec/algorithms/classic/worse_fit_decision_engine.h>
#include <okec/algorithms//machine_learning/DQN_decision_engine.h>
#include <okec/config/config.h>
#include <okec/network/multiple_and_single_LAN_WLAN_network_model.hpp>
#include <okec/network/multiple_LAN_WLAN_network_model.hpp>
#include <okec/utils/log.h>
#include <okec/utils/read_csv.h>
#include <okec/utils/visulizer.hpp>


namespace okec {

struct simulator {
    simulator() {
        log::debug("C++ version: {}", __cplusplus);
        ns3::Time::SetResolution(ns3::Time::NS);
        // ns3::LogComponentEnable("udp_application", static_cast<ns3::LogLevel>(ns3::LOG_LEVEL_INFO | ns3::LOG_PREFIX_TIME));
    }

    auto run() -> void {
        ns3::Simulator::Stop(ns3::Seconds(simulator_stop_time));
        ns3::Simulator::Run();
    };

    ~simulator() {
        ns3::Simulator::Destroy();
    }
};


} // namespace okec

#endif // OKEC_HPP_