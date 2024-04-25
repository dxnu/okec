#ifndef OKEC_HPP_
#define OKEC_HPP_

#include <okec/algorithms/classic/worse_fit_decision_engine.h>
#include <okec/algorithms//machine_learning/DQN_decision_engine.h>
#include <okec/config/config.h>
#include <okec/network/multiple_and_single_LAN_WLAN_network_model.hpp>
#include <okec/network/multiple_LAN_WLAN_network_model.hpp>
#include <okec/utils/read_csv.h>
#include <okec/utils/visulizer.hpp>


namespace okec {

struct simulator {
    simulator() {
        fmt::print("C++ version: {}\n", __cplusplus);
        ns3::Time::SetResolution(ns3::Time::NS);
        // LogComponentEnable("udp_application", LOG_LEVEL_INFO);
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