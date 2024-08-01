///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_WORST_FIT_DECISION_ENGINE_H_
#define OKEC_WORST_FIT_DECISION_ENGINE_H_

#include <okec/algorithms/decision_engine.h>


namespace okec
{

class client_device;
class client_device_container;
class edge_device;

class DiscreteEnv : public std::enable_shared_from_this<DiscreteEnv> {
    using this_type       = DiscreteEnv;
    using done_callback_t = std::function<void(const task&, const device_cache&)>;

public:
    DiscreteEnv(const device_cache& cache, const task& t);

    auto train() -> void;
    auto train_next() -> void;

    auto when_done(done_callback_t callback) -> void;

    auto trace_resource() -> void;

private:
    task t_;
    device_cache cache_;
    std::vector<double> state_; // 初始状态
    done_callback_t done_fn_;
};


class worst_fit_decision_engine : public decision_engine
{
    using this_type = worst_fit_decision_engine;

public:
    worst_fit_decision_engine() = default;
    worst_fit_decision_engine(client_device_container* clients, base_station_container* base_stations);
    worst_fit_decision_engine(std::vector<client_device_container>* clients_container, base_station_container* base_stations);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element t, std::shared_ptr<client_device> client) -> bool override;

    auto initialize() -> void override;

    auto handle_next() -> void override;

    auto train(const task& t) -> void;

private:
    auto on_bs_decision_message(base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;
    
    auto on_clients_reponse_message(client_device* client, ns3::Ptr<ns3::Packet> packet, const ns3::Address& remote_address) -> void;

private:
    client_device_container* clients_{};
    std::vector<client_device_container>* clients_container_{};
    base_station_container* base_stations_{};
};


} // namespace okec

#endif // OKEC_WORST_FIT_DECISION_ENGINE_H_