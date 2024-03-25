#ifndef INCLUDE_OKEC_HPP_
#define INCLUDE_OKEC_HPP_

#include "multiple_and_single_LAN_WLAN_network_model.hpp"
#include "multiple_LAN_WLAN_network_model.hpp"
#include "scene1_decision_engine.h"
#include "qlearning_decision_engine.h"
#include "read_csv.h"
#include "response_visulizer.hpp"
#include "config.h"


using namespace ns3;


namespace okec {

struct simulator {
    simulator() {
        fmt::print("C++ version: {}\n", __cplusplus);
        Time::SetResolution(Time::NS);
        LogComponentEnable("udp_application", LOG_LEVEL_INFO);
    }

    auto run() -> void {
        Simulator::Stop(Seconds(simulator_stop_time));
        Simulator::Run();
    };

    ~simulator() {
        Simulator::Destroy();
    }
};



} // namespace okec

#endif // INCLUDE_OKEC_HPP_