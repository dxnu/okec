#ifndef OKEC_WORSE_FIT_DECISION_ENGINE_H_
#define OKEC_WORSE_FIT_DECISION_ENGINE_H_

#include <okec/algorithms/decision_engine.h>


namespace okec
{

class client_device;
class client_device_container;
class edge_device;


class worse_fit_decision_engine : public decision_engine
{
    using this_type = worse_fit_decision_engine;

public:
    worse_fit_decision_engine() = default;
    worse_fit_decision_engine(client_device_container* clients, base_station_container* base_stations);
    worse_fit_decision_engine(std::vector<client_device_container>* clients_container, base_station_container* base_stations);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element& t, client_device* client) -> bool override;

    auto initialize() -> void override;

    auto handle_next() -> void override;

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

#endif // OKEC_WORSE_FIT_DECISION_ENGINE_H_