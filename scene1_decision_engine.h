#ifndef OKEC_SCENE1_DECISION_ENGINE
#define OKEC_SCENE1_DECISION_ENGINE

#include "decision_engine.h"

namespace okec
{

class client_device;
class client_device_container;
class edge_device;


class scene1_decision_engine : public decision_engine
{
    using this_type = scene1_decision_engine;

public:
    scene1_decision_engine() = default;
    scene1_decision_engine(client_device_container* clients, base_station_container* base_stations);
    scene1_decision_engine(std::vector<client_device_container>* clients_container, base_station_container* base_stations);

    auto make_decision(const task_element& header) -> result_t override;

    auto local_test(const task_element& header, client_device* client) -> bool override;

    auto send(task_element& t, client_device* client) -> bool override;

    auto initialize() -> void override;

    auto handle_next() -> void override;

private:
    auto on_bs_decision_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;

    auto on_bs_response_message(base_station* bs, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_es_handling_message(edge_device* es, Ptr<Packet> packet, const Address& remote_address) -> void;
    
    auto on_clients_reponse_message(client_device* client, Ptr<Packet> packet, const Address& remote_address) -> void;

private:
    client_device_container* clients_{};
    std::vector<client_device_container>* clients_container_{};
    base_station_container* base_stations_{};
};


} // namespace okec

#endif // OKEC_SCENE1_DECISION_ENGINE